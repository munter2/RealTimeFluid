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
	#include "Aluminum/RendererLinux.hpp"
#elif defined _WIN32 || defined _WIN64
	std::string platform = "WINDOWS";
#else
#error "unknown platform"
#endif



#include "sphModel.hpp"
#include "extendedShapes.hpp"




using glm::vec3;
using glm::mat4;

float pi = glm::pi<float>();

using namespace aluminum;


// TODO: improve performance by only adding one single sphere instead of N spheres (just use different model matrices).
// TODO: make liquid flow in from top
// TODO: pass in only points to shader and use geometry shader to create 3d particles
// TODO: see 3.5.1: flowing water and particle effects, stream output

class Simulation : public RendererLinux {

public:

	static const unsigned N = 40;
	unsigned M = 0;
		
	ResourceHandler rh; 
	Camera camera;
	Program program;

	GLint posLoc = 0;
	GLint normalLoc = 1;
	GLint colLoc = 2;
	
	MeshBuffer* mb;
	
	mat4 view, proj;
	Behavior rotateBehavior;

	bool gravityOn;


	SPH fluidsimulation = SPH(N); // Initialize Fluid simulation model with N particles
		
	
	unsigned stepCounter = 0; // TODO: remove - step counter that keeps track of how many timesteps have been done - model stops after certain number of steps

	enum show { FLUID, FLUIDANDOBJECT, FLUIDANDOBJECTANDWALL };
	unsigned showParticles = show::FLUID;
	enum ccodes { METABALLS, NONE, SPEED, VELOCITY, PRESSURE, DENSITY };
	unsigned colorcoding = ccodes::NONE;


	void onCreate() {

		// Output Simulation state
		std::cout << "\nModel Parameters after Initialization:\n" << fluidsimulation;
		
		rh.loadProgram(program, "resources/simulation", posLoc, normalLoc, -1, colLoc);

		M = fluidsimulation.getTotalParticles();
		mb = new MeshBuffer[M];

		for(unsigned i=0; i<M; ++i) {
			MeshData md;
			// addCube(md,fluidsimulation.getRadius(i),vec3(0,0,0));
			// addRect(md,4.f,4.f,100.f,vec3(0,0,0));
			addSphere(md,5*fluidsimulation.getRadius(i),8,8);
			mb[i].init(md,posLoc,normalLoc,-1,colLoc);
		}

		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, width, height);

		rotateBehavior = Behavior(now()).delay(1000).length(5000).range(vec3(3.14, 3.14, 3.14)).reversing(true).repeats(-1).linear();

		camera = Camera(glm::radians(60.0),1.,0.01,1000.0);
		camera.translateZ(-400);


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

		std::string displayStr;
		std::string colorcodingStr;
		
		switch(colorcoding) {
			case ccodes::METABALLS:
				colorcodingStr = "Metaballs";
				break;
			case ccodes::NONE:
				colorcodingStr = "None";
				break;
			case ccodes::SPEED:
				colorcodingStr = "Speed";
				break;
			case ccodes::VELOCITY:
				colorcodingStr = "Velocity";
				break;
			case ccodes::PRESSURE:
				colorcodingStr = "Pressure";
				break;
			case ccodes::DENSITY:
				colorcodingStr = "Density";
				break;
		}

		switch(showParticles) {
			case show::FLUID: // Render fluid only
				M = fluidsimulation.getFluidParticles();
				displayStr = "Fluid";
				break;
			case show::FLUIDANDOBJECT: // Render fluid and immersed object
				M = fluidsimulation.getObjectParticles();
				displayStr = "Fluid + Object";
				break;
			case show::FLUIDANDOBJECTANDWALL: // Render all particles
				M = fluidsimulation.getTotalParticles();
				displayStr = "Fluid + Object + Walls";
				break;
		}
		std::cout << "\nDisplay: " << displayStr << ",\tColorcoding: " << colorcodingStr;

		///////////////////////////////////////////////////////////
		// PROPAGATE MODEL
		///////////////////////////////////////////////////////////

		if(stepCounter < 5000) {
			++stepCounter;
			fluidsimulation.timestep(.02); // Propagate fluidsimulation in time
			std::cout << fluidsimulation; // Output current status of Fluid particles
		}
	

		// Start displaying
    
