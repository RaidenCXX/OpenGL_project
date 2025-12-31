
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 inverse;

void main()
{
  vec3 viewPos  = (view * model * vec4(aPos, 1.0)).xyz;
  vec3 viewNorm = normalize(inverse * aNormal);

  float outlineThickness = 0.03;
  vec3 outlinedPos = viewPos + viewNorm * outlineThickness;
  gl_Position = projection * vec4(outlinedPos, 1.0);
}



