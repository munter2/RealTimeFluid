#ifndef SPH_HPP
#define SPH_HPP


#include <iostream>
#include <stdio.h>

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

		inline void getPosition(unsigned index, float* x) {
			x[0] = _x1[index];
			x[1] = _x2[index];
			x[2] = _x3[index];
		}
		
		inline void getVelocity(unsigned index, float* v) {
			v[0] = _v1[index];
			v[1] = _v2[index];
			v[2] = _v3[index];
		}

	private:

		unsigned _nParticles;
		float* _x1;
		float* _x2;
		float* _x3;
		float* _v1;
		float* _v2;
		float* _v3;
};


#endif // SPH_HPP
