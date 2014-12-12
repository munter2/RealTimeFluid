#include "sphModel.hpp"

#include <cmath>
#include <random>
#include <string>
#include <algorithm>



// TODO: implement SPH interaction
// TODO: introduce ghost particles on boundary

SPH::SPH(unsigned N)
	:	_nParticles(N),
		_nGhostWall(100),
		_nGhostObject(0),
		_nTotal(_nParticles+_nGhostObject+_nGhostWall),
		_x1(new float[_nTotal]),
		_x2(new float[_nTotal]),
		_x3(new float[_nTotal]),
		_v1(new float[_nTotal]),
		_v2(new float[_nTotal]),
		_v3(new float[_nTotal]),
		_a1(new float[_nTotal]),
		_a2(new float[_nTotal]),
		_a3(new float[_nTotal]),
		_m(new float[_nTotal]),
		_r(new float[_nTotal]),
		_x1MinWall(-100),
		_x1MaxWall(+100),
		_x2MinWall(-100),
		_x2MaxWall(+100),
		_x3MinWall(-100),
		_x3MaxWall(+100),
		_x1MinBox(-40),
		_x1MaxBox(+40),
		_x2MinBox(-40),
		_x2MaxBox(+40),
		_x3MinBox(-100),
		_x3MaxBox(+100),
		_v1Box(0),
		_v2Box(0),
		_v3Box(0),
		_g(0),
		_damping(.8),
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
		_x1[i] = 0;
		_x2[i] = (i%2 ? -50 : 50) + .1*dist(e2);
		_x3[i] = fmod(rand(),200)-100;
		
		// Masses (assume all particles have the same mass)
		_m[i] = 1; // 1e-6 * (1+i%3);

		// Radius / Support of particles
		_r[i] = 1; // 1+i%3;

		// Compute Forces acting on particles based on positions
		updateForces();

		// Velocities (sampled from random normal distribution)
		_v1[i] = dist(e2);
		_v2[i] = dist(e2);
		_v3[i] = 0;

		// _v1[i] = 0; // TODO: remove, v_x = 0 only for debugging
		// _v2[i] = 40.f; // TODO: remove, v_y = 40 only for debugging

	}

	// Initialize Ghost Particles in Object
	for(unsigned i=_nParticles; i<(_nParticles+_nGhostObject); ++i) {

	
		int side = (i-_nParticles-_nGhostWall)/(.25*_nGhostObject); 
		float ratio = 0;

		float boxWidth = _x2MaxBox - _x2MinBox;
		float boxHeight = _x3MaxBox - _x3MinBox;
	
		/*
		// TODO: cast float to int
		switch(side) {

			// Position
			case 0: // Bottom
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject); 
				_x2[i] = _x3MinBox + boxWidth*ratio;
				_x3[i] = _x3MinBox;
				break;
			case 1: // Top
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject)-1; 
				_x2[i] = _x3MinBox + boxWidth*ratio;
				_x3[i] = _x3MaxBox;
				break;
			case 2: // Left
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject)-2; 
				_x2[i] = _x3MinBox;
				_x3[i] = _x3MinBox + boxHeight*ratio;
				break;
			case 3: // Right
				ratio = float(i-_nParticles-_nGhostWall)/(.25*_nGhostObject)-3; 
				_x2[i] = _x3MaxBox;
				_x3[i] = _x3MinBox + boxHeight*ratio;
				break;
		}
		*/
		
		
		// Velocities = 0 in Object
		_v1[i] = 0;
		_v2[i] = 0;
		_v3[i] = 0;

		// Masses (assume all particles have the same mass)
		_m[i] = 1e10;

		// Radius / Support of particles
		_r[i] = 3;


	}
	// Initialize Ghost Particles in Wall
	
	for(unsigned i=_nParticles; i<_nTotal; ++i) {
	
		int side = (i-_nParticles)/(.25*_nGhostWall); 
		float ratio = 0;
		
		/*
		switch(side) {

			// Position
			case 0: // Bottom
				ratio = float(i-_nParticles)/(.25*_nGhostWall); 
				_x2[i] = -100 + 200*ratio;
				_x3[i] = -100;
				break;
			case 1: // Top
				ratio = float(i-_nParticles)/(.25*_nGhostWall)-1; 
				_x2[i] = -100 + 200*ratio;
				_x3[i] = +100;
				break;
			case 2: // Left
				ratio = float(i-_nParticles)/(.25*_nGhostWall)-2; 
				_x2[i] = -100;
				_x3[i] = -100 + 200*ratio;
				break;
			case 3: // Right
				ratio = float(i-_nParticles)/(.25*_nGhostWall)-3; 
				_x2[i] = +100;
				_x3[i] = -100 + 200*ratio;
				break;
		}
		*/
		
		_x1[i] = 0;
		
		// Velocities = 0 in boundary
		_v1[i] = 0;
		_v2[i] = 0;
		_v3[i] = 0;

		// Masses (assume all particles have the same mass)
		_m[i] = 1e10;

		// Radius / Support of particles
		_r[i] = .2;
	}

	
}

