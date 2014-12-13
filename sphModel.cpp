#include "sphModel.hpp"

#include <cmath>
#include <random>
#include <string>
#include <algorithm>


SPH::SPH(unsigned N)
	:	_nParticles(N),
		_nGhostObject(10),
		_x1BoxDim(0), // keep it 2d
		_x2BoxDim(200),
		_x3BoxDim(200),
		_nGhostWall(2*(_x2BoxDim+1+_x3BoxDim+1)*_ghostDepth), // keep it 2d
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
		_vmax(1e-10),
		_g(0),
		_damping(.8),
		_T(0.0),
		_tStep(0),
		_dt(0),
		_boxMoved(false)
{

	if(_x1BoxDim%2 || _x2BoxDim%2 || _x3BoxDim%2) {
		std::cout << "Error: please select even dimension for box";
		throw -1;
	}

	std::cout << "\nInitializing Model...";
	
	// Seeding random number generator and set parameters for normal distribution
	// std::random_device rd; // Uncomment to make it even more random ;)
	// std::mt19937 e2(rd());
	std::mt19937 e2(42);
	float mean = 0; // mean velocity
	float stddev = 10; // standard deviation of velocity
	std::normal_distribution<> dist(mean,stddev);

	// Initialize Fluid Particles
	for(unsigned i=0; i<_nParticles; ++i) {
		
		// Position
		_x1[i] = 0; // keep it 2d
		_x2[i] = fmod(rand(),_x2BoxDim)-.5*_x2BoxDim;
		_x3[i] = fmod(rand(),_x3BoxDim)-.5*_x3BoxDim;
		
		// Masses (assume two groups of particle masses)
		_m[i] = .5*(1+i%2);

		// Radius / Support of particles
		_r[i] = 1; // 1+i%3;

		// Compute Forces acting on particles based on positions
		updateForces();

		// Velocities (sampled from random normal distribution)
		_v1[i] = 0; // keep it 2d
		_v2[i] = dist(e2);
		_v3[i] = dist(e2);

	}

	// Initialize Ghost Particles in Object
	for(unsigned i=_nParticles; i<(_nParticles+_nGhostObject); ++i) {

		_x1[i] = 0;
		_x2[i] = 100;
		_x3[i] = 100;
		_v1[i] = 0;
		_v2[i] = 0;
		_v3[i] = 0;
	
		_r[i] = 1;
		_m[i] = 1;

	}
	
	// Initialize Ghost Particles in Wall

	// temporary indices for loop
	unsigned zeroIndex; 
	unsigned index1height, index2height; 

	for(unsigned d=0; d<_ghostDepth; ++d) {
		
		zeroIndex = _nParticles + _nGhostObject + 2*d*(_x2BoxDim+1+_x3BoxDim+1);

		std::cout << "\n\n---------------------------------\n d = " << d << "\t\tZEROINDEX = " << zeroIndex;

		for(unsigned i=0; i<(2*_x2BoxDim+1); ++i) {
			_x1[zeroIndex+i] = 0; // keep it 2d
			// _x2[zeroIndex+i] = (i%2 ? 1 : -1) * (.5*_x3BoxDim + d); // place on alternating sides
			// _x3[zeroIndex+i] = -.5*_x2BoxDim + i; // place at distance 1 apart
			
			// v=0 for wall particles
			_v1[zeroIndex+i] = 0;
			_v2[zeroIndex+i] = 0;
			_v3[zeroIndex+i] = 0;

			_m[zeroIndex+i] = 1e10;
			_r[zeroIndex+i] = 1;
		}

		zeroIndex += 2*_x2BoxDim;

		for(unsigned i=0; i<(2*_x3BoxDim+1); ++i) {
			_x1[zeroIndex+i] = 0; // keep it 2d
			// _x2[zeroIndex+i] = -.5*_x3BoxDim + i; // place at distance 1 apart
			// _x3[zeroIndex+i] = (i%2 ? 1 : -1) * (.5*_x2BoxDim + d); // place on alternating sides
			
			// v=0 for wall particles
			_v1[zeroIndex+i] = 0;
			_v2[zeroIndex+i] = 0;
			_v3[zeroIndex+i] = 0;

			_m[zeroIndex+i] = 1e10;
			_r[zeroIndex+i] = 1;
		}
	
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

		for(unsigned a=0; a<_nTotal; ++a) {

			if(a == i) continue; // Particles don't interact with themselves
			
			d1 = _x1[a] - _x1[i];
			d2 = _x2[a] - _x2[i];
			d3 = _x3[a] - _x3[i];

			// Compute Spherical Coordinates
			R = sqrt(d1*d1+d2*d2+d3*d3);
			if (R == 0) continue; // Avoid infinite values 
			phi = atan2(d2,d1);
			theta = acos(d3/R);

			// Compute Force
			F = .1*(R>10 ? R : -1000000/R); // TODO: replace by SPH force computation

			// Convert back to cartesian coordinates
			_a1[i] += F*sin(theta)*cos(phi);
			_a2[i] += F*sin(theta)*sin(phi);
			_a3[i] += F*cos(theta);
			
		}
		_a1[i] = 0; // Keep it 2d
		_a3[i] += _g; // add gravity
	}
}


