#define GLM_FORCE_RADIANS
#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

#include <string>
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


#ifdef TARGET_OS_MAC // MAC
	std::string platform = "MAC";
	// TODO: Include Mac Headers here
#elif defined __linux__ // LINUX
	std::string platform = "LINUX";
	#include "Aluminum/Renderer.hpp"
#elif defined _WIN32 || defined _WIN64
	std::string platform = "WINDOWS";
#else
#error "unknown platform"
#endif



#include "sphModel.hpp"




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
	GLint colLoc = -1;
	GLint normalLoc = 1;
	MeshBuffer mb;
  
	mat4 model, view, proj;
	Behavior rotateBehavior;


    vec4 l1_position = vec4(0.0,3.0, 4.0, 1.0);
    vec4 l2_position = vec4(0.0,-1.0,-1.0,1.0);


  vec3 ambient = vec3(0.1,0.1,0.5);
  
  vec3 l1_diffuse = vec3(0.0,.5,0.0);
  vec3 l1_specular = vec3(1.0,1.0,1.0);
  
  vec3 l2_diffuse = vec3(0.0,0.0,1.0);
  vec3 l2_specular = vec3(1.0,1.0,1.0);


	float dpsi = 0.1;

	void onCreate() {
	
		// Display System Information
		char* verGL = (char*)glGetString(GL_VERSION);
		char* verGLSL = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
		printf("GL %s GLSL %s\n", verGL, verGLSL);

		
		MeshData md;
		// addCube(md, .4);
		addSphere(md,1,4,4);
		
		// Colored Cubes
		program.create();
		rh.loadProgram(program, "resources/simulation", posLoc, normalLoc, -1, colLoc);
		mb.init(md,posLoc,normalLoc,-1,colLoc);

		// Rotation and Matrices
		rotateBehavior = Behavior(now()).delay(1000).length(5000).range(vec3(3.14, 3.14, 3.14)).reversing(true).repeats(-1).linear();
    
		proj = glm::perspective(45.0, 1.0, 0.1, 100.0);
		view = glm::lookAt(vec3(0.0,0.0,2), vec3(0,0,0), vec3(0,1,0) );
		model = glm::mat4(1.0);
		camera = Camera(glm::radians(60.0),1.,0.01,1000.0);

		glEnable(GL_DEPTH_TEST);
		glViewport(0,0,width,height);

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
 
		if (camera.isTransformed) {
			camera.transform();
		}

		vec3 totals = rotateBehavior.tick(now()).totals();
		
		glViewport(0, 0, width, height);
		glClearColor(0.1,0.1,0.1,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	   
		int nCubes = 18;
		float dphi = 2.*glm::pi<float>()/nCubes;
		float R = 5.;
		
		model = mat4(1.0);
		model = glm::translate(model, vec3(0.,0.,-R));

		model = glm::rotate(model, -totals.x, vec3(1.0f,0.0f,0.0f));
		model = glm::rotate(model, -totals.y, vec3(0.0f,1.0f,0.0f));
		model = glm::rotate(model, -totals.z, vec3(0.0f,0.0f,1.0f));
	 
		mat4 tmpview = camera.view;
		
		for(unsigned i=0; i<nCubes; ++i) {

			tmpview = glm::rotate(tmpview,pi,vec3(0,1.,0));

			program.bind(); {

				glUniformMatrix4fv(program.uniform("model"), 1, 0, ptr(model));
				// glUniformMatrix4fv(program.uniform("view"), 1, 0, ptr(camera.view));
				glUniformMatrix4fv(program.uniform("view"), 1, 0, ptr(tmpview));
				glUniformMatrix4fv(program.uniform("proj"), 1, 0, ptr(camera.projection));

				// Lights
				glUniform4fv(program.uniform("l1_position"), 1, ptr(l1_position));
				glUniform4fv(program.uniform("l2_position"), 1, ptr(l2_position));


				// Lighting parameters
			  glUniform3fv(program.uniform("l1_diffuse"), 1, ptr(vec3(0.0)));
			  glUniform3fv(program.uniform("l1_specular"), 1, ptr(vec3(0.0)));
			  
			  glUniform3fv(program.uniform("l2_diffuse"), 1, ptr(vec3(0.0)));
			  glUniform3fv(program.uniform("l2_specular"), 1, ptr(vec3(0.0)));
			  
			  /* draw light 1 */
			  glUniform3fv(program.uniform("ambient"), 1, ptr(l1_diffuse));




				mb.draw();

			} program.unbind();

			
		}

	}


	// Keyboard Interaction
	
	void specialkeys(int key, int x, int y) {

		// FreeGlutGLView::specialkeys(key,x,y);

		// Switch Cross Compatible with Linux/MacOS
		
		if(key == GLUT_KEY_UP || false) {
			camera.rotateX(glm::radians(-2.));
		} else if(key == GLUT_KEY_DOWN || false) {
			camera.rotateX(glm::radians(2.));
		} else if(key == GLUT_KEY_RIGHT || false) {
			camera.rotateY(glm::radians(2.));
		} else if(key == GLUT_KEY_LEFT || false) {
			camera.rotateY(glm::radians(-2.));
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
		}
	}
    
  
	void mouseMoved(int px, int py) {
	// view = glm::rotate(view,1.f,vec3(0,1,0));
      // printf("in Simulation: mouseMoved %d/%d\n", px, py);
    }
  
};


int main(){ 


	SPH fluidsimulation = SPH(20);

	std::cout << "\nModel Parameters after Initialization:\n" << fluidsimulation;


	// Propagate fluidsimulation in time
	for(unsigned i=0; i<4; ++i) {

		fluidsimulation.timestep(.1);
		std::cout << "\nTimestep " << i << fluidsimulation;

	}


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

	
	std::cout << "\n\nRunning on Platform: " << platform << "\n\n";
	Simulation().start(); //"simulation example"); 
	return 0;

}


