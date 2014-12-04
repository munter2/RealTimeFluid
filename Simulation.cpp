#define GLM_FORCE_RADIANS
#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

#include <string>


#ifdef TARGET_OS_MAC // MAC
	std::string platform = "MAC";
	// TODO: Include Mac Headers here
#elif defined __linux__ // LINUX
	std::string platform = "LINUX";
	#include "Aluminum/Includes.hpp"
	#include "Aluminum/Program.hpp"
	#include "Aluminum/MeshBuffer.hpp"
	#include "Aluminum/MeshData.hpp"
	#include "Aluminum/Shapes.hpp"
	#include "Aluminum/Camera.hpp"
	#include "Aluminum/Utils.hpp"
	#include "Aluminum/MeshUtils.hpp"
	#include "Aluminum/FBO.hpp"
	#include "Aluminum/Behavior.hpp"
	#include "Aluminum/ResourceHandler.hpp"
	#include "Aluminum/Texture.hpp"
	#include "Aluminum/Renderer.hpp"
#elif defined _WIN32 || defined _WIN204
	std::string platform = "WINDOWS";
#else
#error "unknown platform"
#endif



#include "sphModel.hpp"
#include "positionCube.hpp"




using glm::vec3;
using glm::mat4;

float pi = glm::pi<float>();

using namespace aluminum;



class Simulation : public Renderer {

public:
		
	ResourceHandler rh; 
	Camera camera;
	Program program;

	GLint posLoc = 0;
	GLint normalLoc = 1;
	GLint colLoc = 2;
	
	MeshBuffer mb[20];
	
	mat4 view, proj;
	Behavior rotateBehavior;

	bool gravityOn;
 



	SPH fluidsimulation = SPH(20); // Initialize Fluid simulation model with 20 particles
		
	
	unsigned stepCounter = 0; // TODO: remove - step counter that keeps track of how many timesteps have been done - model stops after certain number of steps
	


	void onCreate() {

		rh.loadProgram(program, "resources/simulation", posLoc, normalLoc, -1, colLoc);

		for(int i=0; i<20; ++i) {
			MeshData md;
			// addCube(md,10.f,vec3(0,0,0));
			addSphere(md,4.f,10,10);
			mb[i].init(md,posLoc,normalLoc,-1,colLoc);
		}

		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, width, height);

		rotateBehavior = Behavior(now()).delay(1000).length(5000).range(vec3(3.14, 3.14, 3.14)).reversing(true).repeats(-1).linear();

		camera = Camera(glm::radians(200.0),1.,0.01,1000.0);
		camera.translateZ(-40);

		// Output Simulation state
		std::cout << "\nModel Parameters after Initialization:\n" << fluidsimulation;

