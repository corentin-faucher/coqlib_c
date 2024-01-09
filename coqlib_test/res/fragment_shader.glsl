/*-- Fragment shader -----------------*/
/*-- Corentin Faucher, 3 fev 2019 --*/
#version 410

// precision mediump float;
in vec2  out_uv;
in vec4  out_color;

uniform sampler2D tex;

out vec4 out_frag_color;

void main()
{
    out_frag_color = texture(tex, out_uv) * out_color;
//    out_frag_color = vec4(out_uv.x+0.25, out_uv.y + 0.25, 0.25, 1);
}


