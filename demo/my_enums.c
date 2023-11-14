//
//  localizable.c
//  coqlib_c_xcode_test
//
//  Created by Corentin Faucher on 2023-10-26.
//

#include <stdio.h>
#include "my_enums.h"

static const char* _Localizable_stringKeys[] = {
    "app_name",
    "error",
    "error_empty",
    "error_empty_directory",
    "menu",
    "ok",
    "quit",
};

const PngInfo MyProject_pngInfos[] = {
    {"bar_gray", 1, 1},
    {"bar_in", 1, 1},
    {"country_flags", 8, 4},
    {"digits_black", 12, 2},
    {"digits_brown", 12, 2},
    {"digits_green", 12, 2},
    {"digits_red", 12, 2},
    {"disks", 4, 4},
    {"frame_gray_back", 1, 1},
    {"frame_mocha", 1, 1},
    {"frame_red", 1, 1},
    {"frame_white_back", 1, 1},
    {"icons", 8, 4},
    {"language_flags", 4, 4},
    {"some_animals", 4, 7},
};

const char* MyProject_wavNames[] = {
    "clap_clap",
    "duck_error",
    "fireworks",
    "note_piano",
    "sheep",
};


const char* Loc_strKey(Localizable key) {
    return _Localizable_stringKeys[key];
}
