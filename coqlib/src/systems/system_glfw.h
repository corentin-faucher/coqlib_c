//
//  system_glfw.h
//  Set les callback d'event pour GLFW
//  (OpenGL Framework, gestion de la fenêtre avec l'OS).
//
//  Created by Corentin Faucher on 2024-11-02.
//
#ifndef COQ_UTIL_GLFW_H
#define COQ_UTIL_GLFW_H

#include <GLFW/glfw3.h>

void CoqGlfw_error_callback(int const error, const char* const description);
void CoqGlfw_Init(GLFWwindow* window);

#endif
