//
//  graph_glyphs.c
//  Texture contenant les glyphs d'une police de caractères.
//
//  Created by Corentin Faucher on 2024-09-11.
//

#include "graph_glyphs.h"
#include "graph_texture_private.h"

#include "../utils/util_base.h"
#include "../coq_map.h"

// MARK: - Map de glyphs (pour dessiner les strings)
/// Une texture avec les glyphs/caractères d'une font.
typedef struct GlyphMap {
    Texture           tex;  // "upcasting"
    TextureDims const texDims;

    /// Info de la Font
    CoqFont*   font;

    size_t     currentTexX; // (Où est rendu pour dessiner la next glyphe)
    size_t     currentTexY;
    StringMap* glyphInfos;
    GlyphInfo  _defaultGlyph;

    // Custom chars...
    Texture   *cc_texOpt;
    TextureDims cc_texDims;
    Character *cc_charsOpt;
    Rectangle *cc_uvRectsOpt;
    size_t     cc_count;
} GlyphMap;

// MARK: - Dessin des glyphs avec CoreGraphics / Metal
GlyphInfo  glyphmap_drawCharacter_(GlyphMap* const gm, Character const c) {
    CoqFontDims const fd = coqfont_dims(gm->font);
    if(gm->currentTexY + fd.fullHeight > gm->tex.dims.height) {
        printerror("Cannot add glyph of %s. Glyphmap full.", c.c_str);
        return (GlyphInfo) {};
    }
    // Création de la string en pixels
    GlyphInfo gi = {};
    with_beg(PixelBGRAArray, pa, Pixels_engine_createArrayFromCharacter(c, gm->font))
    // Vérifier si on peut copier le glyph sur la ligne courante (sinon aller à la prochaine ligne).
    float const width = pa->width;
    if(gm->currentTexX + width > gm->tex.dims.width) {
        gm->currentTexX = 0;
        gm->currentTexY += fd.fullHeight;
        if(gm->currentTexY + fd.fullHeight > gm->tex.dims.height) {
            printerror("Cannot add glyph of %s. Glyphmap full.", c.c_str);
            goto free_pixels;
        }
        if(gm->currentTexY + fd.fullHeight > 0.65*gm->tex.dims.height)
            printwarning("Glyph map texture seems too small... Texture size is %d for font of height %f.",
                (int)gm->tex.dims.height, fd.fullHeight);
    }
    
    // Setter les infos (dimensions) du glyphs.
    // -> normalisés par rapport à solidHeight (la hauteur `hit-box`)
    gi = (GlyphInfo) {
        .uvRect = {{
            (float)gm->currentTexX   / gm->tex.dims.width, (float)gm->currentTexY / gm->tex.dims.height,
            (float)width / gm->tex.dims.width, (float)fd.fullHeight / gm->tex.dims.height
        }},
        .relGlyphX =     pa->deltaX     / fd.solidHeight,
        .relGlyphWidth = width          / fd.solidHeight,
        .relSolidWidth = pa->solidWidth / fd.solidHeight,
    };
    // Copier les pixels du char dans la texture de la glyph map.
    texture_engine_writePixelsToRegion(&gm->tex, pa->pixels,
        (RectangleUint){{ (uint32_t)gm->currentTexX, (uint32_t)gm->currentTexY,
                         (uint32_t)width, (uint32_t)pa->height }}
    );
    // Ok, fini, placer x tex coord pour next.
    gm->currentTexX += width;
    // Libérer les pixels
free_pixels:
    with_end(pa)
    return gi;
}
GlyphInfo  glyphmap_tryToDrawAsCustomCharacter_(GlyphMap* const gm, Character const c) {
    if(!gm->cc_texOpt) { return (GlyphInfo){}; }
    // Trouver si c'est un custom char.
    uint32_t charIndex = 0;
    for(; charIndex < gm->cc_count; charIndex++) {
        if(gm->cc_charsOpt[charIndex].c_data8 == c.c_data8)
            break;
    }
    if(charIndex >= gm->cc_count) return (GlyphInfo) {};

    // Vérifier les dimensions de la région à copier.
    CoqFontDims const fd = coqfont_dims(gm->font);
    if(gm->currentTexY + fd.fullHeight > gm->tex.dims.height) {
        printerror("Cannot add texture glyph. Glyphmap full.");
        return (GlyphInfo) {};
    }
    RectangleUint srcRegion = texturedims_pixelRegionFromUVrect(gm->cc_texDims, gm->cc_uvRectsOpt[charIndex]);
    if(srcRegion.h > fd.fullHeight) {
        printwarning("H > fullHeight."); srcRegion.h = (uint32_t)fd.fullHeight;
    }
    if(srcRegion.w > 3*fd.fullHeight) {
        printwarning("W > 3*glyphHeight."); srcRegion.w = (uint32_t)(3*fd.fullHeight);
    }
    size_t const extra_margin = FONT_extra_margin(gm->tex.flags & tex_flag_nearest);
    size_t const dstWidth = srcRegion.w + 2*extra_margin;

     // Vérifier s'il reste assez de place sur la ligne.
    if(gm->currentTexX + dstWidth > gm->tex.dims.width) {
        gm->currentTexX = 0;
        gm->currentTexY += fd.fullHeight;
        if(gm->currentTexY + fd.fullHeight > gm->tex.dims.height) {
            printerror("Cannot add texture glyph. Glyphmap full.");
            return (GlyphInfo){};
        }
        if(gm->currentTexY + fd.fullHeight > 0.65*gm->tex.dims.height)
            printwarning("Glyph map texture seems too small... Texture size is %d for font of height %f.",
                (int)gm->tex.dims.height, fd.fullHeight);
    }

    // Copier l'image pour ce char.
    UintPair dstOrigin = {(uint32_t)gm->currentTexX + (uint32_t)extra_margin,
                          (uint32_t)gm->currentTexY + (uint32_t)(fd.fullHeight - srcRegion.h)/2};

    // Dessin !
    texture_engine_copyRegionTo(gm->cc_texOpt, &gm->tex, srcRegion, dstOrigin);

    // Region du dessin (avec marge)
    RectangleUint glyphRegion = {{
        (uint32_t)gm->currentTexX, (uint32_t)gm->currentTexY,
        (uint32_t)dstWidth,        (uint32_t)fd.fullHeight,
    }};
    Rectangle glyphUVrect = texturedims_UVrectFromPixelRegion(gm->cc_texDims, glyphRegion);
    // Placer x tex coord sur next.
    gm->currentTexX += dstWidth;

    // 7. Infos du glyph dans la texture (glyphHeight_pixels est la dimension de référence)
    return (GlyphInfo) {
        .uvRect = glyphUVrect,
        .relGlyphX = 0,
        .relGlyphWidth = dstWidth    / fd.solidHeight,    // (avec marge)
        .relSolidWidth = srcRegion.w / fd.solidHeight, // (sans marge)
    };
}

