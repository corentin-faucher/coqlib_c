//
//  button_icon.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//
#include "node_button.h"

#include "node_drawable.h"
#include "node_tree.h"
#include "../utils/util_base.h"
#include <string.h>

// MARK: - Button ------
void   button_default_touch_(NodeTouch const bt) {
    printwarning("Button %p action not overrided. Touching at %f, %f.", bt.n, bt.posAbs.x, bt.posAbs.y);
}
static Button* button_last_ = NULL;
void    button_init(Button* b, void (* const touch)(NodeTouch)) {
    b->n._type |= node_type_button;
    if(touch)
        b->touch = touch;
    else {
        printwarning("No action for button!");
        b->touch = button_default_touch_;
    }
    node_tree_addRootFlags(&b->n, flag_parentOfButton);
    button_last_ = b;
}
Button* Button_create(Node* refOpt, void (*const touch)(NodeTouch),
                                  float const x, float const y, float const height, 
                                  float const lambda, flag_t const flags) 
{
    Button* b = coq_callocTyped(Button);
    node_init(&b->n, refOpt, x, y, height, height, flags, 0);
    fluid_init(&b->f, lambda);
    button_init(b, touch);
    return b;
}

Button* const Button_getLastOpt(void) {
    return button_last_;
}

// Fonction dummy quand il n'y a rien à faire.
void    button_touchOrDrag_pass_(NodeTouch bt) {
    // (pass)
}

// MARK: - Switch ON/OFF ---------
enum {
    button_switch_flag_isOn =     0x01,
    button_switch_flag_didDrag =  0x02,
    button_switch_flag_inverted = 0x04,
};
void button_switch_touch_(NodeTouch const bt) {
    bt.n->nodrawData.data1.u3 &= ~ button_switch_flag_didDrag;
}
/** Déplacement en cours du "nub", aura besoin de letGoNub.
* newX doit être dans le ref. du SwitchButton.
* Effectue l'action si changement d'état. */
void button_switch_drag_(NodeTouch const bt) {
    guard_let(Button*, b, node_asButtonOpt(bt.n), printerror("No button."), )
    Fluid* nub = node_asFluidOpt(b->n._lastChild);
    Drawable* back = node_asDrawableOpt(b->n._firstChild);
    if(!nub || !back) { printerror("Switch without nub or back."); return; }
    Vector2 const posRel = nodetouch_evalPosRel(bt);
    // 1. Ajustement de la position du nub.
    fluid_setX(nub, fminf(fmaxf(posRel.x, -0.375), 0.375));
    // 2. Vérif si changement
    bool oldIsOn = (bt.n->nodrawData.data1.u3 & button_switch_flag_isOn) != 0;
    bool newIsOn = posRel.x >= 0;
    if(oldIsOn != newIsOn) {
        if(newIsOn)
            b->n.nodrawData.data1.u3 |= button_switch_flag_isOn;
        else
            b->n.nodrawData.data1.u3 &= ~ button_switch_flag_isOn;
        back->n.renderIU.color = newIsOn ? color4_green_electric : color4_red_vermilion;
        if(b->draggableActionOpt) b->draggableActionOpt(bt);
    }
    b->n.nodrawData.data1.u3 |= button_switch_flag_didDrag;
}
void button_switch_letGo_(NodeTouch const bt) {
    guard_let(Button*, b, node_asButtonOpt(bt.n), printerror("No button."), )
    Fluid* nub = node_asFluidOpt(b->n._lastChild);
    Drawable* back = node_asDrawableOpt(b->n._firstChild);
    if(!nub || !back) { printerror("Switch without nub or back."); return; }
    // Pas dragé ? suppose simple touche pour permuter
    bool isOn;
    if(!(b->n.nodrawData.data1.u3 & button_switch_flag_didDrag)) {
        isOn = !(b->n.nodrawData.data1.u3 & button_switch_flag_isOn);
        if(isOn)
            b->n.nodrawData.data1.u3 |= button_switch_flag_isOn;
        else
            b->n.nodrawData.data1.u3 &= ~ button_switch_flag_isOn;
        back->n.renderIU.color = isOn ? color4_green_electric : color4_red_vermilion;
        if(b->draggableActionOpt) b->draggableActionOpt(bt);
    } else {
        isOn = b->n.nodrawData.data1.u3 & button_switch_flag_isOn;
    }
    fluid_setX(nub, isOn ? 0.375 : -0.375);
}
Button* Button_createSwitch(Node* refOpt, void (*const action)(NodeTouch), bool const isOn,
                            float const x, float const y, float const height, 
                            float const lambda, flag_t const flags) 
{
    Button* b = coq_callocTyped(Button);
    node_init(&b->n, refOpt, x, y, 2, 1, flags, 0);
    b->n.sx = height;
    b->n.sy = height;
    fluid_init(&b->f, lambda);
    button_init(b, button_switch_touch_);
    // Init as switch
    b->n.nodrawData.data1.u3 = isOn ? 1 : 0;
    // La switch est un bouton "draggable"...
    b->dragOpt =  button_switch_drag_;
    b->letGoOpt = button_switch_letGo_;
    b->draggableActionOpt = action;
    if(!action) printwarning("No action for switch.");
    // Structure de switch
    Drawable* back = Drawable_createImageWithName(&b->n, "coqlib_switch_back", 0, 0, 1, 0);
    back->n.renderIU.color = isOn ? color4_green_electric : color4_red_vermilion;
    Fluid* nub = Fluid_create(&b->n, isOn ? 0.375 : -0.375, 0, 1, 1, 10, 0, 0);
    Drawable_createImageWithName(&nub->n, "coqlib_switch_front", 0, 0, 1, 0);

    return b;
}
void button_switch_set(Button* b, bool isOn) {
    Drawable* back = node_asDrawableOpt(b->n._firstChild);
    if(!back) { printerror("Switch without back."); return; }
    if(isOn)
        b->n.nodrawData.data1.u3 |= button_switch_flag_isOn;
    else
        b->n.nodrawData.data1.u3 &= ~ button_switch_flag_isOn;
    back->n.renderIU.color = isOn ? color4_green_electric : color4_red_vermilion;
}
bool    button_switch_value(Button* b) {
    return b->n.nodrawData.data1.u3 & button_switch_flag_isOn;
}
Button* Button_createDummySwitch(Node* refOpt, void (*const touchOpt)(NodeTouch), uint32_t data,
                                 float x, float y, float height, float lambda, flag_t flags) 
{
    Button* b = coq_callocTyped(Button);
    node_init(&b->n, refOpt, x, y, 2, 1, flags, 0);
    b->n.sx = height;
    b->n.sy = height;
    fluid_init(&b->f, lambda);
    button_init(b, touchOpt ? touchOpt : button_touchOrDrag_pass_);
    // Structure de switch
    Drawable_createImageWithName(&b->n, "coqlib_switch_back", 0, 0, 1, 0);
    drawable_last_setColor(color4_gray_60);
    Drawable_createImageWithName(&b->n, "coqlib_switch_front", 0, 0, 1, 0);
    drawable_last_setColor(color4_gray_60);

    return b;
}

