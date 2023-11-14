//
//  sparkles.c
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-04.
//

#include "sparkles.h"
#include "sound.h"
#include "timer.h"
#include "node_tree.h"
#include "node_squirrel.h"

#define _SPARKLES_N 8

struct _Sparkle {
    union {
        Node       n;        // Peut être casté comme un noeud
        Fluid s;      // ou comme un smooth.
    };
    Timer*    timer;
};

static View*    _Sparkle_frontView = NULL;
static Texture* _Sparkle_tex = NULL;
static uint32_t _Sparkle_soundId = 0;

void _sparkle_test_deinit(Node* nd) {
    printdebug("🐷 deinit of sparkle...");
}
void _sparkle_node_close(Node* nd) {
    timer_scheduled(NULL, 500, false, nd, node_tree_throwToGarbage);
}

void Sparkle_init(View* frontView, const char* pngName, uint32_t soundId) {
    _Sparkle_frontView = frontView;
    _Sparkle_tex = Texture_sharedImageByName(pngName);
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
    Fluid* spk = Fluid_create(&_Sparkle_frontView->n,
         rand_floatAt(xabs, 0.25*delta), rand_floatAt(yabs, 0.25*delta),
         1.f, 1.f, 5.f, 0, 0);
    spk->n.close = _sparkle_node_close;
//    spk->n.deinit = _sparkle_test_deinit;
    fluid_setX(spk, rand_floatAt(xabs, 0.5*delta), false);
    fluid_setY(spk, rand_floatAt(yabs, 0.5*delta), false);
    Texture* tex = texOpt ? texOpt : _Sparkle_tex;
    uint32_t MN = texture_mn(tex);
    for(int i = 0; i < _SPARKLES_N; i ++) {
        Fluid* part = Fluid_create(&spk->n,
             rand_floatAt(0, 0.1*delta), rand_floatAt(0, 0.1*delta), 1, 1, 5, 0, 0);
        Drawable* d = _Drawable_create(&part->n, 0, 0, flag_poping, 0,
             tex, mesh_sprite);
        drawable_updateDimsWithDeltas(d, 0.f, 0.4f*delta);
        drawable_setTile(d, rand() % MN, 0);
        fluid_setX(part, rand_floatAt(0, delta), false);
        fluid_setY(part, rand_floatAt(0, delta), false);
    }
    Sound_play(_Sparkle_soundId, 1.f, 0, 1.f);
    node_tree_openAndShow(&spk->n);
    timer_scheduled(NULL, 600, false, &spk->n, node_tree_close);
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
