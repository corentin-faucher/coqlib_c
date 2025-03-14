//
//  drawable_frame.c
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-06.
//

#include "node_drawable.h"
#include "../utils/util_base.h"

/*-- Surface de "Bar". Un segment de taille ajustable. Version 1D de "Frame". --------------*/
void   bar_update_(Frame* bar, Vector2 deltas) {
    float inside = fminf(1.f, fmaxf(0.f, bar->inside));
    float delta = bar->delta;
    float smallDeltaX = fmaxf(0.f, deltas.x - 2*delta * inside);
    float actualDx = (smallDeltaX + 2.f * delta);
    bar->n.w = 1.f;
    bar->n.h = 1.f;
    bar->n.sx = 2.f*actualDx;
    bar->n.sy = 2.f*delta;
    float xPos = 0.5f * smallDeltaX / actualDx;
    Vertex* vertices = mesh_retainVerticesOpt(bar->d._mesh);
    if(!vertices) { printerror("No vertices for bar."); return; }
    vertices[2].pos.x = -xPos;
    vertices[3].pos.x = -xPos;
    vertices[4].pos.x =  xPos;
    vertices[5].pos.x =  xPos;
    mesh_releaseVertices(bar->d._mesh, 0);
}
void   vbar_update_(Frame* vbar, Vector2 deltas) {
    float inside = fminf(1.f, fmaxf(0.f, vbar->inside));
    float delta = vbar->delta;
    float smallDeltaY = fmaxf(0.f, deltas.y - 2.f*delta * inside);
    float actualDy = (smallDeltaY + 2.f * delta);
    vbar->n.w = 1.f;
    vbar->n.h = 1.f;
    vbar->n.sx = 2.f*delta;
    vbar->n.sy = 2.f*actualDy;
    float yPos = 0.5f * smallDeltaY / actualDy;
    Vertex* vertices = mesh_retainVerticesOpt(vbar->d._mesh);
    if(!vertices) { printerror("No vertices for vbar."); return; }
    vertices[2].pos.y =  yPos;
    vertices[3].pos.y =  yPos;
    vertices[4].pos.y = -yPos;
    vertices[5].pos.y = -yPos;
    mesh_releaseVertices(vbar->d._mesh, 0);
}
void   frame_update_(Frame* frame, Vector2 deltas) {
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
    Vertex* vertices = mesh_retainVerticesOpt(frame->d._mesh);
    if(!vertices) { printerror("No vertices for frame."); return; }
    float xPos = 0.5f * smallDeltaX / (smallDeltaX + delta);
    vertices[4].pos.x = -xPos;
    vertices[5].pos.x = -xPos;
    vertices[6].pos.x = -xPos;
    vertices[7].pos.x = -xPos;
    vertices[8].pos.x =  xPos;
    vertices[9].pos.x =  xPos;
    vertices[10].pos.x = xPos;
    vertices[11].pos.x = xPos;
    float yPos = 0.5f * smallDeltaY / (smallDeltaY + delta);
    vertices[1].pos.y =   yPos;
    vertices[5].pos.y =   yPos;
    vertices[9].pos.y =   yPos;
    vertices[13].pos.y =  yPos;
    vertices[2].pos.y =  -yPos;
    vertices[6].pos.y =  -yPos;
    vertices[10].pos.y = -yPos;
    vertices[14].pos.y = -yPos;
    mesh_releaseVertices(frame->d._mesh, 0);

    if(!(frame->n.flags & flag_giveSizeToParent)) return;
    Node* parent = frame->n._parent;
    if(parent == NULL) return;
    parent->w = 2.f*actualDx;
    parent->h = 2.f*actualDy;
//    Fluid* parent_f = node_asFluidOpt(parent);
//    if(parent_f) if(parent_f->n.flags & flags_fluidRelative)
//        node_setXYrelatively(parent, (uint32_t)parent->flags, false);
}
void   frame_open_getSizesOfParent_(Node* nd) {
    Frame* f = (Frame*)nd;
    Node* p = nd->_parent;
    if(!p) { printerror("No parent."); return; }
    f->setDims(f, (Vector2){{0.5f*p->w, 0.5f*p->h}});
}
void   drawable_and_frame_init_(Frame* frame, Texture* tex,
                                float inside, float delta, uint16_t options) {
    frame->d.trShow =  SmoothFlag_new();
    frame->d.trExtra = SmoothFlag_new();
    textureref2_init(&frame->d.texr, tex);
    frame->n._type |= node_type_drawable|node_type_frame;
    // Mesh (owner) et fonction pour setter les dimensions du frame.
    if(options & frame_option_horizotalBar) {
        frame->d._mesh = Mesh_createHorizontalBar();
        frame->setDims = bar_update_;
    } else if(options & frame_option_verticalBar) {
        frame->d._mesh = Mesh_createVerticalBar();
        frame->setDims = vbar_update_;
    } else {
        frame->d._mesh = Mesh_createFrame();
        frame->setDims = frame_update_;
    }
    frame->n.renderer_updateInstanceUniforms = Drawable_renderer_defaultUpdateInstanceUniforms;
    frame->n.deinitOpt = drawable_deinit_;
    frame->n.renderIU = InstanceUniforms_default;
    frame->n.renderIU.uvRect.size = frame->d.texr.dims.DuDv;
    // Init as frame
    frame->delta = delta;
    frame->inside = inside;
    if(options & frame_option_getSizesFromParent) {
        frame->n.openOpt =    frame_open_getSizesOfParent_;
        frame->n.reshapeOpt = frame_open_getSizesOfParent_;
    }
    if(options & frame_option_giveSizesToParent)
        frame->n.flags |= flag_giveSizeToParent;
}
/// Surface de "Frame". Un cadre ajustable.
///  Typiquement autour d'un autre noeud.
Frame* Frame_create(Node* const refOpt, float inside, float delta,
                    float twoDxOpt, float twoDyOpt, Texture* tex, uint16_t options) {
    Frame* frame = coq_callocTyped(Frame);
    node_init(&frame->n, refOpt, 0, 0, 1, 1, 0, 0);
    drawable_and_frame_init_(frame, tex, inside, delta, options);
    // Setter tout de suite les dimensions ?
    if(twoDxOpt <= 0.f && twoDyOpt <= 0.f)
        return frame;
    frame->setDims(frame, (Vector2) {{0.5f*twoDxOpt, 0.5f*twoDyOpt}});
    return frame;
}
void  node_tryUpdatingAsFrameOfBro(Node* nodeOpt, Node* broOpt) {
    if(!nodeOpt || !broOpt) return;
    if(!(nodeOpt->_type & node_type_frame)) return;
    Frame* f = (Frame*)nodeOpt;
    f->setDims(f, node_deltas(broOpt));
}