		gravityOn = false;

	}

    void loadProgram(Program &p, const std::string& name) {

		p.create();
		p.attach(p.loadText(name + ".vsh"), GL_VERTEX_SHADER);

		glBindAttribLocation(p.id(), posLoc, "vertexPosition");
		// glBindAttribLocation(p.id(), colLoc, "vertexColor");
		glBindAttribLocation(p.id(), normalLoc, "vertexNormal");

		p.attach(p.loadText(name + ".fsh"), GL_FRAGMENT_SHADER);
		p.link();

    }


	void onFrame(){

		///////////////////////////////////////////////////////////
		// PROPAGATE MODEL
		///////////////////////////////////////////////////////////

		if(stepCounter < 500) {
			++stepCounter;
			fluidsimulation.timestep(.05); // Propagate fluidsimulation in time
			std::cout << fluidsimulation; // Output current status of Fluid particles
		}
	
		// Getting position data for rendering
		/*

			unsigned M = 5;

			float* X = new float[3*M];
			float* V = new float[3*M];

			for(unsigned i=0; i<M; ++i) {
				fluidsimulation.getPosition(i,(X+3*i));
				fluidsimulation.getVelocity(i,(V+3*i));
			}

			// TODO: position = position, velocity = colorcoded
			// TODO: opengl: allow switching from particle view to grid view

			delete[] V;
			delete[] X;
		*/
		

		///////////////////////////////////////////////////////////



		// Start displaying
    
		glViewport(0, 0, width, height);
		glClearColor(0.1,0.1,0.1,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
		
		if (camera.isTransformed) {
			camera.transform();
		}
	 
		vec3 totals = vec3(.0f,.0f,.0f); // rotateBehavior.tick(now()).totals(); // TODO: uncomment for rotation

		// Draw Cubes
		for(int i=0; i<20; ++i) {
			program.bind(); {
		/*
		proj = glm::perspective(45.0, 1.0, 0.1, 100.0);
		view = glm::lookAt(vec3(0.0,0.0,100), vec3(0,0,0), vec3(0,1,0) );
	*/	

				mat4 model = mat4(1.0);

				float position[3];
				fluidsimulation.getPosition(i,position);

				model = glm::translate(model,vec3(position[0],position[1],0));

				// For Rotation of Cubes
				/*
				model = glm::rotate(model, -totals.x, vec3(1.0f,0.0f,0.0f));
				model = glm::rotate(model, -totals.y, vec3(0.0f,1.0f,0.0f));
				model = glm::rotate(model, -totals.z, vec3(0.0f,0.0f,1.0f));
				*/
				
				glUniformMatrix4fv(program.uniform("model"), 1, 0, ptr(model));
				glUniformMatrix4fv(program.uniform("view"), 1, 0, ptr(camera.view));
				glUniformMatrix4fv(program.uniform("proj"), 1, 0, ptr(camera.projection));

				glUniform3f(program.uniform("velocity"), .5,.5,.5);

				mb[i].draw();
			} program.unbind();

		} 			  

	}


	// Keyboard Interaction
	
	void specialkeys(int key, int x, int y) {

		// FreeGlutGLView::specialkeys(key,x,y);

		// Switch Cross Compatible with Linux/MacOS
		
		float dxBox = 1;
		
		if(key == GLUT_KEY_UP || false) {
			// camera.rotateX(glm::radians(-2.));
			fluidsimulation.moveBoxY(-dxBox);
		} else if(key == GLUT_KEY_DOWN || false) {
			// camera.rotateX(glm::radians(2.));
			fluidsimulation.moveBoxY(+dxBox);
		} else if(key == GLUT_KEY_RIGHT || false) {
			// camera.rotateY(glm::radians(2.));
			fluidsimulation.moveBoxX(+dxBox);
		} else if(key == GLUT_KEY_LEFT || false) {
			fluidsimulation.moveBoxX(-dxBox);
			// camera.rotateY(glm::radians(-2.));
		}

	}


	void keyboard(unsigned char key, int x, int y) {

		if(key == ' ' || false) {
			camera.resetVectors();
		} else if(key == 'a' || false) {
			camera.rotateZ(glm::radians(2.));
		} else if(key == 's' || false) {
			camera.rotateZ(glm::radians(-2.));
		} else if(key == 'n' || false) {
			camera.translateZ(-0.5);
		} else if(key == 'u' || false) {
			camera.translateZ(0.5);
		} else if(key == 'h' || false) {
			camera.translateX(0.5);
		} else if(key == 'l' || false) {
			camera.translateX(-0.5);
		} else if(key == 'k' || false) {
			camera.translateY(-0.5);
		} else if(key == 'j' || false) {
			camera.translateY(0.5);
		} else if(key == 'g' || false) {
			if(gravityOn) {
				fluidsimulation.setGravity(0);
				gravityOn = false;
			} else {
				fluidsimulation.setGravity(-9.81);
				gravityOn = true;
			}
		}
	}
    
};


int main(){ 
	std::cout << "\n\nRunning on Platform: " << platform << "\n\n";
	Simulation().start(); 
	return 0;
}


