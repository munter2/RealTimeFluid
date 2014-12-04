#version 150 
uniform mat4 proj, view, model;
uniform vec3 velocity;

in vec4 vertexPosition, vertexNormal, vertexColor;

out vec3 color;

void main() {  
  
  vec4 position = view * model * vertexPosition;

  color = velocity; // vertexColor.xyz;

  gl_Position = proj * position; 

} 


