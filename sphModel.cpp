#include "sphModel.hpp"

// TODO: implement SPH interaction
// TODO: introduce ghost particles on boundary

SPH::SPH(unsigned N)
	:	_nParticles(N),
		_nGhostWall(100),
		_nGhostObject(100),
		_nTotal(_nParticles+_nGhostWall+_nGhostObject),
		_x1(new float[_nTotal]),
		_x2(new float[_nTotal]),
		_x3(new float[_nTotal]),
		_v1(new float[_nTotal]),
		_v2(new float[_nTotal]),
		_v3(new float[_nTotal]),
		_m(new float[_nTotal]),
		_r(new float[_nTotal]),
		_x1MinWall(-100),
		_x1MaxWall(+100),
		_x2MinWall(-100),
		_x2MaxWall(+100),
		_x1MinBox(-40),
		_x1MaxBox(+40),
		_x2MinBox(-100),
		_x2MaxBox(+100),
		_g(0),
		_damping(.8),
		_ready(true),
		_T(0.0),
		_tStep(0),
		_dt(0),
		_boxMoved(false)
{
	std::cout << "\nInitializing Model...";
	
	// Seeding random number generator and set parameters for normal distribution
	// std::random_device rd; // Uncomment to make it even more random ;)
	// std::mt19937 e2(rd());
	std::mt19937 e2(42);
	float mean = 0; // mean velocity
	float stddev = 50; // standard deviation of velocity
	std::normal_distribution<> dist(mean,stddev);

	// Initialize Fluid Particles
	for(unsigned i=0; i<_nParticles; ++i) {
		
		// Position
		_x1[i] = (i%2 ? -50 : 50); // 10*float(i)/_nParticles;
		_x2[i] = 0;
		_x3[i] = 0;
		
		// Velocities (sampled from random normal distribution)
		_v1[i] = dist(e2);
		_v2[i] = dist(e2);
		_v3[i] = 0;

		// Masses (assume all particles have the same mass)
		_m[i] = 1e-6 * (1+i%3);

		// Radius / Support of particles
		_r[i] = 1+i%3;

		// _v1[i] = 0; // TODO: remove, v_x = 0 only for debugging
		// _v2[i] = 40.f; // TODO: remove, v_y = 40 only for debugging

	}

	// Initialize Ghost Particles in Wall
	for(unsigned i=_nParticles; i<_nParticles+_nGhostWall; ++i) {
		
		int side = (i-_nParticles)/(.25*_nGhostWall); 
		float ratio = 0;
		
		switch(side) {

			// Position
			case 0: // Bottom
				ratio = float(i-_nParticles)/(.25*_nGhostWall); 
				_x1[i] = -100 + 200*ratio;
				_x2[i] = -100;
				break;
			case 1: // Top
				ratio = float(i-_nParticles)/(.25*_nGhostWall)-1; 
				_x1[i] = -100 + 200*ratio;
				_x2[i] = +100;
				break;
			case 2: // Left
				ratio = float(i-_nParticles)/(.25*_nGhostWall)-2; 
				_x1[i] = -100;
				_x2[i] = -100 + 200*ratio;
				break;
			case 3: // Right
				ratio = float(i-_nParticles)/(.25*_nGhostWall)-3; 
				_x1[i] = +100;
				_x2[i] = -100 + 200*ratio;
				break;
		}
		
		_x3[i] = 0;
		
		// Velocities (sampled from random normal distribution)
		_v1[i] = 0;
		_v2[i] = 0;
		_v3[i] = 0;

		// Masses (assume all particles have the same mass)
		_m[i] = 1e10;

		// Radius / Support of particles
		_r[i] = 2;

	}

	// Initialize Ghost Particles in Object
	for(unsigned i=_nParticles+_nGhostWall; i<_nTotal; ++i) {
	
		int side = (i-_nParticles-_nGhostWall)/(.25*_nGhostObject); 
		float ratio = 0;

		float boxWidth = _x1MaxBox - _x1MinBox;
		float boxHeight = _x2MaxBox - _x2MinBox;
	
		// TODO: cast float to int
		switch(side) {

			// Position
			case 0: // Bottom
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject); 
				_x1[i] = _x1MinBox + boxWidth*ratio;
				_x2[i] = _x2MinBox;
				break;
			case 1: // Top
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject)-1; 
				_x1[i] = _x1MinBox + boxWidth*ratio;
				_x2[i] = _x2MaxBox;
				break;
			case 2: // Left
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject)-2; 
				_x1[i] = _x1MinBox;
				_x2[i] = _x2MinBox + boxHeight*ratio;
				break;
			case 3: // Right
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject)-3; 
				_x1[i] = _x1MaxBox;
				_x2[i] = _x2MinBox + boxHeight*ratio;
				break;
		}
		
		
		// Velocities (sampled from random normal distribution)
		_v1[i] = 0;
		_v2[i] = 0;
		_v3[i] = 0;

		// Masses (assume all particles have the same mass)
		_m[i] = 1e10;

		// Radius / Support of particles
		_r[i] = 3;

	}
	
}

