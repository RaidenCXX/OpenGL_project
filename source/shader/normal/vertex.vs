
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
  vec3 normal;
  vec4 wiewPos;
}vs_out;

layout(std140) uniform Matrices{
  mat4 projection;
  mat4 view;
};

uniform mat4 model;
uniform mat3 inverse;

void main(){
  vs_out.wiewPos = view * model * vec4(aPos, 1.0f);
  vs_out.normal = inverse * aNormal;
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
