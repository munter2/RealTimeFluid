#version 150 
uniform mat4 proj, view, model;

void main() {  
  
  //get the vertex position in CAMERA coordinates
  vec4 position = view * model * vertexPosition;

  //of course, we always have to output our vertices in clip coords by multiplying through a projection matrix.
  gl_Position = proj * position; 
} 


