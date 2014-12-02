#include "sphModel.hpp"


// TODO: implement boundary conditions
//		elastic boundary conditions: invert velocity near plane
//		initialize velocities as random normal distribution
//		Introduce gravity??


SPH::SPH(unsigned N)
	:	_nParticles(N),
		_x1(new float[_nParticles]),
		_x2(new float[_nParticles]),
		_x3(new float[_nParticles]),
		_v1(new float[_nParticles]),
		_v2(new float[_nParticles]),
		_v3(new float[_nParticles])
{
	std::cout << "\nInitializing Model...";

	// Initialize Particle Position
	for(unsigned i=0; i<_nParticles; ++i) {
		
		// Position
		_x1[i] = float(i)/_nParticles;
		_x2[i] = 0;
		_x3[i] = 0;
		
		// Velocities
		_v1[i] = 0;
		_v2[i] = .2f;
		_v3[i] = 0;

	}

}

SPH::~SPH() {
	// Free memory
	if(_x1) { delete[] _x1; }
	if(_x2) { delete[] _x2; }
	if(_x3) { delete[] _x3; }
	if(_v1) { delete[] _v1; }
	if(_v2) { delete[] _v2; }
	if(_v3) { delete[] _v3; }
	std::cout << "\nObject deleted";
}

void SPH::timestep(float dt) {
	for(unsigned i=0; i<_nParticles; ++i) {
		_x1[i] += dt*_v1[i];
		_x2[i] += dt*_v2[i];
		_x3[i] += dt*_v3[i];
	}
}

ostream& operator<<(ostream& os, const SPH& s) {

	unsigned nOutput = 5; // s._nParticles;

	os << "\n\tPositions:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%1.4f %1.4f %1.4f | ", s._x1[i], s._x2[i], s._x3[i]);
	}
	
	
	os << "\n\tVelocities:\t| ";
	for(unsigned i=0; i<nOutput; ++i) {
		printf("%1.4f %1.4f %1.4f | ", s._v1[i], s._v2[i], s._v3[i]);
	}

	os << "\n";

	return os;
}
