#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUVCord;

layout(std140) uniform Matrices{
  mat4 projection;
  mat4 view;
};

uniform mat4 model;
uniform mat3 inverse;

out vec3 Normal;
out vec3 FragPos;
out vec2 UVCord;

void main()
{
  UVCord = aUVCord;
  Normal = inverse * aNormal;
  FragPos = vec3(view * model * vec4(aPos, 1.0));
  gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
