//
//  node_types.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-20.
//

#ifndef node_types_h
#define node_types_h

//typedef uint8_t node_t;
// Les grandes catégorie de noeuds.
typedef enum {
    node_type_bare =            0x0000,
    node_type_smooth =          0x0001,
    node_type_selectable =      0x0002,
    node_type_surface =         0x0004,
    node_type_root =            0x0008,
    node_type_screen =          0x0010,
    node_type_firstCustomType = 0x0020,
} node_t;

#endif /* node_types_h */
