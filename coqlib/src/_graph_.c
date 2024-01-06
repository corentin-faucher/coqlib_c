//
//  _graph_.c
//
//  Created by Corentin Faucher on 2023-10-12.
//

#include "_graph/_graph_.h"

const  PerInstanceUniforms piu_default = PIU_DEFAULT;

const PerTextureUniforms ptu_default = PTU_DEFAULT;

PerFrameUniforms pfu_default = {
    {{
        1.f, 0.f, 0.f, 0.f,
        0.f, 1.f, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        0.f, 0.f, 0.f, 1.f,
    }},
    0.f, 0.f, 0.f, 0.f
};


