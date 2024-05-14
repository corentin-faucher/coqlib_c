//
//  button_icon.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#ifndef COQ_NODE_BUTTON_H
#define COQ_NODE_BUTTON_H

#include "node_poping.h"
#include "../utils/util_string.h"

#pragma mark - Base de Bouton

/// Donnée par defaut pour un bouton : 16 bytes/4 float a utiliser comme on veut.
/// (évite de faire des sous-struct pour de simple boutons.)
typedef union ButtonData {
    char     char_arr[16];
    uint32_t uint_arr[4];
    struct {
        uint32_t uint0, uint1, uint2, uint3;
    };
    float    float_arr[4];
    struct {
        float    float0, float1, float2, float3;
    };
    struct {
        Node* node0;
        Node* node1;
    };
} ButtonData;

// Boutons, slider, objet deplacable...
typedef struct coq_Button Button;
typedef struct coq_Button {
    union {  // Upcasting...
        Node   n;
        Fluid  f;  // A priori, un bouton est "fluide", peut bouger de façon smooth.
    };
    /*-- Activation! --*/
    void (*action)(Button*);         // Doit etre definie/override.
    /// Donnée par defaut pour un bouton : 16 bytes/4 float a utiliser comme on veut.
    /// (évite de faire des sous-struct pour de simple boutons.)
    union ButtonData data;
    
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

Button* Button_create(Node* refOpt, void (*action)(Button*),
                      float x, float y, float height, float lambda, flag_t flags);
/// Sub-struct init.
void    button_init_(Button* b, void (*action)(Button*));

/// Reprendre le dernier bouton créé.
Button* const Button_getLastOpt(void);

/// Downcasting
Button* node_asActiveButtonOpt(Node* n);
Button* node_asButtonOpt(Node* n);

// Convenience setters
void    button_last_setData(union ButtonData data);
void    button_last_setDataUint0(uint32_t data_uint0);

#pragma mark - Switch ON/OFF et Slider
/// Bouton ON/OFF. La valeur ON/OFF est le premier bit de `uint3`.
Button* Button_createSwitch(Node* refOpt, void (*action)(Button*), bool isOn,
                            float x, float y, float height, float lambda, flag_t flags);
void    button_switch_fix(Button* b, bool isOn);
Button* Button_createDummySwitch(Node* refOpt, void (*action)(Button*), uint32_t data,
                                 float x, float y, float height, float lambda, flag_t flags);
/// Bouton de "fine-tuning". la valeur du slider est dans `float3` et varie de
/// 0 (gauche) à 1 (droite).
Button* Button_createSlider(Node* refOpt, void (*action)(Button*),
                            float value, float x, float y, float width, float height,
                            float lambda, flag_t flags);
/// Pour les sous-struct qui veulent overrider le drag du slider.
void    button_slider_drag_(Button* b, Vector2 pos_rel);

#pragma mark - Secure button with pop over.

typedef struct  {
    float          holdTimeSec;
    uint32_t       popPngId;
    uint32_t       popTile;
    uint32_t       failPopFramePngId;
    StringDrawable failMessage;
    bool           failPopInFrontScreen;
} SecurePopInfo;

Button* ButtonSecureHov_create(Node* refOpt, void (*action)(Button*),
                SecurePopInfo spi, uint32_t popFramePngId, StringDrawable popMessage,
                float x, float y, float height, float lambda, flag_t flags);

/// Juste secure pas de pop-over lors du survol.
Button* ButtonSecure_create(Node* refOpt, void (*action)(Button*),
                SecurePopInfo spi, float x, float y, float height, float lambda, flag_t flags);
/// Juste hoverable (pop-over) pas de secure (hold to activate).
Button* ButtonHoverable_create(Node* refOpt, void (*action)(Button*),
                uint32_t popFramePngId, StringDrawable popMessage,
                float x, float y, float height, float lambda, flag_t flags);
void    buttonhoverable_last_setToPopInFrontView(void);



// La structure (plus ou moins `privé`), pour si on veut faire une sous-struct.
typedef struct ButtonSecureHov_ {
    union {  // Upcasting...
        Node      n;
        Fluid     f;
        Button    b;
    };
    Timer*        timer;
    // Hoverable
    uint32_t       popFramePngId;
    StringDrawable popMessage;
    bool           popInFrontView;
    // Secure
    SecurePopInfo spi; // Info supplementaire pour un bouton secure. (plus commode en un packet)
    PopDisk*      pop;
    bool          didActivate;
} ButtonSecureHov_;

void buttonsecurehov_initJustSecure_(ButtonSecureHov_* bsh, SecurePopInfo spi);
void buttonsecurehov_initJustHoverable_(ButtonSecureHov_* bsh, uint32_t popFramePngId, StringDrawable popMessage);

//extern Button* button_last_;

#endif /* button_icon_h */
