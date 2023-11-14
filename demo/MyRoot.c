//
//  MyRoot.c
//  Test2
//
//  Created by Corentin Faucher on 2023-10-18.
//

#include "MyRoot.h"

#include "MyUtils.h"
#include "pop_disk.h"
#include "button.h"
#include "utils.h"
#include "language.h"
#include "sound.h"
#include "sliding_menu.h"

View* View_createTest(Root* rt);

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
    root_changeActiveScreenTo(bt->root, View_createTest(bt->root));
}

void button_action_doubleDeltaT(Button *bt) {
    Chrono_UpdateDeltaTMS *= 2;
}

void button_action_terminate(Button* bt) {
    Sound_play(sound_sheep, 1, 0, 0);
    root_changeActiveScreenTo(bt->root, NULL);
}

View* View_createTest(Root* rt) {
    /// Par defaut les premiers enfants du screen (les "blocs") sont alignes.
    View* v = View_create(rt, flag_viewDontAlignElements);
    node_last_tryToAddTestFrame();
    
    Fluid* bloc = Fluid_create(&v->n, 0.5f, 0.f, 1.f, 1.f, 5.f,
                               flag_fluidFadeInRight, 0);
    Drawable_createImageWithName(&bloc->n, "coqlib_the_cat", -1.f, 0.f, 0.5f, 0, 0);
    
    // Setter la police avant d'ajouter des strings.
    Texture_setCurrentFontSize(144);
    Texture_setCurrentFont("American Typewriter");
    
    UnownedString str = { Loc_strKey(loc_ok), NULL, true };
    ButtonHoverable_create(&v->n, rt, button_action,
                           png_frame_white_back, str,
                           -0.55, -0.4, 0.15, 10, 0);
    str.c_str = Loc_strKey(loc_menu);
    node_last_addFramedString(png_frame_mocha, str,
                              0.8f, 0.20f, 0.f, true);
    Drawable_createImageWithFixedWidth(&v->n, png_frame_red, -0.55, -0.4, 0.8, 0.15, 0, 0);
    str.c_str = Loc_strKey(loc_quit);
    ButtonHoverable_create(&v->n, rt, button_action_terminate,
                           png_frame_white_back, str,
                           -0.55, -0.6, 0.15, 10, 0);
    node_last_addFramedString(png_frame_mocha, str,
                              0.8f, 0.20f, 0.f, false);
    Drawable_createImageWithFixedWidth(&v->n, png_frame_red, -0.55, -0.6, 0.8, 0.15, 0, 0);
    
    SecurePopInfo spi = {
        2.f, png_disks, disk_color_red,
        png_frame_red, "Hold Button !", false
    };
    ButtonSecure_create(&v->n, rt, button_action_doubleDeltaT, spi, -0.5, 0.5, 0.25, 10, 0);
    node_last_addIcon(png_disks, disk_color_orange, png_icons, icon_help);
    str.c_str = Loc_strKey(loc_app_name);
    Drawable_createString(&v->n, str, 0.f,  0.85f, 2.f, 0.15f, 0, 0);
    
    PopDisk_spawn(&v->n, NULL, png_disks, disk_color_blue, 3.f, -0.15f, -0.25f, 0.3f);
    
    
   
    SlidingMenu_create(&v->n, 4, 1.2f,  0.75, 0.f, 1.2, 1.5, 0);
    float itemW = slidingmenu_last_itemRelativeWidth()*0.95;
    printdebug("target rel item width %f.", itemW);
    slidingmenu_last_addItem(
        MyNode_struct_createFramedLocStr(loc_ok, itemW)
    );
    slidingmenu_last_addItem(
        MyNode_struct_createFramedLocStr(loc_error, itemW)
    );
    slidingmenu_last_addItem(
        MyNode_struct_createFramedLocStr(loc_menu, itemW)
    );
    slidingmenu_last_addItem(
        MyNode_struct_createFramedLocStr(loc_app_name, itemW)
    );
    slidingmenu_last_addItem(
        MyNode_struct_createFramedLocStr(loc_quit, itemW)
    );
    slidingmenu_last_addItem(
        MyNode_struct_createFramedLocStr(loc_error_empty, itemW)
    );
    
    return v;
}

Root* Root_createMyRoot(void) {
    Root* root = Root_create();
    root_changeActiveScreenTo(root, View_createTest(root));
    
    return root;
}
