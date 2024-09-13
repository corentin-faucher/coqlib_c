//
//  node_string.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 17/6/24.
//

#include "node_string.h"
#include "utils/util_base.h"
#include "utils/util_language.h"
#include "node_fluid.h"
#include "node_tree.h"


#pragma mark - StringGlyphed : Une string avec info pour dessiner.
typedef struct StringGlyphed {
    size_t const     charCount;
// Dimensions relatives de la string (sans scaling)
// o_y est le shift relatif en y. o_x est la marge en x (x_margin).
// w n'inclus pas les marges (2*x_margin), mais inclus le spacing.
    Rectangle        relGlyphDims; 
    float            spacing;      // Le scaling voulu (espace entre les CharacterGlyphed).
    FontGlyphMap*    glyphMap;     // La glyph map utilisée pour définir les dimensions des chars.
    CharacterGlyphed chars[1];     // Les chars avec info de dimensions.
} StringGlyphed;
float SG_charWidth_(float width, float spacing) {
    return fmaxf(0.5*width, width + spacing);
}
/// Glyph map par défaut. (font du system avec resolution moyenne)
static FontGlyphMap* StringGlyphed_defaultGlyphMap_ = NULL;
FontGlyphMap* StringGlyphed_getDefaultGlyphMap_(void) {
    if(!StringGlyphed_defaultGlyphMap_) {
        printwarning("Default font glyph map not init. Setting default.");
        StringGlyphed_defaultGlyphMap_ = FontGlyphMap_create(NULL, 40, 512, false, NULL);
    }
    return StringGlyphed_defaultGlyphMap_;
}

void        StringGlyphed_giveDefaultFontGlyphMap(FontGlyphMap** const defaultFontGlyphMapGivenRef)
{
    if(StringGlyphed_defaultGlyphMap_) {
        printwarning("Default glyph map already init.");
        return;
    }
    FontGlyphMap* const fontGlyphMap = *defaultFontGlyphMapGivenRef;
    *defaultFontGlyphMapGivenRef = NULL;
    if(!fontGlyphMap) {
        printwarning("No given glyph map.");
        return;
    }
    StringGlyphed_defaultGlyphMap_ = fontGlyphMap;
}
void        StringGlyphed_deinit(void) {
    if(StringGlyphed_defaultGlyphMap_) return;
    fontglyphmap_deinit(StringGlyphed_defaultGlyphMap_);
    free(StringGlyphed_defaultGlyphMap_);
    StringGlyphed_defaultGlyphMap_ = NULL;
}
Texture*    StringGlyphed_defaultGlyphMapTexture(void) {
    FontGlyphMap* fgm = StringGlyphed_getDefaultGlyphMap_();
    return fontglyphmap_getTexture(fgm);
}

StringGlyphed* StringGlyphed_create(StringGlyphedInit const data) {
    // Map de glyphs
    FontGlyphMap* glyphMap;
    if(data.glyphMapOpt) {
        glyphMap = data.glyphMapOpt;
    } else {
        glyphMap = StringGlyphed_getDefaultGlyphMap_();
    }
    // Array de char
    CharacterArray const * caOwn;
    if(data.isLocalized) {
        const char* localized = String_createLocalized(data.c_str);
        caOwn = CharacterArray_createFromString(localized);
        coq_free((char*)localized);
    } else {
        caOwn = CharacterArray_createFromString(data.c_str);
    }
    size_t charCount = characterarray_count(caOwn);
    StringGlyphed* str = coq_callocArray(StringGlyphed, CharacterGlyphed, charCount);
    size_initConst(&str->charCount, charCount);
    str->glyphMap = glyphMap;
    // Init des dimensions des chars avec la GlyphMap
    str->relGlyphDims.h = fontglyphmap_getRelHeight(glyphMap);
    str->relGlyphDims.o_y = fontglyphmap_getRelY(glyphMap);
    str->relGlyphDims.o_x = data.x_margin;
    str->spacing = data.spacing;
    CharacterGlyphed* charDim = str->chars;
    Character const * character =      characterarray_first(caOwn);
    Character const * const char_end = characterarray_end(caOwn);
    float stringWidth = 0;
    bool spacePunctAdded = false;
    while(character < char_end) {
        GlyphInfo const info = fontglyphmap_getGlyphInfoOfChar(glyphMap, *character);
        charDim->c = *character;
        charDim->info = info;
        stringWidth += SG_charWidth_(info.relSolidWidth, data.spacing);
        // Mot
        bool nextIsNewWord = false;
        // Return ajouter et finir le mot.
        if(character_isEndLine(*character)) {
            spacePunctAdded = false;
            nextIsNewWord = true;
            goto go_next_char;
        }
        // Les espaces et ponctuations s'ajoute tant qu'il n'y a pas un nouveau mot...
        // (Règles pourrait changer...)
        if(character_isWordFinal(*character)) {
            spacePunctAdded = true;
            goto go_next_char;
        }
        // Commence un nouveau mot ? (on a déjà des espaces/pounct en bout de mot)
        //  -> finir le précédent (et commencer le nouveau)
        if(spacePunctAdded) {
            charDim->firstOfWord = true;
            spacePunctAdded = false;
        }
        // Next char
    go_next_char:
//        printdebug("char %s, new Word  %s.", character->c_str, bool_toString(charDim->firstOfWord));
        character++;
        charDim++;
        if(nextIsNewWord && character < char_end)
            charDim->firstOfWord = true;
    }
    // Largeur totale
    str->relGlyphDims.w = stringWidth;
    // Fini, libérer l'array de char.
    coq_free((void*)caOwn);
    
    return str;
}

