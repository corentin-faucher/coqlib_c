//
//  node_root.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//
#include "node_root.h"

#include "node_tree.h"
#include "../utils/util_base.h"
static float const camera_default_eye_[3] =    { 0, 0, 5 };
static float const camera_default_up_[3] =     { 0, 1, 0 };
static float const camera_default_center_[3] = { 0, 0, 0 };

/// Mise à jour ordinaire de la matrice modèle pour une root.
void root_renderer_defaultUpdateIU_(Node* const n) {
    Root* rt = (Root*)n;
    float yShift = fl_evalPos(&rt->yShift);
    Vector3 eye = fl_array_toVec3(rt->camera_eye);
    eye.y += yShift;
    Vector3 center = fl_array_toVec3(rt->camera_center);
    center.y += yShift;
    Vector3 up =     fl_array_toVec3(rt->camera_up);
    // Projection ordinaire (pyramide de perspective...)
    float deltaX = 0.5*fl_evalPos(&rt->fullSizeWidth);
    float deltaY = 0.5*fl_evalPos(&rt->fullSizeHeight);

    // Si on veut garder projection et view séparés...
//    matrix4_initAsPerspectiveDeltas(&rt->projectionOpt, 0.1, 50, middleZ, deltaX, deltaY);
//    matrix4_initAsLookAt(&n->model, eye, center, up);

    matrix4_initAsPerspectiveAndLookAt(&n->renderIU.model, eye, center, up, 0.1, 50, deltaX, deltaY);
}

void  root_deinit_(Node* n) {
    timer_cancel(&((Root*)n)->_timer);
}

void  root_and_super_init(Root* const root, Root* const parentRootOpt,
                          Node* const parentOpt) {
    Node* parent;
    if(parentRootOpt && parentOpt)
        parent = parentOpt;
    else {
        if(parentOpt) printwarning("Missing parent root.");
        if(parentRootOpt) printwarning("Missing parent.");
        parent = (Node*)parentRootOpt;
    }
    node_init(&root->n, parent, 0, 0, 4, 4, flags_rootDefault, 0);
    root->n._type |= node_type_root;
    root->rootParentOpt = parentRootOpt;
    root->n.deinitOpt = root_deinit_;
    // Cadre de la view...
    root->viewSizePt = (Vector2){{ 800, 600 }};
    root->margins = (Margins){ 5, 5, 5, 5 };
    fl_init(&root->fullSizeWidth, 2.2f, 20.f, false);
    fl_init(&root->fullSizeHeight, 2.2f, 20.f, false);
    fl_init(&root->yShift, 0.f, 20.f, false);
    // Caméra
    fl_array_init(root->camera_up,     camera_default_up_, 3, 5.f);
    fl_array_init(root->camera_eye,    camera_default_eye_, 3, 5.f);
    fl_array_init(root->camera_center, camera_default_center_, 3, 5.f);
    // Couleur de fond/lumière ambiante.
    fl_array_init(root->back_RGBA, color4_gray_40.f_arr, 4, 5.f);
    fl_init(&root->ambiantLight, 0, 5, false);
    // Override de l'update du model.
    root->n.renderer_updateInstanceUniforms = root_renderer_defaultUpdateIU_;
}

