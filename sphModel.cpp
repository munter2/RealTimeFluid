#include "sphModel.hpp"

// TODO: implement SPH interaction

SPH::SPH(unsigned N)
	:	_nParticles(N),
		_x1(new float[_nParticles]),
		_x2(new float[_nParticles]),
		_x3(new float[_nParticles]),
		_v1(new float[_nParticles]),
		_v2(new float[_nParticles]),
		_v3(new float[_nParticles]),
		_m(new float[_nParticles]),
		_g(0),
		_damping(1),
		_ready(true),
		_T(0.0),
		_tStep(0)
{
	std::cout << "\nInitializing Model...";
	
	// Seeding random number generator and set parameters for normal distribution
	// std::random_device rd; // Uncomment to make it even more random ;)
	// std::mt19937 e2(rd());
	std::mt19937 e2(42);
	float mean = 0; // mean velocity
	float stddev = 50; // standard deviation of velocity
	std::normal_distribution<> dist(mean,stddev);

	// Initialize Particle Position
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
		_m[i] = 1;

		// _v1[i] = 0; // TODO: remove, v_x = 0 only for debugging
		// _v2[i] = 40.f; // TODO: remove, v_y = 40 only for debugging

	}
	
	_x1MinWall = -100;
	_x1MaxWall = 100;
	_x2MinWall = -100;
	_x2MaxWall = 100;

	_x1MinBox = -20;
	_x1MaxBox = 20;
	_x2MinBox = -100;
	_x2MaxBox = 100;

	_g = 0; // TODO: comment to introduce gravity

}

SPH::~SPH() {
	// Free memory
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
	
	_T += dt;
	++_tStep;

	for(unsigned i=0; i<_nParticles; ++i) {
		
		// Propagation of particles by velocity
		_x1[i] += dt*_v1[i];
		_x2[i] += dt*_v2[i];
		// _x3[i] += dt*_v3[i]; // not needed in 2d case

		// Apply gravitational force to velocities
		_v2[i] += _g*dt;

	}


	applyBoundary();

	// TODO: remove - sleeping only for debugging, simulates longer execution time
	unsigned microseconds = 50000;
	usleep(microseconds); 

	_ready = true; // computation done, ready for rendering

}


void SPH::applyBoundary() {

	for(unsigned i=0; i<_nParticles; ++i) {

		// Elastic reflection on wall
		if(_x1[i] <= _x1MinWall) _v1[i] = +_damping*std::abs(_v1[i]);
		if(_x1[i] >= _x1MaxWall) _v1[i] = -_damping*std::abs(_v1[i]);
		if(_x2[i] <= _x2MinWall) _v2[i] = +_damping*std::abs(_v2[i]);
		if(_x2[i] >= _x2MaxWall) _v2[i] = -_damping*std::abs(_v2[i]);


		// Elastic reflection on box
		float center1Box = .5*(_x1MinBox+_x1MaxBox);
		float center2Box = .5*(_x2MinBox+_x2MaxBox);

		if(_x1[i] >= _x1MinBox && _x1[i] < center1Box /*&& _x2[i] >= _x2MinBox && _x2[i] < center2Box*/) _v1[i] = -_damping*std::abs(_v1[i]);
		if(_x1[i] >= _x1MinBox && _x1[i] < center1Box /*&& _x2[i] <= _x2MaxBox && _x2[i] > center2Box*/) _v1[i] = -_damping*std::abs(_v1[i]);
		if(_x1[i] <= _x1MaxBox && _x1[i] > center1Box /*&& _x2[i] >= _x2MinBox && _x2[i] < center2Box*/) _v1[i] = +_damping*std::abs(_v1[i]);
		if(_x1[i] <= _x1MaxBox && _x1[i] > center1Box /*&& _x2[i] <= _x2MaxBox && _x2[i] > center2Box*/) _v1[i] = +_damping*std::abs(_v1[i]);

		/*
		if(_x2[i] >= _x2MinBox && _x2[i] < center2Box) _v2[i] = -std::abs(_v2[i]);
		if(_x2[i] <= _x2MaxBox && _x2[i] > center2Box) _v2[i] = +std::abs(_v2[i]);
*/	
	}

}


void SPH::moveBoxX(float dx) {
	float tmpMinX = _x1MinBox + dx;
	float tmpMaxX = _x1MaxBox + dx;
	if(tmpMinX >= _x1MinWall && tmpMaxX <= _x1MaxWall) {
		_x1MinBox = tmpMinX;
		_x1MaxBox = tmpMaxX;
	} else {
		std::cout << "You hit the wall";
	}
}

void SPH::moveBoxY(float dy) {
	float tmpMinY = _x2MinBox + dy;
	float tmpMaxY = _x2MaxBox + dy;
	if(tmpMinY >= _x2MinWall && tmpMaxY <= _x2MaxWall) {
		_x2MinBox = tmpMinY;
		_x2MaxBox = tmpMaxY;
	} else {
		std::cout << "You hit the wall";
	}
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

bool SPH::isReady() {
	return _ready;
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
