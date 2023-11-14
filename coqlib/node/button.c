//
//  button_icon.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-30.
//

#include "button.h"
#include "drawable.h"
#include "colors.h"
#include <string.h>

void* test(void) { return NULL; }

void    _button_init(Button* b, Root* root, void (*action)(Button*),
                     float x, float y, float height, float lambda) {
    // Node dims.
    b->n.x = x;
    b->n.y = y;
    b->n.w = height;  // Icon carre
    b->n.h = height;
    // Init as Fluid.
    _fluid_init(&b->f, lambda);
    // Init as Button
    if(action == NULL) {
        printwarning("NO action for button!");
    }
    b->action = action;
    b->root = root;
}
Button* Button_create(Node* refOpt, Root* root, void (*action)(Button*),
                                  float x, float y, float height, float lambda, flag_t flags) {
    Button* b = _Node_createEmptyOfType(node_type_smooth_button, sizeof(Button),
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
    float height = nd->h;
    PopMessage_spawn(nd, hv->popFramePngId, hv->popMessage,
                     0, 0.5*height, 5*height, 0.5*height, 2.5);
}
void _buttonhov_startHovering(Button* b) {
    ButtonHov* h = (ButtonHov*)b;
    timer_cancel(&h->timer);
    timer_scheduled(&h->timer, 350, false, (Node*)h, _buttonhov_showPopMessage);
}
void _buttonhov_stopHovering(Button* b) {
    ButtonHov* hv = (ButtonHov*)b;
    timer_cancel(&hv->timer);
}

ButtonHov* ButtonHoverable_create(Node* refOpt, Root* root, void (*action)(Button*),
                                  uint32_t popFramePngId, UnownedString popMessage,
                                  float x, float y, float height,
                                  float lambda, flag_t flags) {
    ButtonHov* h = _Node_createEmptyOfType(node_type_smooth_button, sizeof(ButtonHov),
                                       flags, refOpt, 0);
    // Init as button
    _button_init(&h->b, root, action, x, y, height, lambda);
    // Init as hoverable
    h->b.startHovering = _buttonhov_startHovering;
    h->b.stopHovering = _buttonhov_stopHovering;
    h->popFramePngId = popFramePngId;
    h->popMessage = popMessage;
    return h;
}

/*-- Secure button -----------------------------------------------------*/
void _buttonsecure_action_callback(Node* nd) {
    ButtonSecure* bt = (ButtonSecure*)nd;
    popdisk_cancel(&bt->pop);
    bt->b.action(&bt->b);
}
void _grab_secure(Button* sel, Vector2 pos) {
    ButtonSecure* bt = (ButtonSecure*)sel;
    popdisk_cancel(&bt->pop);
    PopDisk_spawn(&bt->n, &bt->pop, bt->spi.popPngId, bt->spi.popTile,
                  bt->spi.holdTimeSec, 0, 0, bt->n.h);
    timer_scheduled(&bt->timer, (int64_t)(bt->spi.holdTimeSec * 1000.f), false,
                    &bt->n, _buttonsecure_action_callback);
}
void _drag_secure(Button* sel, Vector2 pos) {
    // pass
}
void _letGo_secure(Button* but) {
    ButtonSecure* bt = (ButtonSecure*)but;
    popdisk_cancel(&bt->pop);
    timer_cancel(&bt->timer);
    float h = bt->n.h;
    PopMessage_spawn(&bt->n, bt->spi.failPopFramePngId, bt->spi.failMessage,
                     0, 0.5f*h, 10.f*h, 0.5*h, 2.5f);
}

ButtonSecure* ButtonSecure_create(Node* refOpt, Root* root, void (*action)(Button*),
        SecurePopInfo spi, float x, float y, float height, float lambda, flag_t flags) {
    ButtonSecure *sec = _Node_createEmptyOfType(node_type_smooth_button, sizeof(ButtonSecure),
                                       flags, refOpt, 0);
    // Init as button
    _button_init(&sec->b, root, action, x, y, height, lambda);
    // Init as draggable button
    sec->b.grab =  _grab_secure;
    sec->b.drag =  _drag_secure;
    sec->b.letGo = _letGo_secure;
    // Init as secure button
    sec->spi = spi;
    return sec;
}


