//
//  my_root.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#include "my_root.h"
#include "coq_sound.h"
#include "my_enums.h"

View* View_createTest(Root* rt);

void view_enter(View* v) {
    root_changeViewActiveTo(v->root, View_createTest(v->root));
}

void button_action(Button* bt) {
    if(Language_current() != language_french)
        Language_setCurrent(language_french);
    else
        Language_setCurrent(language_english);
    Sound_play(0, 1, 0, 0);
    if(!bt->root) {
        printerror("No root ref.");
        return;
    }
    root_changeViewActiveTo(bt->root, View_createTest(bt->root));
}

void button_action_doubleDeltaT(Button *bt) {
    Chrono_UpdateDeltaTMS *= 2;
}

void button_action_terminate(Button* bt) {
    Sound_play(sound_sheep, 1, 0, 0);
    root_changeViewActiveTo(bt->root, NULL);
}

Node* _Node_createFramedLoc(Localizable loc, float widthMax) {
    Node* n = Node_create(NULL, 0.f, 0.f, widthMax, 1.f, 0, 0);
    loc_stringKey(loc);
    UnownedString ustr = { loc_stringKey(loc), NULL, true };
    Texture* str_tex = Texture_createString(ustr, false);
    node_addFramedString(n, png_frame_mocha, str_tex, framedString_defPars);
    return n;
}

View* View_createTest(Root* rt) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = View_create(rt, flag_viewDontAlignElements);
    // Action au "enter"
    v->enterOpt = view_enter;
    
    // Image sur un `bloc` fluid.
    Fluid* bloc = Fluid_create(&v->n, 0.5f, 0.f, 1.f, 1.f, 5.f,
                               flag_fluidFadeInRight, 0);
    // fl_fix(&bloc->z, 2.f); // (image en z = 2)
    Drawable_createImageWithName(&bloc->n, "coqlib_the_cat", -1.f, 0.f, 0.5f, 0);
    
    // bouton test (change la langue)...
    UnownedString str = { loc_stringKey(loc_menu), NULL, true };
    ButtonHoverable_create(&v->n, rt, button_action,
                           png_frame_white_back, str,
                           -0.55, -0.4, 0.20, 10, 0);
    _node_last_created->w = 0.8f;
    node_last_addFramedString(png_frame_mocha, Texture_createString(str, false),
                              framedString_defPars);
    
    // Bouton quitter
    str.c_str = loc_stringKey(loc_quit);
    ButtonHoverable_create(&v->n, rt, button_action_terminate,
                           png_frame_white_back, str,
                           -0.55, -0.6, 0.15, 10, 0);
    _node_last_created->w = 0.6f;
    node_last_addFramedString(png_frame_mocha, Texture_createString(str, false),
                              framedString_defPars);
    
    // Bouton secure
    SecurePopInfo spi = {
        2.f, png_disks, disk_color_red,
        png_frame_red, {"Hold Button !", NULL, false}
    };
    ButtonSecure_create(&v->n, rt, button_action_doubleDeltaT, spi, -0.5, 0.5, 0.25, 10, 0);
    node_last_addIcon(png_disks, disk_color_orange, png_icons, icon_help);
    str.c_str = loc_stringKey(loc_app_name);
    Drawable_createString(&v->n, str, 0.f, 0.9f,  2.f, 0.15f, 0);
    
    // Pop disk
    PopDisk_spawn(&v->n, NULL, png_disks, disk_color_blue, 3.f, -0.15f, -0.25f, 0.3f);

    // Menu deroulant
    SlidingMenu_create(&v->n, 4, 1.2f,  0.75, 0.f, 1.2, 1.5, 0);
    float itemW = slidingmenu_last_itemRelativeWidth()*0.95;
    slidingmenu_last_addItem(_Node_createFramedLoc(loc_ok, itemW));
    slidingmenu_last_addItem(_Node_createFramedLoc(loc_error, itemW));
    slidingmenu_last_addItem(_Node_createFramedLoc(loc_menu, itemW));
    slidingmenu_last_addItem(_Node_createFramedLoc(loc_app_name, itemW));
    slidingmenu_last_addItem(_Node_createFramedLoc(loc_quit, itemW));
    slidingmenu_last_addItem(_Node_createFramedLoc(loc_error_empty, itemW));
    
    return v;
}

Root* Root_createMyRoot(void) {    
    Root* root = Root_create();
    
    root_changeViewActiveTo(root, View_createTest(root));
    
    return root;
}
