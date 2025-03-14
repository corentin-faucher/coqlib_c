//
//  util_glfw.h
//  xc_coqlib_test
//
//  Created by Corentin Faucher on 2024-11-02.
//
#ifndef COQ_UTIL_GLFW_H
#define COQ_UTIL_GLFW_H

#include <GLFW/glfw3.h>

// Dimension de la window view de glfw.
extern int CoqGLFW_viewWidthPx;
extern int CoqGLFW_viewHeightPx;

void Coq_glfw_error_callback(int const error, const char* const description);
void Coq_glfw_setEventsCallbacks(GLFWwindow* window);

#endif
