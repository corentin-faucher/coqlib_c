//
//  graph_glyphs.c
//  Texture contenant les glyphs d'une police de caractères.
//
//  Created by Corentin Faucher on 2024-09-11.
//

#include "graph_glyphs.h"
#include "graph_texture_private.h"

#include "../systems/system_base.h"
#include "../systems/system_file.h"
#include "../systems/system_language.h"
#include "../utils/util_base.h"
#include "../utils/util_map.h"

// MARK: - Map de glyphs (pour dessiner les strings)
/// Une texture avec les glyphs (caractères) d'une font.
typedef struct GlyphMap {
    // 1. Une texture
    Texture           tex;  // "upcasting"
    TextureDims const texDims;
    
    // 2. Une map pour retrouver les glyphes
    StringMap* glyphInfos;  // Map char -> glyph
    size_t     currentTexX; // (Où on est rendu pour dessiner la next glyphe)
    size_t     currentTexY;
    
    // 3. La Font utilisée pour dessiner les glyphes de char dans la texture.
    CoqFont*const     font;
    CoqFontDims const fontDims;
    
    GlyphInfo  _defaultGlyph;

    // Custom chars...
    Texture   *cc_texOpt;
    TextureDims cc_texDims;
    Character *cc_charsOpt;
    Rectangle *cc_uvRectsOpt;
    size_t     cc_count;
} GlyphMap;
// La glyph map par défaut.
static GlyphMap* GlyphMap_default_ = NULL;

void glyphmap_render_deinit(GlyphMap* gm) {
    texture_render_deinit_(&gm->tex);
    map_destroyAndNull(&gm->glyphInfos, NULL);
    coqfont_engine_destroy((CoqFont**)&gm->font);
    // Custom chars...
    if(gm->cc_charsOpt) coq_free(gm->cc_charsOpt);
    gm->cc_charsOpt = NULL;
    if(gm->cc_uvRectsOpt) coq_free(gm->cc_uvRectsOpt);
    gm->cc_uvRectsOpt = NULL;
    gm->cc_count = 0;
    gm->cc_texOpt = NULL;
}
void glyphmapref_render_releaseAndNull(GlyphMap *const*const fgmOptRef) {
    GlyphMap* const fgm = *fgmOptRef;
    if(!fgm) return;
    *(GlyphMap***)&fgmOptRef = (GlyphMap**)NULL;
    // Default reste disponible (shared).
    if(fgm == GlyphMap_default_) { return; }

    glyphmap_render_deinit(fgm);
    coq_free(fgm);
}
// (sous-fonction de `GlyphMap_create`)
size_t textureWidthFromRefHeight_(float refHeight) {
    if(refHeight < 25) return 256;
    if(refHeight < 50) return 512;
    if(refHeight < 100) return 1024;
    return 2048;
}
GlyphInfo glyphmap_drawCharacter_(GlyphMap* gm, Character c);
GlyphMap* GlyphMap_create(GlyphMapInit const info)
{
    // Création de glyphmap
    GlyphMap* gm = coq_callocTyped(GlyphMap);
    *(CoqFont**)&gm->font = CoqFont_engine_create(info.fontInit);
    *(CoqFontDims*)&gm->fontDims = coqfont_dims(gm->font);
    gm->glyphInfos = Map_create(100, sizeof(GlyphInfo));
    // Custom chars
    if(info.customChars_count && info.customChars_texOpt && 
       info.customChars_charsOpt && info.customChars_uvRectsOpt) 
    {
        gm->cc_count = info.customChars_count;
        gm->cc_texOpt = info.customChars_texOpt;
        gm->cc_texDims = texture_dims(gm->cc_texOpt);
        if(!(gm->cc_texOpt->flags & tex_flag_keepPixels)) {
            printwarning("Custom chars texture %s does not keep pixels.", gm->cc_texOpt->string);
        }
        size_t charsSize = info.customChars_count * sizeof(Character);
        gm->cc_charsOpt = coq_malloc(charsSize);
        memcpy(gm->cc_charsOpt, info.customChars_charsOpt, charsSize);
        size_t rectsSize = info.customChars_count * sizeof(Rectangle);
        gm->cc_uvRectsOpt = coq_malloc(rectsSize);
        memcpy(gm->cc_uvRectsOpt, info.customChars_uvRectsOpt, rectsSize);
    } else if(info.customChars_count || info.customChars_texOpt || 
              info.customChars_charsOpt || info.customChars_uvRectsOpt) 
    {
        printwarning("Missing custom chars info.");
    }

    // Init texture
    size_t textureWidth = info.textureWidthOpt;
    if(textureWidth < 5*gm->fontDims.solidHeight) {
        textureWidth = textureWidthFromRefHeight_(gm->fontDims.solidHeight);
    }
    texture_initEmpty_(&gm->tex, textureWidth, textureWidth, 
        tex_flag_mutable|tex_flag_shared|(info.fontInit.nearest ? tex_flag_nearest : 0));
    *(TextureDims*)&gm->texDims = texture_dims(&gm->tex);

    // Prédessiner au moins le `?` (init avec plus de glyph ? genre abcd...)
    gm->_defaultGlyph = glyphmap_drawCharacter_(gm, spchar_questionMark);
    map_put(gm->glyphInfos, spchar_questionMark.c_str, &gm->_defaultGlyph);

    return gm;
}