SPH::~SPH() {
	// Free memory
	if(_r) { delete[] _r; }
	if(_m) { delete[] _m; }
	if(_a3) { delete[] _a3; }
	if(_a2) { delete[] _a2; }
	if(_a1) { delete[] _a1; }
	if(_v3) { delete[] _v3; }
	if(_v2) { delete[] _v2; }
	if(_v1) { delete[] _v1; }
	if(_x3) { delete[] _x3; }
	if(_x2) { delete[] _x2; }
	if(_x1) { delete[] _x1; }
	std::cout << "\nMemory freed";
}

void SPH::timestep(float dt) {

	// Update Time counters
	_dt = dt;
	_T += _dt;
	++_tStep;

	// Update Forces
	updateForces();

	_vmax = 0;

	for(unsigned i=0; i<_nParticles; ++i) {

		// Update Velocities
		_v1[i] += _dt*_a1[i];
		_v2[i] += _dt*_a2[i];
		_v3[i] += _dt*_a3[i];

		_vmax = std::max(std::max(_vmax,std::abs(_v1[i])),std::max(std::abs(_v2[i]),std::abs(_v3[i])));

		// Update Positions
		_x1[i] += _dt*_v1[i];
		_x2[i] += _dt*_v2[i];
		_x3[i] += _dt*_v3[i];

	}

	// applyBoundary();

	// TODO: remove - sleeping only for debugging, simulates longer execution time
	unsigned microseconds = 20000;
	usleep(microseconds); 

}

void SPH::updateForces() {

	float d1, d2, d3; // Particle Distance in each space direction
	float R; // Particle Distance in 3D space
	float theta, phi; // Angles for orientation in 3D space
	float F; // Force between two particles

	for(unsigned i=0; i<_nParticles; ++i) {
		
		_a1[i] = 0;
		_a2[i] = 0;
		_a3[i] = 0;

		for(unsigned a=0; a<_nParticles /*_nTotal*/; ++a) {

			if(a == i) continue; // Particles don't interact with themselves

			d1 = _x1[a] - _x1[i];
			d2 = _x2[a] - _x2[i];
			d3 = _x3[a] - _x3[i];

			R = sqrt(d1*d1+d2*d2+d3*d3);

			if (R == 0) continue;

			phi = atan2(d2,d1); // d2 or d3
			theta = acos(d3/R);

			// F = (std::abs(d1) > 70 ? -d1 : d1);
			F = (R>50 ? R : -10000/R);
			// R-50; // (R > 70 ? -R : R); // (R!=0 ? 1/R : 0); // Only Temporary force computation: Hooke's law

			_a1[i] += F*sin(theta)*cos(phi);
			_a2[i] += F*sin(theta)*sin(phi);
			_a3[i] += F*cos(theta);
			
			// No Interaction
			/*
			_a1[i] = 0;
			_a2[i] = 0;
			_a3[i] = 0;
			*/

		}
		_a3[i] += _g; // add gravity
	}
}


