//
//  my_root.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#include "my_root.h"
#include "my_enums.h"
#include "cellular.h"

#include "coq_sound.h"
#include "utils/util_system.h"
#include "utils/util_file.h"
#include "utils/util_language.h"
#include "nodes/node_number.h"

static Root my_root = {};

View* View_createTest(void);

// Actions pour les boutons et enter...
void view_enter_(View* v) {
    root_changeViewActiveTo(&my_root, View_createTest());
}
void view_keyDown_(View* v, KeyboardInput key) {
    printdebug("Key down. mkc %d, str %s, mod %x.", key.mkc, key.typed.c_str, key.modifiers);
}

void button_changeLanguage_action_(Button* bt, Vector2 unused) {
    if(Language_current() != language_french)
        Language_setCurrent(language_french);
    else
        Language_setCurrent(language_english);
    Sound_play(0, 1, 0, 0);
    root_changeViewActiveTo(&my_root, View_createTest());
}
void button_firework_action_(Button* b) {
    Sparkle_spawnOverAndOpen(&b->n, (0.5f + button_slider_value(b)) * 1.5f);
}
void button_quit_action_(Button* bt, Vector2 unused) {
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
void sl_button_action_(Button* b, Vector2 pos) {
    printdebug("Touching %d at %f, %f.", b->n.nodrawData.uint0, pos.x, pos.y);
}
/// Exemple d'une string localisée avec un cadre.
Node* Node_createFramedLoc_(Localizable loc, float widthMax) {
    Button* b = Button_create(NULL, sl_button_action_, 0, 0, 1, 0, flag_noParent);
    b->n.w = widthMax;
    b->n.nodrawData.uint0 = loc;
    NodeStringInit str = loc_stringGlyphedInit(loc);
    node_addFramedString(&b->n, png_frame_mocha, str, framedString_defPars);
    return &b->n;
}

View* View_createTest(void) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = coq_callocTyped(View);
    view_and_super_init(v, &my_root, flag_viewDontAlignElements);
    // Action au "enter"
    v->enterOpt = view_enter_;
    // Keyboard input
    v->keyDownOpt = view_keyDown_;
    
    // Glyph par défaut pour les strings.
    if(!GlyphMap_default_isInit()) GlyphMap_default_init((GlyphMapInit) {});
    // Structure
    // Title
    NodeString_create(&v->n, loc_stringGlyphedInit(loc_app_name), 0, 0.9, 0.0, 0.1, 0, 0);
    // Image du chat sur un `bloc` fluid qui "fade in".
    Fluid* bloc = Fluid_create(&v->n, -0.75f, 0.1f, 1.f, 1.f, 5.f,
                               flag_fluidFadeInRight, 0);
    Drawable_createImageWithName(&bloc->n, "coqlib_the_cat", 0, 0, 0.5, 0);
    // bouton test (change la langue)...
    Node_last = &ButtonHoverable_create(&v->n, button_changeLanguage_action_,
                           png_frame_white_back, loc_stringGlyphedInit(loc_app_name),
                           -1.0, 0.5, 0.2, 10, 0)->n;
    Node_last->w = 2.2f;
    node_last_addFramedString(png_frame_mocha, loc_stringGlyphedInit(loc_ok),
                              framedString_defPars);
    // Bouton quitter
    Node_last = &ButtonHoverable_create(&v->n, button_quit_action_,
                           png_frame_white_back, loc_stringGlyphedInit(loc_quit),
                           -0.55, -0.6, 0.15, 10, 0)->n;
    Node_last->w = 0.6f;
    node_last_addFramedString(png_frame_mocha, loc_stringGlyphedInit(loc_quit),
                              framedString_defPars);

    // Bouton secure
    SecurePopInfo spi = {
        .holdTimeSec = 2.f,
        .popPngId = png_disks, .popTile = disk_color_red,
        .failPopFramePngId = png_frame_red,
        .failMessage = { .c_str = "Hold Button !" },
    };
    Button *b = ButtonSecure_create(&v->n, button_installFont_action_, spi,
                        -0.5, 0.5, 0.25, 10, flag_nodeLast);
    Drawable_createImage(&b->n, png_disks, 0, 0, 0.25, 0);
    drawable_last_setExtra1(0.1);
    drawable_last_setTile(disk_color_orange, 0);
    Drawable_createImage(&b->n, png_icons, 0, 0, 0.25, 0);
    drawable_last_setTile(icon_help, 0);

    // Drawabe avec array de pixels en bgra (0xAARRGGBB).
    static PixelBGRA pixels[] = {
        {0xFFFF0000}, {0x80FF0000},
        {0xFF00FF00}, {0xFF0000FF},
    };
    if_let(Drawable*, d, coq_callocTyped(Drawable))
    node_init(&d->n, &v->n, -0.2, 0.5, 0.25, 0.25, 0, 0);
    drawable_init(d, Texture_createWithPixels(pixels, 2, 2, false, true), Mesh_drawable_sprite,
                   0.25, 0.25);
    if_let_end
    // Chargement de bitmaps
    const char* bmp_path = FileManager_getResourcePathOpt("test24bits", "bmp", "assets");
    uint32_t bmp_width, bmp_height;
    const PixelBGRA* pixels2 = FILE_bitmapBGRA8ContentOptAt(bmp_path, &bmp_width, &bmp_height, true);
    if(pixels2) {
        Drawable* d2 = coq_callocTyped(Drawable);
        node_init(&d2->n, &v->n, -0.2, 0.22, 0.25, 0.25, 0, 0);
        drawable_init(d2, Texture_createWithPixels(pixels2, bmp_width, bmp_height, false, true),
                      Mesh_drawable_sprite, 0, 0.25);
    }
    bmp_path = FileManager_getResourcePathOpt("test32bits", "bmp", "assets");
    pixels2 = FILE_bitmapBGRA8ContentOptAt(bmp_path, &bmp_width, &bmp_height, true);
    if(pixels2) {
        Drawable* d2 = coq_callocTyped(Drawable);
        node_init(&d2->n, &v->n, -0.2, -0.05, 0.25, 0.25, 0, 0);
        drawable_init(d2, Texture_createWithPixels(pixels2, bmp_width, bmp_height, false, true),
                      Mesh_drawable_sprite, 0, 0.25);
    }
    // Drawable de char
    Drawable_createCharacter(&v->n, (Character) { .c_str = "A" }, NULL, -0.2, -0.35, 0.15, 0);

    // Pop disk
    PopDisk_spawnOverAndOpen(NULL, NULL, png_disks, disk_color_blue, 3.f, 0.0f, -0.75f, 0.2f);
    
    // Test drawable
    if_let(Drawable*, d, coq_callocTyped(Drawable))
    Mesh *const mesh = Mesh_create((MeshInit) {
//        .verticesOpt = mesh_vertices_,
        .vertexCount = 4,
        .primitive_type = mesh_primitive_triangleStrip,
        .cull_mode = mesh_cullMode_none,
        .flags = mesh_flag_mutable,
    });
    node_init(&d->n, &v->n, -1.00, -0.35, 0.5, 0.5, 0, 0);
    drawable_init(d, Texture_sharedImage(png_frame_red), mesh, 0.5, 0.5);
    if_let_end

    // Menu deroulant
    SlidingMenuInit sid = { 4, 1.2 };
    SlidingMenu_create(&v->n, sid,  0.75, 0.f, 1.2, 1.5, 0, 0);
    float itemW = slidingmenu_last_itemRelativeWidth()*0.95;
    slidingmenu_last_addItem(&Button_createSlider(NULL, button_firework_action_, 0.5,
                        0, 0, itemW, 1, 0, flag_noParent)->n);
    slidingmenu_last_addItem(&Button_createSwitch(NULL, button_firework_action_, false,
                        0, 0, 1, 0, flag_noParent)->n);
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_ok, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_error, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_menu, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_app_name, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_quit, itemW));
    slidingmenu_last_addItem(Node_createFramedLoc_(loc_error_empty, itemW));

    return v;
}