StringGlyphed* StringGlyphed_createCopy_(const StringGlyphed* const ref) {
    size_t size = coq_arrayTypeSize(StringGlyphed, CharacterGlyphed, ref->charCount);
    StringGlyphed* new = coq_calloc(1, size);
    memcpy(new, ref, size);
    return new;
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
    Character lastChar = new->chars[charCount - 1].c;
    if(character_isSpace(lastChar)) {
        if(trailingSpace.c_data8 == 0) {  // Enlever le space de la fin.
            charCount = charCount - 1;
            size_initConst(&new->charCount, charCount);
        }
        // Changer le space de la fin seulement si on veut autre chose qu'un espace. 
        else if(!character_isSpace(trailingSpace)) { 
            GlyphInfo const info = fontglyphmap_getGlyphInfoOfChar(ref->glyphMap, trailingSpace);
            new->chars[charCount - 1].c = trailingSpace;
            new->chars[charCount - 1].info = info;
        }
    }
    // Recompter la largeur.
    float const spacing = new->spacing;
    float width = 0;
    const CharacterGlyphed* c = new->chars;
    const CharacterGlyphed* const c_end = &new->chars[charCount];
    for(; c < c_end; c++) {
        width += SG_charWidth_(c->info.relSolidWidth, spacing);
    }
    new->relGlyphDims.w = width;
    return new;
} 

#pragma mark - Drawable d'un seul caractère. (pour test, utiliser plutôt NodeString) -
void drawablechar_updateModel_(Node* const n) {
    DrawableChar* dc = (DrawableChar*)n;
    float show = smtrans_setAndGetValue(&dc->d.trShow, (dc->n.flags & flag_show) != 0);
    // Rien à afficher...
    if(show < 0.001f) {
        n->flags &= ~flag_drawableActive;
        return;
    }
    n->flags |= flag_drawableActive; 
    const Matrix4* const pm = node_parentModel(n);
    dc->n._iu.show = show;
    if((dc->n.flags & flag_poping) == 0)
        show = 1.f;
    Matrix4* m = &n->model; // même chose que d->n._iu.model...
    float const pos_x = n->x + dc->glyphX;
    float const pos_y = n->y + dc->glyphY;
    float const pos_z = n->z;
    Vector2 scl = node_scales(n);
    m->v0.v = pm->v0.v * scl.x * show;
    m->v1.v = pm->v1.v * scl.y * show;
    m->v2 =   pm->v2;  // *scl.z ... si on veut un scaling en z...?
    m->v3 = (Vector4) {{
        pm->v3.x + pm->v0.x * pos_x + pm->v1.x * pos_y + pm->v2.x * pos_z,
        pm->v3.y + pm->v0.y * pos_x + pm->v1.y * pos_y + pm->v2.y * pos_z,
        pm->v3.z + pm->v0.z * pos_x + pm->v1.z * pos_y + pm->v2.z * pos_z,
        pm->v3.w,
    }};
}
DrawableChar*  DrawableChar_create(Node* refOpt, Character c,
                               float x, float y, float twoDy, flag_t flags) {
    DrawableChar* dc = coq_callocTyped(DrawableChar);
    node_init(&dc->n, refOpt, x, y, 1, 1, node_type_n_drawable, flags, 0);
    FontGlyphMap* fgm = StringGlyphed_getDefaultGlyphMap_();
    drawable_init(&dc->d, fontglyphmap_getTexture(fgm), mesh_sprite, 0, twoDy);
    dc->n.updateModel = drawablechar_updateModel_;
    GlyphInfo const glyph = fontglyphmap_getGlyphInfoOfChar(fgm, c);
    drawable_setUVRect(&dc->d, glyph.uvRect, false);
    float const relHeight = fontglyphmap_getRelHeight(fgm);
    float const relY = fontglyphmap_getRelY(fgm);
    dc->glyphX = glyph.relGlyphX * twoDy;
    dc->glyphY = relY * twoDy;
    dc->glyphWidth = glyph.relGlyphWidth * twoDy;
    dc->glyphHeight = relHeight * twoDy;
    dc->solidWidth = glyph.relSolidWidth * twoDy;
    dc->solidHeight = twoDy;
    
    dc->n.sy = dc->glyphHeight; // Sprite un peu plus gros.
    dc->n.sx = dc->glyphWidth;
    dc->n.h = 1.f / relHeight;
    dc->n.w = dc->solidWidth / dc->glyphWidth;
    
    return dc;
}

