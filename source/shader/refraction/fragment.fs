#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform samplerCube skybox;
uniform float ROI;

void main() {
  float ratio = 1.00 / ROI;
  vec3 I = normalize(Position);
  vec3 R = refract(I, normalize(Normal), ratio);
  R.z = -R.z;
  FragColor = vec4(texture(skybox, R).rgb, 1.0);
}


