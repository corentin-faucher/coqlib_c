//
//  node_string.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 17/6/24.
//
#include "node_string.h"
#include "../utils/util_base.h"
#include "../systems/system_language.h"

// MARK: - Drawable d'un seul caractère. (pour test, utiliser plutôt NodeString) -
void drawablechar_renderer_updateIU_(Node* const n) {
    DrawableChar* dc = (DrawableChar*)n;
    float show = drawable_updateShow(&dc->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    if((dc->n.flags & flag_drawablePoping) == 0)
        show = 1.f;
    Matrix4* m = &n->renderIU.model;
    
    float const pos_x = n->x + dc->glyph.relGlyphX * n->sx;
    float const pos_y = n->y + dc->glyph.relGlyphY * n->sy;
    m->v0.v = pm->v0.v * dc->glyph.relGlyphWidth * n->sx * show;
    m->v1.v = pm->v1.v * dc->glyph.relGlyphHeight * n->sy * show;
    m->v2.v = pm->v2.v * n->sz * show;
    m->v3.v = pm->v3.v + pm->v0.v*pos_x + pm->v1.v*pos_y + pm->v2.v*n->z;
}
DrawableChar* DrawableChar_create(Node* refOpt, Character c,
                               float x, float y, float twoDy, flag_t flags) {
    DrawableChar* dc = coq_callocTyped(DrawableChar);
    // Super init
    node_init(&dc->n, refOpt, x, y, 1, 1, flags, 0);
    GlyphMap*const gm = GlyphMap_default();
    drawable_init(&dc->d, glyphmap_texture(gm), Mesh_drawable_sprite, 0, twoDy);
    // └>Set w = 1, h = 1, sy = twoDy.
    dc->n.sx = twoDy;
    dc->n.renderer_updateInstanceUniforms = drawablechar_renderer_updateIU_;
    dc->glyph = glyphmap_glyphInfoOfChar(gm, c);
    // Setter le drawable pour les dimensions du glyph.
    dc->n.renderIU.uvRect = dc->glyph.uvRect;
    dc->n.w = dc->glyph.relSolidWidth; // twoDx = glyph.relSolidWidth * twoDy
 // dc->n.h = 1, i.e. dc->glyph.relSolidHeight...
    return dc;
}
Box             drawablechar_getGlyphBox(DrawableChar const*const dc) {
    return (Box) {
        .c_x = dc->n.x + dc->glyph.relGlyphX * dc->n.sx,
        .c_y = dc->n.y + dc->glyph.relGlyphY * dc->n.sy,
        .Dx = 0.5*dc->glyph.relGlyphWidth *  dc->n.sx,
        .Dy = 0.5*dc->glyph.relGlyphHeight * dc->n.sy,
    };
}

void            drawablechar_updateToChar(DrawableChar*const dc, Character const newChar) {
    GlyphMap*const gm = GlyphMap_default();
    dc->glyph = glyphmap_glyphInfoOfChar(gm, newChar);
    dc->n.renderIU.uvRect = dc->glyph.uvRect;
    dc->n.w = dc->glyph.relSolidWidth;
}
void           drawablechar_updateAsPngTile(DrawableChar*const dc, uint32_t pngId, uint32_t tileId) {
    
}

// MARK: - NodeString
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
    NodeString*const ns = (NodeString*)n;
    if(!ns->sg) { printerror("No string glyphed."); return; }
    StringGlyphedToDraw sg = stringglyphed_getToDraw(ns->sg);
    float const show = drawable_updateShow(&ns->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    // (A priori, sx == sy (hauteur ref), sauf si compression en x.)
    // Affichage "centré" (String va de 0 à x = xEndRel)
    float const x0 = n->x - 0.5*sg.xEndRel * n->sx;
    withIUsToEdit_beg(ius, &ns->dm.iusBuffer) 
    for(; (ius.iu < ius.end) && (sg.c < sg.end); ius.iu++, sg.c++) {
        ius.iu->show = show;
        ius.iu->uvRect = sg.c->glyph.uvRect;
        float const pop = (n->flags & flag_drawablePoping) ? show : 1.f;
        float const pos_x = x0 + sg.c->xRel * n->sx;
        float const pos_y = n->y + sg.c->glyph.relGlyphY*n->sy;
        Matrix4*const m = &ius.iu->model;
        m->v0.v = pm->v0.v * sg.c->glyph.relGlyphWidth * n->sx * pop;
        m->v1.v = pm->v1.v * sg.c->glyph.relGlyphHeight* n->sy * pop;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos_x + pm->v1.x * pos_y,
            pm->v3.y + pm->v0.y * pos_x + pm->v1.y * pos_y,
            pm->v3.z + pm->v0.z * pos_x + pm->v1.z * pos_y,
            pm->v3.w,
        }};
    }
    withIUsToEdit_end(ius)
}
void nodestring_renderer_updateIUsMoving(Node* const n) {
    NodeString* ns = (NodeString*)n;
    if(!ns->sg) { printerror("No string glyphed."); return; }
    StringGlyphedToDraw sg = stringglyphed_getToDraw(ns->sg);
    float const show = drawable_updateShow(&ns->d);
    if(!show) return;
    const Matrix4* const pm = node_parentModel(n);
    
    float const deltaT = -ns->n.nodrawData.data0.v.x;
    float const elapsedSecAngle = ChronoRender_elapsedAngleSec();
    float const elapsed = ChronoRender_elapsedSec() - ns->openTimeSec + deltaT;
    float const x0 = n->x - 0.5*sg.xEndRel * n->sx;
    withIUsToEdit_beg(iusEdit, &ns->dm.iusBuffer) 
    float index = 0;
    for(; (iusEdit.iu < iusEdit.end) && (sg.c < sg.end); iusEdit.iu++, sg.c++, index++) {
        iusEdit.iu->show = fminf(float_appearing(elapsed, 0.1*index, 0.2), show);
        iusEdit.iu->uvRect = sg.c->glyph.uvRect;
        float const pop = (n->flags & flag_drawablePoping) ? show : 1.f;
        float const pos_x = x0 + sg.c->xRel * n->sx + 0.005*sin(sg.c->xRel + 6*elapsedSecAngle);
        float const pos_y = n->y + sg.c->glyph.relGlyphY * n->sy + 0.005*sin(sg.c->xRel + (10+5*pos_x)*elapsedSecAngle);
        Matrix4*const m = &iusEdit.iu->model;
        float const theta = 0.1*sin(20*pos_x + deltaT + ns->n.nodrawData.data0.v.y*elapsedSecAngle);
        float const c = cosf(theta);
        float const s = sinf(theta);
        Vector4 v0, v1;
        v0.v = pm->v0.v * sg.c->glyph.relGlyphWidth * n->sx * pop;
        v1.v = pm->v1.v * sg.c->glyph.relGlyphHeight* n->sy * pop;
        m->v0.v =  c*v0.v + s*v1.v;
        m->v1.v = -s*v0.v + c*v1.v;
        m->v2 =   pm->v2;
        m->v3 = (Vector4) {{
            pm->v3.x + pm->v0.x * pos_x + pm->v1.x * pos_y,
            pm->v3.y + pm->v0.y * pos_x + pm->v1.y * pos_y,
            pm->v3.z + pm->v0.z * pos_x + pm->v1.z * pos_y,
            pm->v3.w,
        }};
    }
    withIUsToEdit_end(iusEdit)
}
void nodestring_checkDimensions_(NodeString* const ns) {
    StringGlyphedToDraw const sg = stringglyphed_getToDraw(ns->sg);
    // Largeur / DeltaX
    float const strWidth = fabsf(sg.xEndRel);
    float const twoDxOpt = ns->twoDxOpt;
    float const twoDy = ns->n.sy;
    float twoMargX = 2*sg.x_margin;
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
                    float const x, float const y, float const twoDxOpt, float const twoDy,
                    flag_t flags, uint8_t node_place)
{
    // Super inits...
    node_init(&ns->n, refOpt, x, y, 1, 1, flags, node_place);
    StringGlyphed *const sg = *sgRefGiven;
    *sgRefGiven = NULL;
    if(sg == NULL) { printerror("No StringGlyphed."); return; }
    drawable_init(&ns->d, stringglyphed_glyphMapTexture(sg), Mesh_drawable_sprite, 0, twoDy);
    // └>Set w = 1, h = 1, sy = twoDy.
    InstanceUniforms iu = InstanceUniforms_default;
    drawablemulti_init(&ns->dm, umaxu((uint32_t)stringglyphed_maxCount(sg), 2), &iu);
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

NodeString* NodeString_create(Node* ref, StringGlyphedInit const data,
                      float x, float y, float widthOpt, float height,
                      flag_t flags, uint8_t node_place) {
    NodeString* ns = coq_callocTyped(NodeString);
    StringGlyphed *sgToGive = StringGlyphed_create(data);
    nodestring_and_super_init_(ns, ref, &sgToGive,
                               x, y, widthOpt, height, flags, node_place);
    return ns;
}

NodeString* NodeString_createWithStringGlyph_(Node* ref, const StringGlyphed* const strRef,
                      float x, float y, float twoDxOpt, float twoDy,
                      flag_t flags, uint8_t node_place) {
    NodeString* ns = coq_callocTyped(NodeString);
    StringGlyphed* sgToGive = StringGlyphed_createCopy(strRef); 
    nodestring_and_super_init_(ns, ref, &sgToGive,
                               x, y, twoDxOpt, twoDy, flags, node_place);
    return ns;
}


// MARK: - Multi String
void  node_addMultiStrings(Node* n, StringGlyphedInit const data,
                           float const lineWidth, float const lineHeight,
                           uint32_t relativeFlags, Character trailingSpace) 
{    
    with_beg(StringGlyphed, str, StringGlyphed_create(data))
    StringGlyphArr*const lines = StringGlyphArr_create(str, 
                            lineWidth / lineHeight, trailingSpace);
    float y0 = 0.5*lineHeight*(float)(lines->lineCount - 1);
    n->w = lineWidth;
    n->h = lineHeight*(float)(lines->lineCount);
    for(uint32_t index = 0; index < lines->lineCount; index++) {
        if(!lines->strOpts[index]) { printwarning("Empty line."); continue; }
        NodeString* ns = NodeString_createWithStringGlyph_(n, lines->strOpts[index],
                                    0, y0 - index * lineHeight, lineWidth, lineHeight, 0, 0);
        ns->n.nodrawData.data0.v.x = -0.5*(float)index;
        node_setXYrelatively(&ns->n, relativeFlags, true);
    }
    stringglypharr_destroyAndNull(&lines);
    with_end(str)
}


// MARK: - Multi String scrollable

void slidingmenu_addMultiStrings(SlidingMenu* sm, StringGlyphedInit const data) {
    float const width = slidingmenu_itemRelativeWidth(sm);
    with_beg(StringGlyphed, str, StringGlyphed_create(data))
    StringGlyphArr*const lines = StringGlyphArr_create(str, width, spchar_null);
    for(uint32_t index = 0; index < lines->lineCount; index++) {
        if(!lines->strOpts[index]) { printwarning("Empty line."); continue; }
        NodeString* ns = NodeString_createWithStringGlyph_(NULL, lines->strOpts[index], 
                                                           0, 0, 0, 1, flag_noParent, 0);
        slidingmenu_addItem(sm, &ns->n);
    }
    stringglypharr_destroyAndNull(&lines);
    with_end(str)
}


// Test...
Drawable*  Drawable_test_createString(Node *const refOpt, const char *const c_str, CoqFont const*const cf,
                                float x, float y, float twoDy, flag_t flags) {
    Drawable *d = coq_callocTyped(Drawable);
    node_init(&d->n, refOpt, x, y, 1, 1, flags, 0);
    CoqFontDims const fd = coqfont_dims(cf);
    PixelBGRAArray *pa = Pixels_engine_test_createArrayFromString_(c_str, cf);
    Texture* tex =  Texture_createWithPixels(pa->pixels,
            (uint32_t)pa->width, (uint32_t)pa->height, 
            fd.nearest ? tex_flag_nearest : 0);
    drawable_init(d, tex, Mesh_drawable_sprite, 0, twoDy);
    float const relGlyphHeight = fd.fullHeight / fd.solidHeight;
    d->n.sy = twoDy * relGlyphHeight;
    d->n.h  = 1.f / relGlyphHeight;  // -> 2Dy = sy * h...
    d->n.sx = twoDy * relGlyphHeight * (float)pa->width / (float)pa->height;
    d->n.w = pa->solidWidth / (float)pa->width; // -> 2Dx = 2Dy*relSolidW = sx * w.

    return d;
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
