//
//  _node_types.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-20.
//

#ifndef node_types_h
#define node_types_h

// Les node types sont sur 32 bits, i.e. uint32_t.
typedef enum {
    // Flag qui definissent les grandes categories de noeuds.
    // e.g. si node->_type contient node_type_flag_smooth
    //     -> on sait que ce noeud est castable comme Fluid.
    node_type_flag_fluid =        0x0001, // Avec smooth x, y, etc.
    node_type_flag_button =       0x0002, // Avec action, drag, etc.
    node_type_flag_scrollable =   0x0004, // Scrolling menu.
    node_type_flag_drawable =     0x0008, // Avec texture, mesh, etc.
    node_type_flag_frame =        0x0010, // Cas particulier de drawable, cadre d'un autre noeud.
    node_type_flag_root =         0x0020,
    node_type_flag_view =         0x0040,
    node_type_flag_number =       0x0080,
    node_type_flag_notCopyable =  0x0100, // Contien des refs de descendants, string, etc.
                                          // i.e. besoin d'une fonction clone dédié.
    node_type_flag_leaf =   0x0200, // i.e. leaf. Utile ?
    /// Premier "type flag" custom pour un noeud.
    /// Seulement besoin d'un "type flag" si la sous-struct de Node a besoin d'etre casté.
    node_type_firstCustomFlag = 0x001000,
    node_type_lastFlag =      0x80000000,  // (uint32_t)
    
    /*-- Les type, i.e. combinaisons de type_flag. --*/
    node_type_node =              0x0000, // node ordinaire... pas besoin d'etre downcasté.
    node_type_n_fluid = node_type_flag_fluid,
    node_type_leaf_drawable = node_type_flag_leaf|node_type_flag_drawable,
    node_type_dl_frame = node_type_flag_leaf|node_type_flag_drawable
        |node_type_flag_frame|node_type_flag_notCopyable,
    // Les boutons sont aussi smooth.
    node_type_nf_button = node_type_flag_fluid|node_type_flag_button
        |node_type_flag_notCopyable,
    node_type_scrollable = node_type_flag_scrollable|node_type_flag_notCopyable,
    node_type_root = node_type_flag_root|node_type_flag_notCopyable,
    node_type_nf_view = node_type_flag_view|node_type_flag_fluid|node_type_flag_notCopyable,
    node_type_n_number = node_type_flag_number|node_type_flag_notCopyable,
    
} node_t;

#endif /* node_types_h */
