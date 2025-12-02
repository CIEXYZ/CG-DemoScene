#version 330
in vec4 vertexColor;

// The final color
out vec4 FragmentColor;

//Color
uniform vec3 OverallColor;

// Direction of light
uniform vec3 LightDirection;

// (optional) Transparency
uniform float Transparency;

// Wordspace normal passed from vertex shader
in vec4 normal;

void main() {
  float diffuse = max(dot(normal, vec4(normalize(LightDirection), 1.0f)), 0.0f);

  float ambient = 0.3;
  float lighting = ambient + diffuse * 0.7;

  // Just pass the color to the output
  FragmentColor = vec4(vertexColor.rgb + OverallColor * lighting, vertexColor.a * Transparency);
}
