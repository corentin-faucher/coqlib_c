/*-- Vertex shader -----------------*/
/*-- Corentin Faucher, 2 fev 2019 --*/
#version 410


// Vertex attributes, i.e. vertex "in"
in vec4 in_position;
in vec2 in_uv;
// attribute vec3 in_normal;

// Vertex "out" (pour le fragment shader)
out vec2  out_uv;
// varying vec4  out_color;

// Per instance uniforms
uniform mat4  inst_model;
// uniform vec4  inst_color;
uniform vec2  inst_ij;  // tile de la texture
// uniform float inst_emph;
// uniform float inst_show;
// uniform int   inst_flags;

// Per texture uniforms
uniform vec2  tex_wh;
uniform vec2  tex_mn;

// Per frame uniforms
uniform mat4  frame_projection;
uniform float frame_time;



void main() {
    // out_color = vec4(inst_color.xyz, 1.) * inst_color.a * inst_show;

    vec4 posTmp = in_position + vec4(0.01*frame_time, 0.01*tex_mn.x, 0.01*tex_wh.x, 0.1*inst_ij.x);
    // // Déformation d'emphase et oscillation
    // if(inst_emph > 0.) {
    //     posTmp = in_position * (1. + inst_emph * 0.15 * 
    //         vec4(sin(frame_time*6.) + 2., sin(frame_time*6.+1.)+2., 0., 0.) );
    // }
    // Coord. uv en fonction de la tile.
    // out_uv = in_uv;
    out_uv = (in_uv + inst_ij) / tex_mn;  // Version simple ok ?
    // // out_uv = (in_uv * (tex_wh - tex_mn) + inst_ij * tex_wh) / (tex_mn*(tex_wh - 1.));

    // gl_Position = frame_projection * inst_model * posTmp;
    gl_Position = frame_projection * inst_model * in_position;
}

/*
    vUVrel = (vVerUV-0.5)*2.; // Espace de travail entre -1 et 1 pour jouer avec les textures...
    // Lumière et normales
    vec4 lightDirection = vec4(0,0,1,0);
    vec4 normalTmp = vec4(normal, 0.);
    normalTmp = normalize(inst_model * normalTmp);

    colorOut = vec4(inst_color.xyz * max(0.2,abs(dot(lightDirection,normalTmp))), inst_color.a);
*/