// Vérifier si on peut copier le glyph sur la ligne courante
// Retourne true si ok.
bool      glyphmap_checkDrawingPoint_(GlyphMap*const gm, 
              float const toAddWidth)
{
    if(gm->currentTexX + toAddWidth <= gm->tex.dims.width &&
       gm->currentTexY + gm->fontDims.fullHeight <= gm->tex.dims.height)
        return true;
    // Plus assez d'espace sur cette "ligne", aller à la prochaine.
    gm->currentTexX = 0;
    gm->currentTexY += gm->fontDims.fullHeight;
    if(gm->currentTexY + gm->fontDims.fullHeight > gm->tex.dims.height) {
        printerror("Cannot add glyph. Glyphmap full.");
        return false;
    }
    if(gm->currentTexY + gm->fontDims.fullHeight > 0.65*gm->tex.dims.height)
        printwarning("Glyph map texture seems too small... "
                     "Texture size is %d for font of height %f.",
                     (int)gm->tex.dims.height, gm->fontDims.fullHeight);
    return true;
}
static inline GlyphInfo glyphmap_getGlyphInfoForPixelArrayAtDrawingPoint_(GlyphMap*const gm, PixelArray*const pa) {
    return (GlyphInfo) {
        .uvRect = {{
            (float)gm->currentTexX   / gm->tex.dims.width, (float)gm->currentTexY / gm->tex.dims.height,
            (float)pa->width / gm->tex.dims.width, (float)gm->fontDims.fullHeight / gm->tex.dims.height
        }},
        .relGlyphX =     pa->deltaX     / gm->fontDims.solidHeight,
        .relGlyphY =     gm->fontDims.deltaY/gm->fontDims.solidHeight,
        .relGlyphWidth = pa->width      / gm->fontDims.solidHeight,
        .relGlyphHeight = gm->fontDims.fullHeight / gm->fontDims.solidHeight,
        .relSolidWidth = pa->solidWidth / gm->fontDims.solidHeight,
    };
}
GlyphInfo   glyphmap_drawCharacter_(GlyphMap* const gm, Character const c) 
{
    if(gm->currentTexY + gm->fontDims.fullHeight > gm->tex.dims.height) {
        printerror("Cannot add glyph of %s. Glyphmap full.", c.c_str);
        return (GlyphInfo) {};
    }
    // Création de la string en pixels
    GlyphInfo gi = {};
    with_beg(PixelArray, pa, PixelsArray_engine_createFromCharacter(c, gm->font))
    if(!glyphmap_checkDrawingPoint_(gm, pa->width))
        goto free_pixels;
    
    // Setter les infos (dimensions) du glyphs.
    // -> normalisés par rapport à solidHeight (la hauteur `hit-box`)
    gi = glyphmap_getGlyphInfoForPixelArrayAtDrawingPoint_(gm, pa); 
    
    // Copier les pixels du char dans la texture de la glyph map.
    withTextureToEdit_beg(texEdit, &gm->tex)
    pixelarray_copyAt(pa, texEdit.pa, (UintPair) { (uint32_t)gm->currentTexX, 
                                                       (uint32_t)gm->currentTexY });
    withTextureToEdit_end(texEdit)
    
    // Ok, fini, placer x tex coord pour next.
    gm->currentTexX += pa->width;
    
    // Libérer les pixels
free_pixels:
    with_end(pa)
    return gi;
}

