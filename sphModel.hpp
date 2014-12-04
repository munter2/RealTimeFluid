#ifndef SPH_HPP
#define SPH_HPP


#include <iostream>
#include <stdio.h>
#include <cmath>
#include <random>
#include <unistd.h>

using std::ostream;

class SPH {

	public:
		
		// Constructor
		SPH(unsigned); 
		
		// Destructor
		~SPH(); 

		// Time Propagation of model
		void timestep(float);

		// Overloading Output Operator
		friend ostream& operator<<(ostream&, const SPH&);

		// Applying elastic boundary conditions
		void applyBoundary();

		// Return status of model - Ready for rendering?
		bool isReady();

		// Write position to the 3-array x
		inline void getPosition(unsigned index, float* x) {
			x[0] = _x1[index];
			x[1] = _x2[index];
			x[2] = _x3[index];
		}
		
		// Write velocity to the 3-array v
		inline void getVelocity(unsigned index, float* v) {
			v[0] = _v1[index];
			v[1] = _v2[index];
			v[2] = _v3[index];
		}

		// Return Kinetic Energy
		float getEkin() const;

		// Return Potential Energy
		float getEpot() const;

		// Setting Gravity
		void setGravity(float);

		// Functions for changing Box position
		void moveBoxX(float);
		void moveBoxY(float);

	private:

		unsigned _nParticles;

		// Array of particle coordinates & velocities
		float* _x1;
		float* _x2;
		float* _x3;
		float* _v1;
		float* _v2;
		float* _v3;

		// Array of particle masses
		float* _m;

		// Wall Coordinates
		float _x1MinWall;
		float _x1MaxWall;
		float _x2MinWall;
		float _x2MaxWall;

		// Box Coordinates
		float _x1MinBox;
		float _x1MaxBox;
		float _x2MinBox;
		float _x2MaxBox;

		// Gravity
		float _g; 

		// Damping factor for elastic bounding on walls
		float _damping;

		// Boolean flag that signalizes whether the model is computing or idle
		bool _ready;

		// Total time
		float _T;
		unsigned _tStep;
		
};


#endif // SPH_HPP
