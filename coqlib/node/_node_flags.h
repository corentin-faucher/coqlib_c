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
    flag_show =                         1<<1,
    flag_hidden =                       1<<2,
    flag_exposed =                      1<<3,
    flag_notToAlign =                   1<<4,
    flag_poping =                       1<<5,
    _flag_toDelete =                    1<<6,
    
    // Flags pour savoir quelle branches où chercher.
    flag_parentOfToDisplay =            1<<7,
    flag_parentOfButton =               1<<8,
    flag_parentOfScrollable =           1<<9,
    flag_parentOfReshapable =           1<<10,
    
    // Action au resize.
    flag_giveSizeToBigbroFrame =        1<<11,
    flag_giveSizeToParent =             1<<12,
    
    // Pour les drawables.
    flag_drawableDontRespectRatio =      1<<16,
    
    // Pour les views.
    flag_viewDontAlignElements =         1<<17,
    flag_viewPersistent =                1<<18,
    
    // Pour les boutons.
    flag_buttonInactive =                1<<19,
    
    /*-- Les flags de positionnement du noeud. Doivent être fournis à la création du noeud smooth. --*/
    flag_fluidRelativeToRight =        1UL<<20,
    flag_fluidRelativeToLeft =         1UL<<21,
    flag_fluidRelativeToTop =          1UL<<22,
    flag_fluidRelativeToBottom =       1UL<<23,
    flag_fluidJustifiedRight =         1UL<<24,
    flag_fluidJustifiedLeft =          1UL<<25,
    flag_fluidJustifiedTop =           1UL<<26,
    flag_fluidJustifiedBottom =        1UL<<27,
    flag_fluidFadeInRight =            1UL<<28,
    
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
    
    flag_rootDefaultFlags = flag_exposed|flag_show|flag_parentOfToDisplay|flag_parentOfButton|
        flag_parentOfScrollable|flag_parentOfReshapable,
    
    flag_firstCustomFlag =              1UL<<32,
} flag_t;



#endif /* node_flags_h */
