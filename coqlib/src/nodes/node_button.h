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
#include "node_string.h"

#pragma mark - Base de Bouton

// Boutons, slider, objet deplacable...
typedef struct coq_Button Button;
typedef struct coq_Button {
    union {  // Upcasting...
        Node   n;
        Fluid  f;  // A priori, un bouton est "fluide", peut bouger de façon smooth.
    };
    /*-- Activation! --*/
    /// Doit être definie/overridé.
    void (*action)(Button* b);
    // (les autres méthodes sont optionnelles)
    /*-- Deplacement --*/
    // Le vecteur est la position relative au button, i.e. dans le ref du bouton.
    void (*grabOpt)(Button* b, Vector2 relPos);
    void (*dragOpt)(Button* b, Vector2 relPos);
    void (*letGoOpt)(Button* b);
    /*--Survole --*/
    void (*startHoveringOpt)(Button* b);
    void (*stopHoveringOpt)(Button* b);
} Button;

Button* Button_create(Node* refOpt, void (*action)(Button*),
                      float x, float y, float height, float lambda, flag_t flags);
/// Sub-struct init.
void    button_init(Button* b, void (*action)(Button*));

/// Reprendre le dernier bouton créé.
Button* const Button_getLastOpt(void);

/// Downcasting
Button* node_asActiveButtonOpt(Node* n);
Button* node_asButtonOpt(Node* n);

// Convenience setters
//void    button_last_setData(union ButtonData data);
//void    button_last_setDataUint0(uint32_t data_uint0);

#pragma mark - Switch ON/OFF
/// Bouton ON/OFF. La valeur ON/OFF est le premier bit de `uint_arr[11]`.
Button* Button_createSwitch(Node* refOpt, void (*action)(Button*), bool isOn,
                            float x, float y, float height, float lambda, flag_t flags);
void    button_switch_set(Button* b, bool isOn);
bool    button_switch_value(Button* b);
Button* Button_createDummySwitch(Node* refOpt, void (*action)(Button*), uint32_t data,
                                 float x, float y, float height, float lambda, flag_t flags);
#pragma mark - Slider ---
/// Bouton de "fine-tuning". la valeur du slider est dans `float_arr[11]` et varie de
/// 0 (gauche) à 1 (droite).
Button* Button_createSlider(Node* refOpt, void (*action)(Button*),
                            float value, float x, float y, float width, float height,
                            float lambda, flag_t flags);
float   button_slider_value(Button* b);
/// Pour les sous-struct qui veulent overrider le drag du slider.
void    button_slider_drag_(Button* b, Vector2 pos_rel);

#pragma mark - Secure button with pop over.

typedef struct  {
    float          holdTimeSec;
    uint32_t       popPngId;
    uint32_t       popTile;
    uint32_t       failPopFramePngId;
    NodeStringInit failMessage;
} SecurePopInfo;

Button* ButtonSecureHov_create(Node* refOpt, void (*action)(Button*),
                SecurePopInfo spi, uint32_t popFramePngId, NodeStringInit popMessage,
                float x, float y, float height, float lambda, flag_t flags);

/// Juste secure pas de pop-over lors du survol.
Button* ButtonSecure_create(Node* refOpt, void (*action)(Button*),
                SecurePopInfo spi, float x, float y, float height, float lambda, flag_t flags);
/// Juste hoverable (pop-over) pas de secure (hold to activate).
Button* ButtonHoverable_create(Node* refOpt, void (*action)(Button*),
                uint32_t popFramePngId, NodeStringInit popMessage,
                float x, float y, float height, float lambda, flag_t flags);
//void    buttonhoverable_last_setToPopInFrontView(void);



// La structure (plus ou moins `privé`), pour si on veut faire une sous-struct.
typedef struct ButtonSecureHov_ {
    union {  // Upcasting...
        Node      n;
        Fluid     f;
        Button    b;
    };
    Timer         timer;
    // Hoverable
    uint32_t       popFramePngId;
    NodeStringInit popMessage;
    // Secure
    SecurePopInfo spi;    // Info supplementaire pour un bouton secure. (plus commode en un packet)
    PopingNode_*  poping; // Ref au poping disk de progres (pour cancel)
    bool          didActivate;
} ButtonSecureHov_;

void buttonsecurehov_initJustSecure_(ButtonSecureHov_* bsh, SecurePopInfo spi);
void buttonsecurehov_initJustHoverable_(ButtonSecureHov_* bsh, uint32_t popFramePngId, 
                                        NodeStringInit popMessage);

//extern Button* button_last_;
// Finalement superflu -> inclus dans Node.
/// Donnée par defaut pour un bouton : 16 bytes/4 float a utiliser comme on veut.
/// (évite de faire des sous-struct pour de simple boutons.)
//typedef union ButtonData {
//    char     char_arr[16];
//    uint32_t uint_arr[4];
//    struct {
//        uint32_t uint0, uint1, uint2, uint3;
//    };
//    float    float_arr[4];
//    struct {
//        float    float0, float1, float2, float3;
//    };
//    struct {
//        Node* node0;
//        Node* node1;
//    };
//} ButtonData;

#endif /* button_icon_h */
