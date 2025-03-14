//
//  _node_types.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-20.
//

#ifndef COQ_NODE__TYPES_H
#define COQ_NODE__TYPES_H

// Les node types sont sur 32 bits, i.e. uint32_t.
typedef enum {
    // Flag qui definissent les grandes categories de noeuds.
    // e.g. si node->_type contient node_type_fluid
    //     -> on sait que ce noeud est castable comme Fluid.
    // node_type_node =      0x0000, pass, tout les noeuds sont des noeuds :P
    node_type_fluid =        0x0001, // Avec smooth x, y, etc.
    node_type_button =       0x0002, // Avec action, drag, etc.
    node_type_scrollable =   0x0004, // Scrolling menu.
    node_type_drawable =     0x0008, // Avec texture, mesh, et Instance Uniforms comme data.
    node_type_frame =        0x0010, // Cas particulier de drawable, cadre d'un autre noeud.
    node_type_root =         0x0020,
    node_type_view =         0x0040,
    node_type_drawMulti =    0x0080,
    node_type_number =       0x0100,
    node_type_string =       0x0200,
    node_type_poping =       0x0400,
    
    
    /// Premier "type flag" custom pour un noeud.
    /// Seulement besoin d'un "type flag" si la sous-struct de Node a besoin d'etre casté.
    node_type_firstCustomType = 0x001000,
    node_types_defaultTypes =     0x0FFF,
    node_types_defaultDrawables = node_type_drawable|node_type_drawMulti,
    node_type__last_ =        0x80000000,  // (uint32_t)
    
} node_t;

#endif /* node_types_h */
