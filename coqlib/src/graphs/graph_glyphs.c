//
//  graph_glyphs.c
//  Texture contenant les glyphs d'une police de caractères.
//
//  Created by Corentin Faucher on 2024-09-11.
//

#include "graph_glyphs.h"

#include "../utils/util_base.h"
#include "../coq_map.h"

#pragma mark - Map de glyphs (pour dessiner les strings)
/// Une texture avec les glyphs/caractères d'une font.
typedef struct GlyphMap {
    Texture      tex;  // "upcasting"

    /// Info de la Font
    coq_Font const font;

    size_t     currentTexX; // (Où est rendu pour dessiner la next glyphe)
    size_t     currentTexY;
    StringMap* glyphInfos;
    GlyphInfo  _defaultGlyph;

    // Custom chars...
    Texture   *cc_texOpt;
    Character *cc_charsOpt;
    Rectangle *cc_uvRectsOpt;
    size_t     cc_count;
} GlyphMap;

#pragma mark - Dessin des glyphs avec CoreGraphics / Metal
GlyphInfo  glyphmap_drawCharacter_(GlyphMap* const gm, Character const c) {
    size_t const glyphHeight = gm->font.glyphHeight;
    if(gm->currentTexY + glyphHeight > gm->tex.height) {
        printerror("Cannot add glyph of %s. Glyphmap full.", c.c_str);
        return (GlyphInfo) {};
    }

    // Création de la string en pixels
    PixelBGRAArray* paOwn = Pixels_engine_createArrayFromCharacter(c, gm->font);

    // Positionnement de la zone de copie...
    float const width = paOwn->width;
    if(gm->currentTexX + width > gm->tex.width) {
        gm->currentTexX = 0;
        gm->currentTexY += glyphHeight;
        if(gm->currentTexY + glyphHeight > gm->tex.height) {
            printerror("Cannot add glyph of %s. Glyphmap full.", c.c_str);
            coq_free(paOwn);
            return (GlyphInfo){};
        }
        if(gm->currentTexY + glyphHeight > 0.65*gm->tex.height)
            printwarning("Glyph map texture seems too small... Texture size is %d for font of height %zu.",
                (int)gm->tex.height, glyphHeight);
    }
    Rectangle uvRect = {{
        (float)gm->currentTexX   / gm->tex.width, (float)gm->currentTexY / gm->tex.height,
        (float)width / gm->tex.width, (float)glyphHeight / gm->tex.height
    }};
    // Copier les pixels du char dans la texture de la glyph map.
    texture_engine_writePixelsToRegion(&gm->tex, paOwn->pixels,
        (RectangleUint){{ (uint32_t)gm->currentTexX, (uint32_t)gm->currentTexY,
                         (uint32_t)width, (uint32_t)paOwn->height }}
    );

    // Placer x tex coord sur next.
    gm->currentTexX += width;
    float const deltaX = paOwn->deltaX;
    float const solidWidth = paOwn->solidWidth;
    // Libérer les pixels
    coq_free(paOwn);
    // Infos du glyph dans la texture
    // -> normalisés (relatifs) par rapport à solidHeight (la hauteur `hit-box`)
    return (GlyphInfo) {
        .uvRect = uvRect,
        .relGlyphX =     deltaX / gm->font.solidHeight,
        .relGlyphWidth = width  / gm->font.solidHeight,
        .relSolidWidth = solidWidth / gm->font.solidHeight,
    };
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
    size_t const glyphHeight = gm->font.glyphHeight;
    if(gm->currentTexY + glyphHeight > gm->tex.height) {
        printerror("Cannot add texture glyph. Glyphmap full.");
        return (GlyphInfo) {};
    }
    RectangleUint srcRegion = texture_pixelRegionFromUVrect(gm->cc_texOpt, gm->cc_uvRectsOpt[charIndex]);
    if(srcRegion.h > glyphHeight) {
        printwarning("H > glyphHeight."); srcRegion.h = (uint32_t)glyphHeight;
    }
    if(srcRegion.w > 3*glyphHeight) {
        printwarning("W > 3*glyphHeight."); srcRegion.w = (uint32_t)(3*glyphHeight);
    }
    size_t const extra_margin = (gm->tex.flags & tex_flag_nearest) ? 1 : 2;
    size_t const dstWidth = srcRegion.w + 2*extra_margin;

     // Vérifier s'il reste assez de place sur la ligne.
    if(gm->currentTexX + dstWidth > gm->tex.width) {
        gm->currentTexX = 0;
        gm->currentTexY += glyphHeight;
        if(gm->currentTexY + glyphHeight > gm->tex.height) {
            printerror("Cannot add texture glyph. Glyphmap full.");
            return (GlyphInfo){};
        }
        if(gm->currentTexY + glyphHeight > 0.65*gm->tex.height)
            printwarning("Glyph map texture seems too small... Texture size is %d for font of height %zu.",
                (int)gm->tex.height, glyphHeight);
    }

    // Copier l'image pour ce char.
    UintPair dstOrigin = {(uint32_t)gm->currentTexX + (uint32_t)extra_margin,
                          (uint32_t)gm->currentTexY + (uint32_t)(glyphHeight - srcRegion.h)/2};

    // Dessin !
    texture_engine_copyRegionTo(gm->cc_texOpt, &gm->tex, srcRegion, dstOrigin);

    // Region du dessin (avec marge)
    RectangleUint glyphRegion = {{
        (uint32_t)gm->currentTexX, (uint32_t)gm->currentTexY,
        (uint32_t)dstWidth,        (uint32_t)glyphHeight,
    }};
    Rectangle glyphUVrect = texture_UVrectFromPixelRegion(&gm->tex, glyphRegion);
    // Placer x tex coord sur next.
    gm->currentTexX += dstWidth;

    // 7. Infos du glyph dans la texture (glyphHeight_pixels est la dimension de référence)
    float const solidHeight = gm->font.solidHeight;
    return (GlyphInfo) {
        .uvRect = glyphUVrect,
        .relGlyphX = 0,
        .relGlyphWidth = dstWidth    / solidHeight,    // (avec marge)
        .relSolidWidth = srcRegion.w / solidHeight, // (sans marge)
    };
}

