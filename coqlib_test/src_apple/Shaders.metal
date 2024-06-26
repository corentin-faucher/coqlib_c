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

struct VertexOut {
    float4 position [[position]];
    float2 uv;
    float4 color;
};

struct PerInstanceUniforms {
    float4x4 model;
    float4   color;
    float2   uv0;
    float2   Duv;
    float    emph;
    float    show;
    uint     flags;
    float    unused1;
};

//struct PerTextureUniforms {
//    float2 wh;  // (width, height)
//    float2 mn;  // tiling (m, n)
//};

struct PerFrameUniforms {
    float4x4 projection;
    float time;
    float unused1;
    float unused2;
    float unused3;
};

vertex VertexOut vertex_function(const device VertexIn        *vertices [[buffer(0)]],
                                 const device PerInstanceUniforms* pius [[buffer(1)]],
                                 const device PerFrameUniforms&    pfu  [[buffer(2)]],
//                                 const device PerTextureUniforms&  ptu  [[buffer(3)]],
                                 unsigned int vid [[vertex_id]],
                                 unsigned int iid [[instance_id]]
) {
    VertexIn in = vertices[vid];
    VertexOut out;
    out.color = float4(pius[iid].color.xyz, pius[iid].color.a * pius[iid].show);
    out.uv =  pius[iid].uv0 + in.uv * pius[iid].Duv;
//    out.uv =    (in.uv + pius[iid].tile_ij) / ptu.mn;
//    out.uv = (in.uv * (ptu.sizes - ptu.dims) + pius[iid].tile_ij * ptu.sizes) / (ptu.dims * (ptu.sizes - 1));
    // Position des vertex
//    out.position = float4(in.position, 1);
    float4 posTmp = float4(in.position, 1);
//    if (pius[iid].emph > 0) {
//        posTmp = posTmp * (1 + pius[iid].emph * 0.15 * float4(sin(pfu.time * 6)+2, sin(pfu.time * 6 + 1)+2, 0, 0));
//    }
    out.position = pfu.projection*pius[iid].model*posTmp; //
    
    return out;
}

fragment float4 fragment_function(VertexOut interpolated [[ stage_in ]],
                                 texture2d<float> tex2D [[ texture(0)]],
                                 sampler sampler2D [[sampler(0)]])
{
//    return float4(0,1,0,0.5);
//    return tex2D[interpolated.textureID].sample(sampler2D, interpolated.uv);
    if (is_null_texture(tex2D)) {
        return float4(0,1,0,0.5);
//        return interpolated.color;
    } else {
        return tex2D.sample(sampler2D, interpolated.uv) * interpolated.color;
    }
}