void   root_deleteOldActiveView_callback_(void* rootIn) {
    Root *r = (Root*)rootIn;
    if(!r->toDeleteViewNodeOpt) return;
    noderef_destroyAndNull(&r->toDeleteViewNodeOpt);
}
void   root_callback_terminate_(void* rtIn) {
    Root* rt = (Root*)rtIn;
//    if(rt->willTerminateOpt)
//        rt->willTerminateOpt(rt);
    rt->shouldTerminate = true;
}
void  root_changeViewActiveTo(Root* const rt, View* const newViewOpt) {
    // Pas le droit de quitter si iOS
#ifdef __APPLE__
#if TARGET_OS_OSX != 1
    if(newViewOpt == NULL) {
        printerror("Trying to quit an iOS app !");
        return;
    }
#endif
#endif
    // Cas réouverture
    if (newViewOpt && (rt->viewActiveOpt == newViewOpt)) {
        node_tree_openAndShow(&newViewOpt->n);
        return;
    }
    // Encore une ancienne view schedulé pour delete ?
    if(rt->_timer && rt->toDeleteViewNodeOpt) {
        printdebug("Still old view not deleted...");
        timer_doNowAndCancel(&rt->_timer);
    }
    if(rt->toDeleteViewNodeOpt) {
        printerror("Still view to delete ?");
    }
    // Fermeture de l'app ?
    if(newViewOpt == NULL) {
        if(rt->viewActiveOpt) { 
            node_tree_close(&rt->viewActiveOpt->n);
            rt->viewActiveOpt = NULL;
        }
        timer_scheduled(&rt->_timer, 800, false, rt, root_callback_terminate_);
        return;
    }
    // Fermer l'ancienne view.
    if(rt->viewActiveOpt) {
        Node *const oldView = &rt->viewActiveOpt->n;
        rt->viewActiveOpt = NULL;
        node_tree_close(oldView);
        if(!(oldView->flags & flag_viewPersistent)) {
            rt->toDeleteViewNodeOpt = oldView;
            timer_scheduled(&rt->_timer, 600, false, rt, root_deleteOldActiveView_callback_);
        }
    }
    // Ouverture de la nouvelle view.
    rt->viewActiveOpt = newViewOpt;
    node_tree_openAndShow(&newViewOpt->n);
    if(rt->changeViewOpt)
        rt->changeViewOpt(rt);
}

typedef struct {
    Vector2 fullSize;
    float realRatio;
    float ratioT;
    float ratioB;
    float ratioLR;
} FullSizeAndRatios_;

FullSizeAndRatios_ FullSizeAndRatios_fromMarginsAndSize_(Margins margins, Vector2 viewSizePt) {
    FullSizeAndRatios_ fsr;
    fsr.realRatio = viewSizePt.w / viewSizePt.h;
    fsr.ratioT = margins.top / viewSizePt.h + 0.01;
    fsr.ratioB = margins.bottom / viewSizePt.h + 0.01;
    fsr.ratioLR = (margins.left + margins.right) / viewSizePt.w + 0.010;
    // Full Frame
    if(fsr.realRatio > 1) { // Landscape
        fsr.fullSize.h = 2 / ( 1 - fsr.ratioT - fsr.ratioB);
        fsr.fullSize.w = fsr.realRatio * fsr.fullSize.h;
    }
    else {
        fsr.fullSize.w = 2 / (1 - fsr.ratioLR);
        fsr.fullSize.h = fsr.fullSize.w / fsr.realRatio;
    }
    return fsr;
}

void root_justSetFrameSize_(Root* r, Vector2 frameSizePt) {
    r->viewSizePt = frameSizePt;
    FullSizeAndRatios_ fsr = FullSizeAndRatios_fromMarginsAndSize_(r->margins, frameSizePt);
    fl_set(&r->fullSizeWidth, fsr.fullSize.w);
    fl_set(&r->fullSizeHeight, fsr.fullSize.h);
//    fl_array_set(r->fullSizeArr, fsr.fullSize.f_arr, 2);
//    r->fullSize = fsr.fullSize;
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

    FullSizeAndRatios_ fsr = FullSizeAndRatios_fromMarginsAndSize_(info.margins, info.framePt.size);

    // 2. Usable Frame
    if (fsr.realRatio > 1) { // Landscape
        rt->n.w = (1.f - fsr.ratioLR) * fsr.fullSize.w;
        rt->n.h = 2.f;
    }
    else {
        rt->n.w = 2.f;
        rt->n.h = (1.f - fsr.ratioT - fsr.ratioB) * fsr.fullSize.h;
    }
    // 3. Shift en y dû aux marge (pour le lookAt)
    rt->viewSizePt = info.framePt.size;
    float yShift = (fsr.ratioT - fsr.ratioB) * fsr.fullSize.h / 2.f;
//    fl_array_set(rt->fullSizeArr, fsr.fullSize.f_arr, 2);
    if(info.dontFix) {
        fl_set(&rt->fullSizeWidth, fsr.fullSize.w);
        fl_set(&rt->fullSizeHeight, fsr.fullSize.h);
        fl_set(&rt->yShift, yShift);
    } else {
        fl_fix(&rt->fullSizeWidth, fsr.fullSize.w);
        fl_fix(&rt->fullSizeHeight, fsr.fullSize.h);
        fl_fix(&rt->yShift, yShift);
    }
//    fl_set(&rt->yShift, (fsr.ratioT - fsr.ratioB) * fsr.fullSize.h / 2.f);
//    rt->fullSize = fsr.fullSize;

//    rt->_yShift = (fsr.ratioT - fsr.ratioB) * fsr.fullSize.h / 2.f;


    // 4. Changement de taille des fonts ?
//    double fontSize = Texture_currentFontSize();
//    double targetFontSize = fmaxf(16.f, fminf(144.f,
//                    0.06f*fminf(info.sizesPix.x, info.sizesPix.y)));
//    if(targetFontSize > fontSize * 1.2f || targetFontSize < 0.8f*fontSize) {
//        Texture_setCurrentFontSize(targetFontSize);
//    }
    // 5. Reshape de la structure
    node_tree_reshape(&rt->n);
    // 6. Action supplementaire au resize ?
root_resize_end:
    if(rt->resizeOpt) rt->resizeOpt(rt, info);
}

