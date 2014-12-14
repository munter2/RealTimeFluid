#include "sphModel.hpp"

#include <cmath>
#include <random>
#include <string>
#include <algorithm>


SPH::SPH(unsigned N)
	:	_nParticles(N),
		_nGhostObject(_uicSize),
		_x1BoxDim(0), // keep it 2d
		_x2BoxDim(280),
		_x3BoxDim(100),
		_nGhostWall(2*(_x2BoxDim+1+_x3BoxDim+1)*_ghostDepth), // keep it 2d
		_nTotal(_nParticles+_nGhostObject+_nGhostWall),
		_nActive(0),
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
		_rho(new float[_nTotal]),
		_p(new float[_nTotal]),
		_r(new float[_nTotal]),
		_vmax(1e-10),
		_g(0),
		_damping(.8),
		_support(10),
		_h(.5*_support),
		_kPressure(1),
		_rho0(1),
		_mu(0),
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

	initObjectCoords();
	
	// Seeding random number generator and set parameters for normal distribution
	// std::random_device rd; // Uncomment to make it even more random ;)
	// std::mt19937 e2(rd());
	std::mt19937 e2(42);
	float mean = 50; // mean velocity
	float stddev = 10; // standard deviation of velocity
	std::normal_distribution<> dist(mean,stddev);

	// Initialize Fluid Particles
	for(unsigned i=0; i<_nParticles; ++i) {
		
		// Position (Locate at source)
		_x1[i] = 0; // keep it 2d
		_x2[i] = -.45*_x2BoxDim; // fmod(rand(),_x2BoxDim)-.5*_x2BoxDim;
		_x3[i] = .45*_x3BoxDim; // fmod(rand(),_x3BoxDim)-.5*_x3BoxDim;
		
		// Masses (assume two groups of particle masses)
		_m[i] = .5*(1+i%2);
		_rho[i] = 1;
		_p[i] = 1;

		// Radius / Support of particles
		_r[i] = 1; // 1+i%3;

		// Compute Forces acting on particles based on positions
		updateForces();

		// Velocities (sampled from random normal distribution)
		_v1[i] = 0; // keep it 2d
		_v2[i] = dist(e2);
		_v3[i] = 0; // dist(e2);

	}

	// Initialize Ghost Particles in Object
	for(unsigned i=_nParticles; i<(_nParticles+_nGhostObject); ++i) {

		unsigned objectIndex = i - _nParticles;

		_x1[i] = _uic[0][objectIndex];
		_x2[i] = _uic[1][objectIndex];
		_x3[i] = _uic[2][objectIndex];

		_v1[i] = 0;
		_v2[i] = 0;
		_v3[i] = 0;
	
		_r[i] = 1;
		_m[i] = 1;
		_p[i] = 1;

	}
	
	// Initialize Ghost Particles in Wall

	// temporary indices for loop
	unsigned zeroIndex; 

	for(unsigned d=0; d<_ghostDepth; ++d) {
		
		zeroIndex = _nParticles + _nGhostObject + 2*d*(_x2BoxDim+1+_x3BoxDim+1);

		for(unsigned i=0; i<(2*_x3BoxDim+1); ++i) {
			_x1[zeroIndex+i] = 0; // keep it 2d
			_x2[zeroIndex+i] = (i%2 ? 1 : -1) * (.5*_x2BoxDim + d); // place on alternating sides
			_x3[zeroIndex+i] = -.5*_x3BoxDim + i/2; // place at distance 1 apart
			
			// v=0 for wall particles
			_v1[zeroIndex+i] = 0;
			_v2[zeroIndex+i] = 0;
			_v3[zeroIndex+i] = 0;

			_m[zeroIndex+i] = 1;
			_p[zeroIndex+i] = 1;
			_r[zeroIndex+i] = .4;
		}

		zeroIndex += 2*_x3BoxDim;
		
		for(unsigned i=0; i<(2*_x2BoxDim+1); ++i) {
			_x1[zeroIndex+i] = 0; // keep it 2d
			_x2[zeroIndex+i] = -.5*_x2BoxDim + i/2; // place at distance 1 apart
			_x3[zeroIndex+i] = (i%2 ? 1 : -1) * (.5*_x3BoxDim + d); // place on alternating sides
			
			// v=0 for wall particles
			_v1[zeroIndex+i] = 0;
			_v2[zeroIndex+i] = 0;
			_v3[zeroIndex+i] = 0;

			_m[zeroIndex+i] = 1;
			_p[zeroIndex+i] = 1;
			_r[zeroIndex+i] = .4;
		}
	
	}
}