// Retourne -1 si n'est pas un custom char.
int         glyphmap_getCustomCharIndex_(GlyphMap*const gm, Character const c) 
{
    for(int index = 0; index < gm->cc_count; index++)
        if(gm->cc_charsOpt[index].c_data8 == c.c_data8)
            return index;
    return -1;
}
GlyphInfo   glyphmap_tryToDrawAsCustomCharacter_(GlyphMap* const gm, Character const c) 
{
    // Vérifier les dimensions de la région à copier.
    if(gm->currentTexY + gm->fontDims.fullHeight > gm->tex.dims.height) {
        printerror("Cannot add texture glyph. Glyphmap full.");
        return (GlyphInfo) {};
    }
    PixelArray* pa = NULL;
    // Custom char ?
    int const index = glyphmap_getCustomCharIndex_(gm, c);
     
    if(index < 0) {
        if(!character_isEmoji(c)) return (GlyphInfo) {};
        uint32_t unicode = character_toUnicode32(c);
        char fileName[16] = {};
        sprintf(fileName, "emoji_u%x", unicode);
        char*const path = FileManager_getResourcePath();
        String_pathAdd(path, fileName, "svg", "svgs");
        pa = PixelArray_createFromSvgFileOpt(path, gm->fontDims.solidHeight, false);
    }
    
    RectangleUint srcRegion = texturedims_pixelRegionFromUVrect(gm->cc_texDims, gm->cc_uvRectsOpt[index]);
    if(srcRegion.h > gm->fontDims.fullHeight) {
        printwarning("H > fullHeight."); srcRegion.h = (uint32_t)gm->fontDims.fullHeight;
    }
    if(srcRegion.w > 3*gm->fontDims.fullHeight) {
        printwarning("W > 3*glyphHeight."); srcRegion.w = (uint32_t)(3*gm->fontDims.fullHeight);
    }
    size_t const extra_margin = FONT_extra_margin(gm->tex.flags & tex_flag_nearest);
    size_t const dstWidth = srcRegion.w + 2*extra_margin;

    // Vérifier s'il reste assez de place sur la ligne.
    if(!glyphmap_checkDrawingPoint_(gm, dstWidth))
        return (GlyphInfo){};
    
    // Endroit où on est rendu pour dessiner le glyph dans la glyphMap. 
    UintPair dstOrigin = {(uint32_t)gm->currentTexX + (uint32_t)extra_margin,
                          (uint32_t)gm->currentTexY + (uint32_t)(gm->fontDims.fullHeight - srcRegion.h)/2};

    // Dessin !
    if_let(PixelArray const*, paSrc, texture_getPixelsOpt(gm->cc_texOpt))
    withTextureToEdit_beg(texEdit, &gm->tex)
    pixelarray_copyRegionAt(paSrc, srcRegion, texEdit.pa, dstOrigin);
    withTextureToEdit_end(texEdit)
    if_let_else printerror("Cannot get pixels from custom char texture.");
    if_let_end

    // Region du dessin (avec marge)
    RectangleUint glyphRegion = {{
        (uint32_t)gm->currentTexX, (uint32_t)gm->currentTexY,
        (uint32_t)dstWidth,        (uint32_t)gm->fontDims.fullHeight,
    }};
    // Placer x tex coord sur next.
    gm->currentTexX += dstWidth;

    // Infos du glyph dans la texture (glyphHeight_pixels est la dimension de référence)
    Rectangle glyphUVrect = texturedims_UVrectFromPixelRegion(gm->cc_texDims, glyphRegion);
    return (GlyphInfo) {
        .uvRect = glyphUVrect,
        .relGlyphX = 0,
        .relGlyphY = gm->fontDims.deltaY / gm->fontDims.solidHeight,
        .relGlyphWidth = dstWidth    / gm->fontDims.solidHeight,    // (avec marge)
        .relGlyphHeight = gm->fontDims.fullHeight / gm->fontDims.solidHeight,
        .relSolidWidth = srcRegion.w / gm->fontDims.solidHeight, // (sans marge)
    };
}

