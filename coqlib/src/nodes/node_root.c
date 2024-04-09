//
//  node_root.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-15.
//
#include "node_tree.h"
#include "node_root.h"
#include "node_squirrel.h"
#include "node_sliding_menu.h"
#include "node_drawable_multi.h"
#include "../utils/utils_base.h"
#include "../graphs/graph_colors.h"

void  root_init(Root* root, Root* parentRootOpt) {
    if(root->n._parent) {
        if(parentRootOpt == NULL) printerror("Non absolute root without parent root.");
        root->rootParentOpt = parentRootOpt;
    } else {
        if(parentRootOpt) printerror("Absolute root have no need for parentRoot.");
        root->rootParentOpt = NULL;
    }
//    root->fullSize =   (Vector2){{ 2.2f, 2.2f }};
//    root->_yShift = 0;
    fl_init(&root->fullSizeWidth, 2.2f, 20.f, false);
    fl_init(&root->fullSizeHeight, 2.2f, 20.f, false);
    fl_init(&root->yShift, 0.f, 20.f, false);
    root->viewSizePt = (Vector2){{ 800, 600 }};
    root->margins = (Margins){ 5, 5, 5, 5 };
    camera_init(&root->camera, 10.f);
    fl_array_init(root->back_RGBA, color4_gray2.f_arr, 4, 5.f);
    
    root->updateModelAndGetDrawable = node_defaultUpdateModelAndGetAsDrawableOpt;
//    printdebug("Root init selected is %p.", root->buttonSelectedOpt);
}
Root* node_asRootOpt(Node* n) {
    if(n->_type & node_type_flag_root) return (Root*)n;
    return NULL;
}
void   root_releaseActiveView_(Root* rt) {
    View* viewLast = rt->viewActiveOpt;
    if(viewLast == NULL) return;
    node_tree_close(&viewLast->n);
    if((viewLast->n.flags & flag_viewPersistent) == 0)
        timer_scheduled(NULL, 600, false, &viewLast->n, node_tree_throwToGarbage);
    
    rt->viewActiveOpt =     NULL;
    rt->buttonSelectedOpt = NULL;
}
void   root_setActiveView_(Root* rt, View* newView) {
    rt->viewActiveOpt = newView;
    node_tree_openAndShow(&newView->n);
    if(rt->changeViewOpt)
        rt->changeViewOpt(rt);
}
void   _root_callback_terminate(Node* node) {
    Root* rt = (Root*)node;
//    if(rt->willTerminateOpt)
//        rt->willTerminateOpt(rt);
    rt->shouldTerminate = true;
}
void  root_changeViewActiveTo(Root* rt, View* const newViewOpt) {
    // Pas le droit de quitter si iOS
#ifdef __APPLE__
#if TARGET_OS_OSX != 1
    if(newViewOpt == NULL) {
        printerror("Trying to quit an iOS app !");
        return;
    }
#endif
#endif
    // 0. Cas réouverture
    if (newViewOpt && (rt->viewActiveOpt == newViewOpt)) {
        node_tree_openAndShow(&newViewOpt->n);
        return;
    }
    // 1. Fermer l'écran actif (déconnecter si evanescent)
    root_releaseActiveView_(rt);
    // 2. Si null -> fermeture de l'app.
    if(newViewOpt == NULL) {
        rt->viewActiveOpt = NULL;
        timer_scheduled(NULL, 800, false, &rt->n, _root_callback_terminate);
        return;
    }
    // 3. Ouverture du nouvel écran.
    root_setActiveView_(rt, newViewOpt);
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
    if(rt->resizeOpt) rt->resizeOpt(rt, info);
}

bool    root_isLandscape(Root* r) {
    return r->n.w > r->n.h;
}

