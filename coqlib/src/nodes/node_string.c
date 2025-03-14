//
//  node_string.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 17/6/24.
//
#include "node_string.h"
#include "../utils/util_base.h"
#include "../utils/util_language.h"

// MARK: - StringGlyphed : Une string avec info pour dessiner.
typedef struct StringGlyphed {
    size_t           charCount;
    size_t const     maxCount;
    GlyphMap        *glyphMap;     // La glyph map utilisée pour définir les dimensions des chars.
    float            widthRel;     // Largeur (relative à solid height)
    float            x_margin;     // Marge en x (relative à solid height)
    float            spacing;      // Le scaling voulu (espace entre les CharacterGlyphed).
    CharacterGlyphed chars[1];     // Les chars avec info de dimensions.
} StringGlyphed;
/// Fonction utile pour avoir un planché sur la largeur d'un `GlyphInfo`.
float SG_charWidth_(float width, float spacing) {
    return fmaxf(0.5*width, width + spacing);
}

void stringglyphed_setChars(StringGlyphed* const sg, const CharacterArray * const ca) {
    size_t charCount = characterarray_count(ca);
    if(charCount > sg->maxCount) {
        printwarning("CharacterArray too long %zu, maxCount %zu.", charCount, sg->maxCount);
        charCount = sg->maxCount;
    }
    sg->charCount = charCount;
    const float spacing = sg->spacing;
    // Mise à jours des glyphes.
    CharacterGlyphed* const cg_end = &sg->chars[charCount];
    const Character* c = characterarray_first(ca);
    float stringWidth = 0;
    bool spacePunctAdded = false;
    bool nextIsNewWord = false;
    for(CharacterGlyphed* cg = sg->chars; cg < cg_end; c++, cg++) {
        GlyphInfo const info = glyphmap_glyphInfoOfChar(sg->glyphMap, *c);
        cg->c = *c;
        cg->info = info;
        cg->firstOfWord = nextIsNewWord;
        stringWidth += SG_charWidth_(info.relSolidWidth, spacing);
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
    // Largeur totale
    sg->widthRel = stringWidth;
}

StringGlyphed* StringGlyphed_createEmpty(size_t const maxCount, GlyphMap * const glyphMapOpt,
                                        float const spacing, float const x_margin)
{
    StringGlyphed* sg = coq_callocArray(StringGlyphed, CharacterGlyphed, maxCount);
    sg->charCount = 0;
    size_initConst(&sg->maxCount, maxCount);
    sg->glyphMap = glyphMapOpt ? glyphMapOpt : GlyphMap_default();
    sg->x_margin = x_margin;
    sg->spacing = spacing;
    return sg;
}

StringGlyphed* StringGlyphed_create(const CharacterArray * const ca, GlyphMap * const glyphMapOpt,
                                    float const spacing, float const x_margin)
{
    size_t charCount = characterarray_count(ca);
    StringGlyphed* sg = coq_callocArray(StringGlyphed, CharacterGlyphed, charCount);
    sg->charCount = charCount;
    size_initConst(&sg->maxCount, charCount);
    sg->glyphMap = glyphMapOpt ? glyphMapOpt : GlyphMap_default();
    sg->x_margin = x_margin;
    sg->spacing = spacing;

    stringglyphed_setChars(sg, ca);

    return sg;
}
StringGlyphed* StringGlyphed_createCopy_(const StringGlyphed* const ref) {
    size_t size = coq_arrayTypeSize(StringGlyphed, CharacterGlyphed, ref->maxCount);
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
    new->charCount = charCount;
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
    new->widthRel = width;
    return new;
}

// MARK: - Drawable d'un seul caractère. (pour test, utiliser plutôt NodeString) -
void drawablechar_renderer_updateIU_(Node* const n) {
    DrawableChar* dc = (DrawableChar*)n;
    float show = drawable_updateShow(&dc->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    if((dc->n.flags & flag_drawablePoping) == 0)
        show = 1.f;
    Matrix4* m = &n->renderIU.model;
    
    float const pos_x = n->x + dc->glyphX;
    float const pos_y = n->y + dc->glyphY;
    m->v0.v = pm->v0.v * n->sx * show;
    m->v1.v = pm->v1.v * n->sy * show;
    m->v2.v = pm->v2.v * n->sz * show;
    m->v3.v = pm->v3.v + pm->v0.v*pos_x + pm->v1.v*pos_y + pm->v2.v*n->z;
}
DrawableChar*  DrawableChar_create(Node* refOpt, Character c,
                               float x, float y, float twoDy, flag_t flags) {
    DrawableChar* dc = coq_callocTyped(DrawableChar);
    node_init(&dc->n, refOpt, x, y, 1, 1, flags, 0);
    GlyphMap* fgm = GlyphMap_default();
    drawable_init(&dc->d, glyphmap_texture(fgm), Mesh_drawable_sprite, 0, twoDy);
    dc->n.renderer_updateInstanceUniforms = drawablechar_renderer_updateIU_;
    // Info du glyph.
    GlyphInfo const glyph = glyphmap_glyphInfoOfChar(fgm, c);
    CoqFont const*const cf = glyphmap_font(fgm);
    CoqFontDims const fd = coqfont_dims(cf); 
    dc->glyphX = glyph.relGlyphX * twoDy;
    dc->glyphY = fd.relDeltaY * twoDy;
    dc->glyphWidth = glyph.relGlyphWidth * twoDy;
    dc->glyphHeight = fd.relFullHeihgt * twoDy;
    dc->solidWidth = glyph.relSolidWidth * twoDy;
    dc->solidHeight = twoDy;
    // Setter le drawable pour les dimensions du glyph.
    dc->n.renderIU.uvRect = glyph.uvRect;
    dc->n.sy = dc->glyphHeight; // Sprite un peu plus gros.
    dc->n.h = 1.f / fd.relFullHeihgt;  // h/w pour hitbox sans fioritures.
    dc->n.sx = dc->glyphWidth;
    dc->n.w = dc->solidWidth / dc->glyphWidth;

    return dc;
}

Drawable *Drawable_createCharacter(Node *const refOpt, Character const c, GlyphMap *const glyphMapOpt,
                               float const x, float const y, float const twoDy, flag_t const flags) {
    Drawable *d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    GlyphMap *const fgm = glyphMapOpt ? glyphMapOpt : GlyphMap_default();
    drawable_init(d, glyphmap_texture(fgm), Mesh_drawable_sprite, 0, twoDy);
    // Glyph...
    GlyphInfo const glyph = glyphmap_glyphInfoOfChar(fgm, c);
    CoqFontDims const fd = coqfont_dims(glyphmap_font(fgm));
    d->n.renderIU.uvRect = glyph.uvRect;
    d->n.sy = twoDy * fd.relFullHeihgt;
    d->n.h  = 1.f / fd.relFullHeihgt;  // -> 2Dy = sy * h...
    d->n.sx = twoDy * glyph.relGlyphWidth;
    d->n.w = glyph.relSolidWidth / glyph.relGlyphWidth; // -> 2Dx = 2Dx*relSolidW = sx * w.

    return d;
}
void  drawable_changeToCharacter(Drawable *const d, Character const c, GlyphMap *const glyphMapOpt) {
    GlyphMap *const fgm = glyphMapOpt ? glyphMapOpt : GlyphMap_default();
    textureref2_releaseAndNull(&d->texr);
    textureref2_init(&d->texr, glyphmap_texture(fgm));

    GlyphInfo const glyph = glyphmap_glyphInfoOfChar(fgm, c);
    CoqFontDims const fd = coqfont_dims(glyphmap_font(fgm));
    float const twoDy = d->n.sy * d->n.h;
    d->n.renderIU.uvRect = glyph.uvRect;
    d->n.sy = twoDy * fd.relFullHeihgt;
    d->n.h  = 1.f / fd.relFullHeihgt;  // -> 2Dy = sy * h...
    d->n.sx = twoDy * glyph.relGlyphWidth;
    d->n.w = glyph.relSolidWidth / glyph.relGlyphWidth; // -> 2Dx = 2Dy*relSolidW = sx * w.
}
Drawable*  Drawable_test_createString(Node *const refOpt, const char *const c_str, CoqFont const*const cf,
                                float x, float y, float twoDy, flag_t flags) {
    Drawable *d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    CoqFontDims const fd = coqfont_dims(cf);
    PixelBGRAArray *pa = Pixels_engine_test_createArrayFromString_(c_str, cf);
    Texture* tex =  Texture_createWithPixels(pa->pixels, (uint32_t)pa->width, (uint32_t)pa->height, false, fd.nearest);
    drawable_init(d, tex, Mesh_drawable_sprite, 0, twoDy);
    d->n.sy = twoDy * fd.relFullHeihgt;
    d->n.h  = 1.f / fd.relFullHeihgt;  // -> 2Dy = sy * h...
    d->n.sx = twoDy * fd.relFullHeihgt * (float)pa->width / (float)pa->height;
    d->n.w = pa->solidWidth / (float)pa->width; // -> 2Dx = 2Dy*relSolidW = sx * w.

    return d;
}

// MARK: - NodeString
CharacterArray* CharacterArray_create_(const char* c_str, bool isLocalized) {
    if(isLocalized) {
        const char* localized = String_createLocalized(c_str);
        CharacterArray* ca = CharacterArray_createFromString(localized);
        coq_free((char*)localized);
        return ca;
    }
    return CharacterArray_createFromString(c_str);
}

void nodestring_open_(Node* n) {
    NodeString* ns = (NodeString*)n;
    float_initConst(&ns->openTimeSec, (float)ChronosEvent.render_elapsedMS * SEC_PER_MS);
}
void nodestring_deinit_(Node* n) {
    NodeString* ns = (NodeString*)n;
    if(ns->sg) {
        coq_free((void*)ns->sg);
        *(StringGlyphed**)&ns->sg = (StringGlyphed*)NULL;
    }
    // Super
    drawablemulti_deinit(n);
}
void nodestring_renderer_updateIUs_(Node* const n) {
    NodeString* ns = (NodeString*)n;
    float const show = drawable_updateShow(&ns->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    StringGlyphed const*const sg = ns->sg; 
    CoqFontDims const fd = coqfont_dims(glyphmap_font(sg->glyphMap));
    float const pos_y = n->y + fd.relDeltaY * n->sy;
    float const scaleY = fd.relFullHeihgt * n->sy;
    float pos_x = n->x - 0.5 * ns->sg->widthRel * n->sx;
    IUsToEdit iusEdit = iusbuffer_rendering_retainIUsToEdit(ns->dm.iusBuffer);
    CharacterGlyphed const* cg = ns->sg->chars;
    CharacterGlyphed const*const cg_end = &ns->sg->chars[ns->sg->charCount];
    for(; (iusEdit.iu < iusEdit.end) && (cg < cg_end); iusEdit.iu++, cg++) {
        iusEdit.iu->show = show;
        iusEdit.iu->uvRect = cg->info.uvRect;
        float const pop = (n->flags & flag_drawablePoping) ? show : 1.f;
        pos_x += 0.5*SG_charWidth_(cg->info.relSolidWidth, sg->spacing) * n->sx;
        // Ajustement en x pour certains glyphs... (e.g. le j en lettres attachées)
        float const pos_x2 = pos_x + cg->info.relGlyphX*n->sx;
        Matrix4* m = &iusEdit.iu->model;
        m->v0.v = pm->v0.v * cg->info.relGlyphWidth * n->sx * pop;
        m->v1.v = pm->v1.v * scaleY * pop;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos_x2 + pm->v1.x * pos_y,
            pm->v3.y + pm->v0.y * pos_x2 + pm->v1.y * pos_y,
            pm->v3.z + pm->v0.z * pos_x2 + pm->v1.z * pos_y,
            pm->v3.w,
        }};
        pos_x += 0.5* SG_charWidth_(cg->info.relSolidWidth, sg->spacing) * n->sx;
    }
    iustoedit_release(iusEdit);
}
void nodestring_renderer_updateIUsMoving(Node* const n) {
    NodeString* ns = (NodeString*)n;
    float const show = drawable_updateShow(&ns->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    StringGlyphed const*const sg = ns->sg; 
    CoqFontDims const fd = coqfont_dims(glyphmap_font(sg->glyphMap));
    
    float const deltaT = -ns->n.nodrawData.float0;
    float const pos_y = n->y + fd.relDeltaY * n->sy;
    float const scaleY = fd.relFullHeihgt * n->sy;
    float const elapsedSecAngle = ChronoRender_elapsedAngleSec();
    float const elapsed = ChronoRender_elapsedSec() - ns->openTimeSec + deltaT;
    float pos_x = n->x - 0.5 * ns->sg->widthRel * n->sx;
    IUsToEdit iusEdit = iusbuffer_rendering_retainIUsToEdit(ns->dm.iusBuffer);
    CharacterGlyphed const* cg = ns->sg->chars; float index = 0;
    CharacterGlyphed const*const cg_end = &ns->sg->chars[ns->sg->charCount];
    for(; (iusEdit.iu < iusEdit.end) && (cg < cg_end); iusEdit.iu++, cg++, index++) {
        iusEdit.iu->show = fminf(float_appearing(elapsed, 0.1*index, 0.2), show);
        iusEdit.iu->uvRect = cg->info.uvRect;
        float const pop = (n->flags & flag_drawablePoping) ? show : 1.f;
        pos_x += 0.5*SG_charWidth_(cg->info.relSolidWidth, sg->spacing) * n->sx;
        
        float const glyphPosX = pos_x + cg->info.relGlyphX * n->sx + 0.005*sin(20*pos_x + 6*elapsedSecAngle);
        float const glyphPosY = pos_y + 0.005*sin(20*pos_x + (10+5*pos_x)*elapsedSecAngle);
        Matrix4* m = &iusEdit.iu->model;
        m->v0.v = pm->v0.v * cg->info.relGlyphWidth * n->sx * pop;
        m->v1.v = pm->v1.v * scaleY * pop;
        float theta = 0.1*sin(20*pos_x + deltaT + ns->n.nodrawData.float1*elapsedSecAngle);
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
        pos_x += 0.5* SG_charWidth_(cg->info.relSolidWidth, sg->spacing) * n->sx;
    }
    iustoedit_release(iusEdit);
}

void nodestring_checkDimensions_(NodeString* const ns) {
    const StringGlyphed * const sg = ns->sg;
    // Largeur / DeltaX
    float const strWidth = sg->widthRel;
    float const twoDxOpt = ns->twoDxOpt;
    float const twoDy = ns->n.sy;
    float twoMargX = 2*sg->x_margin;
    // Compression pour rentrer dans la largeur objectif.
    if(twoDxOpt && ((strWidth + twoMargX) * twoDy > twoDxOpt)) {
        float const scaleM = fminf(twoDy, 0.7*twoDxOpt / twoMargX);
        ns->n.sx = (twoDxOpt - twoMargX * scaleM) / strWidth;
        ns->n.w = twoDxOpt / ns->n.sx;
    }
    // Sinon, scaling normal.
    else {
        ns->n.sx = twoDy;
        ns->n.w = strWidth + twoMargX;
    }
    // Ajustement de taille d'un frame
    Node* const bigBro = ns->n._bigBro;
    if(bigBro && (ns->n.flags & flag_giveSizeToBigbroFrame))
        node_tryUpdatingAsFrameOfBro(bigBro, &ns->n);
}

void nodestring_updateString(NodeString *const ns, const char *const newString) {
    with_beg(CharacterArray const, ca, CharacterArray_createFromString(newString))
    stringglyphed_setChars(ns->sg, ca);
    with_end(ca)
    nodestring_checkDimensions_(ns);
}

void nodestring_and_super_init_(NodeString* ns, Node* refOpt,
                    StringGlyphed ** const sgRefGiven,
                    float x, float y, float twoDxOpt, float twoDy,
                    flag_t flags, uint8_t node_place)
{
    // Super inits...
    node_init(&ns->n, refOpt, x, y, 1, 1, flags, node_place);
    StringGlyphed *const sg = *sgRefGiven;
    *sgRefGiven = NULL;
    if(sg == NULL) { printerror("No StringGlyphed."); return; }
    drawable_init(&ns->d, glyphmap_texture(sg->glyphMap), Mesh_drawable_sprite, 0, twoDy);
    // └>Set w = 1, h = 1, sy = twoDy.
    drawablemulti_init(&ns->dm, umaxu((uint32_t)sg->maxCount, 2), &InstanceUniforms_default);

    // Init as NodeString...
    ns->n._type |= node_type_string;
    *(StringGlyphed**)&ns->sg = sg;
    ns->n.openOpt =     nodestring_open_;
    ns->n.deinitOpt =   nodestring_deinit_;
    ns->n.renderer_updateInstanceUniforms = nodestring_renderer_updateIUs_;
    smoothflag_setDeltaT(&ns->d.trShow, 200);
    float_initConst(&ns->twoDxOpt, twoDxOpt);

    nodestring_checkDimensions_(ns);
}

NodeString* NodeString_create(Node* ref, NodeStringInit const data,
                      float x, float y, float widthOpt, float height,
                      flag_t flags, uint8_t node_place) {
    NodeString* ns = coq_callocTyped(NodeString);
    CharacterArray* ca = CharacterArray_create_(data.c_str, data.isLocalized);
    StringGlyphed *sg;
    if(data.maxCountOpt) {
        sg = StringGlyphed_createEmpty(data.maxCountOpt, data.glyphMapOpt, data.spacing, data.x_margin);
        size_t const charCount = characterarray_count(ca);
        if(charCount) stringglyphed_setChars(sg, ca);
    } else {
        sg = StringGlyphed_create(ca, data.glyphMapOpt, data.spacing, data.x_margin);
    }
    coq_free(ca);
    nodestring_and_super_init_(ns, ref, &sg, x, y, widthOpt, height, flags, node_place);
    return ns;
}

NodeString* node_asNodeStringOpt(Node* const nOpt) {
    if(!nOpt) return NULL;
    if(nOpt->_type & node_type_string) return (NodeString*) nOpt;
    return NULL;
}

NodeString* NodeString_createWithStringGlyph_(Node* ref, const StringGlyphed* const strRef,
                      float x, float y, float twoDxOpt, float twoDy,
                      flag_t flags, uint8_t node_place) {
    NodeString* ns = coq_callocTyped(NodeString);
    StringGlyphed *sg = StringGlyphed_createCopy_(strRef);
    nodestring_and_super_init_(ns, ref, &sg, x, y, twoDxOpt, twoDy, flags, node_place);
    return ns;
}


// MARK: - Array de StringGlyphed.
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
    lineWidth -= 2*str->x_margin;
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


// MARK: - Multi String
void  node_addMultiStrings(Node* n, NodeStringInit const data,
                           float const lineWidth, float const lineHeight,
                           uint32_t relativeFlags, Character trailingSpace) {
    StringGlyphArr *lines;
    {
        CharacterArray* ca = CharacterArray_create_(data.c_str, data.isLocalized);
        StringGlyphed* str = StringGlyphed_create(ca, data.glyphMapOpt, data.spacing, data.x_margin);
        coq_free(ca);
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
        ns->n.nodrawData.float0 = -0.5*(float)index;
        node_setXYrelatively(&ns->n, relativeFlags, true);
    }
    stringglypharr_deinit(lines);
    coq_free(lines);
}


// MARK: - Multi String scrollable

void slidingmenu_addMultiStrings(SlidingMenu* sm, NodeStringInit const data) {
    float const width = slidingmenu_itemRelativeWidth(sm);
    StringGlyphArr *lines;
    {
        CharacterArray* ca = CharacterArray_create_(data.c_str, data.isLocalized);
        StringGlyphed* str = StringGlyphed_create(ca, data.glyphMapOpt, data.spacing, data.x_margin);
        coq_free(ca);
        lines = StringGlyphArr_create(str, width, spchar_null);
        coq_free(str);
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


// GARBAGE
//    CharacterGlyphed* charDim = sg->chars;
//    Character const * character =      characterarray_first(ca);
//    Character const * const char_end = characterarray_end(ca);
//    float stringWidth = 0;
//    bool spacePunctAdded = false;
//    while(character < char_end) {
//        GlyphInfo const info = glyphmap_getGlyphInfoOfChar(sg->glyphMap, *character);
//        charDim->c = *character;
//        charDim->info = info;
//        stringWidth += SG_charWidth_(info.relSolidWidth, spacing);
//        // Mot
//        bool nextIsNewWord = false;
//        // Return ajouter et finir le mot.
//        if(character_isEndLine(*character)) {
//            spacePunctAdded = false;
//            nextIsNewWord = true;
//            goto go_next_char;
//        }
//        // Les espaces et ponctuations s'ajoute tant qu'il n'y a pas un nouveau mot...
//        // (Règles pourrait changer...)
//        if(character_isWordFinal(*character)) {
//            spacePunctAdded = true;
//            goto go_next_char;
//        }
//        // Commence un nouveau mot ? (on a déjà des espaces/pounct en bout de mot)
//        //  -> finir le précédent (et commencer le nouveau)
//        if(spacePunctAdded) {
//            charDim->firstOfWord = true;
//            spacePunctAdded = false;
//        }
//        // Next char
//    go_next_char:
////        printdebug("char %s, new Word  %s.", character->c_str, bool_toString(charDim->firstOfWord));
//        character++;
//        charDim++;
//        if(nextIsNewWord && character < char_end)
//            charDim->firstOfWord = true;
//    }
//    // Largeur totale
//    sg->widthRel = stringWidth;
