//
//  definitions.h
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 12/6/24.
//

#ifndef definitions_h
#define definitions_h

struct VertexIn {
    packed_float3 position;
    packed_float2 uv;
    packed_float3 normal;
};

struct InstanceUniforms {
    float4x4 model;
    uint32_t flags;
    float    show;
    float    extra1;
    float    extra2;
    float4   color;
    float2   uv0;
    float2   Duv;
};

#endif /* definitions_h */
