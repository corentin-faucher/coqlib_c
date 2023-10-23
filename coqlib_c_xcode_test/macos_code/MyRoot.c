//
//  MyRoot.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#include "MyRoot.h"
#include "Drawables.h"
#include "node_surface.h"



NodeScreen* NodeScreen_createTest(NodeRoot* rt) {
    NodeScreen* screen = NodeScreen_create(rt);
    
    NodeSmooth* ns = NodeSmooth_create(&screen->nd, 0.5f, 0.f, 1.f, 1.f, 5.f,
                                       flag_smoothFadeInRight, 0);
//    sp_updateToConstants(&ns->sx, 1.5f, 10.f);
//    sp_fadeIn(&ns->sx, 1.f);
//    sp_fadeIn(&ns->y, 2.f);
    
    NodeSurf_createPng(&ns->nd, 0.f, 0.f, 0.5f, flag_show, 0, png_the_cat);
    return screen;
}

NodeRoot* NodeRoot_createMyRoot(void) {
    NodeRoot* root = NodeRoot_createEmpty(0);
    
    noderoot_changeToActiveScreen(root,
                                  NodeScreen_createTest(root));
    
    
    return root;
}
