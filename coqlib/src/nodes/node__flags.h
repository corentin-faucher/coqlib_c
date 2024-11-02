//
//  _node_flags.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_NODE__FLAGS_H
#define COQ_NODE__FLAGS_H

#include <stdint.h>

// Flags éditable par la thread du renderer.
enum {
    renderflag_toDraw =           1,
    renderflag_firstCustom =   1<<1,
};

typedef uint64_t flag_t;
// Flags ordinaire de noeud (bits de `Node.flags`)
enum {
    /// Noeud initialisé dans le "vide", sans attache (un peu risqué).
    /// Mettre ce flag pour éviter d'avoir un warning.
    flag_noParent =                     1ULL,
    /// Mettre ce flag pour enregistrer ce noeud comme `Node_lastNode`.
    flag_nodeLast =                     1ULL<<15,
    flags_initFlags = flag_noParent|flag_nodeLast, // Des flags utiles qu'à l'init.
    
    /// Noeud/branche devant être affiché.
    flag_show =                         1ULL<<1,
    /// Le flag "show" n'est pas ajouté lors de l'ouverture de la branche (`node_tree_openAndShow`).
    flag_hidden =                       1ULL<<2,
    /// Le flag "show" n'est pas retiré lors de la fermeture de la branche (`node_tree_close`).
    flag_exposed =                      1ULL<<3,
    /// Skippé lors de l'alignement avec `node_tree_alignTheChildren`.
    flag_notToAlign =                   1ULL<<4,
    /// Le noeud est dans la poubelle... Quittez le navire !
    flag_toDelete_ =                    1ULL<<6,
//    flag_toDeleteScheduled_ =           1ULL<<7, // Sera bientôt dans la poubelle.
    
    // Flags pour savoir quelle branches où chercher.
//    flag_parentOfToDraw =               1ULL<<8,  Branche ayant des noeud `show` ou `drawableActive`.
    flag_parentOfButton =               1ULL<<9,
    flag_parentOfScrollable =           1ULL<<10,
    flag_parentOfReshapable =           1ULL<<11,
    
    // Action au resize.
    flag_giveSizeToBigbroFrame =        1ULL<<12,
    flag_giveSizeToParent =             1ULL<<13,
    
    // Drawables
    /// Drawable encore actif (meme si show est off, transition pas encore fini).
//    flag_drawableActive =                1ULL<<14,
    /// Qui apparaît en grossissant.
    flag_drawablePoping =                1ULL<<5,
    
    // Root
    /// La matrice projection et view sont séparé. (pour calculs sur vecteur normal dans shader par exemple)
    flags_rootDefault = flag_exposed|flag_show|flag_parentOfButton|
        flag_parentOfScrollable|flag_parentOfReshapable|flag_noParent,
        
    // 1ULL<<17,
    
    // Pour les views.
    /// Les "blocs" (premiers descendants) ne sont pas alignées en fonction de la dimention de la vue principale/fenêtre.
    flag_viewDontAlignElements =         1ULL<<18,
    /// Pas automatiquement jetté à la poubelle (par defaut, quand on quitte une view, elle est détruite)
    flag_viewPersistent =                1ULL<<19,
    /// Defaut pour l'avant-plan et l'arrière-plan, i.e. toujours présent et pas d'alignemment des blocs.
    flags_viewBackAndFrontDefault =      flag_viewPersistent|flag_viewDontAlignElements|flag_exposed|flag_show,
    
    // Pour les fluids
    /*-- Les flags de positionnement du noeud. Doivent être fournis à la création du noeud smooth. --*/
    // `Relative` au parent, e.g. si flag_fluidRelativeToTop -> se place en haut du cadre du parent.
    flag_fluidRelativeToRight =        1ULL<<20,
    flag_fluidRelativeToLeft =         1ULL<<21,
    flag_fluidRelativeToTop =          1ULL<<22,
    flag_fluidRelativeToBottom =       1ULL<<23,
    flag_fluidJustifiedRight =         1ULL<<24,
    flag_fluidJustifiedLeft =          1ULL<<25,
    flag_fluidJustifiedTop =           1ULL<<26,
    flag_fluidJustifiedBottom =        1ULL<<27,
    // Sera centré (-> 0) lors d'un setRelatively.
//    flag_fluidRelativeCentered =       1ULL<<28,
    
    flag_fluidFadeInRight =            1ULL<<29,
    
    flags_fluidRelatives = flag_fluidRelativeToTop|flag_fluidRelativeToBottom|
                           flag_fluidRelativeToLeft|flag_fluidRelativeToRight,
//                           flag_fluidRelativeCentered,
    flags_fluidJustifieds = flag_fluidJustifiedTop|flag_fluidJustifiedBottom|
                            flag_fluidJustifiedLeft|flag_fluidJustifiedRight,
    /// Flags ayaant besoin de  `fluid_reshape_` au reshape.
    flags_fluidReshape = flags_fluidRelatives|flags_fluidJustifieds,
    /// Flags ayant besoin de `fluid_open_` à l'open.
    flags_fluidOpen = flags_fluidReshape|flag_fluidFadeInRight,
    /// Flags ayant besoin de `fluid_close_` au close.
    flags_fluidClose = flag_fluidFadeInRight,
    
    // Pour les boutons.
    flag_buttonInactive =                1ULL<<30,
    
    // 1ULL<<31
    
    flag_firstCustomFlag =              1ULL<<32, // Assure d'avoir des uint64...
};



#endif /* node_flags_h */