// MARK: - Constructor / destructor
// (sous-fonction de `GlyphMap_create`)
size_t         textureWidthFromRefHeight_(float refHeight) {
    if(refHeight < 25) return 256;
    if(refHeight < 50) return 512;
    if(refHeight < 100) return 1024;
    return 2048;
}
void           glyphmap_deinit(GlyphMap* gm) {
    texture_deinit_(&gm->tex);
    map_destroyAndNull(&gm->glyphInfos, NULL);
    coqfont_engine_destroy(&gm->font);
    // Custom chars...
    if(gm->cc_charsOpt) coq_free(gm->cc_charsOpt);
    gm->cc_charsOpt = NULL;
    if(gm->cc_uvRectsOpt) coq_free(gm->cc_uvRectsOpt);
    gm->cc_uvRectsOpt = NULL;
    gm->cc_count = 0;
    gm->cc_texOpt = NULL;
}
GlyphMap*  GlyphMap_create(GlyphMapInit const info)
{
    // Création de glyphmap
    GlyphMap* gm = coq_callocTyped(GlyphMap);
    gm->font = CoqFont_engine_create(info.fontInit);
    gm->glyphInfos = Map_create(100, sizeof(GlyphInfo));
    // Custom chars
    if(info.customChars_count) {
        gm->cc_count = info.customChars_count;
        gm->cc_texOpt = info.customChars_texOpt;
        if(gm->cc_texOpt) gm->cc_texDims = texture_dims(gm->cc_texOpt);
        size_t charsSize = info.customChars_count * sizeof(Character);
        gm->cc_charsOpt = coq_malloc(charsSize);
        memcpy(gm->cc_charsOpt, info.customChars_charsOpt, charsSize);
        size_t rectsSize = info.customChars_count * sizeof(Rectangle);
        gm->cc_uvRectsOpt = coq_malloc(rectsSize);
        memcpy(gm->cc_uvRectsOpt, info.customChars_uvRectsOpt, rectsSize);
    }

    // 3. Init texture
    CoqFontDims const fd = coqfont_dims(gm->font);
    size_t textureWidth = info.textureWidthOpt;
    if(textureWidth < 5*fd.solidHeight) {
        textureWidth = textureWidthFromRefHeight_(fd.solidHeight);
    }
    texture_initEmpty_(&gm->tex);
    texture_engine_load_(&gm->tex, textureWidth, textureWidth, false, NULL);
    gm->tex.flags |= tex_flag_shared|(info.fontInit.nearest ? tex_flag_nearest : 0);
    *(TextureDims*)&gm->texDims = texture_dims(&gm->tex);

    // 4. Prédessiner au moins le ? (init avec plus de glyph ? genre abcd...)
    gm->_defaultGlyph = glyphmap_drawCharacter_(gm, spchar_questionMark);
    map_put(gm->glyphInfos, spchar_questionMark.c_str, &gm->_defaultGlyph);

    return gm;
}
void           glyphmapref_releaseAndNull(GlyphMap *const*const fgmOptRef) {
    GlyphMap* const fgm = *fgmOptRef;
    if(!fgm) return;
    *(GlyphMap***)&fgmOptRef = (GlyphMap**)NULL;

    glyphmap_deinit(fgm);
    coq_free(fgm);
}

