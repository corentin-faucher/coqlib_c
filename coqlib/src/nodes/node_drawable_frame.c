//
//  drawable_frame.c
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-06.
//

#include "nodes/node_drawable.h"
#include "nodes/node_fluid.h"

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
    Vertex* vertices = mesh_vertices(bar->d.mesh);
    vertices[2].x = -xPos;
    vertices[3].x = -xPos;
    vertices[4].x =  xPos;
    vertices[5].x =  xPos;
    mesh_needToUpdateVertices(bar->d.mesh);
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
    Vertex* vertices = mesh_vertices(vbar->d.mesh);
    vertices[2].y =  yPos;
    vertices[3].y =  yPos;
    vertices[4].y = -yPos;
    vertices[5].y = -yPos;
    mesh_needToUpdateVertices(vbar->d.mesh);
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
    mesh_needToUpdateVertices(frame->d.mesh);
    
    if(!(frame->n.flags & flag_giveSizeToParent)) return;
    Node* parent = frame->n.parent;
    if(parent == NULL) return;
    parent->w = 2.f*actualDx;
    parent->h = 2.f*actualDy;
    Fluid* parent_f = node_asFluidOpt(parent);
    if(parent_f) if(parent_f->n.flags & flag_fluidRelativeFlags)
        fluid_setRelatively_(parent_f, true);
}
void   _frame_open_getSizesOfParent(Node* nd) {
    Frame* f = (Frame*)nd;
    Node* p = nd->parent;
    if(!p) { printerror("No parent."); return; }
    f->setDims(f, (Vector2){{0.5f*p->w, 0.5f*p->h}});
}
/// Surface de "Frame". Un cadre ajustable.
///  Typiquement autour d'un autre noeud.
Frame* Frame_createWithTex(Node* const refOpt, float inside,
                    float delta, float twoDxOpt, float twoDyOpt,
                    Texture* tex, uint16_t options) {
    Frame* frame = Node_createEmptyOfType_(node_type_dl_frame, sizeof(Frame),
                                   flag_drawableDontRespectRatio, refOpt, 0);
    // Init as drawable.
    smtrans_init(&frame->d.trShow);
    smtrans_init(&frame->d.trExtra);
    frame->d.tex = tex;
    // Mesh (owner) et fonction pour setter les dimensions du frame.
    if(options & frame_option_horizotalBar) {
        frame->d.mesh = Mesh_createHorizontalBar();
        frame->setDims = _bar_update;
    } else if(options & frame_option_verticalBar) {
        frame->d.mesh = Mesh_createVerticalBar();
        frame->setDims = _vbar_update;
    } else {
        frame->d.mesh = Mesh_createFrame();
        frame->setDims = _frame_update;
    }
    if(texture_isShared(tex))
        frame->n.deinitOpt = _drawable_deinit_freeMesh;
    else
        frame->n.deinitOpt = _drawable_deinit_freeTextureAndMesh;
    // Init as frame
    frame->delta = delta;
    frame->inside = inside;
    if(options & frame_option_getSizesFromParent) {
        frame->n.openOpt =    _frame_open_getSizesOfParent;
        frame->n.reshapeOpt = _frame_open_getSizesOfParent;
    }
    if(options & frame_option_giveSizesToParent)
        frame->n.flags |= flag_giveSizeToParent;
    // Setter tout de suite les dimensions ?
    if(twoDxOpt <= 0.f && twoDyOpt <= 0.f)
        return frame;
    frame->setDims(frame, (Vector2) {{0.5f*twoDxOpt, 0.5f*twoDyOpt}});
    return frame;
}
Frame* Frame_create(Node* const refOpt, float inside, float delta,
                    float twoDxOpt, float twoDyOpt, uint32_t pngId, uint16_t options) {
    Texture* frameTex = Texture_sharedImage(pngId);
    return Frame_createWithTex(refOpt, inside, delta,
           twoDxOpt, twoDyOpt, frameTex, options);
}
void  node_tryUpdatingAsFrameOfBro(Node* nodeOpt, Node* broOpt) {
    if(!nodeOpt || !broOpt) return;
    if(!(nodeOpt->_type & node_type_flag_frame)) return;
    Frame* f = (Frame*)nodeOpt;
    f->setDims(f, node_deltas(broOpt));
}


