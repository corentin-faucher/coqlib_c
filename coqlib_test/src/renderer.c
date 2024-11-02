#include "renderer.h"

#include "graphs/graph__opengl.h"
#include "nodes/node_squirrel.h"
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

/*-- Dims de la view ---------*/
int           Renderer_width = 800;
int           Renderer_height = 500;

/*-- deltaT (temps entre deux frame) --*/
static Chrono   chronoDeltaT_;
static FluidPos flDeltaT_;

void   Renderer_drawView(Root* root) {
    // 1. Check le chrono/sleep.
    float deltaTMS = fminf(60.f, fmaxf(3.f,
        (float)chrono_elapsedMS(&chronoDeltaT_)));
    chrono_start(&chronoDeltaT_);
    fl_set(&flDeltaT_, deltaTMS);
    deltaTMS = fl_evalPos(flDeltaT_);
    int64_t deltaTMSint = rand_float_toInt(deltaTMS);
    ChronoRender_update(deltaTMSint);

    // 3. Set viewport, et clear color (cc) red, green, blue, alpha.
    glViewport(0, 0, Renderer_width, Renderer_height);
    Vector4 cc = fl_array_toVec4(root->back_RGBA);
    glClearColor(cc.r, cc.g, cc.b, cc.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // 4. Dessiner tous les objets...
    if(!root) return;
    rendering_opengl_initForDrawing();
    Squirrel sq;
    sq_init(&sq, &root->n, sq_scale_ones);
    do {
        sq.pos->renderer_updateInstanceUniforms(sq.pos);
        if(!(sq.pos->_iu.render_flags & renderflag_toDraw)) continue;
        DrawableMulti *dm = node_asDrawableMultiOpt(sq.pos);
        if(dm) {
            rendering_opengl_drawMulti(dm->d._mesh, dm->d._tex, &dm->iusBuffer);
            continue;
        }
        Drawable const *d = node_asDrawableOpt(sq.pos);
        if(d) {
            rendering_opengl_draw(d->_mesh, d->_tex, &d->n._iu);
            continue;
        }
    } while(sq_renderer_goToNextToDisplay(&sq));
}


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
