//
//  node_root.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#include "nodes/node_tree.h"
#include "nodes/node_root.h"
#include "graphs/graph_colors.h"

Root* Root_create(void) {
    Root* root = Node_createEmptyOfType_(node_type_root, sizeof(Root), 0, NULL, 0);
    root_init(root, NULL, NULL);
    return root;
}
void  root_init(Root* root, Node* parentOpt, Root* parentRootOpt) {
    if(parentOpt) {
        if(parentRootOpt == NULL)
            printerror("Non absolute root without parent.");
        root->rootParentOpt = parentRootOpt;
        node_simpleMoveToParent(&root->n, parentOpt, false);
    } else {
        if(parentRootOpt)
            printwarning("Absolute root have no need for parentRoot.");
        root->rootParentOpt = NULL;
    }
    root->n.flags = flag_rootDefaultFlags;
    root->n.w = 4.f;
    root->n.h = 4.f;
    root->fullSize =   (Vector2){{ 2.2f, 2.2f }};
    root->viewSizePt = (Vector2){{ 800, 600 }};
    root->margins = (Margins){ 5, 5, 5, 5 };
    camera_init(&root->camera, 10.f);
    fl_array_init(root->back_RGBA, color4_gray2.f_arr, 4, 5.f);
    
    root->updateModelAndGetDrawable = node_defaultUpdateModelAndGetAsDrawableOpt;
}
Root* node_asRootOpt(Node* n) {
    if(n->_type & node_type_flag_root) return (Root*)n;
    return NULL;
}
void   _root_closeActiveScreen(Root* rt) {
    View* viewLast = rt->viewActiveOpt;
    if(viewLast == NULL) return;
    node_tree_close(&viewLast->n);
    if((viewLast->n.flags & flag_viewPersistent) == 0)
        timer_scheduled(NULL, 600, false, &viewLast->n, node_tree_throwToGarbage);
    
    rt->viewActiveOpt =     NULL;
    rt->buttonSelectedOpt = NULL;
}
void   _root_setActiveScreen(Root* rt, View* newScreen) {
    rt->viewActiveOpt = newScreen;
    node_tree_openAndShow(&newScreen->n);
    if(rt->changeScreenActionOpt)
        rt->changeScreenActionOpt(rt);
}
void   _root_callback_terminate(Node* node) {
    Root* rt = (Root*)node;
//    if(rt->willTerminateOpt)
//        rt->willTerminateOpt(rt);
    rt->shouldTerminate = true;
}
void   root_changeViewActiveTo(Root* rt, View* const newViewOpt) {
    // 0. Cas réouverture
    if (newViewOpt && (rt->viewActiveOpt == newViewOpt)) {
        node_tree_openAndShow(&newViewOpt->n);
        return;
    }
    // 1. Fermer l'écran actif (déconnecter si evanescent)
    _root_closeActiveScreen(rt);
    // 2. Si null -> fermeture de l'app.
    if(newViewOpt == NULL) {
        rt->viewActiveOpt = NULL;
        timer_scheduled(NULL, 800, false, &rt->n, _root_callback_terminate);
        return;
    }
    // 3. Ouverture du nouvel écran.
    _root_setActiveScreen(rt, newViewOpt);
}

void root_viewResized(Root *rt, ResizeInfo info)
{
    if(info.justMoving)
        goto root_resize_end;
    if(info.framePt.size.w < 2 || info.framePt.size.h < 2 ||
       info.sizesPix.w < 2 || info.sizesPix.h < 2) {
        printerror("Bad resize dims.");
        goto root_resize_end;
    }
    // 0. Marges et taille en pts.
    rt->margins = info.margins;
    rt->viewSizePt = info.framePt.size;
    float realRatio = info.framePt.size.w / info.framePt.size.h;
    float ratioT = rt->margins.top / info.framePt.size.h + 0.01;
    float ratioB = rt->margins.bottom / info.framePt.size.h + 0.01;
    float ratioLR = (rt->margins.left + rt->margins.right) / info.framePt.size.w + 0.010;
    // 1. Full Frame
    if(realRatio > 1) { // Landscape
        rt->fullSize.h = 2 / ( 1 - ratioT - ratioB);
        rt->fullSize.w = realRatio * rt->fullSize.h;
    }
    else {
        rt->fullSize.w = 2 / (1 - ratioLR);
        rt->fullSize.h = rt->fullSize.w / realRatio;
    }
    if (info.inTransition)
        return;
    // 2. Usable Frame
    if (realRatio > 1) { // Landscape
        rt->n.w = (1.f - ratioLR) * rt->fullSize.w;
        rt->n.h = 2.f;
    }
    else {
        rt->n.w = 2.f;
        rt->n.h = (1.f - ratioT - ratioB) * rt->fullSize.h;
    }
    // 3. Shift en y dû aux marge (pour le lookAt)
    rt->_yShift = (ratioT - ratioB) * rt->fullSize.h / 2.f;
    // 4. Changement de taille des fonts ?
    double fontSize = Texture_currentFontSize();
    double targetFontSize = fmaxf(16.f, fminf(144.f,
                    0.06f*fminf(info.sizesPix.x, info.sizesPix.y)));
    if(targetFontSize > fontSize * 1.2f || targetFontSize < 0.8f*fontSize) {
        Texture_setCurrentFontSize(targetFontSize);
    }
    // 5. Reshape de la structure
    node_tree_reshape(&rt->n);
    // 6. Action supplementaire au resize ?
root_resize_end:
    if(rt->resizeActionOpt) rt->resizeActionOpt(rt, info);
}

void root_updateModelMatrix(Root *rt) {
    matrix4_initAsLookAtWithCameraAndYshift(&rt->n.piu.model, &rt->camera, rt->_yShift);
}
/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pts) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_viewRectangleFromPosAndDim(Root *rt, Vector2 pos, Vector2 deltas,
                                              bool invertedY) {
    
    float widthPt = 2.f * deltas.x / rt->fullSize.w * rt->viewSizePt.w;
    float heightPt = 2.f * deltas.y / rt->fullSize.h * rt->viewSizePt.h;
    // (le point (0, 0) est au milieu, d'où le +0.5)
    float x = (0.5f + (pos.x - deltas.x) / rt->fullSize.w) * rt->viewSizePt.w;
    float y;
    if(invertedY) { // e.g. iOS. -> axe des y part du haut et va vers le bas.
        y = (0.5f - (pos.y + deltas.y - rt->_yShift) / rt->fullSize.h ) * rt->viewSizePt.h;
    } else {
        y = (0.5f + (pos.y - deltas.y - rt->_yShift) / rt->fullSize.h ) * rt->viewSizePt.h;
    }
    return (Rectangle) {{ x, y, widthPt, heightPt }};
}
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en points).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   root_absposFromViewPos(Root *rt, Vector2 viewPos, bool invertedY) {
    float x = (viewPos.x / rt->viewSizePt.w - 0.5f) * rt->fullSize.w;
    float y = (invertedY ? -1.f : 1.f)*(viewPos.y / rt->viewSizePt.h - 0.5f) * rt->fullSize.h + rt->_yShift;
    return (Vector2) {{ x, y }};
}

void      matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt) {
    float middleZ = fl_pos(&rt->camera.pos[2]);
    matrix4_initAsPerspectiveDeltas(m, 0.1f, 50.f, middleZ, rt->fullSize.w, rt->fullSize.h);
}