GlyphMap*   GlyphMap_default(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting with default params...");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return GlyphMap_default_;
}
bool        GlyphMap_default_isInit(void) {
    return GlyphMap_default_ != NULL;
}
void        GlyphMap_default_init(GlyphMapInit info)
{
    if(GlyphMap_default_) {
        printwarning("Default glyph map already init.");
        return;
    }
    GlyphMap_default_ = GlyphMap_create(info);
}
void        GlyphMap_default_deinit(void) {
    if(!GlyphMap_default_) return;
    glyphmap_render_deinit(GlyphMap_default_);
    free(GlyphMap_default_);
    GlyphMap_default_ = NULL;
}
CoqFont*    GlyphMap_default_font(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting default.");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return GlyphMap_default_->font;
}
Texture*    GlyphMap_default_texture(void) {
    if(!GlyphMap_default_) {
        printwarning("Default font glyph map not init. Setting default.");
        GlyphMap_default_ = GlyphMap_create((GlyphMapInit){});
    }
    return &GlyphMap_default_->tex;
}

// MARK: - Getters...
GlyphInfo      glyphmap_glyphInfoOfChar(GlyphMap*const gm, Character const c) {
    // Déjà dessiné ?
    if_let(GlyphInfo*, info,
            (GlyphInfo*)map_valueRefOptOfKey(gm->glyphInfos, c.c_str))
        return *info;
    if_let_end
    // N'existe pas encore, dessiner et enregistrer.
    // Vérifier en premier s'il est dans la liste des "custom". 
    GlyphInfo newGlyph = glyphmap_tryToDrawAsCustomCharacter_(gm, c);
    // Sinon, dessiner normalement.
    if(newGlyph.relSolidWidth == 0.f)
        newGlyph = glyphmap_drawCharacter_(gm, c);
    // Erreur ?
    if(newGlyph.relSolidWidth == 0.f) {
        printerror("Bad glyph for %s, %d.", c.c_str, c.c_data4);
        return gm->_defaultGlyph;
    }
    // Ajouter à la liste.
    map_put(gm->glyphInfos, c.c_str, &newGlyph);
    return newGlyph;
}
Texture*       glyphmap_texture(GlyphMap*const gm) {
    return &gm->tex;
}
CoqFont const* glyphmap_font(GlyphMap const*const gm) {
    return gm->font;
}
CoqFontDims    glyphmap_fontDims(GlyphMap const* gm) {
    return gm->fontDims;
}

// MARK: - StringGlyphed : Une string avec info pour dessiner.

