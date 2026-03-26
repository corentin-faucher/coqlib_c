//
//  my_root.c
//  coqlib_test
//
//  Created by Corentin Faucher on 2024-01-04.
//

#include "my_root.h"
#include "my_enums.h"
#include "cellular.h"
#include "my_particules.h"

#include "systems/system_sound.h"
#include "systems/system_base.h"
#include "systems/system_file.h"
#include "systems/system_language.h"
#include "nodes/node_number.h"

//static Root my_root = {};

View* View_createTest(Root* root);

// Actions pour les boutons et enter...
void view_enter_(View* v) {
    guard_let(Root*, root, node_asRootOpt(v->n._parent), , )
    root_changeViewActiveTo(root, View_createTest(root));
}
//void view_keyDown_(View* v, KeyboardInput key) {
//    printdebug("Key down. mkc %d, str %s, mod %x.", key.mkc, key.typed.c_str, key.modifiers);
//}

void button_changeLanguage_action_(NodeTouch nt) {
    if(Language_current() != language_french)
        Language_setCurrent(language_french);
    else
        Language_setCurrent(language_english);
    Sound_play(0, 1, 0, 0);
    root_changeViewActiveTo(nt.root, View_createTest(nt.root));
}
void button_firework_action_(NodeTouch const nt) {
    Sparkle_spawnOverAndOpen(nt.n, (0.5f + nt.n->nodrawData.data1.v.w) * 1.5f);
}
void button_quit_action_(NodeTouch nt) {
    Sound_play(sound_sheep, 1, 0, 0);
    root_changeViewActiveTo(nt.root, NULL);
}
static bool install_font_ = true;
void button_installFont_action_(NodeTouch __attribute__((unused)) unused) {
    Sound_play(sound_note_piano, 1, 0, 0);
    if(CoqSystem_OS_type() == coqsystem_os_desktop) return;
    CoqEvent_addToWindowEvent((CoqEventWin) {
        .type = install_font_ ? eventtype_win_ios_fonts_install : eventtype_win_ios_fonts_uninstall
    });
    install_font_ = !install_font_;
}

/// Dummy action pour bouton...
void sl_button_action_(NodeTouch const bt) {
    printdebug("Touching %d at %f, %f.", bt.n->nodrawData.data0.u0, bt.posAbs.x, bt.posAbs.y);
}
/// Exemple d'une string localisée avec un cadre.
Node* Node_createFramedLoc_(Localizable loc, float widthMax) {
    Button* b = Button_create(NULL, sl_button_action_, 0, 0, 1, 0, flag_noParent);
    b->n.w = widthMax;
    b->n.nodrawData.data0.u0 = loc;
    StringGlyphedInit str = loc_stringGlyphedInit(loc);
    node_addFramedString(&b->n, png_coqlib_frame_mocha, str, framedString_defPars);
    return &b->n;
}

View* View_createTest(Root*const root) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = coq_callocTyped(View);
    view_and_super_init(v, root, flag_viewDontAlignElements);
    // Action au "enter"
    v->enterOpt = view_enter_;
    // Keyboard input
