//
//  button_icon.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#include "nodes/node_button.h"
#include "nodes/node_drawable.h"
#include "nodes/node_root.h"
#include <string.h>

void   _button_default_action(Button* b) {
    printwarning("Button %d action not overrided.", b->n._nodeId);
}

void    _button_init(Button* b, Root* root, void (*action)(Button*),
                     float x, float y, float height, float lambda) {
    // Node dims.
    b->n.x = x;
    b->n.y = y;
    b->n.w = height;  // Icon carre
    b->n.h = height;
    // Init as Fluid.
    fluid_init_(&b->f, lambda);
    // Init as Button
    if(action)
        b->action = action;
    else {
        printwarning("No action for button!");
        b->action = _button_default_action;
    }
    b->root = root;
    b->prefsRef = root->prefsRef;
}
Button* Button_create(Node* refOpt, Root* root, void (*action)(Button*),
                                  float x, float y, float height, float lambda, flag_t flags) {
    Button* b = Node_createEmptyOfType_(node_type_nf_button, sizeof(Button),
                                       flags, refOpt, 0);
    _button_init(b, root, action, x, y, height, lambda);
    return b;
}
Button* node_asActiveButtonOpt(Node* n) {
    if(n->_type & node_type_flag_button) if(!(n->flags & flag_buttonInactive))
        return (Button*)n;
    return NULL;
}

/*-- Hoverable button -----------------------------------------------------*/
void _buttonhov_showPopMessage(Node* nd) {
    ButtonHov* hv = (ButtonHov*)nd;
    float h = 0.3f*nd->h;
    Rectangle rect = {{0.f, -0.25f*nd->h, -3.f*h, -0.3*h }};
    PopMessage_spawn(nd, hv->popFramePngId, hv->popMessage,
                     0, 0.4f*nd->h, 10.f*h, h, 2.5, rect);
}
void _buttonhov_startHovering(Button* b) {
    ButtonHov* h = (ButtonHov*)b;
    timer_cancel(&h->timer);
    timer_scheduled(&h->timer, 350, false, &h->n, _buttonhov_showPopMessage);
}
void _buttonhov_stopHovering(Button* b) {
    ButtonHov* hv = (ButtonHov*)b;
    timer_cancel(&hv->timer);
}
void _buttonhov_deinit(Node* n) {
    timer_cancel(&((ButtonHov*)n)->timer);
}
void       _buttonhoverable_init(ButtonHov* h, Root* root, void (*action)(Button*),
                                 uint32_t popFramePngId, UnownedString popMessage,
                                 float x, float y, float height,
                                 float lambda) {
    // Init as button
    _button_init(&h->b, root, action, x, y, height, lambda);
    // Init as hoverable
    h->b.startHoveringOpt = _buttonhov_startHovering;
    h->b.stopHoveringOpt = _buttonhov_stopHovering;
    h->popFramePngId = popFramePngId;
    h->popMessage = popMessage;
    // S'assurer de d'annuler le timer. (superflu... mais pas grave)
    h->n.deinitOpt = _buttonhov_deinit;
}

ButtonHov* ButtonHoverable_create(Node* refOpt, Root* root, void (*action)(Button*),
                                  uint32_t popFramePngId, UnownedString popMessage,
                                  float x, float y, float height,
                                  float lambda, flag_t flags) {
    ButtonHov* h = Node_createEmptyOfType_(node_type_nf_button, sizeof(ButtonHov),
                                       flags, refOpt, 0);
    _buttonhoverable_init(h, root, action, popFramePngId, popMessage, x, y, height, lambda);
    
    return h;
}

/*-- Secure button -----------------------------------------------------*/
void _buttonsecure_action_callback(Node* nd) {
    ButtonSecure* bt = (ButtonSecure*)nd;
    popdisk_cancel(&bt->pop);
    bt->b.action(&bt->b);
}
void _buttonsecure_grab(Button* sel, Vector2 pos) {
    ButtonSecure* bt = (ButtonSecure*)sel;
    popdisk_cancel(&bt->pop);
    PopDisk_spawn(&bt->n, &bt->pop, bt->spi.popPngId, bt->spi.popTile,
                  bt->spi.holdTimeSec, 0, 0, bt->n.h);
    timer_scheduled(&bt->timer, (int64_t)(bt->spi.holdTimeSec * 1000.f), false,
                    &bt->n, _buttonsecure_action_callback);
}
void _buttonsecure_drag(Button* sel, Vector2 pos) {
    // pass
}
void _buttonsecure_letGo(Button* but) {
    ButtonSecure* bt = (ButtonSecure*)but;
    popdisk_cancel(&bt->pop);
    timer_cancel(&bt->timer);
    float h = bt->n.h;
    Rectangle rect = {{0.f, -0.5f*h, -7.f*h, -0.4*h}};
    PopMessage_spawn(&bt->n, bt->spi.failPopFramePngId, bt->spi.failMessage,
                     0, 0.5f*h, 10.f*h, 0.5*h, 2.5f, rect);
}
void _buttonsecure_deinit(Node* n) {
    timer_cancel(&((ButtonSecure*)n)->timer);
}
ButtonSecure* ButtonSecure_create(Node* refOpt, Root* root, void (*action)(Button*),
        SecurePopInfo spi, float x, float y, float height, float lambda, flag_t flags) {
    ButtonSecure *sec = Node_createEmptyOfType_(node_type_nf_button, sizeof(ButtonSecure),
                                       flags, refOpt, 0);
    // Init as button
    _button_init(&sec->b, root, action, x, y, height, lambda);
    // Init as draggable button
    sec->b.grabOpt =  _buttonsecure_grab;
    sec->b.dragOpt =  _buttonsecure_drag;
    sec->b.letGoOpt = _buttonsecure_letGo;
    // Init as secure button
    sec->spi = spi;
    sec->n.deinitOpt = _buttonsecure_deinit;
    return sec;
}