CharacterArray* CharacterArray_create_(const char*const c_str, bool isLocalized) {
    if(isLocalized) {
        CharacterArray* ca = NULL;
        with_beg(char const, localized, String_createLocalized(c_str))
        ca = CharacterArray_createFromString(localized);
        with_end(localized)
        return ca;
    }
    return CharacterArray_createFromString(c_str);
}
typedef struct StringGlyphed {
    size_t const     charCount;
    size_t const     maxCount;
    GlyphMap*const   glyphMap;     // La glyph map utilisée pour définir les dimensions des chars.
    float const      xEndRel;     // Largeur totale de la string (relative à solid height, négatif si right-to-left)
//    float const      fullWidthRel; // Largeur totale + 2x x_margin.
    float const      x_margin;     // Marge en x (relative à solid height)
    float const      spacing;      // Le scaling voulu (espace entre les CharacterGlyphed).
    bool const       isRightToLeft; // (Arabe)
    CharacterGlyphed chars[1];     // Les chars avec info de dimensions.
} StringGlyphed;
StringGlyphed* StringGlyphed_create(StringGlyphedInit const init) {
    CharacterArray*const caOpt = init.c_str ? CharacterArray_create_(init.c_str, init.isLocalized) : NULL;
    size_t const charCount = caOpt ? characterarray_count(caOpt) : 0;
    size_t const maxCount = (init.maxCountOpt > charCount) ? init.maxCountOpt : (charCount > 0 ? charCount : 1); 
    StringGlyphed* sg = coq_callocArray(StringGlyphed, CharacterGlyphed, maxCount);
    size_initConst(&sg->charCount, charCount);
    size_initConst(&sg->maxCount, maxCount);
    *(GlyphMap**)&sg->glyphMap = init.glyphMapOpt ? init.glyphMapOpt : GlyphMap_default();
    float_initConst(&sg->x_margin, init.x_margin);
    float_initConst(&sg->spacing, init.spacing);
    bool_initConst(&sg->isRightToLeft, init.isLocalized ? Language_currentIsRightToLeft() : init.isRightToLeft);
    if(caOpt) {
        stringglyphed_setChars(sg, caOpt);
        coq_free(caOpt);
    }
    return sg;
}
StringGlyphed* StringGlyphed_createCopy(StringGlyphed const*const src) {
    return coq_createArrayCopy(StringGlyphed, CharacterGlyphed, src->maxCount, src);
}
#define SG_charWidth_(w, spacing) fmaxf(0.5*w, w + spacing)
void stringglyphed_setChars(StringGlyphed*const sg, CharacterArray const*const ca) {
    size_t charCount = characterarray_count(ca);
    if(charCount > sg->maxCount) {
        printwarning("CharacterArray too long %zu, maxCount %zu.", charCount, sg->maxCount);
        charCount = sg->maxCount;
    }
    size_initConst(&sg->charCount, charCount);
    const float spacing = sg->spacing;
    // Mise à jours des glyphes.
    CharacterGlyphed* const cg_end = &sg->chars[charCount];
    const Character* c = characterarray_first(ca);
    float const direction = sg->isRightToLeft ? -1.f : 1.f;
    float x = 0;
    bool spacePunctAdded = false;
    bool nextIsNewWord = false;
    for(CharacterGlyphed* cg = sg->chars; cg < cg_end; c++, cg++) {
        GlyphInfo const info = glyphmap_glyphInfoOfChar(sg->glyphMap, *c);
        float const charSolidWidth = SG_charWidth_(info.relSolidWidth, spacing);
        x += 0.5*direction*charSolidWidth;
        *cg = (CharacterGlyphed) {
            .c = *c,
            .glyph = info,
            .xRel = x + info.relGlyphX,
            .firstOfWord = nextIsNewWord,
        };
        x += 0.5*direction*charSolidWidth;
        nextIsNewWord = false;
        // Return ajouter et finir le mot.
        if(character_isEndLine(*c)) {
            spacePunctAdded = false;
            nextIsNewWord = true;
            continue;
        }
        // Les espaces et ponctuations s'ajoute tant qu'il n'y a pas un nouveau mot...
        // (Règles pourrait changer...)
        if(character_isWordFinal(*c)) {
            spacePunctAdded = true;
            continue;
        }
        // Commence un nouveau mot ? (on a déjà des espaces/pounct en bout de mot)
        //  -> finir le précédent (et commencer le nouveau)
        if(spacePunctAdded) {
            cg->firstOfWord = true;
            spacePunctAdded = false;
        }
    }
    // Bout de la string.
    float_initConst(&sg->xEndRel, x);
//    float_initConst(&sg->widthRel, fabsf(x));
//    float_initConst(&sg->fullWidthRel, 2*sg->x_margin + fabsf(x));
}
void stringglyphed_removeLast(StringGlyphed*const sg) {
    if(sg->charCount < 1) { printwarning("Already empty."); return; }
    size_initConst(&sg->charCount, sg->charCount - 1);
    if(!sg->charCount) {
        float_initConst(&sg->xEndRel, 0);
        return;
    }
    // Mettre à jour la largeur totale
    CharacterGlyphed const*const last = &sg->chars[sg->charCount - 1];
    float const direction = sg->isRightToLeft ? -1.f : 1.f;
    float const charSolidWidth = SG_charWidth_(last->glyph.relSolidWidth, sg->spacing);
    float_initConst(&sg->xEndRel,
        last->xRel - last->glyph.relGlyphX + 0.5*direction*charSolidWidth);
}
void stringglyphed_addCharacter(StringGlyphed*const sg, Character const newChar) {
    if(sg->charCount >= sg->maxCount) { printwarning("Already full."); return; }
    size_initConst(&sg->charCount, sg->charCount + 1);
    CharacterGlyphed*const cgNew = &sg->chars[sg->charCount - 1];
    GlyphInfo const glyph = glyphmap_glyphInfoOfChar(sg->glyphMap, newChar);
    float const direction = sg->isRightToLeft ? -1.f : 1.f;
    float const charSolidWidth = SG_charWidth_(glyph.relSolidWidth, sg->spacing);
    // Cas premier char.
    if(sg->charCount == 1) { 
        *cgNew = (CharacterGlyphed) {
            .c = newChar,
            .glyph = glyph,
            .xRel = 0.5*direction*charSolidWidth + glyph.relGlyphX,
            .firstOfWord = true,
        };
        float_initConst(&sg->xEndRel, direction*charSolidWidth);
        return;
    }
    // Cas ajout en bout de chaine
    *cgNew = (CharacterGlyphed) {
        .c = newChar,
        .glyph = glyph,
        .xRel = sg->xEndRel + 0.5*direction*charSolidWidth + glyph.relGlyphX,
        .firstOfWord = true,
    };
    float_initConst(&sg->xEndRel, sg->xEndRel + direction*charSolidWidth);
}

