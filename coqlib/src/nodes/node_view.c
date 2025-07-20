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
    guard_let(Node*, parent, v->n._parent, printerror("No parent"), )
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
        Vector3 scales = {{ scale, scale, scale }};
        if(isOpening) fluid_fixScales(&v->f, scales);
        else          fluid_setScales(&v->f, scales);
        return;
    }
    // Sinon, cas pas d'alignement, juste reprendre les dimensions du parent.
    if(isOpening) fluid_fixScales(&v->f, vector3_ones);
    else          fluid_setScales(&v->f, vector3_ones);
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

void  view_touchHoveringDefault_(NodeTouch const nt) {
    View*const v = (View*)nt.n;
    NodeTouch hoveredNTOpt = node_tree_searchActiveButtonWithPosOpt(nt, NULL);
    Button*const newHoveredOpt = node_asButtonOpt(hoveredNTOpt.n); 
    Button *const lastHoveredOpt = v->buttonSelectedOpt;
    if(lastHoveredOpt == newHoveredOpt) return;
    if(lastHoveredOpt && lastHoveredOpt->stopHoveringOpt)
        lastHoveredOpt->stopHoveringOpt(lastHoveredOpt);
    v->buttonSelectedOpt = newHoveredOpt;
    if(newHoveredOpt && newHoveredOpt->startHoveringOpt)
        newHoveredOpt->startHoveringOpt(newHoveredOpt);
} 
void  view_touchDownDefault_(NodeTouch const nt) {
    if(nt.touchId != 0) return; // Action par defaut juste pour clique gauche.
    View*const v = (View*)nt.n;
    Box v_ref = node_referentialInParent(&v->n, NULL);
    Vector2 posInView = vector2_referentialIn(nt.posAbs, v_ref);
    v->lastTouchedPos = posInView;
    Button* const lastSelected = v->buttonSelectedOpt;
    if(lastSelected && lastSelected->stopHoveringOpt)
        lastSelected->stopHoveringOpt(lastSelected);
    v->buttonSelectedOpt = NULL;
    // 0. Trouver un bouton sélectionable
    NodeTouch const touchedNTOpt = node_tree_searchActiveButtonWithPosOpt(nt, NULL);
    guard_let(Button*, newTouched, node_asButtonOpt(touchedNTOpt.n), , )
    // Effectuer son action au "touch".
    newTouched->touch(touchedNTOpt);
    // Si "draggable", on garde en mémoire comme "selected".
    if(newTouched->dragOpt) {
        v->buttonSelectedOpt = newTouched;
        // Aussi -> désactiver un scrollview dans iOS quand un noeud est grabbé
        #if TARGET_OS_OSX != 1
        CoqEvent_addToWindowEvent((CoqEventWin) {
            .type = eventtype_win_ios_scrollviewDisable_
        });
        #endif
    }
}
void  view_touchDragDefault_(NodeTouch const nt) {
    if(nt.touchId != 0) return;
    View*const v = (View*)nt.n;
    guard_let(Button*, grabbed, v->buttonSelectedOpt,,)
    if(grabbed->dragOpt == NULL) {
        printwarning("Grabbed node without drag function.");
        return;
    }
    grabbed->dragOpt((NodeTouch) {
        .n = &grabbed->n,
        .posAbs = nt.posAbs,
        .touchId = nt.touchId,
    });
}
void  view_touchUpDefault_(NodeTouch const nt) {
    if(nt.touchId != 0) return;
    View*const v = (View*)nt.n;
    guard_let(Button*, grabbed, v->buttonSelectedOpt,,)
    v->buttonSelectedOpt = NULL;
    if(grabbed->letGoOpt == NULL) {
        printwarning("Grabbed node without letGo function.");
        return;
    }
    grabbed->letGoOpt((NodeTouch) {
        .n = &grabbed->n,
        .posAbs = nt.posAbs,
        .touchId = nt.touchId,
    });
}

void  view_and_super_init(View* const v, Root* const root, flag_t const flags) {
    /** Les écrans sont toujours ajoutés juste après l'ainé.
    * add back : root->back,  add front : root->{back,front},
    * add 3 : root->{back,3,front},  add 4 : root->{back,4,3,front}, ...
    * i.e. les deux premiers écrans sont le back et le front respectivement,
    * les autres sont au milieu. */
    Node* const bigBro = root->n._firstChild;
    node_init(&v->n, bigBro ? bigBro : &root->n, 0, 0, 4, 4, 
               flags|flag_parentOfReshapable, bigBro ? node_place_asBro : 0);
    // Init as Smooth (avec lambda = 10)
    fluid_init(&v->f, 10.f);
    // Init as View (avec méthodes par défault)
    v->n._type |= node_type_view;
    v->n.openOpt =     view_open_;
    v->n.reshapeOpt =  view_reshape_;
    v->touchHovering = view_touchHoveringDefault_;
    v->touchDown =     view_touchDownDefault_;
    v->touchDrag =     view_touchDragDefault_;
    v->touchUp =       view_touchUpDefault_;
}


