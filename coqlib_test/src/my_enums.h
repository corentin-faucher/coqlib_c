//
//  my_enums.h
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//
#ifndef my_enums_h
#define my_enums_h

#include "nodes/node_string.h"

extern const uint32_t node_type_flag_particule;
extern const uint32_t node_type_nf_particule;

typedef enum _Localizable {
    loc_app_name,
    loc_error,
    loc_error_empty,
    loc_error_empty_directory,
    loc_menu,
    loc_ok,
    loc_quit,
    
    locstring_total_locstring,
} Localizable;
const char*       loc_stringKey(Localizable loc);
NodeStringInit loc_stringGlyphedInit(Localizable loc);

typedef enum {
    png_bar_gray,
    png_bar_in,
    png_country_flags,
    png_digits_brown,
    png_digits_green,
    png_digits_red,
    png_disks,
    png_frame_gray_back,
    png_frame_mocha,
    png_frame_red,
    png_frame_white_back,
    png_icons,
    png_language_flags,
    png_some_animals,
    
    png_coqlib_test_frame,
    
    png_total_pngs
} MyPngEnum;
extern const PngInfo MyProject_pngInfos[];

typedef enum {
    sound_clap_clap,
    sound_duck_error,
    sound_fireworks,
    sound_note_piano,
    sound_sheep,
    
    sound_total_sounds,
} MySound;
extern const char*   MyProject_wavNames[];

typedef enum {
    disk_color_yellow,
    disk_color_green,
    disk_color_red,
    disk_color_blue,
    disk_color_orange,
    disk_color_purple,
    disk_color_bluesky,
    disk_color_pink,
    disk_color_black,
    disk_color_white,
    disk_color_gray,
    disk_color_blackwhite,
    disk_color_beige,
} DiskColor;

typedef enum {
    icon_menu,
    icon_previous,
    icon_next,
    icon_play,
    icon_pause,
    icon_redo,
    icon_fullscreen,
    icon_windowed,
    
    icon_ok,
    icon_nope,
    icon_user,
    icon_userAdd,
    icon_userGray,
    icon_log_out,
    icon_soundOn,
    icon_soundOff,

    icon_options,
    icon_emailContact,
    icon_selectLanguage,
    icon_buyCart,
    icon_help,
    icon_undefined,
    icon_garbage,
    icon_edit,
    
    icon_lessons_set,
    icon_add_lessons_set,
    icon_add_lesson,
    icon_plot,
    icon_stats,
    icon_export,
    icon_import_,
    icon_cloud_sync,
    
} ButtonIcon;

typedef enum {
    country_us,
    country_britain,
    country_canada,
    country_france,
    country_belgium,
    country_spanish,
    country_italia,
    country_german,

    country_swiss,
    country_sweden,
    country_greece,
    country_japan,
    country_china,
    country_arabic,
    country_australia,
    country_russia,

    country_korea,
    country_vietnam,
    country_portugal,
    country_brazil,
    country_turkey,
    country_belarus,
    country_bulgaria,
    country_kazakhstan,

    country_macedonian,
    country_mongolia,
    country_ukraine,
    country_coq,
    country_neo,
    country_bepo,
    country_engram,
} CountryFlag;


#endif /* my_enums_h */
