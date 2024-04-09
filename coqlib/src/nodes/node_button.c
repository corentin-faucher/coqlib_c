//
//  button_icon.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//
#include "node_button.h"

#include "node_drawable.h"
#include "node_root.h"
#include "node_tree.h"
#include "../graphs/graph_colors.h"
#include "../utils/utils_base.h"
#include <string.h>


void   button_default_action_(Button* b) {
    printwarning("Button %p action not overrided.", b);
}
static Button* button_last_ = NULL;
void    button_init_(Button* b, void (*action)(Button*)) {
    if(action)
        b->action = action;
    else {
        printwarning("No action for button!");
        b->action = button_default_action_;
    }
    node_tree_addRootFlags(&b->n, flag_parentOfButton);
    button_last_ = b;
}
Button* Button_create(Node* refOpt, void (*action)(Button*),
                                  float x, float y, float height, float lambda, flag_t flags) {
    Button* b = coq_calloc(1, sizeof(Button));
    node_init_(&b->n, refOpt, x, y, height, height, node_type_nf_button, flags, 0);
    fluid_init_(&b->f, lambda);
    button_init_(b, action);
    return b;
}
Button* node_asActiveButtonOpt(Node* n) {
    if(n->_type & node_type_flag_button) if(!(n->flags & flag_buttonInactive))
        return (Button*)n;
    return NULL;
}
Button* node_asButtonOpt(Node* n) {
    if(n->_type & node_type_flag_button)
        return (Button*)n;
    return NULL;
}
void    button_last_setData(ButtonData data) {
    if(!button_last_) { printerror("No last button."); return; }
    button_last_->data = data;
}
void    button_last_setDataUint0(uint32_t data_uint0) {
    if(!button_last_) { printerror("No last button."); return; }
    button_last_->data.uint0 = data_uint0;
}
//void    button_last_overrideAction(void (*newAction)(Button*)) {
//    if(!button_last_) { printerror("No last button."); return; }
//    button_last_->action = newAction;
//}
Button* const Button_getLastOpt(void) {
    return button_last_;
}

enum {
    button_switch_flag_isOn =     0x01,
    button_switch_flag_didDrag =  0x02,
    button_switch_flag_inverted = 0x04,
};
void button_switch_grab_(Button* b, Vector2 pos_init) {
    b->data.uint3 &= ~ button_switch_flag_didDrag;
}
/** Déplacement en cours du "nub", aura besoin de letGoNub.
* newX doit être dans le ref. du SwitchButton.
* Effectue l'action si changement d'état. */
void button_switch_drag_(Button* b, Vector2 pos_rel) {
    Fluid* nub = node_asFluidOpt(b->n._lastChild);
    Drawable* back = node_asDrawableOpt(b->n._firstChild);
    if(!nub || !back) { printerror("Switch without nub or back."); return; }
    // 1. Ajustement de la position du nub.
    fluid_setX(nub, fminf(fmaxf(pos_rel.x, -0.375), 0.375), false);
    // 2. Vérif si changement
    bool oldIsOn = (b->data.uint3 & button_switch_flag_isOn) != 0;
    bool newIsOn = pos_rel.x >= 0;
    if(oldIsOn != newIsOn) {
        if(newIsOn)
            b->data.uint3 |= button_switch_flag_isOn;
        else
            b->data.uint3 &= ~ button_switch_flag_isOn;
        back->n._piu.color = newIsOn ? color4_green_electric : color4_red_vermilion;
        b->action(b);
    }
    b->data.uint3 |= button_switch_flag_didDrag;
}
void button_switch_letGo_(Button* b) {
    Fluid* nub = node_asFluidOpt(b->n._lastChild);
    Drawable* back = node_asDrawableOpt(b->n._firstChild);
    if(!nub || !back) { printerror("Switch without nub or back."); return; }
    // Pas dragé ? suppose simple touche pour permuter
    bool isOn;
    if(!(b->data.uint3 & button_switch_flag_didDrag)) {
        isOn = !(b->data.uint3 & button_switch_flag_isOn);
        if(isOn)
            b->data.uint3 |= button_switch_flag_isOn;
        else
            b->data.uint3 &= ~ button_switch_flag_isOn;
        back->n._piu.color = isOn ? color4_green_electric : color4_red_vermilion;
        b->action(b);
    } else {
        isOn = b->data.uint3 & button_switch_flag_isOn;
    }
    fluid_setX(nub, isOn ? 0.375 : -0.375, false);
}
Button* Button_createSwitch(Node* refOpt, void (*action)(Button*), bool isOn,
                            float x, float y, float height, float lambda, flag_t flags) {
    Button* b = coq_calloc(1, sizeof(Button));
    node_init_(&b->n, refOpt, x, y, 2, 1, node_type_nf_button, flags, 0);
    b->n.sx = height;
    b->n.sy = height;
    fluid_init_(&b->f, lambda);
    button_init_(b, action);
    // Init as switch
    b->data.uint3 = isOn ? 1 : 0;
    // La switch est un bouton "draggable"...
    b->grabOpt = button_switch_grab_;
    b->dragOpt = button_switch_drag_;
    b->letGoOpt = button_switch_letGo_;
    // Structure de switch
    Drawable* back = Drawable_createImageWithName(&b->n, "coqlib_switch_back", 0, 0, 1, 0);
    back->n._piu.color = isOn ? color4_green_electric : color4_red_vermilion;
    Fluid* nub = Fluid_create(&b->n, isOn ? 0.375 : -0.375, 0, 1, 1, 10, 0, 0);
    Drawable_createImageWithName(&nub->n, "coqlib_switch_front", 0, 0, 1, 0);
    
    return b;
}
void button_switch_fix(Button* b, bool isOn) {
    Drawable* back = node_asDrawableOpt(b->n._firstChild);
    if(!back) { printerror("Switch without back."); return; }
    if(isOn)
        b->data.uint3 |= button_switch_flag_isOn;
    else
        b->data.uint3 &= ~ button_switch_flag_isOn;
    back->n._piu.color = isOn ? color4_green_electric : color4_red_vermilion;
}

