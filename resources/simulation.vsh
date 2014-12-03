#version 150 
uniform mat4 proj, view, model;

in vec4 vertexPosition, vertexNormal, vertexColor;

out vec3 color;

void main() {  
  
  vec4 position = view * model * vertexPosition;

  color = vertexColor.xyz;

  gl_Position = proj * position; 

} 