SPH::~SPH() {
	// Free memory
	if(_r) { delete[] _r; }
	if(_m) { delete[] _m; }
	if(_v3) { delete[] _v3; }
	if(_v2) { delete[] _v2; }
	if(_v1) { delete[] _v1; }
	if(_x3) { delete[] _x3; }
	if(_x2) { delete[] _x2; }
	if(_x1) { delete[] _x1; }
	std::cout << "\nMemory freed";
}

void SPH::timestep(float dt) {

	_ready = false; // start computing

	_dt = dt;
	_T += _dt;
	++_tStep;

	for(unsigned i=0; i<_nParticles; ++i) {
		
		// Propagation of particles by velocity
		_x1[i] += _dt*_v1[i];
		_x2[i] += _dt*_v2[i];
		// _x3[i] += _dt*_v3[i]; // not needed in 2d case

		// Apply gravitational force to velocities
		_v2[i] += _g*_dt;

	}

	applyBoundary();

	// TODO: remove - sleeping only for debugging, simulates longer execution time
	unsigned microseconds = 50000;
	usleep(microseconds); 

	_ready = true; // computation done, ready for rendering

}


void SPH::applyBoundary() {

	float center1Box = .5*(_x1MinBox+_x1MaxBox);
	float center2Box = .5*(_x2MinBox+_x2MaxBox);

	for(unsigned i=0; i<_nParticles; ++i) {

		// Additional velocitiy componentes introduced by box movement
		float v1Box = 0;
		float v2Box = 0;

		// Check if the box was moved within the last time interval
		if(_boxMoved) {
			v1Box = _v1Box;
			v2Box = _v2Box;
			_boxMoved = false; // movement of box has been considered, set to false now
			_v1Box = 0; // reset velocity components of Box to zero
			_v2Box = 0; // reset velocity components of Box to zero
		}

		// Elastic reflection on wall
		if(_x1[i] <= _x1MinWall) _v1[i] = +_damping*std::abs(_v1[i]);
		if(_x1[i] >= _x1MaxWall) _v1[i] = -_damping*std::abs(_v1[i]);
		if(_x2[i] <= _x2MinWall) _v2[i] = +_damping*std::abs(_v2[i]);
		if(_x2[i] >= _x2MaxWall) _v2[i] = -_damping*std::abs(_v2[i]);

		// Elastic reflection on box
		if(_x1[i] >= _x1MinBox && _x1[i] < center1Box /*&& _x2[i] >= _x2MinBox && _x2[i] < center2Box*/) _v1[i] = -_damping*std::abs(_v1[i]) + v1Box;
		if(_x1[i] <= _x1MaxBox && _x1[i] > center1Box /*&& _x2[i] <= _x2MaxBox && _x2[i] > center2Box*/) _v1[i] = +_damping*std::abs(_v1[i]) + v1Box;
		// if(_x2[i] >= _x2MinBox && _x2[i] < center2Box /*&& _x2[i] >= _x2MinBox && _x2[i] < center2Box*/) _v2[i] = +_damping*std::abs(_v2[i]) + v2Box;
		// if(_x2[i] <= _x2MaxBox && _x2[i] > center2Box /*&& _x2[i] <= _x2MaxBox && _x2[i] > center2Box*/) _v2[i] = +_damping*std::abs(_v2[i]) + v2Box;

		/*
		if(_x2[i] >= _x2MinBox && _x2[i] < center2Box) _v2[i] = -std::abs(_v2[i]);
		if(_x2[i] <= _x2MaxBox && _x2[i] > center2Box) _v2[i] = +std::abs(_v2[i]);
*/	
	}

}


