//
//  font_manager.h
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-26.
//

#ifndef _coq_graph_font_manager_h
#define _coq_graph_font_manager_h

#include "coq_utils.h"
#include "_math/_math_.h"

typedef struct {
    char    name[40];
    char    short_name[20];
    Vector2 spreading;
} CoqFontInfo;

void               Font_init(void);
SharedStringsArray Font_allFontNamesForLanguage(Language language);
const char*        Font_defaultFontNameForLanguage(Language language);
const CoqFontInfo* Font_getFontInfoOf(const char* fontName);

void _test_font_manager(void);


#endif /* font_manager_h */
