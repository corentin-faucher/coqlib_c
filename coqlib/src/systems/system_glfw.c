//
//  util_glfw.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-11-02.
//
#include "system_glfw.h"

#include <stdio.h>
#include "system_base.h"
#include "../utils/util_event.h"

/*-- Callbacks d'events  -------------*/
void CoqGlfw_error_callback(int const error, const char* const description) {
    printf("⚠️ GLFW Error %d: %s.\n", error, description);
}
void glfw_key_callback_(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action != GLFW_PRESS) return;
    if(key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
        return;
    }
    // Le "scancode" de GLFW semble etre le keycode de l'OS.
    // Le "key" de GLFW semble etre la correspondance
    // entre le Qwerty-US et les ASCII,
    // e.g. la touch "A" (sur qwerty) est key = 65.
    // Ici, on va garder les "scancodes..."
    uint32_t modifiers = 0;
    if(mods & GLFW_MOD_SHIFT) modifiers |= modifier_shift;
    if(mods & GLFW_MOD_ALT) modifiers |= modifier_option;
    if(mods & GLFW_MOD_SUPER) modifiers |= modifier_command;
    if(mods & GLFW_MOD_CONTROL) modifiers |= modifier_control;
    if(mods & GLFW_MOD_CAPS_LOCK) modifiers |= modifier_capslock;
    CoqEvent event = {
        .type = eventtype_key_down,
        .key = {
            .modifiers = modifiers,
            .keycode = scancode,
        },
    };
    CoqEvent_addToRootEvent(event);
}
void glfw_scroll_callback_(GLFWwindow* window, double xoffset, double yoffset) {
    CoqEvent_addToRootEvent((CoqEvent){
        .type = eventtype_scroll,
        .scroll_info = {
            .scrollType = scrolltype_scroll,
            .scroll_deltas = {{ xoffset, yoffset }}
        },
    });
}

static bool MouseLeftDown_ = false;
static Vector2 mousePos = {};
void glfw_cursorPos_callback_(GLFWwindow* window, double x, double y) {
  // !! Ici y est inversé !!
  mousePos = (Vector2){{ x, y }};
//  printf("🐔GLFW Mouse at %f: %f.\n", x, y);
  CoqEvent_addToRootEvent((CoqEvent){
      .type = MouseLeftDown_ ? eventtype_touch_drag : eventtype_touch_hovering,
      .touch_info = {
          .pos = mousePos,
          .yInverted = true,
      },
  });
}
void glfw_mouseButton_callback_(GLFWwindow* window, int buttonId, int action, int mods) {
    MouseLeftDown_ = action; // GLFW_RELEASE = 0
    if(action == GLFW_PRESS) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = eventtype_touch_down,
            .touch_info = {
                .pos = mousePos,
                .touchId = buttonId,
                .yInverted = true,
            },
        });
    } else if(action == GLFW_RELEASE) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = eventtype_touch_up,
            .touch_info = { // un peu superflu pour touchUp...
                .pos = mousePos,
                .touchId = buttonId,
                .yInverted = true,
            },
        });
    }
}


void glfw_framebufferSize_callback_(GLFWwindow* window, int widthPx, int heightPx)  {
  int widthPt, heightPt;
  int x, y;
  glfwGetWindowSize(window, &widthPt, &heightPt);
  glfwGetWindowPos(window, &x, &y);
  CoqEvent_addToRootEvent((CoqEvent){
        eventtype_resize, .resize_info = {
            .margins = { 0, 0, 0, 0 },
            .originPt = {{ x, y }},
            .framePt = {{ widthPt, heightPt }},
            .framePx = {{ widthPx, heightPx }},
            .fullScreen = false, 
            .justMoving = false, 
            .dontFix = false,
  }});
  // GLFW semble bloquer la boucle principale durant un resize ???
  // Il faut donc checker les event et dessiner ici... ??
//  CoqEvent_processEvents(root);
//  Renderer_drawView(root);
}


void CoqGlfw_Init(GLFWwindow* window) {
    // Setter les infos de dimensions de la view.
    int x, y;
    int widthPt, heightPt;
    int widthPx, heightPx;
    glfwGetWindowPos(window, &x, &y);
    glfwGetWindowSize(window, &widthPt, &heightPt);
    glfwGetFramebufferSize(window, &widthPx, &heightPx);
    CoqSystem_setViewSize((ViewSizeInfo) {
        .margins = {},
        .originPt = {{ x, y }},
        .framePt =  {{ widthPt, heightPt }},
        .framePx =  {{ widthPx, heightPx }},
    });
    // Les callback d'events pour glfw...
    glfwSetKeyCallback(window, glfw_key_callback_);
    glfwSetScrollCallback(window, glfw_scroll_callback_);
    glfwSetCursorPosCallback(window, glfw_cursorPos_callback_);
    glfwSetMouseButtonCallback(window, glfw_mouseButton_callback_);
    glfwSetFramebufferSizeCallback(window, glfw_framebufferSize_callback_);
}
