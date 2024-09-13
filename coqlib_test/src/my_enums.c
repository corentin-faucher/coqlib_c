//
//  my_enums.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//
#include "my_enums.h"
#include <stdbool.h>
#include "nodes/node__types.h"

static const char* Localizable_stringKeys_[] = {
    "app_name",
    "error",
    "error_empty",
    "error_empty_directory",
    "menu",
    "ok",
    "quit",
};
const char* loc_stringKey(Localizable loc) {
    return Localizable_stringKeys_[loc];
}
StringGlyphedInit loc_stringGlyphedInit(Localizable loc) {
    return (StringGlyphedInit) {
        .c_str = Localizable_stringKeys_[loc],
        .isLocalized = true,
        .x_margin = 0.5,
    };
}

const PngInfo MyProject_pngInfos[] = {
    {"bar_gray", 1, 1, false},
    {"bar_in", 1, 1, false},
    {"country_flags", 8, 4, false},
    {"digits_brown", 12, 2, false},
    {"digits_green", 12, 2, false},
    {"digits_red", 12, 2, false},
    {"disks", 4, 4, false},
    {"frame_gray_back", 1, 1, false},
    {"frame_mocha", 1, 1, false},
    {"frame_red", 1, 1, false},
    {"frame_white_back", 1, 1, false},
    {"icons", 8, 4, false},
    {"language_flags", 4, 4, false},
    {"some_animals", 4, 7, false},
    
    {"coqlib_test_frame", 1, 1, true, true},
};

const char* MyProject_wavNames[] = {
    "clap_clap",
    "duck_error",
    "fireworks",
    "note_piano",
    "sheep",
};


