#version 410

// IN
// (4e élément de position automatiquement setter à 1)
layout(location=0) in vec4 in_position;
layout(location=1) in vec2 in_uv;
// Ici, on n'a pas besoin du vecteur normal...
// layout(location=2) in vec3 in_normal;

// Les "uniforms" d'une instance à afficher.
struct InstanceUniforms {
  mat4  model;
  int   flags;
  float show;
  float extra1;
  float extra2;
  vec4  color;
  vec2  uv0;
  vec2  Duv;
};
layout(std140) uniform InstanceBufferType {
  InstanceUniforms pius[100];
};

// OUT
struct RasterizedData {
  vec4 color;
  vec2 uv;
};
out RasterizedData rd;


void main() {
    gl_Position = pius[gl_InstanceID].model * in_position;
    rd.color = vec4(pius[gl_InstanceID].color.xyz, 
                    pius[gl_InstanceID].color.a * pius[gl_InstanceID].show);
    rd.uv = pius[gl_InstanceID].uv0 + in_uv * pius[gl_InstanceID].Duv;    
}
