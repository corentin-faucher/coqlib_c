//
//  cellular.c
//  xc_coqlib_test
//
//  Created by Mathieu et Corentin on 3/5/24.
//

#include "cellular.h"

// Cell Grid states... up, left, right, etc.
enum {
// Coq: Style horizontal/vertical avec petites "rotations locales"
// ← ↑ ← ↑       ↓ ← ↓ ←
// ↓ → ↓ →  <->  → ↑ → ↑  propagation / flickering
// ← ↑ ← ↑       ↓ ← ↓ ←
// ↓ → ↓ →       → ↑ → ↑
    cg_0 = 0x0000,
    cg_r = 0x0001,
    cg_u = 0x0002,
    cg_l = 0x0004,
    cg_d = 0x0008,
    cg_A = 0x0010,
    cg_B = 0x0020,

// Mathieu: Style diagonales avec petites "contraction"/"extensions" locales
// ↖︎ ↗︎ ↘︎ ↙︎       ↘︎ ↙︎ ↖︎ ↗︎
// ↙︎ ↘︎ ↗︎ ↖︎  <->  ↗︎ ↖︎ ↙︎ ↘︎  propagation / flickering
// ↘︎ ↙︎ ↖︎ ↗︎       ↖︎ ↗︎ ↘︎ ↙︎
// ↗︎ ↖︎ ↙︎ ↘︎       ↙︎ ↘︎ ↗︎ ↖︎
    cgm_dr =     0x0000,
    cgm_dl =     0x0001,
    cgm_ul =     0x0002,
    cgm_ur =     0x0003,
    cgm_dirs =   0x000F,
    cgm_ON =     0x0010,
    cgm_OFF =    0x0000,
    cgm_WALL =   0x0020,
    cgm_states = 0x00F0,
};

#pragma mark - Méthodes d'une cellule

uint8_t   cel_evalCoq(uint8_t c, uint8_t left, uint8_t right, uint8_t down, uint8_t up) {
    // Propagation vers cette cellule...
    uint8_t v = (left & cg_r) | (right & cg_l) | (down & cg_u) | (up & cg_d);
    if(v == 0) {
//        if((rand() % 10) == 0) return 1 << (rand() % 4);
        return v;
    }
    // Comptage de bits...
    uint8_t n = (v & 0x05) + ((v & 0x0a) >> 1);
    n = (n & 0x03) + ((n & 0x0c) >> 2);
    // Ok, simple propagation...
    if(n == 1) return v;
    // Cas plusieurs propagations dans la même cellule -> choix random.
    uint8_t new = 1 << (rand() % 4);
    if(new & v) return new;
    // Sinon on essaie encore... (cas rare de toute façon ?)
    new <<= new;
    if(!new) new = cg_d;
    if(new & v) return new;
    return cg_r;
}
uint8_t   cel_evalMathieu(uint8_t c, uint8_t ul, uint8_t ur, uint8_t dl, uint8_t dr) {
    uint8_t new = 0;
    if((c & cgm_states) == cgm_OFF) {
//        printdebug(" %d %d %d %d", ul, ur, dl, dr);
        // OFF -> Propagation d'états suivant le "courant"
        if((ur & cgm_states) == cgm_ON && (ur & cgm_dirs) == cgm_dl) {
            new = cgm_ON;
        }
        else if((dl & cgm_states) == cgm_ON && (dl & cgm_dirs) == cgm_ur) {
            new = cgm_ON;
        }
        else if((dr & cgm_states) == cgm_ON && (dr & cgm_dirs) == cgm_ul)
            new = cgm_ON;
        else if((ul & cgm_states) == cgm_ON && (ul & cgm_dirs) == cgm_dr)
            new = cgm_ON;
    }
    else if((c & cgm_states) == cgm_ON) {
        // ON -> go off ou collision -> deviennent "mur"
        if((c & cgm_dirs) == cgm_ur)
            new = (ur & cgm_states) == cgm_OFF ? cgm_OFF : cgm_WALL;
        else if((c & cgm_dirs) == cgm_dl)
            new = (dl & cgm_states) == cgm_OFF ? cgm_OFF : cgm_WALL;
        else if((c & cgm_dirs) == cgm_dr)
            new = (dr & cgm_states) == cgm_OFF ? cgm_OFF : cgm_WALL;
        else if((c & cgm_dirs) == cgm_ul)
            new = (ul & cgm_states) == cgm_OFF ? cgm_OFF : cgm_WALL;
    }
    else if((c & cgm_states) == cgm_WALL) {
        // Cas "mur" -> Redevient ON si particule ON adjacente (~ transfer de quantité de mouvement)
        if((c & cgm_dirs) == cgm_ur && (ur & cgm_states) == cgm_ON)  new = cgm_ON;
        if((c & cgm_dirs) == cgm_dl && (dl & cgm_states) == cgm_ON)  new = cgm_ON;
        if((c & cgm_dirs) == cgm_dr && (dr & cgm_states) == cgm_ON)  new = cgm_ON;
        if((c & cgm_dirs) == cgm_ul && (ul & cgm_states) == cgm_ON)  new = cgm_ON;
    }
    // Update de direction / propagation (flickering down-right <-> up-left et down-left <-> up-right...)
    new |= ((c & cgm_dirs) + 2) % 4;
    return new;
}

