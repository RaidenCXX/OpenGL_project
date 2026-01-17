#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUVCord;
layout (location = 3) in mat4 instanceMatrix;
layout (location = 7) in mat3 instanceNormal;

layout(std140) uniform Matrices{
  mat4 projection;
  mat4 view;
};

out vec3 Normal;
out vec3 FragPos;
out vec2 UVCord;

void main()
{
  UVCord = aUVCord;
  Normal =  transpose(inverse(mat3(view * instanceMatrix))) * aNormal;
  FragPos = vec3(view * instanceMatrix * vec4(aPos, 1.0));
  gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0f);
}
