//
//  _node_flags.h
//  Test2
//
//  Created by Corentin Faucher on 2023-10-12.
//

#ifndef node_flags_h
#define node_flags_h

//typedef uint64_t flag_t;
typedef enum {
    /// Noeud/branche devant être affiché.
    flag_show =                         1ULL<<1,
    /// Le flag "show" n'est pas ajouté lors de l'ouverture de la branche (`node_tree_openAndShow`).
    flag_hidden =                       1ULL<<2,
    /// Le flag "show" n'est pas retiré lors de la fermeture de la branche (`node_tree_close`).
    flag_exposed =                      1ULL<<3,
    /// Skippé lors de l'alignement avec `node_tree_alignTheChildren`.
    flag_notToAlign =                   1ULL<<4,
    flag_poping =                       1ULL<<5,
    _flag_toDelete =                    1ULL<<6,
    
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
    flag_viewBackAndFrontDefaultFlags = flag_viewPersistent|flag_viewDontAlignElements|flag_exposed|flag_show,
    
    // Pour les fluids
    /*-- Les flags de positionnement du noeud. Doivent être fournis à la création du noeud smooth. --*/
    flag_fluidRelativeToRight =        1ULL<<20,
    flag_fluidRelativeToLeft =         1ULL<<21,
    flag_fluidRelativeToTop =          1ULL<<22,
    flag_fluidRelativeToBottom =       1ULL<<23,
    flag_fluidJustifiedRight =         1ULL<<24,
    flag_fluidJustifiedLeft =          1ULL<<25,
    flag_fluidJustifiedTop =           1ULL<<26,
    flag_fluidJustifiedBottom =        1ULL<<27,
    flag_fluidFadeInRight =            1ULL<<28,
    
    flag_fluidRelativeFlags =
        flag_fluidRelativeToTop|flag_fluidRelativeToBottom|
        flag_fluidRelativeToLeft|flag_fluidRelativeToRight|
        flag_fluidJustifiedTop|flag_fluidJustifiedBottom|
        flag_fluidJustifiedLeft|flag_fluidJustifiedRight,
    /// Besoin de la methode fluid_open_default
    flag_fluidOpenFlags = flag_fluidRelativeFlags|flag_fluidFadeInRight,
    /// Besoin de la methode fluid_close_default
    flag_fluidCloseFlags = flag_fluidFadeInRight,
    /// Besoin de la methode fluid_reshape_relatively
    flag_fluidReshapeFlags = flag_fluidRelativeFlags,
    
    // Pour les boutons.
    flag_buttonInactive =                1ULL<<30,
    
    // Pour les root
    flag_rootDefaultFlags = flag_exposed|flag_show|flag_parentOfToDisplay|flag_parentOfButton|
        flag_parentOfScrollable|flag_parentOfReshapable,

    
    flag_firstCustomFlag =              1ULL<<32,
} flag_t;



#endif /* node_flags_h */
