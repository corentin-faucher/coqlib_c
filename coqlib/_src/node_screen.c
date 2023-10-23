//
//  node_screen.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#include "node_screen.h"
#include "utils.h"
#include "node_root.h"
#include "node_tree.h"
#include <math.h>

void    _node_screen_open_default(Node* const node) {
    nodescreen_alignElements((NodeScreen*)node, true);
}
void    _node_screen_reshape_default(Node* const node) {
    nodescreen_alignElements((NodeScreen*)node, false);
}

NodeScreen* NodeScreen_create(NodeRoot* const root) {
    /** Les écrans sont toujours ajoutés juste après l'ainé.
    * add back : root->back,  add front : root->{back,front},
    * add 3 : root->{back,3,front},  add 4 : root->{back,4,3,front}, ...
    * i.e. les deux premiers écrans sont le back et le front respectivement,
    * les autres sont au milieu. */
    // Init as Node.
    Node* bigBro = root->nd.firstChild;
    Node *nd;
    printdebug("Create screen...");
    if(bigBro) {
        nd = _Node_createEmptyOfType(node_type_screen|node_type_smooth, sizeof(NodeScreen),
                                     flag_rootOfReshapable, bigBro, node_place_asBro);
    } else {
        nd = _Node_createEmptyOfType(node_type_screen|node_type_smooth, sizeof(NodeScreen),
                                     flag_rootOfReshapable, (Node*)root, 0);
    }
    nd->w = 4;
    nd->h = 4;
    NodeScreen *screen = (NodeScreen*)nd;
    // Init as Smooth.
    printdebug("Init smooth.");
    _nodesmooth_init(&screen->ns, 10.f);
    // Init as Screen -> Open and reshape for screen.
    nd->open = _node_screen_open_default;
    nd->reshape = _node_screen_reshape_default;
    printdebug("screen fini.");
    // Les autres methodes sont laissee a NULL par defaut.
    return screen;
}
float nodescreen_ratio(NodeScreen* screen) {
    Node* parent = screen->nd.parent;
    if(parent == NULL) {
        printerror("No parent"); return 1.f;
    }
    return parent->w / parent->h;
}
void  nodescreen_alignElements(NodeScreen* screen, Bool isOpening) {
    Node* parent = screen->nd.parent;
    if(parent == NULL) {
        printerror("No parent"); return;
    }
    if((screen->nd.flags & flag_screenDontAlignElements) == 0) {
        float screenRatio = nodescreen_ratio(screen);
        uint8_t alignOpt = node_align_setSecondaryToDefPos|node_align_respectRatio;
        if(screenRatio < 1.f)
            alignOpt |= node_align_vertically;
        if(isOpening)
            alignOpt |= node_align_fixPos;
        node_tree_alignTheChildren(&screen->nd, alignOpt, screenRatio, 1.f);
        float scale = fminf(parent->w / screen->nd.w, parent->h / screen->nd.h);
        nodesmooth_setScaleX(&screen->ns, scale, isOpening);
        nodesmooth_setScaleY(&screen->ns, scale, isOpening);
        return;
    }
    // Sinon, cas pas d'alignement, juste reprendre les dimensions du parent.
    nodesmooth_setScaleX(&screen->ns, 1, isOpening);
    nodesmooth_setScaleY(&screen->ns, 1, isOpening);
    screen->nd.w = parent->w;
    screen->nd.h = parent->h;
}
