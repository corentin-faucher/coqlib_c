//
//  node_selectable.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-14.
//

#include "node_selectable.h"

// Selectable quelconque. methodes non definies.
NodeSel* NodeSel_create(Node* const refOpt, float x, float y, float h,
                        float lambda, flag_t flags, uint8_t node_place) {
    // Init as Node.
    Node *nd = _Node_createEmptyOfType(node_type_smooth|node_type_selectable,
                                       sizeof(NodeSel),
                                       flags, refOpt, node_place);
    nd->x = x;
    nd->y = y;
    nd->w = h;  // Carre a priori.
    nd->h = h;
    NodeSel *sel = (NodeSel*)nd;
    // Init as NodeSmooth.
    _nodesmooth_init(&sel->ns, lambda);
    // Rien d'autre à init a priori...
    return sel;
}