SPH::~SPH() {
	// Free memory
	if(_r) { delete[] _r; }
	if(_p) { delete[] _p; }
	if(_rho) { delete[] _rho; }
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
	std::cout << "\nAllocated memory freed";
}

void SPH::timestep(float dt) {

	if(_nActive < _nParticles && _tStep%5 == 0) {
		++_nActive;
	}

	// Update Time counters
	_dt = dt;
	_T += _dt;
	++_tStep;

	// Update Parameters
	updateDensityPressure();
	updateForces();

	_vmax = 0;

	for(unsigned i=0; i<_nActive; ++i) {

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

	/*
	unsigned microseconds = 20000;
	usleep(microseconds); 
	*/

}

void SPH::updateDensityPressure() {

	float d1, d2, d3; // Particle Distance in each space direction
	float R; // Particle Distance in 3D space

	// Compute density and prassure
	for(unsigned i=0; i<_nTotal; ++i) {

		_rho[i] = 0;
		_p[i] = 0;

		for(unsigned a=0; a<_nTotal; ++a) {
			
			if(a == i) continue; // Particles don't interact with themselves
			
			d1 = _x1[a] - _x1[i];
			d2 = _x2[a] - _x2[i];
			d3 = _x3[a] - _x3[i];

			R = sqrt(d1*d1+d2*d2+d3*d3);

			if (R == 0 || R>_support) continue; // Avoid infinite values and particles that are too far away

			_rho[i] += _m[i] * d1W3(R,_h);
			_p[i] += _kPressure*(_rho[i] - _rho0);
			
		}
	}

}


void SPH::updateForces() {

	float d1, d2, d3; // Particle Distance in each space direction
	float R; // Particle Distance in 3D space
	float theta, phi; // Angles for orientation in 3D space
	float f1, f2, f3; // Force components per direction
	float Fp, Fv, F; // Different forces
	
	// Compute Force
	for(unsigned i=0; i<_nParticles; ++i) {

		f1 = 0;
		f2 = 0;
		f3 = 0;

		for(unsigned j=0; j<_nTotal; ++j) {
			
			if(j == i) continue; // Particles don't interact with themselves
			
			d1 = _x1[j] - _x1[i];
			d2 = _x2[j] - _x2[i];
			d3 = _x3[j] - _x3[i];

			// Compute Spherical Coordinates
			R = sqrt(d1*d1+d2*d2+d3*d3);

			if (R == 0 || R>_support) continue; // Avoid infinite values and particles that are too far away

			phi = atan2(d2,d1);
			theta = acos(d3/R);

			// Compute Force
			Fp = (_rho[j] != 0 ? _m[j]*(_p[i]+_p[j])/(2*_rho[j]) * d1W3(R,_h) : 0); // TODO: uncomment
			Fv = 0; // _mu*_m[j]*(_v[j]+_v[i])/_rho[j] * d2W3(R,_h); // TODO: uncomment
			F = Fp + Fv;			
			F *= 100;

			// Convert back to cartesian coordinates
			f1 += F*sin(theta)*cos(phi);
			f2 += F*sin(theta)*sin(phi);
			f3 += F*cos(theta);
			
		}
		
		_a1[i] = 0; // Keep it 2d
		_a2[i] = f2/_m[i];
		_a3[i] = f3/_m[i] + _g; // add gravity
	
	}

}


void SPH::applyBoundary() {

	for(unsigned i=0; i<_nParticles; ++i) {

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

	std::cout << "\nMINWALL = " << xMinWall;
	std::cout << "\nMAXWALL = " << xMaxWall;

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
	Egrav *= _g;
	// TODO: add internal potential energy of fluid Einternal
	float Epot = Egrav + Einternal;
	return Epot;
}

void SPH::setGravity(float g) {
	_g = g;
}

// Overloaded output operator
ostream& operator<<(ostream& os, const SPH& s) {

	os << "\n==========================================================";
	os << "\nTime:   " << s._T << "\tTimestep:   " << s._tStep;
	os << "\nActive particles:\t" << s._nActive;
	os << "\nGravity:\t" << s._g;
	os << "\nKinetic Energy:   \t" << s.getEkin();
	os << "\nPotential Energy: \t" << s.getEpot();
	
	unsigned nOutput = 1; // Only output first particle
	// unsigned nOutput = s._nParticles; // All particles


	os << "\nMass:\t\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("  %36.4f | ", s._m[i]);
	}
	os << "\nDensity:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("  %36.4f | ", s._rho[i]);
	}
	os << "\nPressure:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("  %36.4f | ", s._p[i]);
	}
	
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
