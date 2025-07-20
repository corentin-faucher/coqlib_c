//
//  button_icon.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#ifndef COQ_NODE_BUTTON_H
#define COQ_NODE_BUTTON_H

#include "node_poping.h"
#include "node_string.h"
#include "../utils/util_string.h"
#include "../utils/util_event.h"

// MARK: - Base de Bouton

// Boutons, slider, objet deplacable...
typedef struct coq_Button Button;
typedef struct coq_Button {
    union {  // Upcasting...
        Node   n;
        Fluid  f;  // A priori, un bouton est "fluide", peut bouger de façon smooth.
    };
    /*-- Activation! --*/
    /// Doit être definie/overridé.
    
    void (*touch)(NodeTouch);
    // (les autres méthodes sont optionnelles)
    /*-- Deplacement (quand on garde le click enfoncé sur le bouton et on se déplace) --*/
    void (*dragOpt)(NodeTouch);
    void (*letGoOpt)(NodeTouch);
    void (*draggableActionOpt)(NodeTouch); // Action optionnel d'un draggable.
    /*--Survole --*/
    void (*startHoveringOpt)(Button* b);
    void (*stopHoveringOpt)(Button* b);
} Button;

Button* Button_create(Node* refOpt, void (*touch)(NodeTouch),
                      float x, float y, float height, float lambda, flag_t flags);
/// Sub-struct init.
void    button_init(Button* b, void (*touch)(NodeTouch));

/// Reprendre le dernier bouton créé.
Button* const Button_getLastOpt(void);

/// Downcasting
static inline Button* node_asActiveButtonOpt(Node *const n) {
    return (n && (n->_type & node_type_button) && !(n->flags & flag_buttonInactive)) ?
         (Button*)n : NULL;
}
static inline Button* node_asButtonOpt(Node *const nOpt) {
    return (nOpt && (nOpt->_type & node_type_button)) ? (Button*)nOpt : NULL;
}

// Convenience setters
//void    button_last_setData(union ButtonData data);
//void    button_last_setDataUint0(uint32_t data_uint0);

// MARK: - Switch ON/OFF
/// Bouton ON/OFF. La valeur ON/OFF est le premier bit de `uint_arr[11]`.
Button* Button_createSwitch(Node* refOpt, void (*action)(NodeTouch), bool isOn,
                            float x, float y, float height, float lambda, flag_t flags);
void    button_switch_set(Button* b, bool isOn);
bool    button_switch_value(Button* b);
Button* Button_createDummySwitch(Node* refOpt, void (*touchOpt)(NodeTouch), uint32_t data,
                                 float x, float y, float height, float lambda, flag_t flags);
// MARK: - Slider ---
/// Bouton de "fine-tuning". la valeur du slider est dans `float_arr[11]` et varie de
/// 0 (gauche) à 1 (droite).
Button* Button_createSlider(Node* refOpt, void (*action)(NodeTouch),
                            float value, float x, float y, float width, float height,
                            float lambda, flag_t flags);
float   button_slider_value(Button* b);
/// Pour les sous-struct qui veulent overrider le drag du slider.
void    button_slider_drag_(NodeTouch bt);

// MARK: - Secure button with pop over.

typedef struct  {
    float          holdTimeSec;
    uint32_t       popPngId;
    uint32_t       popTile;
    uint32_t       failPopFramePngId;
    StringGlyphedInit failMessage;
} SecurePopInfo;

Button* ButtonSecureHov_create(Node* refOpt, void (*action)(NodeTouch),
                SecurePopInfo spi, uint32_t popFramePngId, StringGlyphedInit popMessage,
                float x, float y, float height, float lambda, flag_t flags);

/// Juste secure pas de pop-over lors du survol.
Button* ButtonSecure_create(Node* refOpt, void (*action)(NodeTouch),
                SecurePopInfo spi, float x, float y, float height, float lambda, flag_t flags);
/// Juste hoverable (pop-over) pas de secure (hold to activate).
Button* ButtonHoverable_create(Node* refOpt, void (*touch)(NodeTouch),
                uint32_t popFramePngId, StringGlyphedInit popMessage,
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
    StringGlyphedInit popMessage;
    // Secure
    SecurePopInfo spi;    // Info supplementaire pour un bouton secure. (plus commode en un packet)
    PopingNode_*  poping; // Ref au poping disk de progres (pour cancel)
    NodeTouch     touch;
    bool          didActivate;
} ButtonSecureHov_;

//void buttonsecurehov_initJustSecure_(ButtonSecureHov_* bsh, SecurePopInfo spi);
void buttonsecurehov_initHoverable_(ButtonSecureHov_* bsh, uint32_t popFramePngId, 
                                    StringGlyphedInit popMessage);

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
