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
    // e.g. si node->_type contient node_type_flag_fluid
    //     -> on sait que ce noeud est castable comme Fluid.
    // node_type_flag_node =      0x0000, pass, tout les noeuds sont des noeud :P
    node_type_flag_fluid =        0x0001, // Avec smooth x, y, etc.
    node_type_flag_button =       0x0002, // Avec action, drag, etc.
    node_type_flag_scrollable =   0x0004, // Scrolling menu.
    node_type_flag_drawable =     0x0008, // Avec texture, mesh, et Instance Uniforms comme data.
    node_type_flag_frame =        0x0010, // Cas particulier de drawable, cadre d'un autre noeud.
    node_type_flag_root =         0x0020,
    node_type_flag_view =         0x0040,
    node_type_flag_secHov =       0x0080,
    node_type_flag_drawMulti =    0x0100,
    node_type_flag_number =       0x0200,
    node_type_flag_poping =       0x0400,
    
//  #warning Enlever ? Complique les choses pour rien. Tout est non copyable par defaut...
//    node_type_flag_notCopyable =  0x0100, // Contien des refs de descendants, string, etc.
//                                          // i.e. besoin d'une fonction clone dédié.
//    node_type_flag_leaf =         0x0400, // Utile ?
    /// Premier "type flag" custom pour un noeud.
    /// Seulement besoin d'un "type flag" si la sous-struct de Node a besoin d'etre casté.
    node_type_firstCustomFlag = 0x001000,
    node_type_flags_defaultTypes =0x0FFF,
    node_type_lastFlag =      0x80000000,  // (uint32_t)
    
    /*-- Les type, i.e. combinaisons de type_flag. --*/
    node_type_node =              0x0000, // node ordinaire... pas besoin d'etre downcasté.
    // Les types `fluid` (bouge de façon smooth)
    node_type_n_fluid =       node_type_flag_fluid,
    node_type_nf_poping =     node_type_flag_fluid|node_type_flag_poping,
    node_type_nf_button =     node_type_flag_fluid|node_type_flag_button,
    node_type_nfb_secHov =    node_type_flag_fluid|node_type_flag_button|node_type_flag_secHov,
    node_type_nf_view =       node_type_flag_fluid|node_type_flag_view,
    // Les types drawables
    node_type_n_drawable =    node_type_flag_drawable,
    node_type_nd_frame =      node_type_flag_drawable|node_type_flag_frame,
    node_type_nd_multi =      node_type_flag_drawable|node_type_flag_drawMulti,
    node_type_ndm_number =    node_type_flag_drawable|node_type_flag_drawMulti|node_type_flag_number,
    // Autres
    node_type_scrollable =    node_type_flag_scrollable,
    node_type_root =          node_type_flag_root,
    
} node_t;

#endif /* node_types_h */