		glViewport(0, 0, width, height);
		// glClearColor(0.1,0.1,0.1,1.0);
		// glClearColor(0,0,0,1.0);
		glClearColor(0,.5,0,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
		
		if (camera.isTransformed) {
			camera.transform();
		}
	 
		vec3 totals = vec3(.0f,.0f,.0f);

		
		// Draw Cubes
		float position[3];
		float velocity[3];
		vec3 normalizedVelocity;
		float vmax = fluidsimulation.getVmax();
		for(unsigned i=0; i<M; ++i) {
			program.bind(); {

				mat4 model = mat4(1.0);

				fluidsimulation.getPosition(i,position);
				fluidsimulation.getVelocity(i,velocity);
				normalizedVelocity = vec3(velocity[0],velocity[1],velocity[2]);
				normalizedVelocity = .5f*(vec3(1.f)+normalizedVelocity/vmax);

				model = glm::translate(model,vec3(position[0],position[1],position[2]));

				glUniformMatrix4fv(program.uniform("model"), 1, 0, ptr(model));
				glUniformMatrix4fv(program.uniform("view"), 1, 0, ptr(camera.view));
				glUniformMatrix4fv(program.uniform("proj"), 1, 0, ptr(camera.projection));

				glUniform1i(program.uniform("ccflag"), colorcoding);
				glUniform3fv(program.uniform("velocity"), 1, ptr(normalizedVelocity));

				mb[i].draw();
			} program.unbind();

		} 			  

	}


	// Keyboard Interaction
	
	void specialkeys(int key, int x, int y) {

		// FreeGlutGLView::specialkeys(key,x,y);

		// For interaction with MacOS Keycodes, replace '|| false' by '|| KEYCODE'
	
		// Rotating the Camera
		if(key == GLUT_KEY_UP || false) {
			camera.rotateX(glm::radians(-2.));
		} else if(key == GLUT_KEY_DOWN || false) {
			camera.rotateX(glm::radians(2.));
		} else if(key == GLUT_KEY_RIGHT || false) {
			camera.rotateY(glm::radians(-2.));
		} else if(key == GLUT_KEY_LEFT || false) {
			camera.rotateY(glm::radians(+2.));
		}

	}


	void keyboard(unsigned char key, int x, int y) {

		float dxCamera = 5;
		float dxBox = 1;

		// For interaction with MacOS Keycodes, replace '|| false' by '|| KEYCODE'
		
		if(key == ' ' || false) {
			showParticles = (showParticles+1)%3;
		
		// Moving the Cube
		} else if(key == 'q' || false) {
			fluidsimulation.moveBox(-dxBox,SPH::_axis::X1);
		} else if(key == 'w' || false) {
			fluidsimulation.moveBox(+dxBox,SPH::_axis::X1);
		} else if(key == 'a' || false) {
			fluidsimulation.moveBox(-dxBox,SPH::_axis::X2);
		} else if(key == 's' || false) {
			fluidsimulation.moveBox(+dxBox,SPH::_axis::X2);
		} else if(key == 'z' || false) {
			fluidsimulation.moveBox(-dxBox,SPH::_axis::X3);
		} else if(key == 'x' || false) {
			fluidsimulation.moveBox(+dxBox,SPH::_axis::X3);

		// Moving the Camera
		} else if(key == 'e' || false) {
			camera.translateX(-dxCamera);
		} else if(key == 'r' || false) {
			camera.translateX(+dxCamera);
		} else if(key == 'd' || false) {
			camera.translateZ(+dxCamera);
		} else if(key == 'f' || false) {
			camera.translateZ(-dxCamera);
		} else if(key == 'c' || false) {
			camera.translateY(-dxCamera);
		} else if(key == 'v' || false) {
			camera.translateY(+dxCamera);

		// Toggle Gravity
		} else if(key == 'g' || false) {
			if(gravityOn) {
				fluidsimulation.setGravity(0);
				gravityOn = false;
			} else {
				fluidsimulation.setGravity(-20);
				gravityOn = true;
			}

		// Switch between colorcoding schemes
		} else if(key == '0' || false) {
			colorcoding = ccodes::METABALLS;
		} else if(key == '9' || false) {
			colorcoding = ccodes::NONE;
		} else if(key == '8' || false) {
			colorcoding = ccodes::SPEED;
		} else if(key == '7' || false) {
			colorcoding = ccodes::VELOCITY;
		} else if(key == '6' || false) {
			colorcoding = ccodes::PRESSURE;
		} else if(key == '5' || false) {
			colorcoding = ccodes::DENSITY;
		}
	}
    
};


int main(){ 
	std::cout << "\n\nRunning on Platform: " << platform << "\n\n";
	Simulation().start(); 
	return 0;
}


