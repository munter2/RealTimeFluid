#ifndef SPH_HPP
#define SPH_HPP

#define _USE_MATH_DEFINES // make M_PI available

#include <iostream>
#include <stdio.h>
#include <cmath>
#include <random>
#include <unistd.h>

using std::ostream;

class SPH {

	public:

		static const unsigned _ghostDepth = 3;
		
		// Constructor
		SPH(unsigned); 
		
		// Destructor
		~SPH(); 
		
		// Time Propagation of model
		void timestep(float);
		
		// Update forces on particles based on SPH
		void updateForces();

		// Overloading Output Operator
		friend ostream& operator<<(ostream&, const SPH&);

		// Applying elastic boundary conditions
		void applyBoundary();

		// Get Radius of Particle i
		unsigned getTotalParticles() const;

		// Write position to the 3-array x
		inline void getPosition(unsigned index, float* x) {
			if(index >= _nTotal) {
				std::cout << "ERROR: Invalid index";
				return;
			}
			// Take into account switching of axes for OpenGL
			x[0] = _x2[index];
			x[1] = _x3[index];
			x[2] = _x1[index];
		}
		
		// Write velocity to the 3-array v
		inline void getVelocity(unsigned index, float* v) {
			if(index >= _nTotal) {
				std::cout << "ERROR: Invalid index";
				return;
			}
			// Take into account switching of axes for OpenGL
			v[0] = _v2[index];
			v[1] = _v3[index];
			v[2] = _v1[index];
		}

		// Return Kinetic Energy
		float getEkin() const;

		// Return Potential Energy
		float getEpot() const;
		
		// Get Radius of Particle i
		float getRadius(unsigned) const;

		// Setting Gravity
		void setGravity(float);

		// Functions for changing Box position
		// void moveBoxX(float);
		// void moveBoxY(float);

	private:

		unsigned _nParticles; // Number of fluid particles
		unsigned _nGhostWall; // Number of ghost particles in the walls
		unsigned _nGhostObject; // Number of ghost particles in the object
		unsigned _nTotal; // Total number of particles

		// Array of particle coordinates & velocities & accelerations
		float* _x1;
		float* _x2;
		float* _x3;
		float* _v1;
		float* _v2;
		float* _v3;
		float* _a1;
		float* _a2;
		float* _a3;

		// Array of particle masses
		float* _m;
		
		// Array of particle radii
		float* _r;

		// Wall Coordinates
		float _x2MinWall;
		float _x2MaxWall;
		float _x3MinWall;
		float _x3MaxWall;

		// Box Coordinates
		float _x2MinBox;
		float _x2MaxBox;
		float _x3MinBox;
		float _x3MaxBox;

		// Velocity component introduced by Box movement
		float _v2Box;
		float _v3Box;


		// Gravity
		float _g; 

		// Damping factor for elastic bounding on walls
		float _damping;

		// Total time
		float _T;
		unsigned _tStep;

		// Size of current timestep
		float _dt;

		// Was the Box moved?
		bool _boxMoved;
		
};


#endif // SPH_HPP
