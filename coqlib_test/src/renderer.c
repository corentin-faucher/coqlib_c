#include "renderer.h"

#include "graphs/graph__opengl.h"
#include "utils/util_base.h"
#include "nodes/node_tree.h"
#include "util_glfw.h"
#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif


/*-- deltaT (temps entre deux frame) --*/
static Chrono   chronoDeltaT_;
static FluidPos flDeltaT_;

void   Renderer_drawView(Root* root) {
    // 1. Check le chrono/sleep.
    float deltaTMS = fminf(60.f, fmaxf(3.f,
        (float)chrono_elapsedMS(&chronoDeltaT_)));
    chrono_start(&chronoDeltaT_);
    fl_set(&flDeltaT_, deltaTMS);
    deltaTMS = fl_evalPos(&flDeltaT_);
    int64_t deltaTMSint = rand_float_toInt(deltaTMS);
    ChronoRender_update(deltaTMSint);

    // 3. Set viewport, et clear color (cc) red, green, blue, alpha.
    glViewport(0, 0, CoqGLFW_viewWidthPx, CoqGLFW_viewHeightPx);
    Vector4 cc = fl_array_toVec4(root->back_RGBA);
    glClearColor(cc.r, cc.g, cc.b, cc.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // 4. Dessiner tous les objets...
    if(!root) return;
    rendering_opengl_initForDrawing();
    Node* sq = &root->n;
    do {
        sq->renderer_updateInstanceUniforms(sq);
        if(!(sq->renderIU.flags & renderflag_toDraw)) continue;
        if_let(DrawableMulti*, dm, node_asDrawableMultiOpt(sq))
        rendering_opengl_setCurrentMesh(dm->d._mesh);
        rendering_opengl_setCurrentTexture(dm->d.texr.tex);
        rendering_opengl_setIUs(iusbuffer_rendering_getToDraw(dm->iusBuffer));
        rendering_opengl_drawWithCurrents();
        if_let_end
        if_let(Drawable*, d, node_asDrawableOpt(sq))
        rendering_opengl_setCurrentMesh(d->_mesh);
        rendering_opengl_setCurrentTexture(d->texr.tex);
        rendering_opengl_setIU(&d->n.renderIU);
        rendering_opengl_drawWithCurrents();
        if_let_end
    } while(nodeptr_renderer_goToNextToDisplay(&sq));
}


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
