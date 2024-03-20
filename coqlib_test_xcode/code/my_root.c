//
//  my_root.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#include "my_root.h"
#include "coq_sound.h"
#include "my_enums.h"
#include "my_particules.h"

static Root my_root = {};

View* View_createTest(void);

// Actions pour les boutons et enter...

void view_enter(View* v) {
    root_changeViewActiveTo(&my_root, View_createTest());
}
void view_keyDown_(View* v, KeyboardInput key) {
//    printdebug("Key down. mkc %d, str %s, mod %x.", key.mkc, key.typed.c_str, key.modifiers);
}

static bool need_keyboard_ = false;
void button_action(Button* bt) {
    if(Language_current() != language_french)
        Language_setCurrent(language_french);
    else
        Language_setCurrent(language_english);
#if TARGET_OS_OSX != 1
    need_keyboard_ = !need_keyboard_;
    if(need_keyboard_) {
        CoqEvent_addWindowEvent((CoqEvent) {.type = event_type_win_ios_keyboardNeeded});
    } else {
        CoqEvent_addWindowEvent((CoqEvent) {.type = event_type_win_ios_keyboardNotNeeded});
    }
#endif
    Sound_play(0, 1, 0, 0);
    root_changeViewActiveTo(&my_root, View_createTest());
}
void button_action_firework_(Button* b) {
    Sparkle_spawnOver(&b->n, (0.5f + b->data.float3) * 1.5f);
}
void button_action_terminate(Button* bt) {
    Sound_play(sound_sheep, 1, 0, 0);
    root_changeViewActiveTo(&my_root, NULL);
}
static bool install_font_ = true;
void button_action_installFont_(Button* b) {
    CoqEvent_addWindowEvent((CoqEvent) {
        .type = install_font_ ? event_type_win_ios_fonts_install
                              : event_type_win_ios_fonts_uninstall,
//        .win_font_list = { ... },
    });
    install_font_ = !install_font_;
}

/// Exemple d'une string localisée avec un cadre.
void sl_button_action_(Button* b) {
    printdebug("Touching %d.", b->data.uint0);
}
Node* Node_createFramedLoc_(Localizable loc, float widthMax) {
    Button* b = Button_create(NULL, sl_button_action_, 0, 0, 1, 0, flag_noParent);
    b->n.w = widthMax;
    b->data.uint0 = loc;
    Texture* str_tex = Texture_createString((UnownedString){ loc_stringKey(loc), NULL, true }, false);
    node_addFramedString(&b->n, png_frame_mocha, str_tex, framedString_defPars);
    return &b->n;
}

View* View_createTest(void) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = View_create(&my_root, flag_viewDontAlignElements, 0);
    // Action au "enter"
    v->enterOpt = view_enter;
    // Keyboard input
    v->keyDownOpt = view_keyDown_;
    
    // Image sur un `bloc` fluid.
    Fluid* bloc = Fluid_create(&v->n, 0.5f, 0.f, 1.f, 1.f, 5.f,
                               flag_fluidFadeInRight, 0);
    // fl_fix(&bloc->z, 2.f); // (image en z = 2)
    Drawable_createImageWithName(&bloc->n, "coqlib_the_cat", -1.f, 0.f, 0.5f, 0);
    
    // bouton test (change la langue)...
    UnownedString str = { loc_stringKey(loc_menu), NULL, true };
    ButtonHoverable_create(&v->n, button_action,
                           png_frame_white_back, str,
                           -0.55, -0.4, 0.20, 10, 0);
    node_last_nonLeaf->w = 0.8f;
    node_last_addFramedString(png_frame_mocha, Texture_createString(str, false),
                              framedString_defPars);
    
    // Bouton quitter
    str.c_str = loc_stringKey(loc_quit);
    ButtonHoverable_create(&v->n, button_action_terminate,
                           png_frame_white_back, str,
                           -0.55, -0.6, 0.15, 10, 0);
    node_last_nonLeaf->w = 0.6f;
    node_last_addFramedString(png_frame_mocha, Texture_createString(str, false),
                              framedString_defPars);
    
    // Bouton secure
    SecurePopInfo spi = {
        2.f, png_disks, disk_color_red,
        png_frame_red, {"Hold Button !", NULL, false}
    };
    ButtonSecure_create(&v->n, button_action_installFont_, spi, -0.5, 0.5, 0.25, 10, 0);
    node_last_addIcon(png_disks, disk_color_orange, png_icons, icon_help);
    
    
    // Title
    str.c_str = loc_stringKey(loc_app_name);
    Drawable_createString(&v->n, str, 0.f, 0.9f,  2.f, 0.15f, 0);
    
    // Pop disk
    PopDisk_spawn(&v->n, NULL, png_disks, disk_color_blue, 3.f, -0.15f, -0.25f, 0.3f);

    // Menu deroulant
    SlidingMenu_create(&v->n, 4, 1.2f,  0.75, 0.f, 1.2, 1.5, 0);
    float itemW = slidingmenu_last_itemRelativeWidth()*0.95;
    slidingmenu_last_addItem(&Button_createSlider(NULL, button_action_firework_, 0.5,
                        0, 0, itemW, 1, 0, flag_noParent)->n);
    slidingmenu_last_addItem(&Button_createSwitch(NULL, button_action_firework_, false,
                        0, 0, 1, 0, flag_noParent)->n);
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_ok, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_error, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_menu, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_app_name, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_quit, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_error_empty, itemW));
    
    return v;
}

Root* Root_initAndGetProjectRoot(void) {
    if(my_root.n._type) { printerror("Root already init."); return &my_root; }
    node_init_(&my_root.n, NULL, 0, 0, 4, 4, node_type_root, sizeof(Root), flags_rootDefault, 0);
    root_init(&my_root, NULL);
    
    // Tester avec une font custom.
    Texture_setCurrentFont("OpenDyslexic3");
    
    // Création de l'arrière plan (avec particule pool)
    my_root.viewBackOpt = View_create(&my_root, flags_viewBackAndFrontDefault, 0);
    Node* pool = (Node*)PPNode_create(&my_root.viewBackOpt->n);
    node_tree_openAndShow(pool);
    
    // Création de l'avant plan (pour les feux d'artifices, popover...)
    my_root.viewFrontOpt = View_create(&my_root, flags_viewBackAndFrontDefault, 0);
    PopingNode_init(my_root.viewFrontOpt, "coqlib_sparkle_stars", sound_fireworks);
    
    // Ouvrir la view de test.
    root_changeViewActiveTo(&my_root, View_createTest());
    
//    printdebug("selected %p", root->buttonSelectedOpt);
    
    return &my_root;
}