// Dessiner...
PixelBGRA cel_toPixelCoq(uint8_t c) {
    if(c & cg_A) return (PixelBGRA){0xFFFFFF00};
    if(c & cg_B) return (PixelBGRA){0xFF00FFFF};
    if(c & cg_r) return (PixelBGRA){0xA0FF2020};
    if(c & cg_l) return (PixelBGRA){0xA020FF20};
    if(c & cg_u) return (PixelBGRA){0xA0FF8020};
    if(c & cg_d) return (PixelBGRA){0xA02020FF};
    return (PixelBGRA){0x80808080};
}
PixelBGRA cel_toPixelMathieu(uint8_t c) {
    if(c & cgm_ON)   return (PixelBGRA){0xFFFFFFFF};
    if(c & cgm_WALL) return (PixelBGRA){0xFF000000};
    if(c == cgm_ul)   return (PixelBGRA){0x10FF2020};
    if(c == cgm_dr)   return (PixelBGRA){0x1020FF20};
    if(c == cgm_dl)   return (PixelBGRA){0x10FF8020};
    if(c == cgm_ur)   return (PixelBGRA){0x102020FF};

    return (PixelBGRA){0x80808080};
}

#pragma mark - Une grille de cellules

typedef struct CelGrid {
    union {
        Node     node;
        Drawable d;
    };
    uint32_t const m; // Dimension de la grille
    uint32_t const n;
    // Alternance entre grille 0 et 1 (lecture/écriture)
    bool      now01;
    uint8_t   *cels0, *cels1;

    bool      mathieu;
    Timer     timer;
    PixelBGRA pixels[1]; // Array des pixels pour l'affichage
} CelGrid;

