//
//  node_root.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#include "root.h"
#include "node_tree.h"
#include "utils.h"
#include "timer.h"
#include "colors.h"

Root* Root_create(void) {
    Root* root = _Node_createEmptyOfType(node_type_root, sizeof(Root), 0, NULL, 0);
    _root_init(root);
    return root;
}
void  _root_init(Root* root) {
    root->n.flags = flag_rootDefaultFlags;
    root->n.w = 4.f;
    root->n.h = 4.f;
    root->frameWidth =  2.f;
    root->frameHeight = 2.f;
    root->viewWidthPx =  800;
    root->viewHeightPx = 600;
    root->margins = (Margins){ 5, 5, 5, 5 };
    camera_init(&root->camera, 10.f);
    sp_array_init(root->back_RGBA, color4_gray2.f_arr, 4, 5.f);
}
Root* node_asRootOpt(Node* n) {
    if(n->_type & node_type_flag_root) return (Root*)n;
    return NULL;
}
void   _root_closeActiveScreen(Root* rt) {
    Node* lastScreen = &rt->activeView->n;
    if(lastScreen == NULL) return;
    node_tree_close(lastScreen);
    if((lastScreen->flags & flag_viewPersistent) == 0)
        timer_scheduled(NULL, 600, false, lastScreen, node_tree_throwToGarbage);
    
    rt->activeView = NULL;
    rt->selectedButton = NULL;
}
void   _root_setActiveScreen(Root* rt, View* newScreen) {
    rt->activeView = newScreen;
    node_tree_openAndShow(&newScreen->n);
    if(rt->changeScreenAction)
        rt->changeScreenAction(rt);
}
void   _root_callback_terminate(Node* node) {
    Root* rt = (Root*)node;
    if(rt->willTerminate) rt->willTerminate(rt);
    rt->shouldTerminate = true;
}
void   root_changeActiveScreenTo(Root* rt, View* const newScreen) {
    // 0. Cas réouverture
    if (rt->activeView == newScreen) {
        node_tree_openAndShow(&newScreen->n);
        return;
    }
    // 1. Fermer l'écran actif (déconnecter si evanescent)
    _root_closeActiveScreen(rt);
    // 2. Si null -> fermeture de l'app.
    if(newScreen == NULL) {
        rt->activeView = NULL;
        timer_scheduled(NULL, 800, false, &rt->n, _root_callback_terminate);
        return;
    }
    // 3. Ouverture du nouvel écran.
    _root_setActiveScreen(rt, newScreen);
}

void root_setFrame(Root *rt, float widthPx, float heightPx, Bool inTransition) {
    rt->viewWidthPx = widthPx;
    rt->viewHeightPx = heightPx;
    // 0. Marges...
    float realRatio = widthPx / heightPx;
    float ratioT = rt->margins.top / heightPx + 0.01;
    float ratioB = rt->margins.bottom / heightPx + 0.01;
    float ratioLR = (rt->margins.left + rt->margins.right) / widthPx + 0.015;
    // 1. Full Frame
    if(realRatio > 1) { // Landscape
        rt->frameHeight = 2 / ( 1 - ratioT - ratioB);
        rt->frameWidth = realRatio * rt->frameHeight;
    }
    else {
        rt->frameWidth = 2 / (1 - ratioLR);
        rt->frameHeight = rt->frameWidth / realRatio;
    }
    if (inTransition)
        return;
    // 2. Usable Frame
    if (realRatio > 1) { // Landscape
        rt->n.w = (1.f - ratioLR) * rt->frameWidth;
        rt->n.h = 2.f;
    }
    else {
        rt->n.w = 2.f;
        rt->n.h = (1.f - ratioT - ratioB) * rt->frameHeight;
    }
    // 3. Shift en y dû aux marge (pour le lookAt)
    rt->_yShift = (ratioT - ratioB) * rt->frameHeight / 2.f;
    // 4. Reshape de la structure
    node_tree_reshape(&rt->n);
}
void root_updateModelMatrix(Root *rt) {
    matrix4_initAsLookAtWithCameraAndYshift(&rt->n.piu.model, &rt->camera, rt->_yShift);
}
/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pixels) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_viewRectangleFromPosAndDim(Root *rt, Vector2 pos, Vector2 deltas,
                                              Bool invertedY) {
    float widthPx = 2.f * deltas.x / rt->frameWidth * rt->viewWidthPx;
    float heightPx = 2.f * deltas.y / rt->frameHeight * rt->viewHeightPx;
    // (le point (0, 0) est au milieu, d'où le +0.5)
    float x = (0.5f + (pos.x - deltas.x) / rt->frameWidth) * rt->viewWidthPx;
    float y;
    if(invertedY) { // e.g. iOS. -> axe des y part du haut et va vers le bas.
        y = (0.5f - (pos.y + deltas.y - rt->_yShift) / rt->frameHeight ) * rt->viewHeightPx;
    } else {
        y = (0.5f + (pos.y - deltas.y - rt->_yShift) / rt->frameHeight ) * rt->viewHeightPx;
    }
    return (Rectangle) { x, y, widthPx, heightPx };
}
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en pixels).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   root_absposFromViewPos(Root *rt, Vector2 viewPos, Bool invertedY) {
    float x = (viewPos.x / rt->viewWidthPx - 0.5f) * rt->frameWidth;
    float y = (invertedY ? -1.f : 1.f)*(viewPos.y / rt->viewHeightPx - 0.5f) * rt->frameHeight + rt->_yShift;
    return (Vector2) { x, y };
}

void      matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt) {
    float middleZ = sp_pos(&rt->camera.pos[2]);
    matrix4_initAsPerspectiveDeltas(m, 0.1f, 50.f, middleZ, rt->frameWidth, rt->frameHeight);
}
