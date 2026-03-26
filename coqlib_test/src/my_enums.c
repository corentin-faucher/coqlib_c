//
//  my_enums.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//
#include "my_enums.h"
#include <stdbool.h>

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
        .x_margin = 0.25,
    };
}

const PngInfo MyProject_pngInfos[] = {
    { .name = "bar_gray",      .m = 1, .n = 1,  },
    { .name = "bar_in",        .m = 1, .n = 1,  },
    { .name = "country_flags", .m = 8, .n = 4,  },
    { .name = "digits_brown",  .m = 12, .n = 2, },
    { .name = "digits_green",  .m = 12, .n = 2, },
    { .name = "digits_red",    .m = 12, .n = 2, },
    { .name = "disks",         .m = 4, .n = 4,  },
    { .name = "icons",         .m = 8, .n = 4,  },
    { .name = "language_flags", .m = 4, .n = 4, },
    { .name = "some_animals",  .m = 4, .n = 7,  },

    { .name = "coqlib_frame_gray_back", .m = 1, .n = 1, },
    { .name = "coqlib_frame_mocha",     .m = 1, .n = 1, },
    { .name = "coqlib_frame_red",       .m = 1, .n = 1, },
    { .name = "coqlib_frame_white_back", .m = 1, .n = 1,},
    { .name = "coqlib_test_frame",      .m = 1, .n = 1, },
    { .name = "coqlib_the_cat",      .m = 1, .n = 1, .flags = tex_flag_keepPixels },
};

const char* MyProject_wavNames[] = {
    "clap_clap",
    "duck_error",
    "fireworks",
    "note_piano",
    "sheep",
};
