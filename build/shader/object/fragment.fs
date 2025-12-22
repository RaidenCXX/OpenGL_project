#version 330 core


struct DirLight{
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight{
  vec3 position;

  float constant;
  float linear;
  float quadratic;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct FlashLight{
  vec3 position;
  vec3 direction;

  vec3 diffuse;
  vec3 specular;
  
  float cutOff;
  float outerCutOff;
};

#define NR_POINT_LIGHT 6

struct Material{
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

out vec4 FragColor;

uniform PointLight pointLight[NR_POINT_LIGHT];
uniform DirLight dirLight;
uniform FlashLight flashLight;
uniform Material material;

in vec3 Normal;
in vec3 FragPos;
in vec2 UVCord;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal,vec3 viewDir);
vec3 CalcFlashLight(FlashLight light, vec3 normal, vec3 viewDir);

void main(){

  vec3 viewFromFragToCamera = normalize(-FragPos);
  vec3 normal = normalize(Normal);
  vec3 result = vec3(0.0);
  
  result += CalcDirLight(dirLight ,normal, viewFromFragToCamera);
  
  for(int i = 0; i<NR_POINT_LIGHT; ++i)
    result += CalcPointLight(pointLight[i], normal, viewFromFragToCamera);

  result += CalcFlashLight(flashLight, normal, viewFromFragToCamera);

  FragColor = vec4(result, 1.0);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
  vec3 lightDir = normalize(-light.direction);

  //diffuse
  float diff = max(dot(normal, lightDir), 0.0f);

  //specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

  //combine results
  vec3 ambient = light.ambient * vec3(texture(material.diffuse, UVCord)); 
  vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, UVCord));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, UVCord));

  return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal,vec3 viewDir){
 
  vec3 lightDirection = light.position - FragPos;
  float distance = length(lightDirection);
  vec3 lightDir = lightDirection / distance;
 
 //diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  //specular
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

  //attenuation
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  //combine
  vec3 ambient = light.ambient * vec3(texture(material.diffuse, UVCord));
  vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, UVCord));
  vec3 specular = light.specular * spec * vec3(texture(material.specular, UVCord));

  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;
  
  return (ambient + diffuse + specular);
}

vec3 CalcFlashLight(FlashLight light, vec3 normal, vec3 viewDir){

 
  vec3 diffuse;
  vec3 specular;
  
  vec3 lightDirection = light.position - FragPos;
  float distance = length(lightDirection);
  vec3 lightDir = lightDirection / distance;
  float theta = dot(lightDir, normalize(-light.direction));

  if(theta > light.cutOff){

    
    float epsilon = light.outerCutOff - light.cutOff ;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    //diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    //combine
    diffuse = light.diffuse * diff * vec3(texture(material.diffuse, UVCord));
    diffuse *= intensity;
    specular = light.specular * spec * vec3(texture(material.specular, UVCord));
    specular *= intensity;
    
  } else {
    diffuse = vec3(0.0);
    specular = vec3(0.0);
  }
  
  return (diffuse + specular);
}