StringGlyphedToDraw stringglyphed_getToDraw(StringGlyphed const* sg) {
    return (StringGlyphedToDraw) {
        .c =    sg->chars,
        .beg =  sg->chars,
        .end = &sg->chars[sg->charCount],
        .xEndRel = sg->xEndRel,
        .x_margin = sg->x_margin,
    };
}
Texture* stringglyphed_glyphMapTexture(StringGlyphed*const sg) {
    return glyphmap_texture(sg->glyphMap);
}
size_t stringglyphed_maxCount(StringGlyphed const*const sg) {
    return sg->maxCount;
}
size_t stringglyphed_charCount(StringGlyphed const*const sg) {
    return sg->charCount;
}
#define      NS_BUFFER_SIZE_ 1024
static char  NodeString_buffer_[NS_BUFFER_SIZE_] = {};
// TODO: tester...
char const* stringglyphed_getString(StringGlyphed const*const sg) {
    char* dst =      NodeString_buffer_;
    char* dst_end = &NodeString_buffer_[NS_BUFFER_SIZE_ - sizeof(Character)];
    CharacterGlyphed const*const c_end = &sg->chars[sg->charCount]; 
    memset(NodeString_buffer_, 0, sizeof(NodeString_buffer_));
    for(CharacterGlyphed const* c = sg->chars; c < c_end; c++) {
        if(dst >= dst_end) {
            printwarning("String of StringGlyphed %p too big.", sg);
            break;
        }
        
        *(Character*)dst = c->c;
        size_t const charSize = character_size(c->c);
        dst += charSize;
    }
    return NodeString_buffer_;
}

// MARK: - Array de StringGlyphed.
#define StringGlyphArr_LineAdd_ 10

