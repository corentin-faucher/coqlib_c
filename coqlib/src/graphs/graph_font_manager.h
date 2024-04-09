//
//  font_manager.h
//  N'est qu'une liste de font utilisées dans macOS/iOS 
//  avec leurs ajustements (cadre de `spreading`).
//
//  Created by Corentin Faucher on 2023-10-26.
//
#ifndef COQ_GRAPH_FONT_MANAGER_H
#define COQ_GRAPH_FONT_MANAGER_H

#include "../coq_utils.h"
#include "../maths/math_base.h"

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
