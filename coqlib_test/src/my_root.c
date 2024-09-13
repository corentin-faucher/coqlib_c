//
//  my_root.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#include "my_root.h"
#include "my_enums.h"
#include "my_particules.h"
#include "cellular.h"

#include "coq_sound.h"
#include "utils/util_system.h"
#include "utils/util_file.h"
#include "utils/util_language.h"

static Root my_root = {};

View* View_createTest(void);

// Actions pour les boutons et enter...
void view_enter_(View* v) {
    root_changeViewActiveTo(&my_root, View_createTest());
}
void view_keyDown_(View* v, KeyboardInput key) {
    printdebug("Key down. mkc %d, str %s, mod %x.", key.mkc, key.typed.c_str, key.modifiers);
}

void button_changeLanguage_action_(Button* bt) {
    if(Language_current() != language_french)
        Language_setCurrent(language_french);
    else
        Language_setCurrent(language_english);
    Sound_play(0, 1, 0, 0);
    root_changeViewActiveTo(&my_root, View_createTest());
}
void button_firework_action_(Button* b) {
    Sparkle_spawnOverAndOpen(&b->n, (0.5f + b->n.float3) * 1.5f);
}
void button_quit_action_(Button* bt) {
    Sound_play(sound_sheep, 1, 0, 0);
    root_changeViewActiveTo(&my_root, NULL);
}
static bool install_font_ = true;
void button_installFont_action_(Button* b) {
    Sound_play(sound_note_piano, 1, 0, 0);
    if(CoqSystem_OS_type() == coqsystem_os_desktop) return;
    CoqEvent_addToWindowEvent((CoqEventWin) {
        .type = install_font_ ? event_type_win_ios_fonts_install : event_type_win_ios_fonts_uninstall
    });
    install_font_ = !install_font_;
}

/// Dummy action pour bouton...
void sl_button_action_(Button* b) {
    printdebug("Touching %d.", b->n.uint0);
}
/// Exemple d'une string localisée avec un cadre.
Node* Node_createFramedLoc_(Localizable loc, float widthMax) {
    Button* b = Button_create(NULL, sl_button_action_, 0, 0, 1, 0, flag_noParent);
    b->n.w = widthMax;
    b->n.uint0 = loc;
    StringGlyphedInit str = loc_stringGlyphedInit(loc);
    node_addFramedString(&b->n, png_frame_mocha, str, framedString_defPars);
    return &b->n;
}

View* View_createTest(void) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = coq_callocTyped(View);
    view_initWithSuper(v, &my_root, flag_viewDontAlignElements);
    // Action au "enter"
    v->enterOpt = view_enter_;
    // Keyboard input
    v->keyDownOpt = view_keyDown_;
    // Lettre
    DrawableChar* dc = DrawableChar_create(&v->n, (Character) { .c_str = "j" }, 0, 0.0, 1, 0);
//    DrawableChar_create(&v->n, (Character) { .c_str = "p" }, 0.25, 0, 1, 0);
    Drawable_createTestFrame(&v->n,  0, 0, 2*node_deltaX(&dc->n), 2*node_deltaY(&dc->n));
//    Drawable_createTestFrame(&v->n, 0.2, 0, 1, 1);
//    Drawable_createTestFrame(&v->n, -0.25, 0, 0.5, 1);
    
    // Image du chat sur un `bloc` fluid.
//    Fluid* bloc = Fluid_create(&v->n, -0.75f, 0.1f, 1.f, 1.f, 5.f,
//                               flag_fluidFadeInRight, 0);
//    Drawable_createImageWithName(&bloc->n, "coqlib_the_cat", 0, 0, 0.5, 0);
    
    // bouton test (change la langue)...
//    StringGlyphedInit str = loc_stringGlyphedInit(loc_app_name);
//    ButtonHoverable_create(&v->n, button_changeLanguage_action_,
//                           png_frame_white_back, str,
//                          0, 0.0, 0.5, 10, 0);
//    node_last_nonDrawable->w = 2.2f;
//    node_last_addFramedString(png_frame_mocha, str,
//                              framedString_defPars);
                              
    
    // Bouton quitter
//    str.c_str = loc_stringKey(loc_quit);
//    ButtonHoverable_create(&v->n, button_quit_action_,
//                           png_frame_white_back, str,
//                           -0.55, -0.6, 0.15, 10, 0);
//    node_last_nonDrawable->w = 0.6f;
//    node_last_addFramedString(png_frame_mocha, str,
//                              framedString_defPars);
    
    // Bouton secure