#pragma mark - NodeString
void nodestring_open_(Node* n) {
    NodeString* ns = (NodeString*)n;
    chrono_start(&ns->chrono);
}
void nodestring_deinit_(Node* n) {
    NodeString* ns = (NodeString*)n;
    if(ns->_strGlyph) {
        coq_free(ns->_strGlyph);
        ns->_strGlyph = NULL;
    }
    // Super
    drawablemulti_deinit_(n);
}
void nodestring_updateModelsDefault_(Node* const n) {
    NodeString* ns = (NodeString*)n;
    float const show = smtrans_setAndGetValue(&ns->d.trShow, (n->flags & flag_show) != 0);
    if(show < 0.001f) {
        n->flags &= ~flag_drawableActive;
        return;
    }
    n->flags |= flag_drawableActive;
    const Matrix4* const pm = node_parentModel(n);
    
    Rectangle const relDims = nodestring_getRelativeBox(ns);
    float const spacing = nodestring_getSpacing(ns);
    float const pos_y = n->y + relDims.o_y * n->sy;
    float const scaleY = relDims.h * n->sy;
    float pos_x = n->x - 0.5*relDims.w*n->sx;
    InstanceUniforms* const end = &ns->dm.iusBuffer.ius[ns->dm.iusBuffer.actual_count];
    CharacterGlyphed* cg = nodestring_firstCharacterGlyphed(ns); float index = 0;
    for(InstanceUniforms* piu = ns->dm.iusBuffer.ius; piu < end; piu++, cg++, index++) {
        piu->show = show;
        float const pop = (n->flags & flag_poping) ? show : 1.f;
        pos_x += 0.5*SG_charWidth_(cg->info.relSolidWidth, spacing) * n->sx;
        // Ajustement en x pour certains glyphs... (e.g. le j en lettres attachées)
        float const pos_x2 = pos_x + cg->info.relGlyphX*n->sx;
        Matrix4* m = &piu->model;
        m->v0.v = pm->v0.v * cg->info.relGlyphWidth * n->sx * pop;
        m->v1.v = pm->v1.v * scaleY * pop;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos_x2 + pm->v1.x * pos_y,
            pm->v3.y + pm->v0.y * pos_x2 + pm->v1.y * pos_y,
            pm->v3.z + pm->v0.z * pos_x2 + pm->v1.z * pos_y,
            pm->v3.w,
        }};
        pos_x += 0.5* SG_charWidth_(cg->info.relSolidWidth, spacing) * n->sx;
    }
}
void nodestring_updateModelsMoving(Node* const n) {
    NodeString* ns = (NodeString*)n;
    float const show = smtrans_setAndGetValue(&ns->d.trShow, (n->flags & flag_show) != 0);
    if(show < 0.001f) {
        n->flags &= ~flag_drawableActive;
        return;
    }
    n->flags |= flag_drawableActive;
    const Matrix4* const pm = node_parentModel(n);
    
    Rectangle const relDims = nodestring_getRelativeBox(ns);
    float const spacing = nodestring_getSpacing(ns);
    float const deltaT = ns->n.float0;
    float const pos_y = n->y + relDims.o_y * n->sy;
    float const scaleY = relDims.h * n->sy;
    float const elapsed = chrono_elapsedSec(&ns->chrono) + deltaT;
    float pos_x = n->x - 0.5*relDims.w*n->sx;
    InstanceUniforms* const end = &ns->dm.iusBuffer.ius[ns->dm.iusBuffer.actual_count];
    CharacterGlyphed* cg = nodestring_firstCharacterGlyphed(ns); float index = 0;
    for(InstanceUniforms* piu = ns->dm.iusBuffer.ius; piu < end; piu++, cg++, index++) {
        piu->show = fminf(float_appearing(elapsed, 0.1*index, 0.2), show);
        float const pop = (n->flags & flag_poping) ? show : 1.f;
        pos_x += 0.5*SG_charWidth_(cg->info.relSolidWidth, spacing) * n->sx;
        float const glyphPosX = pos_x + cg->info.relGlyphX * n->sx + 0.005*sin(20*pos_x + 6*(float)CR_elapsedMS_/1000.f);
        float const glyphPosY = pos_y + 0.005*sin(20*pos_x + (10+5*pos_x)*(float)CR_elapsedMS_/1000.f);
        Matrix4* m = &piu->model;
        m->v0.v = pm->v0.v * cg->info.relGlyphWidth * n->sx * pop;
        m->v1.v = pm->v1.v * scaleY * pop;
        float theta = 0.1*sin(20*pos_x + deltaT + 10.f*(float)CR_elapsedMS_/1000.f);
        float c = cosf(theta);
        float s = sinf(theta);
        Vector4 v0 = m->v0;
        Vector4 v1 = m->v1;
        m->v0.v =  c*v0.v + s*v1.v;
        m->v1.v = -s*v0.v + c*v1.v;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * glyphPosX + pm->v1.x * glyphPosY,
            pm->v3.y + pm->v0.y * glyphPosX + pm->v1.y * glyphPosY,
            pm->v3.z + pm->v0.z * glyphPosX + pm->v1.z * glyphPosY,
            pm->v3.w,
        }};
        pos_x += 0.5* SG_charWidth_(cg->info.relSolidWidth, spacing) * n->sx;
    }
}
void nodestring_and_drawable_init_(NodeString* ns, float twoDxOpt, float twoDy) {
    if(!ns->_strGlyph) { 
        printerror("StringGlyphed not init.");
        ns->_strGlyph = StringGlyphed_create((StringGlyphedInit){.c_str = "Error"});
    }
    StringGlyphed* const sg = ns->_strGlyph;
    // Init as drawable
    drawable_init(&ns->d, fontglyphmap_getTexture(sg->glyphMap), mesh_sprite, 0, twoDy);
    // └>Set w = 1, h = 1, sy = twoDy.
    // Largeur / DeltaX
    float const strWidth = sg->relGlyphDims.w;
    float twoMargX = 2*sg->relGlyphDims.o_x;
    // Compression pour rentrer dans la largeur objectif.
    if(twoDxOpt && ((strWidth + twoMargX) * twoDy > twoDxOpt)) {
        float const scaleM = fminf(twoDy, 0.7*twoDxOpt / twoMargX);
        ns->n.sx = (twoDxOpt - twoMargX * scaleM) / strWidth;
        ns->n.w = twoDxOpt / ns->n.sx;
//        ns->n.sx = twoDxOpt / (strWidth + twoMargX);
    }
    // Sinon, scaling normal.
    else {
        ns->n.sx = twoDy;
        ns->n.w = strWidth + twoMargX;
    }
    // Init as drawable multi
    drawablemulti_init_(&ns->dm, (uint32_t)sg->charCount);
    ns->chrono.isRendering = true;
    ns->n.openOpt =     nodestring_open_;
    ns->n.deinitOpt =   nodestring_deinit_;
    ns->n.updateModel = nodestring_updateModelsDefault_;
    smtrans_setDeltaT(&ns->d.trShow, 200);
    // Init Instance uniforms (glyph)
    const CharacterGlyphed* const charDim_end = &sg->chars[sg->charCount];
    InstanceUniforms* iu = ns->dm.iusBuffer.ius;
    for(const CharacterGlyphed* charDim = sg->chars; charDim < charDim_end; charDim++, iu++) {
        *iu = InstanceUnifoms_default;
        iu->uvRect = charDim->info.uvRect;
    }
    // Ajustement de taille d'un frame
    Node* const bigBro = ns->n._bigBro;
    if(bigBro && (ns->n.flags & flag_giveSizeToBigbroFrame))
        node_tryUpdatingAsFrameOfBro(bigBro, &ns->n);
}
NodeString* NodeString_create(Node* ref, StringGlyphedInit const data,
                      float x, float y, float widthOpt, float height,
                      flag_t flags, uint8_t node_place) {
    NodeString* ns = coq_callocTyped(NodeString);
    node_init(&ns->n, ref, x, y, 1, 1, node_type_nd_multi, flags, node_place);
    ns->_strGlyph = StringGlyphed_create(data);
    nodestring_and_drawable_init_(ns, widthOpt, height);
    return ns; 
}
NodeString* NodeString_createWithStringGlyph_(Node* ref, const StringGlyphed* const strRef,
                      float x, float y, float twoDxOpt, float twoDy,
                      flag_t flags, uint8_t node_place) {
    NodeString* ns = coq_callocTyped(NodeString);
    node_init(&ns->n, ref, x, y, 1, 1, node_type_nd_multi, flags, node_place);
    ns->_strGlyph = StringGlyphed_createCopy_(strRef);
    nodestring_and_drawable_init_(ns, twoDxOpt, twoDy);
    return ns; 
}
/// Espace occupé par la string (relativement à height)
Rectangle         nodestring_getRelativeBox(NodeString* ns) {
    return ns->_strGlyph->relGlyphDims;
}
float             nodestring_getSpacing(NodeString* ns) {
    return ns->_strGlyph->spacing;
}
/// Accès à la liste de caractère avec dimensions de glyph
CharacterGlyphed* nodestring_firstCharacterGlyphed(NodeString* ns) {
    return ns->_strGlyph->chars;
}


