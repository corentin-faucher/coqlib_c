//
//  pop_disk.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-28.
//

#include "pop_disk.h"
#include <math.h>
#include "utils.h"
#include "colors.h"
#include "node_tree.h"

/*-- PopDisk disk de progres qui s'autodetruit --------------------------------------------*/
typedef struct _PopDisk {
    union {
        Node     n;        // Peut être casté comme un noeud
        Drawable d;      // ou comme une surface.
    };
    Timer*    timer;
    Chrono    chrono;
    float     deltaT;
    PopDisk** referer;
} PopDisk;

void _popdisk_deinit(Node* nd) {
    PopDisk* pop = (PopDisk*)nd;
    mesh_destroy(pop->d.mesh);
//    timer_cancel(&pd->timer); -> cancel dans close
}
void _popdisk_close(Node* nd) {
    PopDisk* pop = (PopDisk*)nd;
    timer_cancel(&pop->timer);
    if(pop->referer)
        *pop->referer = (PopDisk*)NULL;
    timer_scheduled(NULL, 500, false, nd, node_tree_throwToGarbage);
}
void _popdisk_updateRatioCallback(Node* nd) {
    PopDisk* pd = (PopDisk*)nd;
    float elapsedSec = chrono_elapsedSec(&pd->chrono);
    if(elapsedSec < pd->deltaT + 0.10f) {
        float ratio = fminf(elapsedSec / pd->deltaT, 1.f);
        mesh_fan_update(pd->d.mesh, ratio);
        return;
    }
    // Fini -> unspawn.
    node_tree_close(nd);
}
// Mesh owner.
void PopDisk_spawn(Node* const refOpt, PopDisk** const refererOpt,
                   uint32_t pngId, uint32_t tile, float deltaT,
                   float x, float y, float twoDy) {
    // Node init
    PopDisk* pop = _Node_createEmptyOfType(node_type_flag_drawable, sizeof(PopDisk),
                                       0, refOpt, 0);
    pop->n.x = x;      pop->n.y = y;
    pop->n.w = 1.f;    pop->n.h = 1.f;
    pop->n.sx = twoDy; pop->n.sy = twoDy;
    pop->n.deinit = _popdisk_deinit;
    pop->n.close = _popdisk_close;
    // Surface init
    smtrans_init(&pop->d.trShow);
    smtrans_init(&pop->d.trExtra);
    pop->d.tex = Texture_sharedImage(pngId);
    pop->d.mesh =  Mesh_createFan();
    drawable_setTile(&pop->d, tile, 0);
    // PopDisk init
    pop->deltaT = deltaT;
    chrono_start(&pop->chrono);
    timer_scheduled(&pop->timer, 50, true, &pop->n, _popdisk_updateRatioCallback);
    pop->referer = refererOpt;
    if(refererOpt)
        *refererOpt = pop;
    pop->n.flags |= flag_show;
}
void     popdisk_cancel(PopDisk** popRef) {
    if(*popRef == NULL) return;
    if(popRef != (*popRef)->referer) {
        printerror("timerRef is not the timer referer.");
        return;
    }
    node_tree_close((Node*)*popRef);
}

/*-- PopMessage : Message temporaire. ----------------------------------*/
typedef struct _PopMessage {
    union {
        Node       n;      // Peut être casté comme un noeud
        Fluid s;      // ou smooth.
    };
    Timer* timer;
} PopMessage;
void  _popmessage_close(Node* nd) {
    timer_cancel(&((PopMessage*)nd)->timer);
    timer_scheduled(NULL, 500, false, nd, node_tree_throwToGarbage);
}
void   PopMessage_spawn(Node* refOpt,
                        uint32_t framePngId, UnownedString str,
                        float x, float y, float maxWidth, float height,
                        float timeSec) {
    // Node init
    PopMessage *pm = _Node_createEmptyOfType(node_type_smooth, sizeof(PopMessage),
                                       0, refOpt, 0);
    pm->n.x = x;
    pm->n.y = y;
    pm->n.w = height;  // Temporaire... Setter par la string et frame.
    pm->n.h = height;
    pm->n.close = _popmessage_close;
    // Init as Fluid.
    _fluid_init(&pm->s, 10.f);
    // Structure (frame et localized string)
    Frame_create(&pm->n, 0, 0.35*height, 0, 0, framePngId, frametype_giveSizesToParent);
    Texture* tex = Texture_createString(str);
    Drawable* string = _Drawable_create(&pm->n, 0, 0, flag_giveSizeToBigbroFrame, 0,
                                        tex, mesh_sprite);
    string->n.piu.color = color4_black;
    string->x_margin = 0.5;
    drawable_updateDimsWithDeltas(string, maxWidth, height);
    // PopMessage init
    timer_scheduled(&pm->timer, (int64_t)(timeSec*1000.f), false, &pm->n, node_tree_close);
    node_tree_openAndShow(&pm->n);
}