//    SecurePopInfo spi = {
//        2.f, png_disks, disk_color_red,
//        png_frame_red, { .c_str = "Hold Button !" },
//    };
//    ButtonSecure_create(&v->n, button_installFont_action_, spi, -0.5, 0.5, 0.25, 10, 0);
//    node_last_addIcon(png_disks, disk_color_orange, png_icons, icon_help);
    
    // Drawabe avec array de pixels en bgra (0xAARRGGBB).
//    static PixelBGRA pixels[] = {
//        {0xFFFF0000}, {0x80FF0000},
//        {0xFF00FF00}, {0xFF0000FF},
//    };
//    Drawable* d = coq_callocTyped(Drawable);
//    node_init(&d->n, &v->n, -0.2, 0.5, 0.25, 0.25, node_type_n_drawable, 0, 0);
//    drawable_init(d, Texture_createWithPixels(pixels, 2, 2, false, true), mesh_sprite,
//                   0.25, 0.25);
    // Chargement de bitmaps
//    const char* bmp_path = FileManager_getResourcePathOpt("test24bits", "bmp", "assets");
//    uint32_t bmp_width, bmp_height;
//    const PixelBGRA* pixels2 = FILE_bitmapBGRA8ContentOptAt(bmp_path, &bmp_width, &bmp_height, true);
//    if(pixels2) {
//        Drawable* d2 = coq_callocTyped(Drawable);
//        node_init(&d2->n, &v->n, -0.2, 0.22, 0.25, 0.25, node_type_n_drawable, 0, 0);
//        drawable_init(d2, Texture_createWithPixels(pixels2, bmp_width, bmp_height, false, true),
//                       mesh_sprite, 0, 0.25);
//    }
//    bmp_path = FileManager_getResourcePathOpt("test32bits", "bmp", "assets");
//    pixels2 = FILE_bitmapBGRA8ContentOptAt(bmp_path, &bmp_width, &bmp_height, true);
//    if(pixels2) {
//        Drawable* d2 = coq_callocTyped(Drawable);
//        node_init(&d2->n, &v->n, -0.2, -0.05, 0.25, 0.25, node_type_n_drawable, 0, 0);
//        drawable_init(d2, Texture_createWithPixels(pixels2, bmp_width, bmp_height, false, true),
//                       mesh_sprite, 0, 0.25);
//    }
    
    // Title
//    str.c_str = loc_stringKey(loc_app_name);
//    NodeString* ns = NodeString_create(&v->n, str, 0, 0.0, 0.0, 0.5, 0, 0);
//    Drawable_createTestFrame(&v->n, 0, 0, 2*node_deltaX(&ns->n), 2*node_deltaY(&ns->n));
    
    // Pop disk
//    PopDisk_spawnAndOpen(&v->n, NULL, png_disks, disk_color_blue, 3.f, -0.5f, -0.75f, 0.3f);

    // Menu deroulant
//    SlidingMenuInit sid = { 4, 1.2 };
//    SlidingMenu_create(&v->n, sid,  0.75, 0.f, 1.2, 1.5, 0, 0);
//    float itemW = slidingmenu_last_itemRelativeWidth()*0.95;
//    slidingmenu_last_addItem(&Button_createSlider(NULL, button_firework_action_, 0.5,
//                        0, 0, itemW, 1, 0, flag_noParent)->n);
//    slidingmenu_last_addItem(&Button_createSwitch(NULL, button_firework_action_, false,
//                        0, 0, 1, 0, flag_noParent)->n);
//    slidingmenu_last_addItem(Node_createFramedLoc_(loc_ok, itemW));
//    slidingmenu_last_addItem(Node_createFramedLoc_(loc_error, itemW));
//    slidingmenu_last_addItem(Node_createFramedLoc_(loc_menu, itemW));
//    slidingmenu_last_addItem(Node_createFramedLoc_(loc_app_name, itemW));
//    slidingmenu_last_addItem(Node_createFramedLoc_(loc_quit, itemW));
//    slidingmenu_last_addItem(Node_createFramedLoc_(loc_error_empty, itemW));
    
    return v;
}

View* View_createTest2(void) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = coq_callocTyped(View);
    view_initWithSuper(v, &my_root, flag_viewDontAlignElements);
    // Test cellular
    CelGrid_create(&v->n, 0,  0.5, 0.9, 128, 64, false);
    CelGrid_create(&v->n, 0, -0.5, 0.9, 128, 64, true);
     
    return v;
}

void  view3_enter_(View* v) {
    
}

