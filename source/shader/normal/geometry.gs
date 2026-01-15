
#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT{
  vec3 normal;
  vec4 wiewPos;
}gs_in[];

layout(std140) uniform Matrices{
  mat4 projection;
  mat4 view;
};

const float MAGNITUDE = 0.1;



void GenerateLine(int index){

  vec4 p0 = gs_in[index].wiewPos;
  vec4 p1 = gs_in[index].wiewPos + vec4(gs_in[index].normal * MAGNITUDE, 0.0);

  gl_Position = projection * p0;
  EmitVertex();

  gl_Position = projection * p1;
  EmitVertex();
  EndPrimitive();
}

void main(){
  GenerateLine(0);
  GenerateLine(1);
  GenerateLine(2);
}
