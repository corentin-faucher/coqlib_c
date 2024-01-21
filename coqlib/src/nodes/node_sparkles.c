//
//  sparkles.c
//  MasaKiokuGameOSX
//
//  Created by Corentin Faucher on 2023-11-04.
//

#include "nodes/node_sparkles.h"
#include "nodes/node_tree.h"
#include "nodes/node_squirrel.h"
#include "coq_sound.h"

#define _SPARKLES_N 8

typedef struct {
    union {
        Node   n;        // Peut être casté comme un noeud
        Fluid  f;      // ou comme un smooth.
    };
    Timer*    timer;
} _Sparkle;

static View*    Sparkle_frontView_ = NULL;
static Texture* Sparkle_tex_ = NULL;
static uint32_t Sparkle_soundId_ = 0;

void _sparkle_deinit(Node* n) {
    _Sparkle* spk = (_Sparkle*)n;
    timer_cancel(&spk->timer);
}
void _sparkle_close(Node* n) {
    _Sparkle* spk = (_Sparkle*)n;
    timer_scheduled(&spk->timer, 500, false, n, node_tree_throwToGarbage);
}
_Sparkle* _Sparkle_create(Node* ref, float x, float y, float lambda) {
    _Sparkle* spk = Node_createEmptyOfType_(node_type_n_fluid, sizeof(_Sparkle), 0, ref, 0);
    spk->n.closeOpt =  _sparkle_close;
    spk->n.deinitOpt = _sparkle_deinit;
    spk->n.x = x;  spk->n.y = y;
    fluid_init_(&spk->f, lambda);
    
    return spk;
}

void Sparkle_init(View* frontView, const char* pngNameOpt, uint32_t soundId) {
    Sparkle_frontView_ = frontView;
    Sparkle_tex_ = Texture_sharedImageByName(pngNameOpt ? pngNameOpt : "coqlib_sparkle_stars");
    Sparkle_soundId_ = soundId;
}
void Sparkle_setPng(uint32_t pngId) {
    Sparkle_tex_ = Texture_sharedImage(pngId);
}
void Sparkle_spawnAt(float xabs, float yabs, float delta, Texture* texOpt) {
    if(Sparkle_frontView_ == NULL || Sparkle_tex_ == NULL) { printerror("Sparkle not init."); return; }
    _Sparkle* spk = _Sparkle_create(&Sparkle_frontView_->n,
                rand_floatAt(xabs, 0.25*delta), rand_floatAt(yabs, 0.25*delta), 5.f);
    // Mouvement moyen.
    fl_set(&spk->f.x, rand_floatAt(xabs, 0.5*delta));
    fl_set(&spk->f.y, rand_floatAt(yabs, 0.5*delta));
    
    Texture* tex = texOpt ? texOpt : Sparkle_tex_;
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
    Sound_play(Sparkle_soundId_, 1.f, 0, 1.f);
    node_tree_openAndShow(&spk->n);
    
    // Despawn après 0.6 s.
    timer_scheduled(&spk->timer, 600, false, &spk->n, node_tree_close);
}
// Convenience constructor.
void Sparkle_spawnOver(Node* nd, float deltaRatio) {
    Box box = node_hitBoxInParentReferential(nd, NULL);
    float delta = deltaRatio * fmaxf(box.Dx, box.Dy);
    Sparkle_spawnAt(box.c_x, box.c_y, delta, NULL);
}

void TmpDrawable_spawnAt(float xabs, float yabs, float twoDy, Texture* tex, float lambda, int64_t deltaTMS) {
    _Sparkle* spk = _Sparkle_create(&Sparkle_frontView_->n, xabs, yabs, lambda);
    fl_fadeIn(&spk->f.sx, 1);
    fl_fadeIn(&spk->f.sy, 1);
    
    Drawable_createAndSetDims(&spk->n, 0, 0, 0, twoDy, tex, mesh_sprite, 0, flag_poping);
    
    node_tree_openAndShow(&spk->n);
    timer_scheduled(&spk->timer, deltaTMS, false, &spk->n, node_tree_close);
}

void TmpDrawable_spawnOver(Node* n, Texture* tex, float lambda, int64_t deltaTMS) {
    if(Sparkle_frontView_ == NULL) { printerror("Sparkle not init."); return; }
    Box box = node_hitBoxInParentReferential(n, NULL);
    TmpDrawable_spawnAt(box.c_x, box.c_y, 2.f*box.Dy, tex, lambda, deltaTMS);
}