void SPH::applyBoundary() {

	for(unsigned i=0; i<_nParticles; ++i) {

		// Additional velocitiy componentes introduced by box movement
		float v2Box = 0;
		float v3Box = 0;

		// Check if the box was moved within the last time interval
		if(_boxMoved) {
			_boxMoved = false; // movement of box has been considered, set to false now
		}

	}

}


void SPH::moveBox(float dx, unsigned a) {

	unsigned *xDim;
	float *xPtr, *vPtr;
	std::string dirStr;
	
	switch(a) {
		case _axis::X1:
			dirStr = "X";
			xDim = &_x1BoxDim;
			xPtr = _x1; vPtr = _v1;
			break;
		case _axis::X2:
			dirStr = "Y";
			xDim = &_x2BoxDim;
			xPtr = _x2; vPtr = _v2;
			break;
		case _axis::X3:
			dirStr = "Z";
			xDim = &_x3BoxDim;
			xPtr = _x3; vPtr = _v3;
			break;
		default:
			std::cout << "Error: Invalid axis selected (moveBox): axis=" << a;
			return;
	}

	float* tmpXObj = new float[_nGhostObject];

	float xMinWall = -.5*(*xDim);
	float xMaxWall = +.5*(*xDim);

	// Translate Ghost particles
	bool hitTheWall = false;
	for(unsigned i=0; i<_nGhostObject; ++i) {
		tmpXObj[i] = xPtr[_nParticles+i] + dx;
		if(tmpXObj[i] < xMinWall || tmpXObj[i] > xMaxWall) {
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

float SPH::getEkin() const {
	float Ekin;
	for(unsigned i=0; i<_nParticles; ++i) {
		Ekin += _m[i] * (_v1[i]*_v1[i] + _v2[i]*_v2[i] + _v3[i]*_v3[i]);
	}
	return .5*Ekin;
}

float SPH::getEpot() const {
	float Egrav = 0;
	float Einternal = 0;
	for(unsigned i=0; i<_nParticles; ++i) {
		Egrav += _m[i] * _x2[i];
	}
	// TODO: add potential energy of fluid itself
	Epot = Egrav; // + Einternal
	return _g*Epot;
}

void SPH::setGravity(float g) {
	_g = g;
}

// Overloaded output operator
ostream& operator<<(ostream& os, const SPH& s) {

	os << "\n==========================================================";
	os << "\nTime:   " << s._T << "\tTimestep:   " << s._tStep;
	os << "\nGravity:\t" << s._g;
	os << "\nKinetic Energy:   \t" << s.getEkin();
	os << "\nPotential Energy: \t" << s.getEpot();
	
	unsigned nOutput = 1; // Only output first particle
	// unsigned nOutput = s._nParticles; // All particles

	os << "\nPosition:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%12.4f %12.4f %12.4f | ", s._x1[i], s._x2[i], s._x3[i]);
	}
	
	os << "\nVelocity:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%12.4f %12.4f %12.4f | ", s._v1[i], s._v2[i], s._v3[i]);
	}

	os << "\nAcceleration:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%12.4f %12.4f %12.4f | ", s._a1[i], s._a2[i], s._a3[i]);
	}

	os << "\n==========================================================";
	os << "\n";

	return os;
}
