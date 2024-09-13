//
//  metal_utils.metal
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 12/6/24.
//

#include <metal_stdlib>
using namespace metal;

float borderFactor(float2 box) {
    return (1 - exp(5*(box.x - 1))) * (1 - exp(-5*(box.x + 1)))
         * (1 - exp(5*(box.y - 1))) * (1 - exp(-5*(box.y + 1)));
}
 
