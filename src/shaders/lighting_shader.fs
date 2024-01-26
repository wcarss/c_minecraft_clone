#version 410 core

out vec4 FragColor;
//in vec3 ourColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform vec3 lightPos;
uniform vec3 viewPos;

struct PointLight {
  vec3 pos;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};
#define MAX_NUM_OF_LIGHTS 100
uniform PointLight pointLights[MAX_NUM_OF_LIGHTS];

struct DirLight {
  vec3 dir;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform DirLight dirLight;

struct Material {
  sampler2D emission;
  sampler2D emission_map;
  vec3 ambient;
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

uniform Material material;
uniform int lightsUsed;
uniform samplerCube skybox;
uniform sampler2D shadowMap;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal);

void main()
{
  // properties
  vec3 norm = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);

  // phase 1: Directional lighting
  vec3 result = CalcDirLight(dirLight, norm, viewDir);

  // phase 2: Point lights
  for (int i = 0; i < lightsUsed; i++) {
    vec3 pointResult = CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    result.x = max(result.x, pointResult.x);
    result.y = max(result.y, pointResult.y);
    result.z = max(result.z, pointResult.z);
  }

  //vec3 emissionResult = texture(material.emission_map, TexCoords).rgb * texture(material.emission, TexCoords).rgb;
  //result.x = max(result.x, emissionResult.x);
  //result.y = max(result.y, emissionResult.y);
  //result.z = max(result.z, emissionResult.z);

  // phase 3: Spot light
  //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

  // vec3 I = normalize(FragPos - viewPos);
  // for reflection:
  //vec3 R = reflect(I, normalize(Normal));

  // for refraction (as opposed to reflection)
  // float ratio = 1.00 / 1.33;
  // air: 1.00, water 1.33, ice 1.309, glass 1.52, diamond 2.42
  // vec3 R = refract(I, normalize(Normal), ratio);
  //vec3 reflect_result = texture(skybox, R).rgb;

  // 0.68 and 0.58 "just because" -- need to darken the values, given we're summing them
  //FragColor = vec4(result * 0.92 + reflect_result * 0.08, 1.0);
  FragColor = vec4(result , 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
  vec3 lightDir = normalize(-light.dir);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
  float shadow = ShadowCalculation(FragPosLightSpace, lightDir, normal);
  return (ambient + (1.0 - shadow) * (diffuse + specular));
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

  if (projCoords.z > 1.0) {
    return 0.0;
  }

  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.005);
  // check whether current frag pos is in shadow

  float shadow = 0.0;
  vec2 texelSize = 0.71 / textureSize(shadowMap, 0);

  for (int x = -2; x <= 2; ++x) {
    for (int y = -2; y <= 2; ++y) {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }

  shadow /= 25.0;
  return shadow;
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
  vec3 lightDir = normalize(light.pos - fragPos);
  // diffuse shading
  float diff = max(dot(normal, lightDir), 0.0);
  // specular shading
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  // attenuation
  float distance    = length(light.pos - fragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance +
                             light.quadratic * (distance * distance));
  // combine results
  vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));
  vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
  ambient  *= attenuation;
  diffuse  *= attenuation;
  specular *= attenuation;
  return (ambient + diffuse + specular);
}
