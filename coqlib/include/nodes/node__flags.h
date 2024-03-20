//
//  _node_flags.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef COQ_NODE__FLAGS_H
#define COQ_NODE__FLAGS_H

//typedef uint64_t flag_t;
typedef enum {
    /// Noeud initialisé dans le "vide", sans attache (un peu risqué).
    /// Mettre ce flag pour éviter d'avoir un warning.
    flag_noParent =                     1ULL,
    /// Noeud/branche devant être affiché.
    flag_show =                         1ULL<<1,
    /// Le flag "show" n'est pas ajouté lors de l'ouverture de la branche (`node_tree_openAndShow`).
    flag_hidden =                       1ULL<<2,
    /// Le flag "show" n'est pas retiré lors de la fermeture de la branche (`node_tree_close`).
    flag_exposed =                      1ULL<<3,
    /// Skippé lors de l'alignement avec `node_tree_alignTheChildren`.
    flag_notToAlign =                   1ULL<<4,
    flag_poping =                       1ULL<<5,
    /// Heu, le noeud est dans la poubelle... Quittez le navire !
    flag_toDelete_ =                    1ULL<<6,
    
    // Flags pour savoir quelle branches où chercher.
    flag_parentOfToDisplay =            1ULL<<7,
    flag_parentOfButton =               1ULL<<8,
    flag_parentOfScrollable =           1ULL<<9,
    flag_parentOfReshapable =           1ULL<<10,
    
    // Action au resize.
    flag_giveSizeToBigbroFrame =        1ULL<<12,
    flag_giveSizeToParent =             1ULL<<13,
    
    // Pour les drawables.
    /// Il faut fournir la largeur, la largeur n'est pas prise avec le ratio w/h du png d'origine.
    flag_drawableDontRespectRatio =      1ULL<<15,
    
    // Pour les views.
    /// Les "blocs" (premiers descendants) ne sont pas alignées en fonction de la dimention de la vue principale/fenêtre.
    flag_viewDontAlignElements =         1ULL<<17,
    /// Pas automatiquement jetté à la poubelle (par defaut, quand on quitte une view, elle est détruite)
    flag_viewPersistent =                1ULL<<18,
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
    
    flag_fluidFadeInRight =            1ULL<<28,
    
    flags_fluidRelatives = flag_fluidRelativeToTop|flag_fluidRelativeToBottom|
                           flag_fluidRelativeToLeft|flag_fluidRelativeToRight,
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
    
    // Pour les root
    flags_rootDefault = flag_exposed|flag_show|flag_parentOfToDisplay|flag_parentOfButton|
        flag_parentOfScrollable|flag_parentOfReshapable,

    
    flag_firstCustomFlag =              1ULL<<32,
} flag_t;



#endif /* node_flags_h */
