//
//  node_screen.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#include "nodes/node_view.h"

#include "utils/utils_base.h"
#include "nodes/node_root.h"
#include "nodes/node_tree.h"

void    view_open(Node* const node) {
    View* v = node_asViewOpt(node);
    if(v) view_alignElements(v, true);
}
void    view_reshape(Node* const node) {
    View* v = node_asViewOpt(node);
    if(v) view_alignElements(v, false);
}

View* View_create(Root* const root, flag_t flags, size_t sizeOpt) {
    /** Les écrans sont toujours ajoutés juste après l'ainé.
    * add back : root->back,  add front : root->{back,front},
    * add 3 : root->{back,3,front},  add 4 : root->{back,4,3,front}, ...
    * i.e. les deux premiers écrans sont le back et le front respectivement,
    * les autres sont au milieu. */
    Node* bigBro = root->n._firstChild;
    View* v = coq_calloc(1, sizeOpt > sizeof(View) ? sizeOpt : sizeof(View));
    node_init_(&v->n, bigBro ? bigBro : &root->n, 0, 0, 4, 4, node_type_nf_view, 
               flags|flag_parentOfReshapable, bigBro ? node_place_asBro : 0);
    // Init as Smooth.
    fluid_init_(&v->f, 10.f);
    // Init as View -> Open and reshape for screen.
    v->n.openOpt =    view_open;
    v->n.reshapeOpt = view_reshape;
    // (Les autres methodes sont laissee a NULL par defaut.)
    return v;
}
View* node_asViewOpt(Node* n) {
    if(n->_type & node_type_flag_view)
        return (View*)n;
    return NULL;
}
//float view_ratio(View* screen) {
//    Node* parent = screen->n.parent;
//    if(parent == NULL) {
//        printerror("No parent"); return 1.f;
//    }
//    return parent->w / parent->h;
//}
void  view_alignElements(View* v, bool isOpening) {
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