// MARK: - Slider ------------------
void button_slider_drag_(NodeTouch const bt) {
    guard_let(Button*, b, node_asButtonOpt(bt.n), printerror("No button."), )
    guard_let(Fluid*, nub, node_asFluidOpt(b->n._lastChild), printerror("No nub."), )
    float half_w = 0.5*fmaxf(b->n.w - b->n.h, b->n.h);
    // 1. Ajustement de la position du nub (et valeur du slider).
    Vector2 const posRel = nodetouch_evalPosRel(bt);
    float nub_x = fminf(fmaxf(posRel.x, -half_w), half_w);
    float value = 0.5f*(nub_x / half_w + 1.f);
    fluid_setX(nub, nub_x);
    b->n.nodrawData.data1.v.w = value;
    // b->action(b); // Non, en fait c'est juste au let go a priori.
    // Si on veut vraiment en peut redéfinir aussi le drag.
}
void button_slider_letGo_(NodeTouch const bt) {
    guard_let(Button*, b, node_asButtonOpt(bt.n), printerror("No button."), )
    if(b->draggableActionOpt) b->draggableActionOpt(bt);
}
Button* Button_createSlider(Node* refOpt, void (*action)(NodeTouch),
                            float value, float x, float y, float width, float height,
                            float lambda, flag_t flags) {
    // Check width
    width = fmaxf(width, 1.5f*height);
    float slide_width = width - height;
    // Init node, fluid, button
    Button* b = coq_callocTyped(Button);
    node_init(&b->n, refOpt, x, y, width, height, flags, 0);
    fluid_init(&b->f, lambda);
    button_init(b, button_touchOrDrag_pass_);
    // Init as slider
    b->dragOpt =  button_slider_drag_;
    b->letGoOpt = button_slider_letGo_;
    b->draggableActionOpt = action;
    if(!action) printwarning("No action for slider.");
    value = fminf(fmaxf(value, 0.f), 1.f);
    b->n.nodrawData.data1.v.w = value;
    // Structure
    Frame_create(&b->n, 0, 0.25*height, slide_width, height,
                 Texture_sharedImageByName("coqlib_bar_in"), frame_option_horizotalBar);
    float nub_x = (value - 0.5f) * slide_width;
    Fluid* nub = Fluid_create(&b->n, nub_x, 0, height, height, 10, 0, 0);
    Drawable_createImageWithName(&nub->n, "coqlib_switch_front", 0, 0, height, 0);

    return b;
}
float   button_slider_value(Button* b) {
    return b->n.nodrawData.data1.v.w;
}

