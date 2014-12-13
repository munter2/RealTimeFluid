#version 150 
uniform mat4 proj, view, model;
uniform int ccflag;
uniform vec3 velocity;
uniform float mass;

in vec4 vertexPosition, vertexNormal, vertexColor;

out vec3 color;

void main() {  
  
	vec4 position = view * model * vertexPosition;
			
	color = vec3(.4,.4,1);

	switch(ccflag) {
		case 0: // METABALLS
			color = vec3(1,1,1); // TODO: implement
			break;
		case 2: // SPEED
			color = vec3(length(2*velocity-vec3(1.)),.2,.2);
			break;
		case 3: // VELOCITY
			color = velocity;
			break;
		case 4: // PRESSURE
			color = vec3(.2,.4,.2); // TODO: implement
			break;
		case 5: // DENSITY
			color = vec3(.2,mass,.2); // TODO: implement
			break;
		case 101: // OBJECT
			color = vec3(1,1,0);
			break;
		case 102: // WALL
			color = vec3(0,.5,.5);
			break;
		default: // NONE
			color = vec3(.4,.4,1);
			break;
	}


	gl_Position = proj * position; 

} 