#pragma mark - Array de StringGlyphed.
#define StringGlyphArr_LineAdd_ 10
typedef struct StringGlyphArr {
    size_t          lineCount;
    size_t          _maxLineCount;
    StringGlyphed*  strOpts[1]; // (Array de pointeurs)
} StringGlyphArr;

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
StringGlyphArr* StringGlyphArr_create(const StringGlyphed* const str, float lineWidth,
                                      Character const trailingSpace) {
    lineWidth -= 2*str->relGlyphDims.o_x;
    if(lineWidth < 2*str->relGlyphDims.h) {
        printwarning("Linewidth too small.");
        lineWidth = 2*str->relGlyphDims.h;
    }
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
        width += SG_charWidth_(chars[index].info.relSolidWidth, spacing), 
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
        width = width - lastWordWidth;
    }
    // Dernière ligne.
    StringGlyphed* newLine = StringGlyphed_createSubCopyOpt_(str, 
                            lineFirst, str->charCount, trailingSpace);
    StringGlyphArr_setAndRealloc_(&strArrOpt, newLine, currentLine);
    
    return strArrOpt;
}
void            stringglypharr_deinit(StringGlyphArr* strArr) {
    StringGlyphed** strOptRef =  strArr->strOpts;
    StringGlyphed** const end = &strArr->strOpts[strArr->lineCount];
    for(; strOptRef < end; strOptRef++) {
        if(*strOptRef) coq_free(*strOptRef);
        *strOptRef = NULL;
    }
}


