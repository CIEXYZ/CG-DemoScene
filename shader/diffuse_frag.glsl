#version 330
// A texture is expected as program attribute
uniform sampler2D Texture;

// Direction of light
uniform vec3 LightDirection;

// (optional) Transparency
uniform float Transparency;

// (optional) Texture offset
uniform vec2 TextureOffset;

//this makes textures visible from any angle
uniform bool DisableLighting;

// Color tint to apply to the texture, no tint by default
uniform vec3 ColorTint = vec3(1.0f, 1.0f, 1.0f);

// The vertex shader will feed this input
in vec2 texCoord;

// Wordspace normal passed from vertex shader
in vec4 normal;

// The final color
out vec4 FragmentColor;

void main() {
  // Compute diffuse lighting if DisableLighting the texture will be visible from any angle
  float diffuse = DisableLighting ? 1.0 : max(dot(normal, vec4(normalize(LightDirection), 1.0f)), 0.0f);
  vec4 texColor = texture(Texture, vec2(texCoord.x, 1.0 - texCoord.y) + TextureOffset);

  float ambient = 0.3;
  float lighting = ambient + diffuse * 0.7;

  // Lookup the color in Texture on coordinates given by texCoord
  // NOTE: Texture coordinate is inverted vertically for compatibility with OBJ
  FragmentColor = vec4(texColor.rgb * ColorTint * lighting, Transparency);
}
