
#version 330 core

in vec4 vertexColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D firstTexture;
uniform sampler2D secondTexture;

void main()
{
  FragColor = mix(texture(firstTexture, texCoord), texture(secondTexture, texCoord), 0.2);
}
