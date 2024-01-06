//
//  pop_disk.c
//  demo_xcode
//
//  Created by Corentin Faucher on 2023-10-28.
//

#include <math.h>
#include "_nodes/_node_pop_disk.h"
#include "_nodes/_node_squirrel.h"
#include "_nodes/_node_tree.h"
#include "_nodes/_node_root.h"

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
    timer_cancel(&pop->timer);
}
void _popdisk_close(Node* nd) {
    PopDisk* pop = (PopDisk*)nd;
    timer_cancel(&pop->timer);
    if(pop->referer)
        *pop->referer = (PopDisk*)NULL;
    timer_scheduledNode(&pop->timer, 500, false, nd, node_tree_throwToGarbage);
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
    pop->n.deinitOpt = _popdisk_deinit;
    pop->n.closeOpt =  _popdisk_close;
    // Surface init
    smtrans_init(&pop->d.trShow);
    smtrans_init(&pop->d.trExtra);
    pop->d.tex =   Texture_sharedImage(pngId);
    pop->d.mesh =  Mesh_createFan();
    drawable_setTile(&pop->d, tile, 0);
    // PopDisk init
    pop->deltaT = deltaT;
    chrono_start(&pop->chrono);
    timer_scheduledNode(&pop->timer, 50, true, &pop->n, _popdisk_updateRatioCallback);
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
        Fluid      f;      // ou smooth.
    };
    Timer* timer;
} PopMessage;
void  _popmessage_close(Node* n) {
    PopMessage* pm = (PopMessage*)n;
    timer_cancel(&pm->timer);
    timer_scheduledNode(&pm->timer, 500, false, n, node_tree_throwToGarbage);
}
void _popmessage_deinit(Node* n) {
    timer_cancel(&((PopMessage*)n)->timer);
}

void   PopMessage_spawn(Node* refOpt,
                        uint32_t framePngId, UnownedString str,
                        float x, float y, float maxWidth, float height,
                        float timeSec, Rectangle rectDeltas) {
    // Node init
    PopMessage *pm = _Node_createEmptyOfType(node_type_n_fluid, sizeof(PopMessage),
                                       0, refOpt, 0);
    Node* n = &pm->n;
    n->x = x + rectDeltas.o_x;
    n->y = y + rectDeltas.o_y;
    n->w = height;  // Temporaire... Setter par la string et frame.
    n->h = height;
    n->sx = (maxWidth + rectDeltas.w) / maxWidth;
    n->sy = (height + rectDeltas.h) / height;
    n->closeOpt =  _popmessage_close;
    n->deinitOpt = _popmessage_deinit;
    // Init as Fluid.
    _fluid_init(&pm->f, 4.f);
    fluid_setScales(&pm->f, vector2_ones, false);
    // Structure (frame et localized string)
    Frame_create(n, 0, 0.35*height, 0, 0, framePngId, frame_option_giveSizesToParent);
    Drawable_createString(n, str, 0.f, 0.f, maxWidth, height, flag_giveSizeToBigbroFrame);
    // PopMessage init
    timer_scheduledNode(&pm->timer, (int64_t)(timeSec*1000.f), false, n, node_tree_close);
    node_tree_openAndShow(n);
    // Position (fonction de la view)
    Squirrel sq;
    sq_init(&sq, n, sq_scale_deltas);
    Root* root = NULL;
    while(sq_goUpPS(&sq)) {
        root = node_asRootOpt(sq.pos);
        if(root) break;
    }
    if(!root) { printerror("No root."); return; }
    printdebug("Pop right %f, left %f,  root dx %f.", sq.v.x + sq.s.x, sq.v.x - sq.s.x, 0.5f*root->n.w);
    float dx = 0.f;
    if(sq.v.x + sq.s.x > 0.5*root->n.w) {
        dx = ( 0.5*root->n.w - (sq.v.x + sq.s.x)) * (node_deltaX(n)/sq.s.x); // (négatif)
    }
    if(sq.v.x - sq.s.x < -0.5*root->n.w) {
        dx = (-0.5*root->n.w - (sq.v.x - sq.s.x)) * (node_deltaX(n)/sq.s.x); // (positif)
    }
    
    fluid_setXY(&pm->f, (Vector2){{x+dx, y}});
}
