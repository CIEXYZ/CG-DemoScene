#version 330
// The inputs will be fed by the vertex buffer objects
layout(location = 0) in vec3 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal;

// Matrices as program attributes
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

// This will be passed to the fragment shader
out vec2 texCoord;
//light calculations
out vec3 FragPos;
// Normal to pass to the fragment shader I changed it to vec3
out vec3 normal;

void main() {
  // Copy the input to the fragment shader
  texCoord = TexCoord;
  FragPos = vec3(ModelMatrix * vec4(Position, 1.0));
//normal transformations
  mat3 normalMatrix = mat3(transpose(inverse(ModelMatrix)));
  // Normal in world coordinates
  normal = normalize(normalMatrix * Normal);

  // Calculate the final position on screen
  gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(Position, 1.0);
}