void            StringGlyphArr_setAndRealloc_(StringGlyphArr** const strArrRef, StringGlyphed* const str,
                                              uint32_t lineIndex) {
    // Pas encore init.
    if(!(*strArrRef)) {
        uint32_t newMaxCount = umaxu(StringGlyphArr_LineAdd_, lineIndex + 1);
        *strArrRef = coq_callocArray(StringGlyphArr, StringGlyphed*, newMaxCount);
        (*strArrRef)->_maxLineCount = newMaxCount;
    }
    // Resize
    else if(lineIndex >= (*strArrRef)->_maxLineCount) {
        uint32_t newMaxCount = umaxu((uint32_t)(*strArrRef)->_maxLineCount + StringGlyphArr_LineAdd_,
                                     lineIndex + 1);
        size_t newSize = coq_arrayTypeSize(StringGlyphArr, StringGlyphed*, newMaxCount);
        *strArrRef = coq_realloc(*strArrRef, newSize);
        for(size_t index = (*strArrRef)->_maxLineCount; index < newMaxCount; index++) {
            (*strArrRef)->strOpts[index] = NULL;
        }
        (*strArrRef)->_maxLineCount = newMaxCount;
    }
    // Changement de lineCount ?
    if(lineIndex >= (*strArrRef)->lineCount)
        (*strArrRef)->lineCount = lineIndex + 1;
    // Set
    (*strArrRef)->strOpts[lineIndex] = str;
}
StringGlyphed* StringGlyphed_createSubCopyOpt_(const StringGlyphed* const ref,
                                             size_t const beg, size_t const end,
                                             Character trailingSpace)
{
    if(beg >= end || end > ref->charCount) { printerror("Bad range."); return NULL; }
    size_t charCount = end - beg;
    size_t const size = coq_arrayTypeSize(StringGlyphed, CharacterGlyphed, charCount);
    size_t const charDim_size = charCount * sizeof(CharacterGlyphed);
    size_t const header_size = size - charDim_size;
    StringGlyphed* const new = coq_calloc(1, size);
    // Copier le "header"
    memcpy(new, ref, header_size);
    // Copier les data (charDims)
    memcpy(&new->chars[0], &ref->chars[beg], charDim_size);
    // Ajustements...
    size_initConst(&new->charCount, charCount);
    size_initConst(&new->maxCount, charCount);
    Character lastChar = new->chars[charCount - 1].c;
    if(character_isSpace(lastChar)) {
        if(trailingSpace.c_data8 == 0) {  // Enlever le space de la fin.
            charCount = charCount - 1;
            size_initConst(&new->charCount, charCount);
        }
        // Changer le space de la fin seulement si on veut autre chose qu'un espace.
        else if(!character_isSpace(trailingSpace)) {
            GlyphInfo const info = glyphmap_glyphInfoOfChar(ref->glyphMap, trailingSpace);
            new->chars[charCount - 1].c = trailingSpace;
            new->chars[charCount - 1].glyph = info;
        }
    }
    // Reseter les positions et largeur.
    float const spacing = new->spacing;
    CharacterGlyphed* c = new->chars;
    CharacterGlyphed* const c_end = &new->chars[charCount];
    float const direction = new->isRightToLeft ? -1.f : 1.f;
    float x = 0;
    for(; c < c_end; c++) {
        float const charSolidWidth = SG_charWidth_(c->glyph.relSolidWidth, spacing);
        x += 0.5*direction*charSolidWidth;
        c->xRel = x + c->glyph.relGlyphX;
        x += 0.5*direction*charSolidWidth;
    }
    float_initConst(&new->xEndRel, x);
    return new;
}
StringGlyphArr* StringGlyphArr_create(const StringGlyphed* const str, float lineWidth,
                                      Character const trailingSpace) {
    if(lineWidth < 2) { printwarning("Linewidth too small."); lineWidth = 2; }
    float const spacing = str->spacing;
    uint32_t currentLine = 0;
    StringGlyphArr* strArrOpt = NULL;
    // Largeur cumulative (sans marges)
    float width = 0;
    size_t lineFirst = 0; // Premier char de ligne courante..
    size_t lastWordEnd = 0; // Fin du dernier mot.
    float lastWordWidth = 0;
    const CharacterGlyphed* const chars = str->chars;
    bool lastIsReturn = false;
    for(size_t index = 0; index < str->charCount;
        width += SG_charWidth_(chars[index].glyph.relSolidWidth, spacing),
        lastIsReturn = character_isEndLine(chars[index].c),
        index ++)
    {
        if(!chars[index].firstOfWord && !lastIsReturn) continue;
        // Création d'une ligne (nouveau mot dépasse, mais au moins un mot)
        if(width >= lineWidth && lastWordEnd > lineFirst) {
            StringGlyphed* newLine = StringGlyphed_createSubCopyOpt_(str,
                                    lineFirst, lastWordEnd, trailingSpace);
            StringGlyphArr_setAndRealloc_(&strArrOpt, newLine, currentLine);
            currentLine++;
            lineFirst = lastWordEnd;
            width = width - lastWordWidth;
        }
        // Mise à jour des infos du dernier mot
        lastWordWidth = width;
        lastWordEnd = index;
        // Return ? -> toute suite une nouvelle ligne.
        if(!lastIsReturn) continue;
        StringGlyphed* newLine = StringGlyphed_createSubCopyOpt_(str,
                                lineFirst, lastWordEnd, trailingSpace);
        StringGlyphArr_setAndRealloc_(&strArrOpt, newLine, currentLine);
        currentLine++;
        lineFirst = lastWordEnd;
        width = 0;
        lastWordWidth = 0;
    }
    // Dernières ligne ?
    if(width >= lineWidth && lastWordEnd > lineFirst) {
        StringGlyphed* newLine = StringGlyphed_createSubCopyOpt_(str,
                                    lineFirst, lastWordEnd, trailingSpace);
        StringGlyphArr_setAndRealloc_(&strArrOpt, newLine, currentLine);
        currentLine++;
        lineFirst = lastWordEnd;
    }
    // Dernière ligne.
    StringGlyphed* newLine = StringGlyphed_createSubCopyOpt_(str,
                            lineFirst, str->charCount, trailingSpace);
    StringGlyphArr_setAndRealloc_(&strArrOpt, newLine, currentLine);

    return strArrOpt;
}
void stringglypharr_destroyAndNull(StringGlyphArr*const*const strArrRef) {
    guard_let(StringGlyphArr*, strArr, *strArrRef, printerror("Already null."),)
    *(StringGlyphArr**)strArrRef = NULL;
    StringGlyphed** const end = &strArr->strOpts[strArr->lineCount];
    for(StringGlyphed** strOptPtr =  strArr->strOpts; strOptPtr < end; strOptPtr++) {
        if(*strOptPtr) coq_free(*strOptPtr);
        *strOptPtr = NULL;
    }
    coq_free(strArr);
}