//    v->keyDownOpt = view_keyDown_;
    
    // Structure
    PPNode_create(&v->n);
    // Title
    NodeString_create(&v->n, loc_stringGlyphedInit(loc_app_name), 0, 0.9, 0.0, 0.1, 0, 0);
    // Image du chat sur un `bloc` fluid qui "fade in".
    Fluid* bloc = Fluid_create(&v->n, -0.75f, 0.1f, 1.f, 1.f, 5.f,
                               flag_fluidFadeInRight, 0);
    Drawable_createImageWithName(&bloc->n, "coqlib_the_cat", 0, 0, 0.5, 0);
    // bouton test (change la langue)...
    Node_last = &ButtonHoverable_create(&v->n, button_changeLanguage_action_,
                           png_coqlib_frame_white_back, loc_stringGlyphedInit(loc_app_name),
                           -1.0, 0.5, 0.2, 10, 0)->n;
    Node_last->w = 2.2f;
    node_last_addFramedString(png_coqlib_frame_mocha, loc_stringGlyphedInit(loc_ok),
                              framedString_defPars);
    // Bouton quitter
    Node_last = &ButtonHoverable_create(&v->n, button_quit_action_,
                           png_coqlib_frame_white_back, loc_stringGlyphedInit(loc_quit),
                           -0.75, -0.6, 0.15, 10, 0)->n;
    Node_last->w = 0.6f;
    node_last_addFramedString(png_coqlib_frame_mocha, loc_stringGlyphedInit(loc_quit),
                              framedString_defPars);

    // Bouton secure
    SecurePopInfo spi = {
        .holdTimeSec = 2.f,
        .popPngId = png_disks, .popTile = disk_color_red,
        .failPopFramePngId = png_coqlib_frame_red,
        .failMessage = { .c_str = "Hold Button !" },
    };
    Button *b = ButtonSecure_create(&v->n, button_installFont_action_, spi,
                        -0.5, 0.5, 0.25, 10, flag_nodeLast);
    Drawable_createImage(&b->n, png_disks, 0, 0, 0.25, 0);
    drawable_last_setExtra1(0.1);
    drawable_last_setTile(disk_color_orange, 0);
    Drawable_createImage(&b->n, png_icons, 0, 0, 0.25, 0);
    drawable_last_setTile(icon_help, 0);

    // Drawabe avec array de pixels en bgra (0xAABBGGRR).
    static PixelRGBA pixels[] = {
        { 0xFFFF0000 }, { 0x80FF0000 },
        { 0xFF00FF00 }, { 0xFF0000FF },
    };
    if_let(Drawable*, d, coq_callocTyped(Drawable))
    node_init(&d->n, &v->n, -0.2, 0.5, 0.25, 0.25, 0, 0);
    drawable_init(d, Texture_createWithPixels(pixels, 2, 2, tex_flag_nearest), NULL,
                   0.25, 0.25);
    if_let_end
    
    // Chargement de bitmaps
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, &v->n, -0.2, 0.22, 0.25, 0.25, 0, 0);
    char* path = FileManager_getResourcePath();
    String_pathAdd(path, "test24bits", "bmp", "assets");
    with_beg(PixelArray, pa, PixelArray_createFromBitmapFileOpt(path, true))
    drawable_init(d, Texture_createWithPixels(pa->pixels, 
                  (uint32_t)pa->width, (uint32_t)pa->height,
                  tex_flag_nearest), NULL, 0, 0.25);
    with_end(pa)
    
    d = coq_callocTyped(Drawable);
    node_init(&d->n, &v->n, -0.2, -0.05, 0.25, 0.25, 0, 0);
    path = FileManager_getResourcePath();
    String_pathAdd(path, "test32bits", "bmp", "assets");
    with_beg(PixelArray, pa, PixelArray_createFromBitmapFileOpt(path, false))
    drawable_init(d, Texture_createWithPixels(pa->pixels, 
                  (uint32_t)pa->width, (uint32_t)pa->height,
                  tex_flag_nearest), NULL, 0, 0.25);
    with_end(pa)
    
    // SVG
    d = coq_callocTyped(Drawable);
    node_init(&d->n, &v->n, -0.2, -0.30, 0.25, 0.25, 0, 0);
    path = FileManager_getResourcePath();
    String_pathAdd(path, "emoji_u1f431", "svg", "assets");
    with_beg(PixelArray, pa, PixelArray_createFromSvgFileOpt(path, 64, true))
    drawable_init(d, Texture_createWithPixels(pa->pixels, 
                  (uint32_t)pa->width, (uint32_t)pa->height,
                  tex_flag_nearest), NULL, 0, 0.25);
    with_end(pa)
    
    // Drawable de char
    DrawableChar_create(&v->n, (Character) { .c_str = "A" }, -0.2, -0.55, 0.15, 0);

    // Pop disk
    PopDisk_spawnOverAndOpen(NULL, NULL, png_disks, disk_color_blue, 3.f, 0.0f, -0.75f, 0.2f);
    
    // Menu deroulant
    SlidingMenu_create(&v->n, 
        (SlidingMenuInit) { .displayedCount = 4, .spacing = 1.2 },
        0.75, 0.f, 1.2, 1.5, 0, 0
    );
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

View* View_createTest2(Root*const root) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = coq_callocTyped(View);
    view_and_super_init(v, root, flag_viewDontAlignElements);
    // Test cellular
    CelGrid_create(&v->n, 0,  0.5, 0.9, 128, 64, false);
    CelGrid_create(&v->n, 0, -0.5, 0.9, 128, 64, true);

    return v;
}


View* View_createTest3(Root*const root) {
    View* v = coq_callocTyped(View);
    view_and_super_init(v, root, flag_viewDontAlignElements);
    
    printdebug("Un disque jaune.");
    Drawable_createImage(&v->n, png_disks, 0, 0, 1, 0);
    // Test
    Drawable* d = coq_callocTyped(Drawable);
    node_init(&d->n, &v->n, 0, 0, 1, 1, 0, 0);
    char*const objPath = FileManager_getResourcePath();
    String_pathAdd(objPath, "singe2", "obj", "assets");
    drawable_init(d, Texture_getPng(png_some_animals), 
        Mesh_createFromObjFile(objPath), 0, 1);
    
    

    return v;
}

