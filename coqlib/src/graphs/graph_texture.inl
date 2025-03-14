//
//  graph_texture_inline.h
//  xc_jeux_de_fusils
//
//  Created by Corentin Faucher on 2025-01-17.
//

static inline Rectangle texturedims_uvRectOfTile(TextureDims const texDims, uint32_t const i, uint32_t const j) {
    return (Rectangle) {
        .origin = {{ 
            (float)(     i % texDims.m)              * texDims.Du,
            (float)((j + i / texDims.m) % texDims.n) * texDims.Dv,
        }},
        .size = texDims.DuDv
    };
}
/// Convertie les coord. UV (dans [0, 1]) en coord. pixels (dans [0, width/height]).
static inline RectangleUint texturedims_pixelRegionFromUVrect(TextureDims const texDims, Rectangle const UVrect) {
    return (RectangleUint) {{
        texDims.width * UVrect.o_x, texDims.height * UVrect.o_y,
        texDims.width * UVrect.w,   texDims.height * UVrect.h,
    }};
}
/// Convertie les coord. pixels (dans [0, width/height]) en coord. UV (dans [0, 1])
static inline Rectangle     texturedims_UVrectFromPixelRegion(TextureDims const texDims, RectangleUint const region) {
    return (Rectangle) {{
        (float)region.o_x / texDims.width, (float)region.o_y / texDims.height,
        (float)region.w   / texDims.width, (float)region.h   / texDims.height,
    }};
}

