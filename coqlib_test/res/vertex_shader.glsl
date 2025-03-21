/*-- Vertex shader -----------------*/
/*-- Corentin Faucher, 2 fev 2019 --*/
// (Version ajouter lors du chargement...)
//#version 300 es
//#version 410
precision mediump float;

// Vertex attributes, i.e. vertex "in"
in vec4 in_position;
in vec2 in_uv;
// in vec3 in_normal;

// Vertex "out" (pour le fragment shader)
out vec2 out_uv;
out vec4 out_color;

// Per instance uniforms
struct PerInstanceUniforms {
  mat4  model;
  int   flags;
  float show;
  float emph;
  int   extra2;
  vec4  color;
  vec2  uv0;
  vec2  Duv;
};
// Meilleur façon de passer un array quelconque pour les info d'instances (particules) ??
layout(std140) uniform InstanceBufferType {
  PerInstanceUniforms pius[100];
};

// Per texture uniforms
// uniform vec2  tex_wh;
//uniform vec2  tex_mn;

// Per frame uniforms
uniform mat4  frame_projection;
uniform float frame_time;


void main() {
    // out_color = vec4(inst_color.xyz, 1.) * inst_color.a * inst_show;
    out_color = vec4(pius[gl_InstanceID].color.xyz, pius[gl_InstanceID].color.a * pius[gl_InstanceID].show);

    vec4 posTmp;
    // // Déformation d'emphase et oscillation
//    if(pius[gl_InstanceID].emph > 0.) {
//        posTmp = in_position * (1. + pius[gl_InstanceID].emph * 0.15 *
//            vec4(sin(frame_time*6.) + 2., sin(frame_time*6.+1.)+2., 0., 0.) );
//    } else {
        posTmp = in_position;
//    }
    // Coord. uv en fonction de la tile. Version simple ok ?
    out_uv = pius[gl_InstanceID].uv0 + in_uv * pius[gl_InstanceID].Duv;
    // Version "pixel perfect"... Obsolete ?
    // out_uv = (in_uv * (tex_wh - tex_mn) + inst_ij * tex_wh) / (tex_mn*(tex_wh - 1.));
    gl_Position = pius[gl_InstanceID].model * posTmp;
}

// Exemple d'effet de lumière...
/*
    vUVrel = (vVerUV-0.5)*2.; // Espace de travail entre -1 et 1 pour jouer avec les textures...
    // Lumière et normales
    vec4 lightDirection = vec4(0,0,1,0);
    vec4 normalTmp = vec4(normal, 0.);
    normalTmp = normalize(inst_model * normalTmp);

    colorOut = vec4(inst_color.xyz * max(0.2,abs(dot(lightDirection,normalTmp))), inst_color.a);
*/