// MARK: - Secure/Hoverable button
// As Hoverable... (Nom du bouton qui apparaaît)
void buttonhov_showPopMessage_callback_(void* hvIn) {
    ButtonSecureHov_* hv = (ButtonSecureHov_*)hvIn;
    PopMessage_spawnOverAndOpen(&hv->n, 0, 0.4, 2.5, hv->popFramePngId, hv->popMessage, framedString_defPars);
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
void buttonsecure_action_callback_(void* bsIn) {
    ButtonSecureHov_*const bt = (ButtonSecureHov_*)bsIn;
    popingnoderef_cancel(&bt->poping);
    if(bt->b.draggableActionOpt && bt->touch.n) bt->b.draggableActionOpt(bt->touch);
    bt->didActivate = true;
}
void buttonsecure_touch_(NodeTouch const touch) {
    ButtonSecureHov_*const bt = (ButtonSecureHov_*)touch.n;
    bt->didActivate = false;
    nodetouch_init(&bt->touch, touch);
    popingnoderef_cancel(&bt->poping);
    PopDisk_spawnOverAndOpen(&bt->n, &bt->poping, bt->spi.popPngId, bt->spi.popTile, bt->spi.holdTimeSec,
                             0, 0, 1);
    timer_scheduled(&bt->timer, (int64_t)(bt->spi.holdTimeSec * 1000.f), false,
                    &bt->n, buttonsecure_action_callback_);
}
void buttonsecure_letGo_(NodeTouch const touch) {
    ButtonSecureHov_*const bt = (ButtonSecureHov_*)touch.n;
    popingnoderef_cancel(&bt->poping);
    timer_cancel(&bt->timer);
    if(bt->didActivate) return;

    PopMessage_spawnOverAndOpen(&bt->n, 0, 0.5, 2.5, bt->spi.failPopFramePngId, bt->spi.failMessage, framedString_defPars);
}

void buttonsecurehov_deinit_(Node* n) {
    timer_cancel(&((ButtonSecureHov_*)n)->timer);
}
Button* ButtonSecureHov_create(Node* refOpt, void (*action)(NodeTouch),
                SecurePopInfo const spi, uint32_t popFramePngId, StringGlyphedInit const popMessage,
                float x, float y, float height, float lambda, flag_t flags)
{
    ButtonSecureHov_ *sec = coq_callocTyped(ButtonSecureHov_);
    node_init(&sec->n, refOpt, x, y, height, height, flags, 0);
    fluid_init(&sec->f, lambda);
    button_init(&sec->b, buttonsecure_touch_);
    sec->b.startHoveringOpt = buttonhov_startHovering_;
    sec->b.stopHoveringOpt =  buttonhov_stopHovering_;
    sec->b.dragOpt =          button_touchOrDrag_pass_;
    sec->b.letGoOpt =         buttonsecure_letGo_;
    sec->b.draggableActionOpt = action;
    if(!action) printwarning("Secure button without action.");
    sec->n.deinitOpt =        buttonsecurehov_deinit_;
    sec->spi = spi;
    sec->popFramePngId = popFramePngId;
    sec->popMessage = popMessage;

    return &sec->b;
}
/// Juste secure pas de pop-over lors du survol.
Button* ButtonSecure_create(Node* refOpt, void (*action)(NodeTouch),
                            SecurePopInfo spi, float x, float y, float height,
                            float lambda, flag_t flags) 
{
    ButtonSecureHov_ *sec = coq_callocTyped(ButtonSecureHov_);
    node_init(&sec->n, refOpt, x, y, height, height, flags, 0);
    fluid_init(&sec->f, lambda);
    button_init(&sec->b, buttonsecure_touch_);
    sec->b.dragOpt =     button_touchOrDrag_pass_;
    sec->b.letGoOpt =    buttonsecure_letGo_;
    sec->b.draggableActionOpt = action;
    if(!action) printwarning("Secure button without action.");
    sec->n.deinitOpt =   buttonsecurehov_deinit_;
    sec->spi = spi;

    return &sec->b;
}

void buttonsecurehov_initHoverable_(ButtonSecureHov_*const bsh, uint32_t const popFramePngId, 
                                    StringGlyphedInit const popMessage) 
{
    bsh->b.startHoveringOpt = buttonhov_startHovering_;
    bsh->b.stopHoveringOpt =  buttonhov_stopHovering_;
    bsh->n.deinitOpt =        buttonsecurehov_deinit_;
    bsh->popFramePngId =      popFramePngId;
    bsh->popMessage =         popMessage;          
}
Button* ButtonHoverable_create(Node* refOpt, void (*const touch)(NodeTouch),
                uint32_t const popFramePngId, StringGlyphedInit const popMessage,
                float const x, float const y, float const height, 
                float const lambda, flag_t const flags) 
{
    ButtonSecureHov_ *sec = coq_callocTyped(ButtonSecureHov_);
    node_init(&sec->n, refOpt, x, y, height, height, flags, 0);
    fluid_init(&sec->f, lambda);
    button_init(&sec->b, touch);
    buttonsecurehov_initHoverable_(sec, popFramePngId, popMessage);

    return &sec->b;
}
