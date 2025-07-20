#include "renderer.h"

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

void   Renderer_drawView(void) {
    ChronoRender_update();

    // 3. Set viewport, et clear color (cc) red, green, blue, alpha.
    ViewSizeInfo viewSize = CoqSystem_getViewSize();
    glViewport(0, 0, viewSize.framePx.w, viewSize.framePx.h);
    guard_let(Root*, root, CoqEvent_root, , )
    Vector4 cc = fl_array_toVec4(root->back_RGBA);
    glClearColor(cc.r, cc.g, cc.b, cc.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // 4. Dessiner tous les objets...
    if(!root) return;
    rendering_opengl_initForDrawing();
    Node* sq = &root->n;
    do {
        sq->renderer_updateInstanceUniforms(sq);
        if(!(sq->renderIU.flags & renderflag_toDraw)) {
            continue;
        }
        if_let(DrawableMulti*, dm, node_asDrawableMultiOpt(sq))
        rendering_opengl_setCurrentMesh(dm->d._mesh);
        rendering_opengl_setCurrentTexture(dm->d.texr.tex);
        rendering_opengl_setIUs(&dm->iusBuffer);
        rendering_opengl_drawWithCurrents();
        continue;
        if_let_end
        if_let(Drawable*, d, node_asDrawableOpt(sq))
        rendering_opengl_setCurrentMesh(d->_mesh);
        rendering_opengl_setCurrentTexture(d->texr.tex);
        rendering_opengl_setIU(&d->n.renderIU);
        rendering_opengl_drawWithCurrents();
        continue;
        if_let_end
    } while(nodeptr_renderer_goToNextToDisplay(&sq));
}


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