void  view_testfont_enter_(View* v) {
    NodeString* ns = node_asNodeStringOpt(v->n.nodrawData.data0.ptr0);
    if(!ns) { printerror("No string ?"); return; }
    printdebug("Updating to Hello");
    nodestring_updateString(ns, "Hello");
}
void  view_testfont_escape_(View* v) {
    NodeString* ns = node_asNodeStringOpt(v->n.nodrawData.data0.ptr0);
    if(!ns) { printerror("No string ?"); return; }
    printdebug("Updating to sale chien...");
    nodestring_updateString(ns, "Ce n'est qu'un sale chien que voudrait bien japper.");
}
View* View_createTestFont(Root*const root) {
    View* v = coq_callocTyped(View);
    view_and_super_init(v, root, flag_viewDontAlignElements);
    v->enterOpt = view_testfont_enter_;
    v->escapeOpt = view_testfont_escape_;

    // Lettre
    DrawableChar* dc = DrawableChar_create(&v->n, (Character) { .c_str = "f" },
                                           -0.5, 0.0, 0.5, 0);
    Box b = node_hitbox(&dc->n);
    Drawable_createTestFrame2(&v->n, b);
    b = drawablechar_getGlyphBox(dc);
    Drawable_createTestFrame2(&v->n, b);
    
    dc = DrawableChar_create(&v->n, (Character) { .c_str = "🐺" },
                                           0, 0.0, 0.5, 0);
    b = node_hitbox(&dc->n);
    Drawable_createTestFrame2(&v->n, b);
    b = drawablechar_getGlyphBox(dc);
    Drawable_createTestFrame2(&v->n, b);
    
    dc = DrawableChar_create(&v->n, (Character) { .c_str = "T" },
                                           0.5, 0.0, 0.5, 0);
    b = node_hitbox(&dc->n);
    Drawable_createTestFrame2(&v->n, b);
    b = drawablechar_getGlyphBox(dc);
    Drawable_createTestFrame2(&v->n, b);
    
    // String
    StringGlyphedInit str = {
        .c_str =  "JWxjfÀ" , // "ينهك",
        .maxCountOpt = 60,
        .x_margin = 0.25,
    };
    // String avec frame
    Node* n = Node_create(&v->n, 0, -0.7, 0, 0.4, 0, 0);
    node_addFramedString(n, png_coqlib_frame_mocha, str, framedString_defPars);

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
    drawable_init(d, GlyphMap_default_texture(), NULL, 0, 1);

    return v;
}

void test_chars_(void) {
    char const*const test_str = "Bonjour ⁉️ Ca va ? 🐔-🐭 🇯🇵";
    with_beg(CharacterArray, ca, CharacterArray_createFromString(test_str))
    Character const*const c_end = characterarray_end(ca);
    printdebug("Test string : %s", test_str);
    for(Character const* c = characterarray_first(ca); c < c_end; c++) {
        printdebug("Char %s, isEmoji %d, hex %#018llx.", 
            c->c_str,
            character_isEmoji(*c), c->c_data8
        );
    }
    with_end(ca)
    printdebug("fin de test.");
    Character const c = (Character) { .c_str = "⁉️"};
    printdebug("Char de ⁉️ : ");
    for(int i = 0; i < 8; i++) {
        printf("%#010x ,", c.c_str[i]);
    }
}

Root* Root_createMyProjectRoot(void) {
    Root*const root = coq_callocTyped(Root);
    root_and_super_init(root, NULL, NULL);
    
    // Création de l'arrière plan (avec particule pool)
    Vector4 backColor = color4_blue_sky;
    fl_array_fix(root->back_RGBA, backColor.f_arr, 4);
    backColor = color4_gray_60;
    fl_array_set(root->back_RGBA, backColor.f_arr, 4);
    root->viewBackOpt = coq_callocTyped(View);
    view_and_super_init(root->viewBackOpt, root, flags_viewBackAndFrontDefault);

    // Création de l'avant plan (pour les feux d'artifices, popover...)
    root->viewFrontOpt = ({
        View *v = coq_callocTyped(View);
        view_and_super_init(v, root, flags_viewBackAndFrontDefault);
        PopingNode_setFrontView(v);
        v;
    });

    // Création d'une view de test.
    root_changeViewActiveTo(root, View_createTest(root));
//    root_changeViewActiveTo(root, View_createTestFont(root));

    // Init des fireworks...
    Sparkle_init(Texture_getPngByName("coqlib_sparkle_stars"), sound_fireworks);

    return root;
}