void SPH::moveBoxX(float dx) {
	float tmpMinX = _x1MinBox + dx;
	float tmpMaxX = _x1MaxBox + dx;
	if(tmpMinX > _x1MinWall && tmpMaxX < _x1MaxWall) {
		_x1MinBox = tmpMinX;
		_x1MaxBox = tmpMaxX;
		
		// Move Ghost particles
		for(unsigned i=_nParticles+_nGhostWall; i<_nTotal; ++i) {
			_x1[i] += dx;
		}

	} else {
		std::cout << "You hit the wall";
	}
	_v1Box = dx/_dt;
	_boxMoved = true;

}

void SPH::moveBoxY(float dy) {
	float tmpMinY = _x2MinBox + dy;
	float tmpMaxY = _x2MaxBox + dy;
	if(tmpMinY > _x2MinWall && tmpMaxY < _x2MaxWall) {
		_x2MinBox = tmpMinY;
		_x2MaxBox = tmpMaxY;
		
		// Move Ghost particles
		for(unsigned i=_nParticles+_nGhostWall; i<_nTotal; ++i) {
			_x2[i] += dy;
		}

	} else {
		std::cout << "You hit the wall";
	}
	_v2Box = dy/_dt;
	_boxMoved = true;
		
}

float SPH::getRadius(unsigned i) const {
	return _r[i];
}

float SPH::getEkin() const {
	float Ekin;
	for(unsigned i=0; i<_nParticles; ++i) {
		Ekin += _m[i] * (_v1[i]*_v1[i] + _v2[i]*_v2[i] + _v3[i]*_v3[i]);
	}
	return .5*Ekin;
}

float SPH::getEpot() const {
	float Epot;
	for(unsigned i=0; i<_nParticles; ++i) {
		Epot += _m[i] * _x2[i];
	}
	return _g*Epot;
}

void SPH::setGravity(float g) {
	_g = g;
}

bool SPH::isReady() const {
	return _ready;
}

unsigned SPH::getTotalParticles() const {
	return _nTotal;
}


// Overloaded output operator
ostream& operator<<(ostream& os, const SPH& s) {

	os << "\n==========================================================";
	os << "\nTime:   " << s._T << "\tTimestep:   " << s._tStep;
	os << "\nGravity:\t" << s._g;
	os << "\nKinetic Energy:   \t" << s.getEkin();
	os << "\nPotential Energy: \t" << s.getEpot();
	os << "\nBox:\t[ " << s._x1MinBox << " , " << s._x1MaxBox << " ] x [ " << s._x2MinBox << " , " << s._x2MaxBox << " ]";
	
	unsigned nOutput = 1; // Only output first particle
	// unsigned nOutput = s._nParticles; // All particles

	os << "\nPositions:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%3.4f %3.4f %3.4f | ", s._x1[i], s._x2[i], s._x3[i]);
	}
	
	
	os << "\nVelocities:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%3.4f %3.4f %3.4f | ", s._v1[i], s._v2[i], s._v3[i]);
	}

	os << "\n==========================================================";
	os << "\n";

	return os;
}
