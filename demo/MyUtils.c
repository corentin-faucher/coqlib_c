//
//  MyUtils.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-11-03.
//

#include "MyUtils.h"
#include "colors.h"

Node* MyNode_struct_createFramedLocStr(Localizable loc_str, float targetW) {
    Node* nd = Node_createEmpty();
    nd->w = targetW;  nd->h = 1.f;
    UnownedString str = (UnownedString) { Loc_strKey(loc_str), NULL, true };
    node_last_addFramedString(png_frame_mocha, str, targetW, 0.35f, 0.f, false);
    return nd;
}