#pragma mark - Constructor / destructor
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
    coqfont_engine_deinit(&gm->font);
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
    coqfont_engine_init(&gm->font, info.fontNameOpt, info.fontSizeOpt, info.nearest);
    gm->glyphInfos = Map_create(100, sizeof(GlyphInfo));
    // Custom chars
    if(info.customChars_count) {
        gm->cc_count = info.customChars_count;
        gm->cc_texOpt = info.customChars_texOpt;
        size_t charsSize = info.customChars_count * sizeof(Character);
        gm->cc_charsOpt = coq_malloc(charsSize);
        memcpy(gm->cc_charsOpt, info.customChars_charsOpt, charsSize);
        size_t rectsSize = info.customChars_count * sizeof(Rectangle);
        gm->cc_uvRectsOpt = coq_malloc(rectsSize);
        memcpy(gm->cc_uvRectsOpt, info.customChars_uvRectsOpt, rectsSize);
    }

    // 3. Init texture
    size_t textureWidth = info.textureWidthOpt;
    if(textureWidth < 5*gm->font.solidHeight) {
        textureWidth = textureWidthFromRefHeight_(gm->font.solidHeight);
    }
    texture_initEmpty_(&gm->tex);
    texture_engine_loadEmptyWithSize_(&gm->tex, textureWidth, textureWidth);
    gm->tex.flags |= tex_flag_shared|(info.nearest? tex_flag_nearest : 0);

    // 4. Prédessiner au moins le ? (init avec plus de glyph ? genre abcd...)
    gm->_defaultGlyph = glyphmap_drawCharacter_(gm, spchar_questionMark);
    map_put(gm->glyphInfos, spchar_questionMark.c_str, &gm->_defaultGlyph);

    return gm;
}
void           glyphmapref_releaseAndNull(GlyphMap** const fgmOptRef) {
    GlyphMap* const fgm = *fgmOptRef;
    if(!fgm) return;
    *fgmOptRef = NULL;

    glyphmap_deinit(fgm);
    coq_free(fgm);
}

static GlyphMap* GlyphMap_default_ = NULL;
GlyphMap* GlyphMap_getDefault(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting default.");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return GlyphMap_default_;
}
void          GlyphMap_setDefault(GlyphMapInit info)
{
    if(GlyphMap_default_) {
        printwarning("Default glyph map already init.");
        return;
    }
    GlyphMap_default_ = GlyphMap_create(info);
}
void          GlyphMap_unsetDefault(void) {
    if(!GlyphMap_default_) return;
    glyphmap_deinit(GlyphMap_default_);
    free(GlyphMap_default_);
    GlyphMap_default_ = NULL;
}
coq_Font const* GlyphMap_getDefaultFont(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting default.");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return &GlyphMap_default_->font;
}
Texture*   GlyphMap_getDefaultTexture(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting default.");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return &GlyphMap_default_->tex;
}

#pragma mark - Getters...
GlyphInfo      glyphmap_getGlyphInfoOfChar(GlyphMap* gm, Character c) {
    GlyphInfo* info = (GlyphInfo*)map_valueRefOptOfKey(gm->glyphInfos, c.c_str);
    if(info) return *info;
    // N'existe pas encore, dessiner et enregistrer
    GlyphInfo newGlyph = glyphmap_tryToDrawAsCustomCharacter_(gm, c);
    if(newGlyph.relSolidWidth == 0.f)
        newGlyph = glyphmap_drawCharacter_(gm, c);
    // Erreur ?
    if(newGlyph.relSolidWidth == 0.f) return gm->_defaultGlyph;
    map_put(gm->glyphInfos, c.c_str, &newGlyph);
    return newGlyph;
}
Texture*       glyphmap_getTexture(GlyphMap*const gm) {
    return &gm->tex;
}
coq_Font const* glyphmap_getFont(GlyphMap const*const gm) {
    return &gm->font;
}