// Garbage
/*
GlyphSimple* GlyphSimpleArray_createFromStringGlyphed(GlyphSimpleArrayInit const init) 
{
    StringGlyphedToDraw sgd = stringglyphed_getToDraw(init.sg);
    float const scaleX = (init.scalesOpt.x == 0.f) ? 1.f : init.scalesOpt.x;
    float const scaleY = (init.scalesOpt.y == 0.f) ? 1.f : init.scalesOpt.y;
    // Point de départ du texte.
    float x0 = init.center.x;
    if(init.centered) // Décalage pour recentrer le texte.
        x0 -= 0.5*sgd.xEndRel * scaleX;
    
    size_t const glyphCount = sgd.end - sgd.c;
    GlyphSimple*const g_beg = coq_callocSimpleArray(glyphCount+1, GlyphSimple);
    GlyphSimple*const g_end = &g_beg[glyphCount];
    GlyphSimple*      g     =  g_beg;
    // Boucle de calcul des positions/dimensions des glyphs.
    for(; g < g_end; sgd.c++, g++) {
        g->b = (Box) {
            .c_x = x0 +           sgd.c->xRel           *scaleX,
            .c_y = init.center.y + sgd.c->glyph.relGlyphY*scaleY,
            .Dx = sgd.c->glyph.relGlyphWidth *scaleX,
            .Dy = sgd.c->glyph.relGlyphHeight*scaleY,
        };
        g->uvRect = sgd.c->glyph.uvRect;
    }
    return g_beg;
}
*/