Button* Button_createDummySwitch(Node* refOpt, void (*action)(Button*), uint32_t data,
                                 float x, float y, float height, float lambda, flag_t flags) {
    Button* b = coq_calloc(1, sizeof(Button));
    node_init_(&b->n, refOpt, x, y, 2, 1, node_type_nf_button, flags, 0);
    b->n.sx = height;
    b->n.sy = height;
    fluid_init_(&b->f, lambda);
    button_init_(b, action);
    
    // Structure de switch
    Drawable_createImageWithName(&b->n, "coqlib_switch_back", 0, 0, 1, 0);
    drawable_last_setColor(color4_gray2);
    Drawable_createImageWithName(&b->n, "coqlib_switch_front", 0, 0, 1, 0);
    drawable_last_setColor(color4_gray3);
         
    return b;
}

void button_slider_grab_(Button* b, Vector2 pos_init) {
    // pass
}
void button_slider_drag_(Button* b, Vector2 pos_rel) {
    Fluid* nub = node_asFluidOpt(b->n._lastChild);
    float half_w = 0.5*fmaxf(b->n.w - b->n.h, b->n.h);
    // 1. Ajustement de la position du nub (et valeur du slider).
    float nub_x = fminf(fmaxf(pos_rel.x, -half_w), half_w);
    float value = 0.5f*(nub_x / half_w + 1.f);
    fluid_setX(nub, nub_x, false);
    b->data.float3 = value;
    // b->action(b); // Non, en fait c'est juste au let go a priori.
    // Si on veut vraiment en peut redéfinir aussi le drag.
}
void button_slider_letGo_(Button* b) {
    b->action(b);
}
Button* Button_createSlider(Node* refOpt, void (*action)(Button*),
                            float value, float x, float y, float width, float height,
                            float lambda, flag_t flags) {
    // Check width
    width = fmaxf(width, 1.5f*height);
    float slide_width = width - height;
    // Init node, fluid, button
    Button* b = coq_calloc(1, sizeof(Button));
    node_init_(&b->n, refOpt, x, y, width, height, node_type_nf_button, flags, 0);
    fluid_init_(&b->f, lambda);
    button_init_(b, action);
    // Init as slider
    b->grabOpt = button_slider_grab_;
    b->dragOpt = button_slider_drag_;
    b->letGoOpt = button_slider_letGo_;
    value = fminf(fmaxf(value, 0.f), 1.f);
    b->data.float3 = value;
    // Structure
    Frame_createWithName(&b->n, 0, 0.25*height, slide_width, height,
                        "coqlib_bar_in", frame_option_horizotalBar);
    float nub_x = (value - 0.5f) * slide_width;
    Fluid* nub = Fluid_create(&b->n, nub_x, 0, height, height, 10, 0, 0);
    Drawable_createImageWithName(&nub->n, "coqlib_switch_front", 0, 0, height, 0);
    
    return b;
}

#pragma mark - Secure/Hoverable button

// As Hoverable... (Nom du bouton qui apparaaît)
void buttonhov_showPopMessage_callback_(Node* nd) {
    ButtonSecureHov_* hv = (ButtonSecureHov_*)nd;
    PopMessage_spawnOver(nd, 0, 0.4, 2.5, hv->popFramePngId, hv->popMessage, 
                         framedString_defPars, hv->popInFrontView);
}
void buttonhov_startHovering_(Button* b) {
    ButtonSecureHov_* h = (ButtonSecureHov_*)b;
    timer_cancel(&h->timer);
    timer_scheduled(&h->timer, 350, false, &h->n, buttonhov_showPopMessage_callback_);
}
void buttonhov_stopHovering_(Button* b) {
    ButtonSecureHov_* hv = (ButtonSecureHov_*)b;
    timer_cancel(&hv->timer);
}

// As secure (hold to activate)