/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pts) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_windowRectangleFromBox(Root const*const rt, Box const box, bool const invertedY) {
    float fullSizeWidth = fl_real(&rt->fullSizeWidth);
    float fullSizeHeight = fl_real(&rt->fullSizeHeight);
    float yShift =   fl_real(&rt->yShift);
    float widthPt = 2.f * box.Dx / fullSizeWidth * rt->viewSizePt.w;
    float heightPt = 2.f * box.Dy / fullSizeHeight * rt->viewSizePt.h;
    // (le point (0, 0) est au milieu, d'où le +0.5)
    float x = (0.5f + (box.c_x - box.Dx) / fullSizeWidth) * rt->viewSizePt.w;
    float y;
    if(invertedY) { // e.g. iOS. -> axe des y part du haut et va vers le bas.
        y = (0.5f - (box.c_y + box.Dy - yShift) / fullSizeHeight ) * rt->viewSizePt.h;
    } else {
        y = (0.5f + (box.c_y - box.Dy - yShift) / fullSizeHeight ) * rt->viewSizePt.h;
    }
    return (Rectangle) {{ x, y, widthPt, heightPt }};
}
/// Semblable à `node_hitBoxInParentReferential`, mais
/// retourne l'espace occupé en pts dans la view de l'OS.
/// invertedY == true pour iOS (y vont vers les bas).
Rectangle node_windowRectangle(Node const*const n, bool const invertedY) {
    Box n_b = node_hitBox(n);
    Root const* root = NULL;
    for(Node const* p = n->_parent; p; p = p->_parent) {
        root = node_asRootOpt(p);
        if(root) break;
        // Sort du referentiel du parent présent avant de passer au prochain.
        n_b = box_referentialOut(n_b, node_asReferential(p));
    }
    if(!root) { printerror("No root."); return (Rectangle) {{ 0, 0, 10, 10 }}; }
    
    return root_windowRectangleFromBox(root, n_b, invertedY);
}
/// Obtenir la position dans le frame de la root à partir de la position de la vue (en points).
/// e.g. (540, 320) -> (0.0, 0.0), (centre d'une vue 1080 x 640).
Vector2   root_absposFromViewPos(Root *rt, Vector2 viewPos, bool invertedY) {
    float fullSizeWidth = fl_real(&rt->fullSizeWidth);
    float fullSizeHeight = fl_real(&rt->fullSizeHeight);
    float yShift =   fl_real(&rt->yShift);
    float x = (viewPos.x / rt->viewSizePt.w - 0.5f) * fullSizeWidth;
    float y = (invertedY ? -1.f : 1.f)*(viewPos.y / rt->viewSizePt.h - 0.5f) * fullSizeHeight + yShift;
    return (Vector2) {{ x, y }};
}

//void      matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt) {
//    float middleZ = fl_evalPos(rt->camera.pos[2]);
//    float fullSizeWidth = fl_evalPos(rt->fullSizeWidth);
//    float fullSizeHeight = fl_evalPos(rt->fullSizeHeight);
//    matrix4_initAsPerspectiveDeltas(m, 0.1f, 50.f, middleZ, fullSizeWidth, fullSizeHeight);
//}
