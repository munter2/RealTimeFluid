#version 150

in vec3 color;

out vec4 frag;

void main(){
  
	frag = vec4(vec3(color),1.0);
  
}
