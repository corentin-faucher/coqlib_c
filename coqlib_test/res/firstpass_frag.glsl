#version 410

// IN
struct RasterizedData {
  vec4 color;
  vec2 uv;
};
in RasterizedData rd;

uniform sampler2D tex;

// OUT
layout(location=0) out vec4 color0;
// Ici, juste une texture de sortie pour le framebuffer,
// mais on pourrait avoir plusieurs sorties possibles.
// layout(location=1) out vec4 color1;

void main() {
    color0 = texture(tex, rd.uv) * rd.color;
}
