//
//  node_screen.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#include "view.h"
#include "utils.h"
#include "root.h"
#include "node_tree.h"
#include <math.h>

void    view_default_open(Node* const node) {
    View* v = node_asViewOpt(node);
    if(v) view_alignElements(v, true);
}
void    view_default_reshape(Node* const node) {
    View* v = node_asViewOpt(node);
    if(v) view_alignElements(v, false);
}

View* View_create(Root* const root, flag_t flags) {
    /** Les écrans sont toujours ajoutés juste après l'ainé.
    * add back : root->back,  add front : root->{back,front},
    * add 3 : root->{back,3,front},  add 4 : root->{back,4,3,front}, ...
    * i.e. les deux premiers écrans sont le back et le front respectivement,
    * les autres sont au milieu. */
    // Init as Node.
    Node* bigBro = root->n.firstChild;
    View *v;
    if(bigBro) {
        v = _Node_createEmptyOfType(node_type_smooth_view, sizeof(View),
                                     flags|flag_parentOfReshapable, bigBro, node_place_asBro);
    } else {
        v = _Node_createEmptyOfType(node_type_smooth_view, sizeof(View),
                                     flags|flag_parentOfReshapable, &root->n, 0);
    }
    v->n.w = 4;
    v->n.h = 4;
    // Init as Smooth.
    _fluid_init(&v->f, 10.f);
    // Init as Screen -> Open and reshape for screen.
    v->root = root;
    v->n.open =    view_default_open;
    v->n.reshape = view_default_reshape;
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
void  view_alignElements(View* screen, Bool isOpening) {
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
        fluid_setScaleX(&screen->f, scale, isOpening);
        fluid_setScaleY(&screen->f, scale, isOpening);
        return;
    }
    // Sinon, cas pas d'alignement, juste reprendre les dimensions du parent.
    fluid_setScaleX(&screen->f, 1, isOpening);
    fluid_setScaleY(&screen->f, 1, isOpening);
    screen->n.w = parent->w;
    screen->n.h = parent->h;
}