View* View_createTest3(void) {
    View* v = coq_callocTyped(View);
    view_initWithSuper(v, &my_root, flag_viewDontAlignElements);
    
    v->enterOpt = view3_enter_;
    
    StringGlyphedInit data = {"Bonjour, comment allez-vous ?\nCe n'est qu'un sale chien qui jappe, mais qui n'a pas le droit de japper.", 
        .x_margin = 0.5, .spacing = 0.1 };
//    SlidingInitData sid = {2, 1, relative_justifiedRight, true};
//    SlidingMenu* sm = SlidingMenu_create(&v->n, sid, 0, -0.5, 1.8, 0.6, 0, 0);
//    slidingmenu_addMultiStrings(sm, data);
//    v->n.node0 = (Node*)sm;

    Node* n = coq_callocTyped(Node);
    node_init(n, &v->n, 0, 0, 1, 1, 0, 0, 0);
    node_addMultiStrings(n, data, 2.5, 0.15, relatives_right, spchar_dodo);
    Node* const first = n->_firstChild;
    Node* const last = n->_lastChild;
    Drawable_createImageWithWidth(&v->n, png_coqlib_test_frame, 0, 0.0, 
            2*node_deltaX(n), 2*node_deltaY(n), 0);
    for(Node* c = first; ; c = c->_littleBro) {
        Drawable_createImageWithWidth(n, png_coqlib_test_frame, c->x, c->y, 
            2*node_deltaX(c), 2*node_deltaY(c), 0);
        if(c == last) break;
    }
    
    
//    data.c_str = "Bonjour.";
//    NodeString* ns = NodeString_create(&v->n, data, 0, 0.5, 0.2, 0, 0);
//    Drawable_createImageWithWidth(&v->n, png_coqlib_test_frame, 0, 0.5, 
//            2*node_deltaX(&ns->n), 2*node_deltaY(&ns->n), 0);
    
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, &v->n, 1, -0.8, 0.4, 0.4, node_type_n_drawable, 0, 0);
    drawable_init(d, StringGlyphed_defaultGlyphMapTexture(), mesh_sprite, 0.4, 0.4);
    
//    Drawable_createTestFrame(&v->n, 0, 0, size*glyphInfo.relSolidWidth, size);
    return v;
}

View* View_createTestFont(void) {
    View* v = coq_callocTyped(View);
    view_initWithSuper(v, &my_root, flag_viewDontAlignElements);
    // Lettre
    DrawableChar* dc = DrawableChar_create(&v->n, (Character) { .c_str = "j" }, 0, 0.5, 1, 0);
    Drawable_createTestFrame(&v->n,  0, 0.5, 2*node_deltaX(&dc->n), 2*node_deltaY(&dc->n));
    
    // String
    StringGlyphedInit str = {
        .c_str = "Bonjour",
    };
    NodeString* ns = NodeString_create(&v->n, str, 0, -0.5, 0, 1, 0, 0);
    Drawable_createTestFrame(&v->n,  0, -0.5, 2*node_deltaX(&ns->n), 2*node_deltaY(&ns->n));
    
    
    return v;
}

Root* Root_initAndGetProjectRoot(void) {
    if(my_root.n._type) { printerror("Root already init."); return &my_root; }
    node_init(&my_root.n, NULL, 0, 0, 4, 4, node_type_root, flags_rootDefault, 0);
    root_init(&my_root, NULL);
    
    // Tester avec une font custom.
    //  Snell Roundhand  Times New Roman
    FontGlyphMap* fgm = FontGlyphMap_create("Snell Roundhand", 80, 0, true, &((FontGlyphMapCustomChars){
            .count = 1,
            .texture = Texture_sharedImageByName("coqlib_the_cat"),
            .chars = (Character[]){ spchar_newline_ }, 
            .uvRects = (Rectangle[]) {{0,0,1,1}},
        }));
    StringGlyphed_giveDefaultFontGlyphMap(&fgm);
    fontglyphmapref_releaseAndNull(&fgm);
//    Texture_setCurrentFont("OpenDyslexic3");
//    Chrono_UpdateDeltaTMS = 20;
    
    // Création de l'arrière plan (avec particule pool)
    my_root.viewBackOpt = coq_callocTyped(View);
    view_initWithSuper(my_root.viewBackOpt, &my_root, flags_viewBackAndFrontDefault);
    
    // Création de l'avant plan (pour les feux d'artifices, popover...)
    my_root.viewFrontOpt = coq_callocTyped(View);
    view_initWithSuper(my_root.viewFrontOpt, &my_root, flags_viewBackAndFrontDefault);
    PopingNode_setFrontView(my_root.viewFrontOpt);
    Sparkle_init(Texture_sharedImageByName("coqlib_sparkle_stars"), sound_fireworks);
    
    // Création d'une view de test.
    root_changeViewActiveTo(&my_root, View_createTestFont());
    
    
    return &my_root;
}
