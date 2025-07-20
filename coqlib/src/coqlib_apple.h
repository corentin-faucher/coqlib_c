//
//  coqlib_gl.h
//  Includes de base pour macOS/iOS avec Metal.
//
//  Created by Corentin Faucher on 2025-07-20.
//
#ifndef COQLIB_APPLE_H
#define COQLIB_APPLE_H

#include "nodes/node_root.h"
#include "nodes/node_tree.h"
#include "systems/system_base.h"
#include "systems/system_sound.h"
#include "utils/util_chrono.h"
#include "utils/util_timer.h"

#include "graph__metal.h"
#include "util_apple.h"
#if TARGET_OS_OSX == 1
#import "macos_appdelegate.h"
#else
#import "apple_view_metal.h"
#endif

#endif
