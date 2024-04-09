//
//  drawable_frame.c
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-06.
//

#include "node_drawable.h"
#include "node_fluid.h"
#include "../utils/utils_base.h"

/*-- Surface de "Bar". Un segment de taille ajustable. Version 1D de "Frame". --------------*/
void   _bar_update(Frame* bar, Vector2 deltas) {
    float inside = fminf(1.f, fmaxf(0.f, bar->inside));
    float delta = bar->delta;
    float smallDeltaX = fmaxf(0.f, deltas.x - 2*delta * inside);
    float actualDx = (smallDeltaX + 2.f * delta);
    bar->n.w = 1.f;
    bar->n.h = 1.f;
    bar->n.sx = 2.f*actualDx;
    bar->n.sy = 2.f*delta;
    float xPos = 0.5f * smallDeltaX / actualDx;
    Vertex* vertices = mesh_vertices(bar->d._mesh);
    vertices[2].x = -xPos;
    vertices[3].x = -xPos;
    vertices[4].x =  xPos;
    vertices[5].x =  xPos;
    mesh_needToUpdateVertices(bar->d._mesh);
}
void   _vbar_update(Frame* vbar, Vector2 deltas) {
    float inside = fminf(1.f, fmaxf(0.f, vbar->inside));
    float delta = vbar->delta;
    float smallDeltaY = fmaxf(0.f, deltas.y - 2.f*delta * inside);
    float actualDy = (smallDeltaY + 2.f * delta);
    vbar->n.w = 1.f;
    vbar->n.h = 1.f;
    vbar->n.sx = 2.f*delta;
    vbar->n.sy = 2.f*actualDy;
    float yPos = 0.5f * smallDeltaY / actualDy;
    Vertex* vertices = mesh_vertices(vbar->d._mesh);
    vertices[2].y =  yPos;
    vertices[3].y =  yPos;
    vertices[4].y = -yPos;
    vertices[5].y = -yPos;
    mesh_needToUpdateVertices(vbar->d._mesh);
}
void   _frame_update(Frame* frame, Vector2 deltas) {
    float inside = fminf(1.f, fmaxf(0.f, frame->inside));
    float delta = frame->delta;
    float smallDeltaX = fmaxf(0.f, deltas.x - delta * inside);
    float smallDeltaY = fmaxf(0.f, deltas.y - delta * inside);
    float actualDx = (smallDeltaX + delta);
    float actualDy = (smallDeltaY + delta);
    frame->n.w = 1.f;
    frame->n.h = 1.f;
    frame->n.sx = 2.f*actualDx;
    frame->n.sy = 2.f*actualDy;
    Vertex* vertices = mesh_vertices(frame->d._mesh);
    float xPos = 0.5f * smallDeltaX / (smallDeltaX + delta);
    vertices[4].x = -xPos;
    vertices[5].x = -xPos;
    vertices[6].x = -xPos;
    vertices[7].x = -xPos;
    vertices[8].x =  xPos;
    vertices[9].x =  xPos;
    vertices[10].x = xPos;
    vertices[11].x = xPos;
    float yPos = 0.5f * smallDeltaY / (smallDeltaY + delta);
    vertices[1].y =   yPos;
    vertices[5].y =   yPos;
    vertices[9].y =   yPos;
    vertices[13].y =  yPos;
    vertices[2].y =  -yPos;
    vertices[6].y =  -yPos;
    vertices[10].y = -yPos;
    vertices[14].y = -yPos;
    mesh_needToUpdateVertices(frame->d._mesh);
    
    if(!(frame->n.flags & flag_giveSizeToParent)) return;
    Node* parent = frame->n._parent;
    if(parent == NULL) return;
    parent->w = 2.f*actualDx;
    parent->h = 2.f*actualDy;
//    Fluid* parent_f = node_asFluidOpt(parent);
//    if(parent_f) if(parent_f->n.flags & flags_fluidRelative)
//        node_setXYrelatively(parent, (uint32_t)parent->flags, false);
}
void   _frame_open_getSizesOfParent(Node* nd) {
    Frame* f = (Frame*)nd;
    Node* p = nd->_parent;
    if(!p) { printerror("No parent."); return; }
    f->setDims(f, (Vector2){{0.5f*p->w, 0.5f*p->h}});
}
void   drawable_and_frame_init_(Frame* frame, Texture* tex,
                                float inside, float delta, uint16_t options) {
    smtrans_init(&frame->d.trShow);
    smtrans_init(&frame->d.trExtra);
    frame->d._tex = tex;
    // Mesh (owner) et fonction pour setter les dimensions du frame.
    if(options & frame_option_horizotalBar) {
        frame->d._mesh = Mesh_createHorizontalBar();
        frame->setDims = _bar_update;
    } else if(options & frame_option_verticalBar) {
        frame->d._mesh = Mesh_createVerticalBar();
        frame->setDims = _vbar_update;
    } else {
        frame->d._mesh = Mesh_createFrame();
        frame->setDims = _frame_update;
    }
    frame->n.deinitOpt = drawable_deinit_freeTextureAndMesh_;
    // Init as frame
    frame->delta = delta;
    frame->inside = inside;
    if(options & frame_option_getSizesFromParent) {
        frame->n.openOpt =    _frame_open_getSizesOfParent;
        frame->n.reshapeOpt = _frame_open_getSizesOfParent;
    }
    if(options & frame_option_giveSizesToParent)
        frame->n.flags |= flag_giveSizeToParent;
}
/// Surface de "Frame". Un cadre ajustable.
///  Typiquement autour d'un autre noeud.
Frame* Frame_createWithName(Node* const refOpt, float inside, float delta, 
                    float twoDxOpt, float twoDyOpt, const char* pngName, uint16_t options) {
    Frame* frame = coq_calloc(1, sizeof(Frame));
    node_init_(&frame->n, refOpt, 0, 0, 1, 1, node_type_nd_frame, flag_drawableDontRespectRatio, 0);
    drawable_and_frame_init_(frame, Texture_sharedImageByName(pngName), inside, delta, options);
    // Setter tout de suite les dimensions ?
    if(twoDxOpt <= 0.f && twoDyOpt <= 0.f)
        return frame;
    frame->setDims(frame, (Vector2) {{0.5f*twoDxOpt, 0.5f*twoDyOpt}});
    return frame;
}
Frame* Frame_create(Node* const refOpt, float inside, float delta,
                    float twoDxOpt, float twoDyOpt, uint32_t pngId, uint16_t options) {
    Frame* frame = coq_calloc(1, sizeof(Frame));
    node_init_(&frame->n, refOpt, 0, 0, 1, 1, node_type_nd_frame, flag_drawableDontRespectRatio, 0);
    drawable_and_frame_init_(frame, Texture_sharedImage(pngId), inside, delta, options);
    // Setter tout de suite les dimensions ?
    if(twoDxOpt <= 0.f && twoDyOpt <= 0.f)
        return frame;
    frame->setDims(frame, (Vector2) {{0.5f*twoDxOpt, 0.5f*twoDyOpt}});
    return frame;
}
void  node_tryUpdatingAsFrameOfBro(Node* nodeOpt, Node* broOpt) {
    if(!nodeOpt || !broOpt) return;
    if(!(nodeOpt->_type & node_type_flag_frame)) return;
    Frame* f = (Frame*)nodeOpt;
    f->setDims(f, node_deltas(broOpt));
}


