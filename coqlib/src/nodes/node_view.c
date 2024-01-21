//
//  node_screen.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#include "nodes/node_root.h"
#include "nodes/node_tree.h"
#include <math.h>

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
    Node* bigBro = root->n.firstChild;
    View* v = Node_createEmptyOfType_(node_type_nf_view, sizeOpt ? sizeOpt : sizeof(View),
                    flags, bigBro ? bigBro : &root->n, bigBro ? node_place_asBro : 0);
    // Init as Node.
    v->n.w = 4;
    v->n.h = 4;
    v->n.flags |= flag_parentOfReshapable;
    // Init as Smooth.
    fluid_init_(&v->f, 10.f);
    // Init as View -> Open and reshape for screen.
    v->root = root;
    v->prefsRef = root->prefsRef;
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
float view_ratio(View* screen) {
    Node* parent = screen->n.parent;
    if(parent == NULL) {
        printerror("No parent"); return 1.f;
    }
    return parent->w / parent->h;
}
void  view_alignElements(View* screen, bool isOpening) {
    Node* parent = screen->n.parent;
    if(parent == NULL) {
        printerror("No parent"); return;
    }
    if((screen->n.flags & flag_viewDontAlignElements) == 0) {
        float screenRatio = view_ratio(screen);
        uint8_t alignOpt = node_align_setSecondaryToDefPos|node_align_respectRatio;
        if(screenRatio < 1.f)
            alignOpt |= node_align_vertically;
        if(isOpening)
            alignOpt |= node_align_fixPos;
        node_tree_alignTheChildren(&screen->n, alignOpt, screenRatio, 1.f);
        float scale = fminf(parent->w / screen->n.w, parent->h / screen->n.h);
        Vector2 scales = {{scale, scale }};
        fluid_setScales(&screen->f, scales, isOpening);
        return;
    }
    // Sinon, cas pas d'alignement, juste reprendre les dimensions du parent.
    fluid_setScales(&screen->f, vector2_ones, isOpening);
    screen->n.w = parent->w;
    screen->n.h = parent->h;
}
