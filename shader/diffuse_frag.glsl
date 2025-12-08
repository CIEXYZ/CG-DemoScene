//#version 330
//// A texture is expected as program attribute
//uniform sampler2D Texture;
//
//// Direction of light
//uniform vec3 LightDirection;
//
//// (optional) Transparency
//uniform float Transparency;
//
//// (optional) Texture offset
//uniform vec2 TextureOffset;
//
////this makes textures visible from any angle
//uniform bool DisableLighting;
//
//// Color tint to apply to the texture, no tint by default
//uniform vec3 ColorTint = vec3(1.0f, 1.0f, 1.0f);
//
//// The vertex shader will feed this input
//in vec2 texCoord;
//
//in vec3 FragPos;
//// Wordspace normal passed from vertex shader
//in vec3 normal;
//
//// The final color
//out vec4 FragmentColor;
//
//void main() {
//  // Compute diffuse lighting if DisableLighting the texture will be visible from any angle
//  float diffuse = DisableLighting ? 1.0 : max(dot(normal, vec4(normalize(LightDirection), 1.0f)), 0.0f);
//  vec4 texColor = texture(Texture, vec2(texCoord.x, 1.0 - texCoord.y) + TextureOffset);
//
//  float ambient = 0.3;
//  float lighting = ambient + diffuse * 0.7;
//
//  // Lookup the color in Texture on coordinates given by texCoord
//  // NOTE: Texture coordinate is inverted vertically for compatibility with OBJ
//  FragmentColor = vec4(texColor.rgb * ColorTint * lighting, texColor.a * Transparency);
//}
#version 330

// ========================================
// MAXIMUM LIGHT COUNTS
// ========================================
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 5

// ========================================
// STRUCTURES
// ========================================

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

struct DirectionalLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float constant;
  float linear;
  float quadratic;
};

struct SpotLight {
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float cutOff;
  float outerCutOff;
  float constant;
  float linear;
  float quadratic;
};

in vec2 texCoord;
in vec3 FragPos;
in vec3 normal;

// Texture
uniform sampler2D Texture;

// Material properties
uniform Material material;

// Lights
uniform DirectionalLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

// Light control
uniform int numPointLights;
uniform int numSpotLights;
uniform bool useDirLight;

// Camera position
uniform vec3 viewPos;

// Options
uniform float Transparency;
uniform vec2 TextureOffset;
uniform bool DisableLighting;
uniform bool useBlinnPhong;
uniform vec3 ColorTint = vec3(1.0, 1.0, 1.0);  // ✅ Kept your color tint

// Legacy uniform variable
uniform vec3 LightDirection;


out vec4 FragmentColor;

vec3 CalcDirLight(DirectionalLight light, vec3 norm, vec3 viewDir, vec3 texColor);
vec3 CalcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texColor);
vec3 CalcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texColor);


void main() {
  // Sample texture with offset
  vec4 texColor = texture(Texture, vec2(texCoord.x, 1.0 - texCoord.y) + TextureOffset);

  // Old lighting for particles, transparent objects
  if (DisableLighting) {
    float ambient = 0.3;
    float diffuse = max(dot(normalize(normal), normalize(LightDirection)), 0.0);
    float lighting = ambient + diffuse * 0.7;
    FragmentColor = vec4(texColor.rgb * ColorTint * lighting, texColor.a * Transparency);
    return;
  }

  vec3 norm = normalize(normal);
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 result = vec3(0.0);

  // Directional light
  if (useDirLight) {
    result += CalcDirLight(dirLight, norm, viewDir, texColor.rgb);
  }

  // Point lights
  for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
    result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, texColor.rgb);
  }

  // Spotlights
  for(int i = 0; i < numSpotLights && i < MAX_SPOT_LIGHTS; i++) {
    result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir, texColor.rgb);
  }

  // Apply color tint
  result *= ColorTint;

  FragmentColor = vec4(result, texColor.a * Transparency);
}


vec3 CalcDirLight(DirectionalLight light, vec3 norm, vec3 viewDir, vec3 texColor) {
  vec3 lightDir = normalize(-light.direction);

  // Ambient
  vec3 ambient = light.ambient * material.ambient * texColor;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * (diff * material.diffuse) * texColor;

  // Specular
  float spec = 0.0;
  if (useBlinnPhong) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
  } else {
    vec3 reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  }
  vec3 specular = light.specular * (spec * material.specular);

  return ambient + diffuse + specular;
}


vec3 CalcPointLight(PointLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texColor) {
  vec3 lightDir = normalize(light.position - fragPos);

  // Ambient
  vec3 ambient = light.ambient * material.ambient * texColor;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * (diff * material.diffuse) * texColor;

  // Specular
  float spec = 0.0;
  if (useBlinnPhong) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
  } else {
    vec3 reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  }
  vec3 specular = light.specular * (spec * material.specular);

  // fall off
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return ambient + diffuse + specular;
}


vec3 CalcSpotLight(SpotLight light, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 texColor) {
  vec3 lightDir = normalize(light.position - fragPos);

  // Ambient
  vec3 ambient = light.ambient * material.ambient * texColor;

  // Diffuse
  float diff = max(dot(norm, lightDir), 0.0);
  vec3 diffuse = light.diffuse * (diff * material.diffuse) * texColor;

  // Specular
  float spec = 0.0;
  if (useBlinnPhong) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
  } else {
    vec3 reflectDir = reflect(-lightDir, norm);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  }
  vec3 specular = light.specular * (spec * material.specular);

  // intensity
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.cutOff - light.outerCutOff;
  float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

  diffuse *= intensity;
  specular *= intensity;

  // fall off
  float distance = length(light.position - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  return ambient + diffuse + specular;
}