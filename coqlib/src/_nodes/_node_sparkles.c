//
//  sparkles.c
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-04.
//

#include "_nodes/_node_sparkles.h"
#include "_nodes/_node_tree.h"
#include "_nodes/_node_squirrel.h"
#include "coq_sound.h"

#define _SPARKLES_N 8

typedef struct {
    union {
        Node   n;        // Peut être casté comme un noeud
        Fluid  f;      // ou comme un smooth.
    };
    Timer*    timer;
} _Sparkle;

static View*    _Sparkle_frontView = NULL;
static Texture* _Sparkle_tex = NULL;
static uint32_t _Sparkle_soundId = 0;

void _sparkle_deinit(Node* n) {
    _Sparkle* spk = (_Sparkle*)n;
    timer_cancel(&spk->timer);
}
void _sparkle_close(Node* n) {
    _Sparkle* spk = (_Sparkle*)n;
    timer_scheduledNode(&spk->timer, 500, false, n, node_tree_throwToGarbage);
}
_Sparkle* _Sparkle_create(Node* ref, float x, float y, float delta) {
    _Sparkle* spk = _Node_createEmptyOfType(node_type_n_fluid, sizeof(_Sparkle), 0, ref, 0);
    spk->n.closeOpt =  _sparkle_close;
    spk->n.deinitOpt = _sparkle_deinit;
    spk->n.x = rand_floatAt(x, 0.25*delta);  spk->n.y = rand_floatAt(y, 0.25*delta);
    _fluid_init(&spk->f, 5.f);
    fl_set(&spk->f.x, rand_floatAt(x, 0.5*delta));
    fl_set(&spk->f.y, rand_floatAt(y, 0.5*delta));
    
    return spk;
}

void Sparkle_init(View* frontView, const char* pngNameOpt, uint32_t soundId) {
    _Sparkle_frontView = frontView;
    _Sparkle_tex = Texture_sharedImageByName(pngNameOpt ? pngNameOpt : "coqlib_sparkle_stars");
    _Sparkle_soundId = soundId;
}
void Sparkle_setPng(uint32_t pngId) {
    _Sparkle_tex = Texture_sharedImage(pngId);
}
void Sparkle_spawnAt(float xabs, float yabs, float delta, Texture* texOpt) {
    if(_Sparkle_frontView == NULL || _Sparkle_tex == NULL) {
        printerror("Sparkle not init.");
        return;
    }
    _Sparkle* spk = _Sparkle_create(&_Sparkle_frontView->n, xabs, yabs, delta);
    
    Texture* tex = texOpt ? texOpt : _Sparkle_tex;
    uint32_t MN = texture_mn(tex);
    for(int i = 0; i < _SPARKLES_N; i ++) {
        Fluid* part = Fluid_create(&spk->n,
             rand_floatAt(0, 0.1*delta), rand_floatAt(0, 0.1*delta), 1, 1, 5, 0, 0);
        Drawable* d = Drawable_createAndSetDims(&part->n, 0, 0, 0, 0.4f*delta,
                                                tex, mesh_sprite, flag_poping, 0);
        drawable_setTile(d, rand() % MN, 0);
        fl_set(&part->x, rand_floatAt(0, delta));
        fl_set(&part->y, rand_floatAt(0, delta));
    }
    Sound_play(_Sparkle_soundId, 1.f, 0, 1.f);
    node_tree_openAndShow(&spk->n);
    timer_scheduledNode(&spk->timer, 600, false, &spk->n, node_tree_close);
}
// Convenience constructor.
void Sparkle_spawnOver(Node* nd, float deltaRatio) {
    if(_Sparkle_frontView == NULL || _Sparkle_tex == NULL) {
        printerror("Sparkle not init.");
        return;
    }
    Box box = node_hitBoxInParentReferential(nd, NULL);
    float delta = deltaRatio * fmaxf(box.Dx, box.Dy);
    Sparkle_spawnAt(box.c_x, box.c_y, delta, NULL);
}
