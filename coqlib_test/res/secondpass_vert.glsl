#version 410

// IN
layout(location=0) in vec2 pos;
layout(location=1) in vec2 uv;

// OUT
out vec2 rd_uv; // (Rasterized Data, ici juste coord. uv.)

void main() {
    gl_Position = vec4(pos, 0, 1);
    rd_uv = uv;
}