void SPH::applyBoundary() {

	float center1Box = .5*(_x2MinBox+_x2MaxBox);
	float center2Box = .5*(_x3MinBox+_x3MaxBox);

	for(unsigned i=0; i<_nParticles; ++i) {

		// Additional velocitiy componentes introduced by box movement
		float v2Box = 0;
		float v3Box = 0;

		// Check if the box was moved within the last time interval
		if(_boxMoved) {
			v2Box = _v2Box;
			v3Box = _v3Box;
			_boxMoved = false; // movement of box has been considered, set to false now
			_v2Box = 0; // reset velocity components of Box to zero
			_v3Box = 0; // reset velocity components of Box to zero
		}

		// Elastic reflection on wall
		if(_x1[i] <= _x2MinWall) _v1[i] = +_damping*std::abs(_v1[i]);
		if(_x1[i] >= _x2MaxWall) _v1[i] = -_damping*std::abs(_v1[i]);
		if(_x2[i] <= _x3MinWall) _v2[i] = +_damping*std::abs(_v2[i]);
		if(_x2[i] >= _x3MaxWall) _v2[i] = -_damping*std::abs(_v2[i]);

		// Elastic reflection on box
		// if(_x1[i] >= _x2MinBox && _x1[i] < center1Box /*&& _x2[i] >= _x3MinBox && _x2[i] < center2Box*/) _v1[i] = -_damping*std::abs(_v1[i]) + v1Box;
		// if(_x1[i] <= _x2MaxBox && _x1[i] > center1Box /*&& _x2[i] <= _x3MaxBox && _x2[i] > center2Box*/) _v1[i] = +_damping*std::abs(_v1[i]) + v1Box;
		// if(_x2[i] >= _x3MinBox && _x2[i] < center2Box /*&& _x2[i] >= _x3MinBox && _x2[i] < center2Box*/) _v2[i] = +_damping*std::abs(_v2[i]) + v2Box;
		// if(_x2[i] <= _x3MaxBox && _x2[i] > center2Box /*&& _x2[i] <= _x3MaxBox && _x2[i] > center2Box*/) _v2[i] = +_damping*std::abs(_v2[i]) + v2Box;

		/*
		if(_x2[i] >= _x3MinBox && _x2[i] < center2Box) _v2[i] = -std::abs(_v2[i]);
		if(_x2[i] <= _x3MaxBox && _x2[i] > center2Box) _v2[i] = +std::abs(_v2[i]);
*/	
	}

}


void SPH::moveBox(float dx, unsigned a) {

	float *xMinWall, *xMaxWall;
	float *xPtr, *vPtr;
	std::string dirStr;
	
	switch(a) {
		case _axis::X1:
			dirStr = "X";
			xMinWall = &_x1MinWall; xMaxWall = &_x1MaxWall;
			xPtr = _x1; vPtr = _v1;
			break;
		case _axis::X2:
			dirStr = "Y";
			xMinWall = &_x2MinWall; xMaxWall = &_x2MaxWall;
			xPtr = _x2; vPtr = _v2;
			break;
		case _axis::X3:
			dirStr = "Z";
			xMinWall = &_x3MinWall; xMaxWall = &_x3MaxWall;
			xPtr = _x3; vPtr = _v3;
			break;
		default:
			std::cout << "Error: Invalid axis selected (moveBox): axis=" << a;
			return;
	}

	float* tmpXObj = new float[_nGhostObject];

	// Translate Ghost particles
	bool hitTheWall = false;
	for(unsigned i=0; i<_nGhostObject; ++i) {
		tmpXObj[i] = xPtr[_nParticles+i] + dx;
		if(tmpXObj[i] < *xMinWall || tmpXObj[i] > *xMaxWall) {
			hitTheWall = true; // You hit the wall - no movement possible
		}
	}

	if(!hitTheWall) {
		for(unsigned i=0; i<_nGhostObject; ++i) {
			xPtr[_nParticles+i] = tmpXObj[i];
			vPtr[_nParticles+i] = dx/_dt;
		}
		_boxMoved = true;
		std::cout << "\nBox moved by " << dx << " along " << dirStr << "\n";
	} else {
		std::cout << "\nYou hit the wall";
	}

	if(tmpXObj) { delete[] tmpXObj; }

}


float SPH::getRadius(unsigned i) const {
	if(i>=_nTotal) {
		std::cout << "Error: Index out of bounds (getRadius): i=" << i;
		return NAN;
	}
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

unsigned SPH::getFluidParticles() const {
	return _nParticles;
}
unsigned SPH::getObjectParticles() const {
	return _nParticles + _nGhostObject;
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
	os << "\nBox:\t[ " << s._x2MinBox << " , " << s._x2MaxBox << " ] x [ " << s._x3MinBox << " , " << s._x3MaxBox << " ]";
	
	unsigned nOutput = 2; // Only output first particle
	// unsigned nOutput = s._nParticles; // All particles

	os << "\nPosition:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%3.4f %3.4f %3.4f | ", s._x1[i], s._x2[i], s._x3[i]);
	}
	
	os << "\nVelocity:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%3.4f %3.4f %3.4f | ", s._v1[i], s._v2[i], s._v3[i]);
	}

	os << "\nAcceleration:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%3.4f %3.4f %3.4f | ", s._a1[i], s._a2[i], s._a3[i]);
	}


	os << "\n==========================================================";
	os << "\n";

	return os;
}
