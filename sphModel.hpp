#ifndef SPH_HPP
#define SPH_HPP

#define _USE_MATH_DEFINES // make M_PI available

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <cmath>


using std::ostream;

class SPH {

	public:

		enum _axis { X1, X2, X3 };

		static const unsigned _ghostDepth = 1;
		
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

		// Get number of particles
		inline unsigned getFluidParticles() const {
			return _nParticles;
		}
		inline unsigned getObjectParticles() const {
			return _nParticles + _nGhostObject;
		}
		inline unsigned getTotalParticles() const {
			return _nTotal;
		}
		inline float getTime() const {
			return _T;
		}
		inline float getTimeStepNumber() const {
			return _tStep;
		}



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

		// Get Maximum Velocity component
		inline float getVmax() const {
			return _vmax;
		}
		
		// Get Radius of Particle i
		inline float getRadius(unsigned i) const {
			if(i>=_nTotal) {
				std::cout << "Error: Index out of bounds (getRadius): i=" << i;
				return NAN;
			}
			return _r[i];
		}
		
		// Get Mass of Particle i
		inline float getMass(unsigned i) const {
			if(i>=_nTotal) {
				std::cout << "Error: Index out of bounds (getMass): i=" << i;
				return NAN;
			}
			return _m[i];
		}


		// Setting Gravity
		void setGravity(float);

		// Functions for changing Box position
		void moveBox(float,unsigned);

	private:

		unsigned _nParticles; // Number of fluid particles
		unsigned _nGhostObject; // Number of ghost particles in the object
		unsigned _x1BoxDim;
		unsigned _x2BoxDim;
		unsigned _x3BoxDim;
		unsigned _nGhostWall; // Number of ghost particles in the walls
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

		// Keep track of maximum velocity, needed for colorcoding in OpenGL
		float _vmax; 

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
