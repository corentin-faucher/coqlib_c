#include "renderer.h"

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

static GLuint  Renderer_firstPassProgramId = 0;
static GLuint  Renderer_secondPassProgramId = 0;

static FramebufferWithColors Renderer_framebuffer = {};

// Ici le multi passe ne fait que donner un style low-resolution.
#define FIRSTPASS_W 400
#define FIRSTPASS_H 250
// Pour laisser tomber le multi-passe.
#define JUST_ONE_PASS 0

// Les ids des textures où on dessine à la première passe (les color attachements) 
#define COLORATT_COUNT 1
//static GLuint Renderer_colorAttTexs[COLORATT_COUNT] = {}; // Textures/ColorAttachements
//static const GLuint GL_ColorAttIds[] = {
//    GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
//    GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, 
//    // ...
//};
static const char* colorAttNames[] = {
    "color0", //..., "color1", ...
};

// Fonction à utiliser pour setter les vertex de la première passe,
// i.e. vec3 pos; vec2 uv; (skip vecteur normal)
void mesh_setFirstPassVertexAttributes_(void) {
    // On n'utilise que position et uv de `Vertex`.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

void Renderer_init(void) {
    // On utilise le blending ordinaire pour cette app...
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 1. Première Passe
    Renderer_firstPassProgramId = CoqGLProgram_createFromShaders(
                           "firstpass_vert", "firstpass_frag");
    IUsBuffer_opengl_init(Renderer_firstPassProgramId);
    // Setting de vertex attributes pour les meshes de la première passe.
    Mesh_opengl_initSetVertexAttributes(mesh_setFirstPassVertexAttributes_);
    
    // Frame buffer avec textures
    Renderer_framebuffer = CoqGLFramebuffer_create(COLORATT_COUNT, FIRSTPASS_W, FIRSTPASS_H); 
    
    // 2. Deuxième Passe
    Renderer_secondPassProgramId = CoqGLProgram_createFromShaders(
        "secondpass_vert", "secondpass_frag");
    // Setter les ids des textures de secondpass_frag.glsl.
    // Ici c'est superflu... Utile quand on a plusieurs 
    // color attachments.
    // Si on a OpenGL 4.2, ça devient superflu... On peut juste utiliser 
    // layout(binding = ...) dans les glsl.
    CoqGLframebuffer_initColorAttProgramLocations(Renderer_secondPassProgramId,
        colorAttNames, COLORATT_COUNT);
//    GLuint color0Loc = glGetUniformLocation(Renderer_secondPassProgramId, "color0");
//    glUniform1i(color0Loc, 0);
//    GLuint color1Loc = glGetUniformLocation(Renderer_secondPassProgramId, "colorAtt1");
//    glUniform1i(color1Loc, 1);
    // ...
}
void Renderer_deinit(void) {
    IUsBuffer_opengl_deinit();
    CoqGLFrameBuffer_delete(Renderer_framebuffer);
    glDeleteProgram(Renderer_firstPassProgramId);
    glDeleteProgram(Renderer_secondPassProgramId);
}

void Renderer_firstPass_(void) {
    if(JUST_ONE_PASS) { // Pas de frame buffer -> bind à l'écran (0).
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ViewSizeInfo viewSize = CoqSystem_getViewSize();
        glViewport(0, 0, viewSize.frameSizePx.w, viewSize.frameSizePx.h);
    } else { // Test de multi-passes
        glBindFramebuffer(GL_FRAMEBUFFER, Renderer_framebuffer.framebufferId);
        glViewport(0, 0, FIRSTPASS_W, FIRSTPASS_H);
    }
    guard_let(Root*, root, CoqEvent_rootOpt, , )
    Vector4 cc = fl_array_toVec4(root->back_RGBA);
    glClearColor(cc.r, cc.g, cc.b, cc.a);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(Renderer_firstPassProgramId);
    
    // Dessiner tous les objets...
    rendering_opengl_initForDrawing();
    Node* sq = &root->n;
    do {
        sq->renderer_updateInstanceUniforms(sq);
        if(!(sq->renderIU.flags & renderflag_toDraw))
            continue;
        if_let(DrawableMulti*, dm, node_asDrawableMultiOpt(sq))
        rendering_opengl_setCurrentMesh(dm->d._mesh);
        rendering_opengl_setCurrentTexture(dm->d._tex);
        rendering_opengl_setIUs(&dm->iusBuffer);
        rendering_opengl_drawWithCurrents();
        continue;
        if_let_end
        if_let(Drawable*, d, node_asDrawableOpt(sq))
        rendering_opengl_setCurrentMesh(d->_mesh);
        rendering_opengl_setCurrentTexture(d->_tex);
        rendering_opengl_setIU(&d->n.renderIU);
        rendering_opengl_drawWithCurrents();
        continue;
        if_let_end
    } while(nodeptr_renderer_goToNextToDisplay(&sq));
    rendering_opengl_drawingEnded();
}
void Renderer_secondPass_(void) {
    // Rendering à l'écran.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ViewSizeInfo viewSize = CoqSystem_getViewSize();
    glViewport(0, 0, viewSize.frameSizePx.w, viewSize.frameSizePx.h);
    glClearColor(0, 0, 0.5, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(Renderer_secondPassProgramId);
    
    // On utilise le quad dans les `utils` glfw.
    CoqGL_glBindRenderQuadVertexArray();
    
    // Ici, c'est superflu de setter la "active texture",
    // mais s'il y a plusieur texture dans secondpass_frag.glsl,
    // il faut les setter une par une...
    CoqGLframebuffer_bindColorAttachsTex(Renderer_framebuffer);
//    glActiveTexture(GL_TEXTURE0 + 0);
//    glBindTexture(GL_TEXTURE_2D, Renderer_framebuffer.colorAttachmentIds[0]);
//    glActiveTexture(GL_TEXTURE0 + 1);
//    glBindTexture(GL_TEXTURE_2D, Renderer_framebuffer.colorAttachmentIds[1]);
//   ...
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Renderer_drawView(void) {
    RendererTimeCapture_update();
    
    Renderer_firstPass_();
    
    if(!JUST_ONE_PASS) {
        Renderer_secondPass_();
    }
}


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