static GlyphMap* GlyphMap_default_ = NULL;
GlyphMap* GlyphMap_default(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting with default params...");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return GlyphMap_default_;
}
bool   GlyphMap_default_isInit(void) {
    return GlyphMap_default_ != NULL;
}
void          GlyphMap_default_init(GlyphMapInit info)
{
    if(GlyphMap_default_) {
        printwarning("Default glyph map already init.");
        return;
    }
    GlyphMap_default_ = GlyphMap_create(info);
}
void          GlyphMap_default_deinit(void) {
    if(!GlyphMap_default_) return;
    glyphmap_deinit(GlyphMap_default_);
    free(GlyphMap_default_);
    GlyphMap_default_ = NULL;
}
CoqFont* GlyphMap_default_font(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting default.");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return GlyphMap_default_->font;
}
Texture*   GlyphMap_default_texture(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting default.");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return &GlyphMap_default_->tex;
}

// MARK: - Getters...
GlyphInfo      glyphmap_glyphInfoOfChar(GlyphMap* gm, Character c) {
    GlyphInfo* info = (GlyphInfo*)map_valueRefOptOfKey(gm->glyphInfos, c.c_str);
    if(info) {
        return *info;
    }
    // N'existe pas encore, dessiner et enregistrer
    GlyphInfo newGlyph = glyphmap_tryToDrawAsCustomCharacter_(gm, c);
    if(newGlyph.relSolidWidth == 0.f)
        newGlyph = glyphmap_drawCharacter_(gm, c);
    // Erreur ?
    if(newGlyph.relSolidWidth == 0.f) {
        printerror("Bad glyph for %s.", c.c_str);
        return gm->_defaultGlyph;
    }
    map_put(gm->glyphInfos, c.c_str, &newGlyph);
    return newGlyph;
}
Texture*       glyphmap_texture(GlyphMap*const gm) {
    return &gm->tex;
}
CoqFont const* glyphmap_font(GlyphMap const*const gm) {
    return gm->font;
}
