//
//  button_icon.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#ifndef COQ_NODE_BUTTON_H
#define COQ_NODE_BUTTON_H

#include "graphs/graph_texture.h"
#include "node_fluid.h"
#include "node_pop_disk.h"
#include "coq_timer.h"

// Boutons, slider, objet deplacable...
typedef struct _Button {
    union {  // Upcasting...
        Node   n;
        Fluid  f;  // A priori, un bouton est "fluide", peut bouger de façon smooth.
    };
    /// Référence à la racine de la structure de l'app.
    /// (Où s'applique généralement l'action d'un bouton...)
    Root*       root;
    Prefs**     prefsRef;
    /*-- Activation! --*/
    void (*action)(Button*);         // Doit etre definie/override.
    // <- Jusqu'ici la structure est +/- commune à View...
    //    i.e. Un button peut être caster comme une view pour root, prefs, action.
    
    // Donnée par defaut pour un bouton
    // (évite de faire des sous-struct pour de simple boutons).
    union {
        uint32_t uint32Data;
        int32_t  int32Data;
        float    floatData;
        char     char4Data[4];
    };
//    Button**   referer;  // Utile ?
    
    /*-- Deplacement --*/
    // Le vecteur est la position relative au button, i.e. dans le ref du bouton.
    void (*grabOpt)(Button*, Vector2);  // (NULL par defaut)
    void (*dragOpt)(Button*, Vector2);
    void (*letGoOpt)(Button*);
    /*--Survole --*/
    void (*startHoveringOpt)(Button*);  // (NULL par defaut,
    void (*stopHoveringOpt)(Button*);   //  voir ButtonHov pour exemple.)
//    void (*showPop)
} Button;

Button* Button_create(Node* refOpt, Root* root, void (*action)(Button*),
                      float x, float y, float height, float lambda, flag_t flags);
/// Sub-struct init.
void    _button_init(Button* b, Root* root, void (*action)(Button*),
                     float x, float y, float height, float lambda);
/// Downcasting
Button* node_asActiveButtonOpt(Node* n);



/*-- Exemple de Hoverable button ----------------------------------------------*/
#define BUTTONHOV_MESSAGE_COUNT 32
//typedef struct {
//    uint32_t      popFramePngId;
//    char          popMessage[BUTTONHOV_MESSAGE_COUNT];
//    bool          popMessageLocalized;
//} HoverablePopInfo;
typedef struct _ButtonHov {
    union {  // Upcasting...
        Node      n;
        Fluid     f;
        Button    b;
    };
    uint32_t      popFramePngId;
    UnownedString popMessage;
    Timer*        timer;
} ButtonHov;
/// Modele de bouton avec survole (avec une string qui pop)
ButtonHov* ButtonHoverable_create(Node* refOpt, Root* root, void (*action)(Button*),
                                  uint32_t popFramePngId, UnownedString popMessage,
                                  float x, float y, float height,
                                  float lambda, flag_t flags);
void       _buttonhoverable_init(ButtonHov* h, Root* root, void (*action)(Button*),
                                 uint32_t popFramePngId, UnownedString popMessage,
                                 float x, float y, float height,
                                 float lambda);

/*-- Exemple de Secure button -------------------------------------------------*/
typedef struct  {
    float    holdTimeSec;
    uint32_t popPngId;
    uint32_t popTile;
    uint32_t failPopFramePngId;
    UnownedString failMessage;
} SecurePopInfo;
typedef struct _ButtonSecure {
    union {  // Upcasting...
        Node      n;
        Fluid     f;
        Button    b;
    };
    SecurePopInfo spi; // Info supplementaire pour un bouton secure. (plus commode en un packet)
    Timer*        timer;
    PopDisk*      pop;
} ButtonSecure;


ButtonSecure* ButtonSecure_create(Node* refOpt,
                  Root* root, void (*action)(Button*), SecurePopInfo spi,
                  float x, float y, float height, float lambda, flag_t flags);





#endif /* button_icon_h */
