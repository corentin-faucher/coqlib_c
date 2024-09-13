//
//  node_screen.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#include "node_view.h"

#include "node_root.h"
#include "node_tree.h"

#include "../utils/util_base.h"

void  view_alignElements_(View* v, bool isOpening) {
    Node* parent = v->n._parent;
    if(parent == NULL) {
        printerror("No parent"); return;
    }
    if((v->n.flags & flag_viewDontAlignElements) == 0) {
        float frame_ratio = parent->w / parent->h;
        uint8_t alignOpt = node_align_setSecondaryToDefPos
            |node_align_dontSetPrimaryDefPos|node_align_respectRatio;
        if(frame_ratio < 1.f)
            alignOpt |= node_align_vertically;
        if(isOpening)
            alignOpt |= node_align_fixPos;
        node_tree_alignTheChildren(&v->n, alignOpt, frame_ratio, 1.f);
        float aligned_ratio = v->n.w / v->n.h;
        float scale;
        if(aligned_ratio < frame_ratio) {
            scale = parent->h / v->n.h;
            v->n.w = v->n.h * frame_ratio; 
        } else {
            scale = parent->w / v->n.w;
            v->n.h = v->n.w / frame_ratio;
        }
        Vector2 scales = {{ scale, scale }};
        fluid_setScales(&v->f, scales, isOpening);
        return;
    }
    // Sinon, cas pas d'alignement, juste reprendre les dimensions du parent.
    fluid_setScales(&v->f, vector2_ones, isOpening);
    v->n.w = parent->w;
    v->n.h = parent->h;
}
void  view_open_(Node* n) {
    View* v = (View*)n;
    v->buttonSelectedOpt = NULL;
    view_alignElements_(v, true);
}
void  view_reshape_(Node* n) {
    view_alignElements_((View*)n, false);
}

void  view_touchHoveringDefault_(View* const v, Vector2 const pos) {
    Button* const hovered = node_tree_searchActiveButtonWithPosOpt(&v->n, pos, NULL);
    Button* const lastHovered = v->buttonSelectedOpt;
    if(lastHovered == hovered) return;
    if(lastHovered) if(lastHovered->stopHoveringOpt)
        lastHovered->stopHoveringOpt(lastHovered);
    v->buttonSelectedOpt = hovered;
    if(hovered) if(hovered->startHoveringOpt)
        hovered->startHoveringOpt(hovered);
} 
void  view_touchDownDefault_(View* const v, Vector2 pos, uint32_t id) {
    if(id != 0) return; // Action par defaut juste pour clique gauche.
    v->lastTouchedPos = pos;
    Button* const lastSelected = v->buttonSelectedOpt;
    if(lastSelected) if(lastSelected->stopHoveringOpt)
        lastSelected->stopHoveringOpt(lastSelected);
    v->buttonSelectedOpt = NULL;
    // 0. Trouver un bouton sélectionable
    Button* const toTouch = node_tree_searchActiveButtonWithPosOpt(&v->n, pos, NULL);
    if(toTouch == NULL) return;
    // 1. Grab le noeud draggable (si on drag on n'active pas)
    if(toTouch->grabOpt) {
        // Désactiver un scrollview quand un noeud est grabbé
        #if TARGET_OS_OSX != 1
        CoqEvent_addToWindowEvent((CoqEventWin) {
            .type = event_type_win_ios_scrollviewDisable_
        });
        #endif
        v->buttonSelectedOpt = toTouch;
        Vector2 relpos = vector2_absPosToPosInReferentialOfNode(pos, &toTouch->n);
        toTouch->grabOpt(toTouch, relpos);
        return;
    }
    // 2. Sinon activer le noeud sélectionné (non grabbable)
    if(toTouch->action) toTouch->action(toTouch);
}
void  view_touchDragDefault_(View* const v, Vector2 pos, uint32_t touchId) {
    if(touchId != 0) return;
    Button* const grabbed = v->buttonSelectedOpt;
    if(grabbed == NULL) return;
    if(grabbed->dragOpt == NULL) {
        printwarning("Grabbed node without drag function.");
        return;
    }
    Vector2 relpos = vector2_absPosToPosInReferentialOfNode(pos, &grabbed->n);
    grabbed->dragOpt(grabbed, relpos);
}
void  view_touchUpDefault_(View* v, uint32_t touchId) {
    if(touchId != 0) return;
    Button* const grabbed = v->buttonSelectedOpt;
    if(grabbed == NULL) return;
    if(grabbed->letGoOpt) {
        grabbed->letGoOpt(v->buttonSelectedOpt);
    } else {
        printwarning("Grabbed node without letGo function.");
    }
    v->buttonSelectedOpt = NULL;
}

void  view_initWithSuper(View* const v, Root* const root, flag_t const flags) {
    /** Les écrans sont toujours ajoutés juste après l'ainé.
    * add back : root->back,  add front : root->{back,front},
    * add 3 : root->{back,3,front},  add 4 : root->{back,4,3,front}, ...
    * i.e. les deux premiers écrans sont le back et le front respectivement,
    * les autres sont au milieu. */
    Node* const bigBro = root->n._firstChild;
    node_init(&v->n, bigBro ? bigBro : &root->n, 0, 0, 4, 4, node_type_nf_view, 
               flags|flag_parentOfReshapable, bigBro ? node_place_asBro : 0);
    // Init as Smooth (avec lambda = 10)
    fluid_init_(&v->f, 10.f);
    // Init as View (avec méthodes par défault)
    v->n.openOpt =     view_open_;
    v->n.reshapeOpt =  view_reshape_;
    v->touchHovering = view_touchHoveringDefault_;
    v->touchDown =     view_touchDownDefault_;
    v->touchDrag =     view_touchDragDefault_;
    v->touchUp =       view_touchUpDefault_;
}

View* node_asViewOpt(Node* n) {
    if(n->_type & node_type_flag_view)
        return (View*)n;
    return NULL;
}
