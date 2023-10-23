//
//  node_flags.h
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
    _flag_deleted =                     1<<6,
    
    flag_rootOfToDisplay =              1<<7,
    flag_rootOfSelectable =             1<<8,
    flag_rootOfReshapable =             1<<9,
    
    flag_giveSizeToBigbroFrame =        1<<10,
    flag_giveSizeToParent =             1<<11,
    
    flag_surfaceIsFrame =               1<<12,
    flag_surfaceFrameOfParent =         1<<13,
    flag_surfaceDontRespectRatio =      1<<14,
    flag_screenDontAlignElements =      1<<16,
    flag_screenPersistent =             1<<17,
    
    /*-- Les flags de positionnement du noeud. Doivent être fournis à la création du noeud smooth. --*/
    flag_smoothRelativeToRight =        1UL<<20,
    flag_smoothRelativeToLeft =         1UL<<21,
    flag_smoothRelativeToTop =          1UL<<22,
    flag_smoothRelativeToBottom =       1UL<<23,
    flag_smoothJustifiedRight =         1UL<<24,
    flag_smoothJustifiedLeft =          1UL<<25,
    flag_smoothJustifiedTop =           1UL<<26,
    flag_smoothJustifiedBottom =        1UL<<27,
    flag_smoothFadeInRight =            1UL<<28,
    flag_firstCustomFlag =              1UL<<32,
    flag_smoothRelativeFlags =
        flag_smoothRelativeToTop|flag_smoothRelativeToBottom|
        flag_smoothRelativeToLeft|flag_smoothRelativeToRight|
        flag_smoothJustifiedTop|flag_smoothJustifiedBottom|
        flag_smoothJustifiedLeft|flag_smoothJustifiedRight,
} flag_t;



#endif /* node_flags_h */
