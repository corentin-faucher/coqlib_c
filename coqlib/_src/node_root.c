//
//  node_root.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//

#include "node_root.h"
#include "node_tree.h"
#include "utils.h"
#include "timer.h"

NodeRoot* NodeRoot_createEmpty(size_t size) {
    if(size < sizeof(NodeRoot)) {
//        printwarning("size < sizeof(NodeRoot).");
        size = sizeof(NodeRoot);
    }
    Node *nd = _Node_createEmptyOfType(node_type_root, size,
         flag_exposed|flag_show|flag_rootOfToDisplay|flag_rootOfSelectable|flag_rootOfReshapable,
         NULL, 0);
    nd->w = 4.f;
    nd->h = 4.f;
    NodeRoot* rt = (NodeRoot*)nd;
    rt->frameWidth =  2.f;
    rt->frameHeight = 2.f;
    rt->viewWidthPx =  800;
    rt->viewHeightPx = 600;
    rt->margins = (Margins){ 5, 5, 5, 5 };
    camera_init(&rt->camera, 10.f);
    return rt;
}
void   _noderoot_closeActiveScreen(NodeRoot* rt) {
    Node* lastScreen = (Node*)rt->activeScreen;
    if(lastScreen == NULL) return;
    node_tree_close(lastScreen);
    if((lastScreen->flags & flag_screenPersistent) == 0)
        Timer_createAndGetId(1000, false, lastScreen, node_destroy);
    
    rt->activeScreen = NULL;
}
void   _noderoot_setActiveScreen(NodeRoot* rt, NodeScreen* newScreen) {
    rt->activeScreen = newScreen;
    node_tree_openAndShow((Node*)newScreen);
    if(rt->changeScreenAction)
        rt->changeScreenAction(rt);
}
void   noderoot_changeToActiveScreen(NodeRoot* rt, NodeScreen* const newScreen) {
    // 0. Cas réouverture
    if (rt->activeScreen == newScreen) {
        node_tree_openAndShow((Node*)newScreen);
        return;
    }
    // 1. Fermer l'écran actif (déconnecter si evanescent)
    _noderoot_closeActiveScreen(rt);
    // 2. Si null -> fermeture de l'app.
#warning TODO et newScreen Opt?
//    if(newScreen == NULL) {
//        rt->activeScreen = NULL;
//        Timer_createAndGetId(800, false, rt, noderoot_terminate);
//        return;
//    }
    // 3. Ouverture du nouvel écran.
    _noderoot_setActiveScreen(rt, newScreen);
}

void noderoot_setFrame(NodeRoot *rt, float widthPx, float heightPx, Bool inTransition) {
    rt->viewWidthPx = widthPx;
    rt->viewHeightPx = heightPx;
    // 0. Marges...
    float realRatio = widthPx / heightPx;
    float ratioT = rt->margins.top / heightPx + 0.01;
    float ratioB = rt->margins.bottom / heightPx + 0.01;
    float ratioLR = (rt->margins.left + rt->margins.right) / widthPx + 0.015;
    printdebug("margin top %f, bottom %f.", rt->margins.top, rt->margins.bottom);
    printdebug("input view dim %f x %f. ratio %f, ratioT %f",
               widthPx, heightPx, realRatio, ratioT);
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
    Node *nd = (Node*)rt;
    if (realRatio > 1) { // Landscape
        nd->w = (1.f - ratioLR) * rt->frameWidth;
        nd->h = 2.f;
    }
    else {
        nd->w = 2.f;
        nd->h = (1.f - ratioT - ratioB) * rt->frameHeight;
    }
    // 3. Shift en y dû aux marge (pour le lookAt)
    rt->yShift = (ratioT - ratioB) * rt->frameHeight / 2.f;
    // 4. Reshape de la structure
    node_tree_reshape(nd);
    printdebug("Setting root frame. frameWidth %f, frameHeight %f. widthPx %d, heightPx %d.",
               rt->frameWidth, rt->frameHeight, rt->viewWidthPx, rt->viewHeightPx);
}
void noderoot_updateModelMatrix(NodeRoot *rt) {
    matrix4_initAsLookAtWithCamera(&rt->nd.piu.model, &rt->camera);
}
/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pixels) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle noderoot_viewRectangleFromPosAndDim(NodeRoot *rt, Vector2 pos, Vector2 deltas,
                                              Bool invertedY) {
    float widthPx = 2.f * deltas.x / rt->frameWidth * rt->viewWidthPx;
    float heightPx = 2.f * deltas.y / rt->frameHeight * rt->viewHeightPx;
    // (le point (0, 0) est au milieu, d'où le +0.5)
    float x = (0.5f + (pos.x - deltas.x) / rt->frameWidth) * rt->viewWidthPx;
    float y;
    if(invertedY) { // e.g. iOS. -> axe des y part du haut et va vers le bas.
        y = (0.5f - (pos.y + deltas.y - rt->yShift) / rt->frameHeight ) * rt->viewHeightPx;
    } else {
        y = (0.5f + (pos.y - deltas.y - rt->yShift) / rt->frameHeight ) * rt->viewHeightPx;
    }
    return (Rectangle) { x, y, widthPx, heightPx };
}
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en pixels).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   noderoot_posFromViewPos(NodeRoot *rt, Vector2 viewPos, Bool invertedY) {
    float x = (viewPos.x / rt->viewWidthPx - 0.5f) * rt->frameWidth;
    float y = (invertedY ? -1.f : 1.f)*(viewPos.y / rt->viewHeightPx - 0.5f) * rt->frameHeight + rt->yShift;
    return (Vector2) { x, y };
}

void      matrix4_initProjectionWithNodeRoot(Matrix4 *m, NodeRoot *rt) {
    float middleZ = sp_pos(&rt->camera.pos[2]);
    matrix4_initAsPerspectiveDeltas(m, 0.1f, 50.f, middleZ, rt->frameWidth, rt->frameHeight);
}
