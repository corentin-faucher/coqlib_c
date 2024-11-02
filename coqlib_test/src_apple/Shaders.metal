//
//  Shaders.metal
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include <metal_stdlib>
using namespace metal;

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

struct RasterizerData {
    float4 position [[position]];
    float4 color;
    float2 uv;
};

vertex RasterizerData vertex_function(const device VertexIn    *vertices [[buffer(0)]],
                                 const device InstanceUniforms *pius [[buffer(1)]],
                                 unsigned int vid [[vertex_id]],
                                 unsigned int iid [[instance_id]]
) {
    VertexIn in = vertices[vid];
    RasterizerData out;
    
    out.color = float4(pius[iid].color.xyz, pius[iid].color.a * pius[iid].show);
    out.uv = pius[iid].uv0 + pius[iid].Duv * in.uv;
    out.position = pius[iid].model*float4(in.position, 1); 
    
    return out;
}

fragment float4 fragment_function(RasterizerData rd [[ stage_in ]],
                                 texture2d<float> texInstance [[ texture(0)]],
                                 sampler sampler2D [[sampler(0)]])
{
    if (is_null_texture(texInstance))
        return rd.color;
        
    return texInstance.sample(sampler2D, rd.uv) * rd.color;
}


struct SecondRasterizerData {
    float4 pos [[position]];
    float2 uv;
};

vertex SecondRasterizerData second_vertex_function(const device VertexIn *vertices [[buffer(0)]],
                                                 unsigned int vid [[vertex_id]]
) {
    VertexIn in = vertices[vid];
    SecondRasterizerData rd;
    rd.pos = float4(in.position.xyz, 1.0);
    rd.uv =  in.uv;
    
    return rd;
}

fragment float4 second_fragment_function(SecondRasterizerData rd [[ stage_in ]],
                                        texture2d<float> texColor [[ texture(0) ]],
                                        sampler sampler2D [[sampler(0)]]
) {
    return texColor.sample(sampler2D, rd.uv);
}