void buttonsecure_action_callback_(Node* nd) {
    ButtonSecureHov_* bt = (ButtonSecureHov_*)nd;
    popdisk_cancel(&bt->pop);
    bt->b.action(&bt->b);
    bt->didActivate = true;
}
void buttonsecure_grab_(Button* sel, Vector2 pos) {
    ButtonSecureHov_* bt = (ButtonSecureHov_*)sel;
    bt->didActivate = false;
    popdisk_cancel(&bt->pop);
    PopDisk_spawn(&bt->n, &bt->pop, bt->spi.popPngId, bt->spi.popTile,
                  bt->spi.holdTimeSec, 0, 0, bt->n.h);
    timer_scheduled(&bt->timer, (int64_t)(bt->spi.holdTimeSec * 1000.f), false,
                    &bt->n, buttonsecure_action_callback_);
}
void buttonsecure_drag_(Button* sel, Vector2 pos) {
    // pass
}
void buttonsecure_letGo_(Button* but) {
    ButtonSecureHov_* bt = (ButtonSecureHov_*)but;
    popdisk_cancel(&bt->pop);
    timer_cancel(&bt->timer);
    if(bt->didActivate) return;
    PopMessage_spawnOver(&bt->n, 0, 0.5, 2.5, bt->spi.failPopFramePngId, bt->spi.failMessage, framedString_defPars, false);
}

void buttonsecurehov_deinit_(Node* n) {
    timer_cancel(&((ButtonSecureHov_*)n)->timer);
}
Button* ButtonSecureHov_create(Node* refOpt, void (*action)(Button*),
                SecurePopInfo spi, uint32_t popFramePngId, StringDrawable popMessage,
                float x, float y, float height, float lambda, flag_t flags) {
    ButtonSecureHov_ *sec = coq_calloc(1, sizeof(ButtonSecureHov_));
    node_init_(&sec->n, refOpt, x, y, height, height, node_type_nf_button, flags, 0);
    fluid_init_(&sec->f, lambda);
    button_init_(&sec->b, action);
    // Init des privates methods
    sec->b.startHoveringOpt = buttonhov_startHovering_;
    sec->b.stopHoveringOpt =  buttonhov_stopHovering_;
    sec->b.grabOpt =          buttonsecure_grab_;
    sec->b.dragOpt =          buttonsecure_drag_;
    sec->b.letGoOpt =         buttonsecure_letGo_;
    sec->n.deinitOpt =        buttonsecurehov_deinit_;
    sec->spi = spi;
    sec->popFramePngId = popFramePngId;
    sec->popMessage = popMessage;
    
    return &sec->b;
}
void buttonsecurehov_initJustSecure_(ButtonSecureHov_* bsh, SecurePopInfo spi) {
    bsh->b.grabOpt =          buttonsecure_grab_;
    bsh->b.dragOpt =          buttonsecure_drag_;
    bsh->b.letGoOpt =         buttonsecure_letGo_;
    bsh->n.deinitOpt =        buttonsecurehov_deinit_;
    bsh->spi =                spi;
}
void buttonsecurehov_initJustHoverable_(ButtonSecureHov_* bsh, uint32_t popFramePngId, StringDrawable popMessage) {
    bsh->b.startHoveringOpt = buttonhov_startHovering_;
    bsh->b.stopHoveringOpt =  buttonhov_stopHovering_;
    bsh->n.deinitOpt =        buttonsecurehov_deinit_;
    bsh->popFramePngId =      popFramePngId;
    bsh->popMessage =         popMessage;
}
/// Juste secure pas de pop-over lors du survol.
Button* ButtonSecure_create(Node* refOpt, void (*action)(Button*),
                            SecurePopInfo spi, float x, float y, float height, float lambda, flag_t flags) {
    ButtonSecureHov_ *sec = coq_calloc(1, sizeof(ButtonSecureHov_));
    node_init_(&sec->n, refOpt, x, y, height, height, node_type_nf_button, flags, 0);
    fluid_init_(&sec->f, lambda);
    button_init_(&sec->b, action);
    
    buttonsecurehov_initJustSecure_(sec, spi);
    
    return &sec->b;
}
/// Juste hoverable (pop-over) pas de secure (hold to activate).
Button* ButtonHoverable_create(Node* refOpt, void (*action)(Button*),
                uint32_t popFramePngId, StringDrawable popMessage,
                float x, float y, float height, float lambda, flag_t flags) {
    ButtonSecureHov_ *sec = coq_calloc(1, sizeof(ButtonSecureHov_));
    node_init_(&sec->n, refOpt, x, y, height, height, node_type_nfb_secHov, flags, 0);
    fluid_init_(&sec->f, lambda);
    button_init_(&sec->b, action);
    
    buttonsecurehov_initJustHoverable_(sec, popFramePngId, popMessage);
    
    return &sec->b;
}
void buttonhoverable_last_setToPopInFrontView(void) {
    if(!button_last_) { printerror("No last button."); return; }
    if(!(button_last_->n._type & node_type_flag_secHov)) {
        printerror("Not a secure/hoverable button."); return;
    }
    ((ButtonSecureHov_*)button_last_)->popInFrontView = true;
}
