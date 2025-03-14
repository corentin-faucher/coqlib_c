//
//  util_glfw.c
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-11-02.
//
#include "util_glfw.h"

#include <stdio.h>
#include "coq_event.h"

/*-- Callbacks d'events  -------------*/
void Coq_glfw_error_callback(int const error, const char* const description) {
    printf("⚠️ GLFW Error: %s.\n", description);
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
        .type = event_type_key_down,
        .key = {
            .modifiers = modifiers,
            .keycode = scancode,
        },
    };
    CoqEvent_addToRootEvent(event);
}
void glfw_scroll_callback_(GLFWwindow* window, double xoffset, double yoffset) {
    CoqEvent_addToRootEvent((CoqEvent){
        .type = event_type_scroll,
        .scroll_info = {
            .scrollType = scroll_type_scroll,
            .scroll_deltas = {{ xoffset, yoffset }}
        },
    });
}
int CoqGLFW_viewWidthPx = 800;
int CoqGLFW_viewHeightPx = 600;
void glfw_framebufferSize_callback_(GLFWwindow* window, int widthPx, int heightPx)  {
  CoqGLFW_viewWidthPx = widthPx;
  CoqGLFW_viewHeightPx = heightPx;
  // int widthPt, heightPt;
  // glfwGetWindowSize(window, &widthPt, &heightPt);
  CoqEvent_addToRootEvent((CoqEvent){
        event_type_resize, .resize_info = {
            { 0, 0, 0, 0 },
            {{ 0, 0, widthPx, heightPx }}, {{ widthPx, heightPx }},
            false, false, false,
  }});
  // GLFW semble bloquer la boucle principale durant un resize ???
  // Il faut donc checker les event et dessiner ici... ??
//  CoqEvent_processEvents(root);
//  Renderer_drawView(root);
}
static bool MouseLeftDown_ = false;
static Vector2 mousePos = {};
void glfw_cursorPos_callback_(GLFWwindow* window, double x, double y) {
  // !! Ici y est inversé !!
  mousePos = (Vector2){{ x, y }};
  CoqEvent_addToRootEvent((CoqEvent){
      .type = MouseLeftDown_ ? event_type_touch_drag : event_type_touch_hovering,
      .touch_info = {
          .touch_pos = mousePos,
          .touch_yInverted = true,
      },
  });
}
void glfw_mouseButton_callback_(GLFWwindow* window, int button, int action, int mods) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    MouseLeftDown_ = action; // GLFW_RELEASE = 0
    if(action == GLFW_PRESS) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = event_type_touch_down,
            .touch_info = {
                .touch_pos = mousePos,
                .touch_yInverted = true,
            },
        });
    } else if(action == GLFW_RELEASE) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = event_type_touch_up,
            // .touch_pos = mousePos, // superflu pour touchUp.
        });
    }
}

void Coq_glfw_setEventsCallbacks(GLFWwindow* window) {
    // Les callback d'events pour glfw...
    glfwSetKeyCallback(window, glfw_key_callback_);
    glfwSetScrollCallback(window, glfw_scroll_callback_);
    // glfwSetWindowSizeCallback(window, glfw_windowSize_callback_); -> coordinates (pas pixels)
    glfwSetFramebufferSizeCallback(window, glfw_framebufferSize_callback_);
    glfwSetCursorPosCallback(window, glfw_cursorPos_callback_);
    glfwSetMouseButtonCallback(window, glfw_mouseButton_callback_);
}
