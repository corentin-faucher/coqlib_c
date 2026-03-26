#version 410

// IN
in  vec2 rd_uv;
// Les textures générées par la première passe. Juste une ici.
// (Color attachment du framebuffer)
uniform sampler2D color0;
// uniform sampler2D color1;

// OUT
out vec4 color;

void main() {
    color = texture(color0, rd_uv);
}
