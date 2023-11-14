//
//  drawable_frame.c
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-06.
//

#include "drawable.h"
#include "fluid.h"
#include "_node_types.h"

/*-- Surface de "Bar". Un segment de taille ajustable. Version 1D de "Frame". --------------*/
void   _bar_update(Frame* bar, float deltaX) {
    float inside = fminf(1.f, fmaxf(0.f, bar->inside));
    float delta = bar->delta;
    float smallDeltaX = fmaxf(0.f, deltaX - 2*delta * inside);
    float actualDx = (smallDeltaX + 2.f * delta);
    bar->n.w = 1.f;
    bar->n.h = 1.f;
    bar->n.sx = 2.f*actualDx;
    bar->n.sy = 2.f*delta;
    float xPos = 0.5f * smallDeltaX / (smallDeltaX + 2.f * delta);
    Vertex* vertices = mesh_vertices(bar->d.mesh);
    vertices[2].x = -xPos;
    vertices[3].x = -xPos;
    vertices[4].x =  xPos;
    vertices[5].x =  xPos;
}
void   _frame_update(Frame* frame, float deltaX, float deltaY) {
    float inside = fminf(1.f, fmaxf(0.f, frame->inside));
    float delta = frame->delta;
    float smallDeltaX = fmaxf(0.f, deltaX - delta * inside);
    float smallDeltaY = fmaxf(0.f, deltaY - delta * inside);
    float actualDx = (smallDeltaX + delta);
    float actualDy = (smallDeltaY + delta);
    frame->n.w = 1.f;
    frame->n.h = 1.f;
    frame->n.sx = 2.f*actualDx;
    frame->n.sy = 2.f*actualDy;
    Vertex* vertices = mesh_vertices(frame->d.mesh);
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
    
    if(!(frame->n.flags & flag_giveSizeToParent)) return;
    Node* parent = frame->n.parent;
    if(parent == NULL) return;
    parent->w = 2.f*actualDx;
    parent->h = 2.f*actualDy;
    Fluid* parent_f = node_asFluidOpt(parent);
    if(parent_f) if(parent_f->n.flags & flag_fluidRelativeFlags)
        _fluid_setRelatively(parent_f, true);
}
void   _frame_open_getSizesOfParent(Node* nd) {
    Frame* f = (Frame*)nd;
    Node* p = nd->parent;
    if(!p) { printerror("No parent."); return; }
    if(f->_isBar)
        _bar_update(f, 0.5f*p->w);
    else
        _frame_update(f, 0.5f*p->w, 0.5f*p->h);
}
Frame* Frame_createBar(Node* const refOpt, float inside,
                       float delta, float twoDxOpt, uint pngId) {
    Frame* bar = _Node_createEmptyOfType(node_type_dl_frame, sizeof(Frame),
                                       flag_drawableDontRespectRatio, refOpt, 0);
    // Init as drawable.
    smtrans_init(&bar->d.trShow);
    smtrans_init(&bar->d.trExtra);
    bar->d.tex = Texture_sharedImage(pngId);
    bar->d.mesh =  Mesh_createBar(); // ** Mesh owner.
    bar->n.deinit = _drawable_deinit_freeMesh;
    // Init as frame
    bar->delta = delta;
    bar->inside = inside;
    bar->_isBar = true;
    if(twoDxOpt > 0.f)
        _bar_update(bar, 0.5f*twoDxOpt);
    return bar;
}
/*-- Surface de "Frame". Un cadre ajustable. Typiquement autour d'un autre noeud. -----------*/
Frame* Frame_createWithTex(Node* const refOpt, float inside,
                    float delta, float twoDxOpt, float twoDyOpt,
                    Texture* shrTex, FrameType frametype) {
    Frame* frame = _Node_createEmptyOfType(node_type_dl_frame, sizeof(Frame),
                                   flag_drawableDontRespectRatio, refOpt, 0);
    // Init as drawable.
    smtrans_init(&frame->d.trShow);
    smtrans_init(&frame->d.trExtra);
    frame->d.tex = shrTex;
    frame->d.mesh =  Mesh_createFrame(); // ** Mesh owner.
    frame->n.deinit = _drawable_deinit_freeMesh;
    // Init as frame
    frame->delta = delta;
    frame->inside = inside;
    frame->_isBar = false;
    if(frametype == frametype_getSizesFromParent) {
        frame->n.open =    _frame_open_getSizesOfParent;
        frame->n.reshape = _frame_open_getSizesOfParent;
    }
    if(frametype == frametype_giveSizesToParent)
        frame->n.flags |= flag_giveSizeToParent;
    if(twoDxOpt > 0.f && twoDyOpt > 0.f)
        _frame_update(frame, 0.5f*twoDxOpt, 0.5f*twoDyOpt);
    return frame;
}
Frame* Frame_create(Node* const refOpt, float inside, float delta,
                    float twoDxOpt, float twoDyOpt, uint pngId, FrameType frametype) {
    Texture* frameTex = Texture_sharedImage(pngId);
    return Frame_createWithTex(refOpt, inside, delta,
           twoDxOpt, twoDyOpt, frameTex, frametype);
}
void  node_tryUpdatingAsFrameOfBro(Node* nodeOpt, Node* refBro) {
    if(!nodeOpt) return;
    if(!(nodeOpt->_type & node_type_flag_frame)) return;
    Frame* f = (Frame*)nodeOpt;
    if(!f || !refBro) return;
    if(f->_isBar)
        _bar_update(f, node_deltaX(refBro));
    else
        _frame_update(f, node_deltaX(refBro), node_deltaY(refBro));
}