View* View_createTest2(void) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = coq_callocTyped(View);
    view_and_super_init(v, &my_root, flag_viewDontAlignElements);
    // Test cellular
    CelGrid_create(&v->n, 0,  0.5, 0.9, 128, 64, false);
    CelGrid_create(&v->n, 0, -0.5, 0.9, 128, 64, true);

    return v;
}

void  view3_enter_(View* v) {

}

View* View_createTest3(void) {
    View* v = coq_callocTyped(View);
    view_and_super_init(v, &my_root, flag_viewDontAlignElements);

    Drawable_createColor(&v->n, color4_red, 0, 0.5, 0.5, 0.5);
    Drawable_createImage(&v->n, png_coqlib_test_frame, 0, -0.5, 0.5, 0);

    return v;
}

void view_testfont_enter_(View* v) {
    NodeString* ns = node_asNodeStringOpt(v->n.nodrawData.node0);
    if(!ns) { printerror("No string ?"); return; }
    printdebug("Updating to Hello");
    nodestring_updateString(ns, "Hello");
}
void view_testfont_escape_(View* v) {
    NodeString* ns = node_asNodeStringOpt(v->n.nodrawData.node0);
    if(!ns) { printerror("No string ?"); return; }
    printdebug("Updating to sale chien...");
    nodestring_updateString(ns, "Ce n'est qu'un sale chien que voudrait bien japper.");
}
View* View_createTestFont(void) {
    View* v = coq_callocTyped(View);
    view_and_super_init(v, &my_root, flag_viewDontAlignElements);
    v->enterOpt = view_testfont_enter_;
    v->escapeOpt = view_testfont_escape_;

    // Tester avec une font custom.
    GlyphMap_default_init((GlyphMapInit) {
        .fontInit = {
//            .nameOpt = "American Typewriter",
//            .nameOpt = "Chalkduster",
//            .nameOpt = "PilGi",
//            .nameOpt = "Snell Roundhand",
//            .nameOpt = "Luciole",
//            .nameOpt = "Noto Nastaliq Urdu",
            .fileNameOpt = "Supplemental/Arial Unicode.ttf",
//            .fileNameOpt = "Supplemental/SnellRoundhand.ttc",
            .sizeOpt = 60,
            .nearest = true,
        },
        .customChars_count = 1,
        .customChars_texOpt = Texture_sharedImageByName("coqlib_the_cat"),
        .customChars_charsOpt = (Character[]){ spchar_newline_ },
        .customChars_uvRectsOpt = (Rectangle[]) { {{0,0,1,1}}, },
    });
    // Lettre
    DrawableChar* dc = DrawableChar_create(&v->n, (Character) { .c_str = "j" },
                                           -0.5, 0.0, 0.5, 0);
    Drawable_createTestFrame(&v->n,  -0.5, 0.0, 2*node_deltaX(&dc->n), 2*node_deltaY(&dc->n));
    Drawable_createTestFrame(&v->n,  -0.5 + dc->glyphX, dc->glyphY, dc->n.sx, dc->n.sy);
    
    dc = DrawableChar_create(&v->n, (Character) { .c_str = "A" },
                                           -0., 0.0, 0.5, 0);
    Drawable_createTestFrame(&v->n,  -0., 0.0, 2*node_deltaX(&dc->n), 2*node_deltaY(&dc->n));
    Drawable_createTestFrame(&v->n,  -0. + dc->glyphX, dc->glyphY, dc->n.sx, dc->n.sy);
    
    dc = DrawableChar_create(&v->n, (Character) { .c_str = "À" },
                                           0.5, 0.0, 0.5, 0);
    Drawable_createTestFrame(&v->n,  0.5, 0.0, 2*node_deltaX(&dc->n), 2*node_deltaY(&dc->n));
    Drawable_createTestFrame(&v->n,  0.5 + dc->glyphX, dc->glyphY, dc->n.sx, dc->n.sy);
    
    // String
//    NodeStringInit str = {
//        .c_str =  "AjxfÀ" , // "ينهك",
//        .maxCountOpt = 60,
//        .x_margin = 0.25,
//    };
// String drawable
//    coq_Font const *cf = GlyphMap_default_font();
//    Drawable* d = Drawable_test_createString(&v->n, str.c_str, cf, 0, -0.0, 0.75, 0);
//    Drawable_createTestFrame(&v->n, 0, -0.0, 2*node_deltaX(&d->n), 2*node_deltaY(&d->n));
//    Drawable_createTestFrame(&v->n, 0, -0.0, d->n.sx, d->n.sy);
//    Node* n = Node_create(&v->n, 0, 0.7, 0, 0.4, 0, 0);
//    node_addFramedString(n, png_frame_mocha, str, framedString_defPars);

//    Number* nb = Number_create(&v->n, 256, 0, 0.5, 0.25, 0, 0);
//    nb->extraDigitOpt = digit_percent;

//    NodeString* ns = NodeString_create(&v->n, str, 0, 0, 2, 0.4, 0, 0);
//    ns->n.renderer_updateInstanceUniforms = nodestring_renderer_updateIUsMoving;
//    ns->n.nodrawData.float0 = 0.5f;
//    ns->n.nodrawData.float1 = 5.0f;
//    v->n.nodrawData.node0 = &ns->n;
//    Drawable_createTestFrame(&v->n,  0, 0, 2*node_deltaX(&ns->n), 2*node_deltaY(&ns->n));
    
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, &v->n, 0.75, 0.5, 1, 1, 0, 0);
    drawable_init(d, GlyphMap_default_texture(), Mesh_drawable_sprite, 0, 1);

    return v;
}

Root* Root_initAndGetProjectRoot(void) {
    if(my_root.n._type) { printerror("Root already init."); return &my_root; }
    root_and_super_init(&my_root, NULL, NULL);

    // Création de l'arrière plan (avec particule pool)
    fl_array_fix(my_root.back_RGBA, color4_blue_sky.f_arr, 4);
    fl_array_set(my_root.back_RGBA, color4_gray_60.f_arr, 4);
    my_root.viewBackOpt = coq_callocTyped(View);
    view_and_super_init(my_root.viewBackOpt, &my_root, flags_viewBackAndFrontDefault);

    // Création de l'avant plan (pour les feux d'artifices, popover...)
    my_root.viewFrontOpt = ({
        View *v = coq_callocTyped(View);
        view_and_super_init(v, &my_root, flags_viewBackAndFrontDefault);
        PopingNode_setFrontView(v);
        v;
    });

    // Création d'une view de test.
    root_changeViewActiveTo(&my_root, View_createTestFont());

    // Init des fireworks...
    Sparkle_init(Texture_sharedImageByName("coqlib_sparkle_stars"), sound_fireworks);

    return &my_root;
}