// Style horizontal/vertical avec petites "rotations locales"
// ← ↑ ← ↑       ↓ ← ↓ ←
// ↓ → ↓ →  <->  → ↑ → ↑  propagation / flickering
// ← ↑ ← ↑       ↓ ← ↓ ←
// ↓ → ↓ →       → ↑ → ↑
void   celgrid_setGridCoq(CelGrid* cg) {
    uint32_t const mn = cg->m*cg->n;
    uint32_t const m = cg->m; uint32_t const n = cg->n;
    if(mn == 0) { printerror("Empty grid."); return; }
    // Boucle sur les cellules pour init.
    uint8_t            (*line)[m] = (uint8_t(*)[m])&cg->cels0[0];
    uint8_t (* const line_end)[m] = (uint8_t(*)[m])&cg->cels0[mn];
    bool odd_line = false;
    uint32_t i = 0, j = 0;
    while(line < line_end) {
        uint8_t *cel = (uint8_t*)line;
        line ++;
        uint8_t const *cel_end = (uint8_t*)line;
        bool odd_cel = false;
        i = 0;
        while(cel < cel_end) {
            if((i < 7*m / 16 || i > 9*m / 16) || (j < 7*n / 16 || j > 9*n / 16))
                *cel = odd_line ? ( odd_cel ? cg_r : cg_u ) : ( odd_cel ? cg_d : cg_l );
            else // Trou initial...
                *cel = 0; // 1 << (rand() % 4);

            cel ++; i ++;
            odd_cel = !odd_cel;
        }
        j ++;
        odd_line = !odd_line;
    }
}
// Style diagonales avec petites "contraction"/"extensions" locales
// ↖︎ ↗︎ ↘︎ ↙︎       ↘︎ ↙︎ ↖︎ ↗︎
// ↙︎ ↘︎ ↗︎ ↖︎  <->  ↗︎ ↖︎ ↙︎ ↘︎  propagation / flickering
// ↘︎ ↙︎ ↖︎ ↗︎       ↖︎ ↗︎ ↘︎ ↙︎
// ↗︎ ↖︎ ↙︎ ↘︎       ↙︎ ↘︎ ↗︎ ↖︎
void   celgrid_setGridMathieu(CelGrid* cg) {
    uint32_t const mn = cg->m*cg->n;
    uint32_t const m = cg->m;
    if(mn == 0) { printerror("Empty grid."); return; }
    // Boucle sur les cellules pour init.
    uint8_t (* const line_end)[m] = (uint8_t(*)[m])&cg->cels0[mn];
    uint32_t j = 0;
    for(uint8_t (*line)[m] = (uint8_t(*)[m])&cg->cels0[0]; line < line_end; line++, j++) {
        uint8_t const *cel_end = (uint8_t*)(line + 1);
        uint32_t i = 0;
        bool const odd_line = j % 2;
        uint32_t const odd_pair_dec = ((j / 2) % 2) ? 2 : 0;
        for(uint8_t *cel = (uint8_t*)line; cel < cel_end; cel++, i++) {
            if(odd_line)
                *cel =      (i + odd_pair_dec) % 4;  // 012301230123...
            else
                *cel = 3 - ((i + odd_pair_dec) % 4); // 321032103210...
            // Activation de quelques cellules...
            if(i == 5) {
                *cel |= cgm_WALL;
            } else if(rand_bool(0.01)) {
//                *cel |= cgm_ON;
            }
        }
    }
}
void   celgrid_updateCoq(CelGrid* cg) {
    uint32_t const m = cg->m, n = cg->n;
    uint8_t const (* const line_beg)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels1[0]   : &cg->cels0[0]);
    uint8_t const (* const line_end)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels1[m*n] : &cg->cels0[m*n]);
    uint8_t const            (*line)[m] = line_beg;
    uint8_t const        (*lineNext)[m] = line_beg + 1;
    uint8_t const        (*linePrev)[m] = line_beg + (n - 1); // On considère que ça loop.
    uint8_t               (*lineOut)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels0[0]   : &cg->cels1[0]);
    while(line < line_end) {
        uint8_t const *cel = (uint8_t*)line;
        uint8_t *celOut = (uint8_t*)lineOut;
        uint8_t* const cel_end = (uint8_t*)(line + 1);
        uint8_t const *celNext = cel + 1;
        uint8_t const *celPrev = cel + (m - 1); // (loop)
        uint8_t const *celUp = (uint8_t*)lineNext;
        uint8_t const *celDown = (uint8_t*)linePrev;
        while(cel < cel_end) {
            *celOut = cel_evalCoq(*cel, *celPrev, *celNext, *celDown, *celUp);
            celPrev = cel;
            cel ++; celOut ++; celDown++; celUp++;
            celNext = cel + 1;
            if(celNext >= cel_end) celNext = (uint8_t*)line;
        }
        linePrev = line;
        line ++; lineOut ++;
        lineNext = line + 1;
        if(lineNext >= line_end) lineNext = line_beg; // (loop)
    }
    // Swap !
    cg->now01 = !cg->now01;
}
void   celgrid_updateMathieu(CelGrid* cg) {
    uint32_t const m = cg->m, n = cg->n;
    uint8_t const (* const line_beg)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels1[0]   : &cg->cels0[0]);
    uint8_t const (* const line_end)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels1[m*n] : &cg->cels0[m*n]);
    uint8_t const            (*line)[m] = line_beg;
    uint8_t const        (*lineNext)[m] = line_beg + 1;
    uint8_t const        (*linePrev)[m] = line_beg + (n - 1); // On considère que ça loop.
    uint8_t               (*lineOut)[m] = (uint8_t(*)[m])(cg->now01 ? &cg->cels0[0]   : &cg->cels1[0]);
    // int j = 0;
    while(line < line_end) {
        uint8_t const *cel = (uint8_t*)line;
        uint8_t *celOut = (uint8_t*)lineOut;
        uint8_t* const cel_end = (uint8_t*)(line + 1);
        uint8_t const *celUR = (uint8_t*)lineNext + 1;
        uint8_t const *celUL = (uint8_t*)lineNext + (m - 1); // (loop)
        uint8_t const *celDR = (uint8_t*)linePrev + 1;
        uint8_t const *celDL = (uint8_t*)linePrev + (m - 1);
        // int i = 0;
        while(cel < cel_end) {
            *celOut = cel_evalMathieu(*cel, *celUL, *celUR, *celDL, *celDR);
            cel++; celOut++;
            celUL = celUR - 1;
            celUR++;
            celDL = celDR - 1;
            celDR++;
            // i++;
            if(celUR >= (uint8_t*)lineNext + m) celUR = (uint8_t*)lineNext;
            if(celDR >= (uint8_t*)linePrev + m) celDR = (uint8_t*)linePrev;
        }
        linePrev = line;
        line ++; lineOut ++;
        lineNext = line + 1;
        // j ++;
        if(lineNext >= line_end) lineNext = line_beg; // (loop)
    }
    // Swap !
    cg->now01 = !cg->now01;
}