#pragma mark - Multi String
void  node_addMultiStrings(Node* n, StringGlyphedInit const data,
                           float const lineWidth, float const lineHeight,
                           uint32_t relativeFlags, Character trailingSpace) {
    StringGlyphArr *lines;
    {
        StringGlyphed* str = StringGlyphed_create(data);
        lines = StringGlyphArr_create(str, lineWidth / lineHeight, trailingSpace);
        coq_free(str);
    }
    float y0 = 0.5*lineHeight*(float)(lines->lineCount - 1);
    n->w = lineWidth;
    n->h = lineHeight*(float)(lines->lineCount);
    for(uint32_t index = 0; index < lines->lineCount; index++) {
        if(!lines->strOpts[index]) { printwarning("Empty line."); continue; }
        NodeString* ns = NodeString_createWithStringGlyph_(n, lines->strOpts[index], 
                                    0, y0 - index * lineHeight, lineWidth, lineHeight, 0, 0);
        ns->n.float0 = -0.5*(float)index;
        node_setXYrelatively(&ns->n, relativeFlags, true);
    }
    stringglypharr_deinit(lines);
    coq_free(lines);
}


#pragma mark - Multi String scrollable

void slidingmenu_addMultiStrings(SlidingMenu* sm, StringGlyphedInit const data) {
    float const width = slidingmenu_itemRelativeWidth(sm);
    StringGlyphArr *lines;
    {
        const StringGlyphed* const str = StringGlyphed_create(data);
        lines = StringGlyphArr_create(str, width, spchar_null);
        coq_free((void*)str);
    }
    for(uint32_t index = 0; index < lines->lineCount; index++) {
        if(!lines->strOpts[index]) { printwarning("Empty line."); continue; }
        NodeString* ns = NodeString_createWithStringGlyph_(NULL, lines->strOpts[index], 
                                    0, 0, 0, 1, flag_noParent, 0);
        slidingmenu_addItem(sm, &ns->n);
    }
    stringglypharr_deinit(lines);
    coq_free(lines);
}
