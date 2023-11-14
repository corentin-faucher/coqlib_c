//
//  button_icon.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#ifndef button_icon_h
#define button_icon_h

#include "fluid.h"
#include "graph_texture.h"
#include "timer.h"
#include "pop_disk.h"

// Boutons, slider, objet deplacable...
typedef struct _Button {
    union {  // Upcasting...
        Node   n;
        Fluid  f;
    };
    Root*      root;
//    Button**   referer;
    /*-- Activation! --*/
    void (*action)(Button*);         // Devrait etre definie.
    /*-- Deplacement --*/
    // Le vecteur est la position relative au button, i.e. dans le ref du bouton.
    void (*grab)(Button*, Vector2);  // (NULL par defaut)
    void (*drag)(Button*, Vector2);
    void (*letGo)(Button*);
    /*--Survole --*/
    void (*startHovering)(Button*);  // (NULL par defaut,
    void (*stopHovering)(Button*);   //  voir ButtonHov pour exemple.)
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
//    Bool          popMessageLocalized;
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