Button* root_searchActiveButtonOptWithPos(Root* const root, Vector2 const absPos,
                                 Node* const nodeToAvoidOpt) {
    if(&root->n == nodeToAvoidOpt) return NULL;
    // Superflu... besoin d'une rechere a partir d'un non-root ?
//    vector2_absPosToPosInReferentialOfNode(absPos, root);
    Squirrel sq;
    sq_initWithRelPos(&sq, &root->n, absPos, sq_scale_ones);
    if(!sq_isIn(&sq)) return NULL;
    if(!(sq.pos->flags & flag_show)) return NULL;
    Button* b = NULL;
    // 2. Se placer au premier child
    if(!sq_goDownP(&sq))
        return NULL;
    while (true) {
#ifdef DEBUG_SEARCH
        printf("🐰 sq at %f, %f in ", sq.v.x, sq.v.y);
        node_print_type(sq.pos);
        printf(" pos %f, %f, in %d, show %lu, ", sq.pos->pos.x, sq.pos->pos.y, sq_isIn(&sq), (sq.pos->flags & flag_show));
#endif
        if(sq_isIn(&sq) && (sq.pos->flags & flag_show)
           && (sq.pos != nodeToAvoidOpt))
        {
#ifdef DEBUG_SEARCH
            printf("In...");
#endif
            // 1. Possibilité trouvé
            b = node_asActiveButtonOpt(sq.pos);
            if(b) {
#ifdef DEBUG_SEARCH
                printf("button !\n");
#endif
                return b;
            }
            // 2. Aller en profondeur
            if(sq.pos->flags & flag_parentOfButton) {
                if(sq_goDownP(&sq)) {
#ifdef DEBUG_SEARCH
                    printf(" Go Down!\n");
#endif
                    continue;
                }
            }
        }
        // 3. Remonter, si plus de petit-frère
#ifdef DEBUG_SEARCH
        printf(" out, next...");
#endif
        while(!sq_goRight(&sq)) {
#ifdef DEBUG_SEARCH
            printf(" up ");
#endif
            if(!sq_goUpP(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == &root->n) {
#ifdef DEBUG_SEARCH
                printf("\n");
#endif
                return NULL;
            }
        }
#ifdef DEBUG_SEARCH
        printf("\n");
#endif
    }
}
Button* root_searchFirstButtonOptWithData(Root* root, uint32_t typeOpt, uint32_t data0) {
    Squirrel sq;
    sq_init(&sq, &root->n, sq_scale_ones);
    // Se placer au premier child
    if(!sq_goDown(&sq))
        return NULL;
    while(true) {
        if(sq.pos->flags & flag_show) {
            Button* b = node_asButtonOpt(sq.pos);
            if(b) if(b->data.uint0 == data0) {
                if(!typeOpt) return b;
                if(b->n._type & typeOpt)
                    return b;
            }
            if(sq.pos->flags & flag_parentOfButton)
                if(sq_goDown(&sq)) continue;
        }
        // 3. Remonter, si plus de petit-frère
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == &root->n) return NULL;
        }
    }
}
SlidingMenu* root_searchFirstSlidingMenuOpt(Root* root) {
//    if(!root) return NULL;
    Squirrel sq;
    sq_init(&sq, &root->n, sq_scale_ones);
    // Se placer au premier child
    if(!sq_goDown(&sq))
        return NULL;
    while(true) {
        if(sq.pos->flags & flag_show) {
            SlidingMenu* sm = node_asSlidingMenuOpt(sq.pos);
            if(sm) return sm;
            if(sq.pos->flags & flag_parentOfScrollable)
                if(sq_goDown(&sq)) continue;
        }
        // 3. Remonter, si plus de petit-frère
        while(!sq_goRight(&sq)) {
            if(!sq_goUp(&sq)) {
                printerror("Pas de root."); return NULL;
            } else if(sq.pos == &root->n) return NULL;
        }
    }
}

void root_updateModelMatrix(Root *rt) {
    float yShift = fl_pos(&rt->yShift);
    matrix4_initAsLookAtWithCameraAndYshift(&rt->n._piu.model, &rt->camera, yShift);
}
/*-- Init d'autres struct utilisant la root. --*/
/// Obtenir le rectangle (en pts) associé à une position (origin)
///  et dimensions dans le frame de la root.
/// e.g. (0.5, 0.5), (1, 1) -> (540, 320), (290, 290).
Rectangle root_windowRectangleFromBox(Root *rt, Box box, bool invertedY) {
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
Rectangle node_windowRectangle(Node* n, bool invertedY) {
    Squirrel sq;
    sq_init(&sq, n, sq_scale_deltas);
    while(sq_goUpPS(&sq)) {}
    Root* root = node_asRootOpt(sq.pos);
    if(!root) { printerror("No root."); return (Rectangle) {{ 0, 0, 10, 10 }}; }
    return root_windowRectangleFromBox(root, (Box){.center = sq.v, .deltas = sq.s }, invertedY);
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

Drawable* node_defaultUpdateModelAndGetAsDrawableOpt(Node* const node) {
    // 0. Cas root
    {
        Root* root = node_asRootOpt(node);
        if(root) {
            root_updateModelMatrix(root);
            return NULL;
        }
    }
    Node* const parent = node->_parent;
    if(parent == NULL) {
        printerror("Non-root without parent.");
        return NULL;
    }
    // 1. Cas branche
    if(node->_firstChild != NULL) {
        node_updateModelMatrixWithParentModel(node, &parent->_piu.model, 1.f);
        return NULL;
    }
    // 3. Cas feuille
    Drawable* d = node_asDrawableOpt(node);
    // Laisser faire si n'est pas affichable...
    if(!d) return NULL;
    // Facteur d'"affichage"
    float alpha = smtrans_setAndGetIsOnSmooth(&d->trShow, (d->n.flags & flag_show) != 0);
    // Rien à afficher...
    if(alpha < 0.001f)
        return NULL;
    d->n._piu.show = alpha;
    if((d->n.flags & flag_poping) == 0)
        alpha = 1.f;
    DrawableMulti* dm = node_asDrawableMultiOpt(node);
    if(dm) {
        dm->updateModels(dm, &parent->_piu.model);
    } else {
        node_updateModelMatrixWithParentModel(&d->n, &parent->_piu.model, alpha);
    }
    return d;
}


void      matrix4_initProjectionWithRoot(Matrix4 *m, Root *rt) {
    float middleZ = fl_pos(&rt->camera.pos[2]);
    float fullSizeWidth = fl_pos(&rt->fullSizeWidth);
    float fullSizeHeight = fl_pos(&rt->fullSizeHeight);
    matrix4_initAsPerspectiveDeltas(m, 0.1f, 50.f, middleZ, fullSizeWidth, fullSizeHeight);
}