// Méthodes diverses de celgrid...
void celgrid_drawPixels_(CelGrid* const cg) {
    PixelBGRA (*cel_toPixel)(uint8_t) = cg->mathieu ? cel_toPixelMathieu : cel_toPixelCoq;
    uint32_t const mn = cg->m * cg->n;
    uint8_t const *       cel     = cg->now01 ?  cg->cels1     :  cg->cels0;
    uint8_t const * const cel_end = cg->now01 ? &cg->cels1[mn] : &cg->cels0[mn];
    PixelBGRA* pixel = cg->pixels;
    while(cel < cel_end) {
        *pixel = cel_toPixel(*cel);
        cel++; pixel++;
    }
}
void celgrid_callback_(void* cgIn) {
    CelGrid* cg = (CelGrid*)cgIn;
    if(cg->mathieu) celgrid_updateMathieu(cg);
    else celgrid_updateCoq(cg);
    celgrid_drawPixels_(cg);
    texture_engine_writeAllPixels(cg->d._tex, cg->pixels);
}
void celgrid_open_(Node* n) {
    CelGrid* cg = (CelGrid*)n;
    timer_scheduled(&cg->timer, 1, true, cg, celgrid_callback_);
}
void celgrid_close_(Node* n) {
    CelGrid* cg = (CelGrid*)n;
    timer_cancel(&cg->timer);
}
void celgrid_deinit_(Node* n) {
    CelGrid* cg = (CelGrid*)n;
    coq_free(cg->cels0);
    coq_free(cg->cels1);
}
CelGrid* CelGrid_create(Node* parent, float x, float y, float height, uint32_t m, uint32_t n, bool mathieu) {
    uint32_t count = m*n;
    if(count == 0) { printerror("No cells."); return NULL; }
    CelGrid* cg = coq_callocArray(CelGrid, PixelBGRA, count);
    // Super inits...
    node_init(&cg->node, parent, x, y, height, height, 0, 0);
    drawable_init(&cg->d, Texture_createWithPixels(cg->pixels, m, n, false, true), &mesh_sprite, 0, height);
    // Init as CelGrid
    cg->node.openOpt =   celgrid_open_;
    cg->node.closeOpt =  celgrid_close_;
    cg->node.deinitOpt = celgrid_deinit_;
    cg->mathieu = mathieu;
    uint_initConst(&cg->m, m);
    uint_initConst(&cg->n, n);
    cg->cels0 = coq_calloc(m*n, sizeof(uint8_t));
    cg->cels1 = coq_calloc(m*n, sizeof(uint8_t));
    if(mathieu) celgrid_setGridMathieu(cg); else celgrid_setGridCoq(cg);

    return cg;
}
