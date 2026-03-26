//
//  graph__opengl.c
//  Pour utiliser OpenGL (au lieu de Metal)
//
//  Created by Corentin Faucher on 2024-10-03.
//

#include "graph__opengl.h"

#include <pthread.h>
#include <sys/_pthread/_pthread_t.h>

#include "graph_font.h"
#include "../nodes/node_base.h"
#include "../systems/system_base.h"
#include "../systems/system_file.h"
#include "../utils/util_base.h"
#include "../utils/util_event.h"
#include "../utils/util_timer.h"

#ifdef __APPLE__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

// MARK: - Callbacks d'events
void CoqGlfw_errorCallback_(int const error, const char* const description) {
    printf("⚠️ GLFW Error %d: %s.\n", error, description);
}
void glfw_key_callback_(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if(action != GLFW_PRESS) return;
    if(key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
        return;
    }
    // Le "scancode" de GLFW semble etre le keycode de l'OS.
    // Le "key" de GLFW semble etre la correspondance
    // entre le Qwerty-US et les ASCII,
    // e.g. la touch "A" (sur qwerty) est key = 65.
    // Ici, on va garder les "scancodes..."
    uint32_t modifiers = 0;
    if(mods & GLFW_MOD_SHIFT) modifiers |= modifier_shift;
    if(mods & GLFW_MOD_ALT) modifiers |= modifier_option;
    if(mods & GLFW_MOD_SUPER) modifiers |= modifier_command;
    if(mods & GLFW_MOD_CONTROL) modifiers |= modifier_control;
    if(mods & GLFW_MOD_CAPS_LOCK) modifiers |= modifier_capslock;
    CoqEvent event = {
        .type = eventtype_key_down,
        .key = {
            .modifiers = modifiers,
            .keycode = scancode,
        },
    };
    CoqEvent_addToRootEvent(event);
}
void glfw_scroll_callback_(GLFWwindow* UNUSED(window), double xoffset, double yoffset) {
    CoqEvent_addToRootEvent((CoqEvent){
        .type = eventtype_scroll,
        .scroll_info = {
            .scrollType = scrolltype_scroll,
            .scroll_deltas = {{ xoffset, yoffset }}
        },
    });
}

static bool MouseLeftDown_ = false;
static Vector2 mousePos = {};
void glfw_cursorPos_callback_(GLFWwindow* UNUSED(window), double x, double y) {
  // !! Ici y est inversé !!
  mousePos = (Vector2){{ x, y }};
//  printf("🐔GLFW Mouse at %f: %f.\n", x, y);
  CoqEvent_addToRootEvent((CoqEvent){
      .type = MouseLeftDown_ ? eventtype_touch_drag : eventtype_touch_hovering,
      .touch_info = {
          .pos = mousePos,
          .yInverted = true,
      },
  });
}
void glfw_mouseButton_callback_(GLFWwindow* UNUSED(window), int buttonId, int action, int UNUSED(mods)) {
    MouseLeftDown_ = action; // GLFW_RELEASE = 0
    if(action == GLFW_PRESS) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = eventtype_touch_down,
            .touch_info = {
                .pos = mousePos,
                .touchId = buttonId,
                .yInverted = true,
            },
        });
    } else if(action == GLFW_RELEASE) {
        CoqEvent_addToRootEvent((CoqEvent){
            .type = eventtype_touch_up,
            .touch_info = { // un peu superflu pour touchUp...
                .pos = mousePos,
                .touchId = buttonId,
                .yInverted = true,
            },
        });
    }
}

void glfw_framebufferSize_callback_(GLFWwindow* window, int widthPx, int heightPx)  {
  int widthPt, heightPt;
  int x, y;
  glfwGetWindowSize(window, &widthPt, &heightPt);
  glfwGetWindowPos(window, &x, &y);
  CoqEvent_addToRootEvent((CoqEvent){
        eventtype_resize, .resize_info = {
            .margins = { 0, 0, 0, 0 },
            .framePt = {{ x, y, widthPt, heightPt }},
            .frameSizePx = {{ widthPx, heightPx }},
            .fullScreen = false, 
            .justMoving = false, 
            .dontFix = false,
  }});
  // GLFW semble bloquer la boucle principale durant un resize ???
  // Il faut donc checker les event et dessiner ici... ??
//  CoqEvent_processEvents(root);
//  Renderer_drawView(root);
}

// MARK: - Thread d'events et drawing des textures...
/// Les events/timers à vérifier durant une `tick`.
void* thread_processTick(void* UNUSED()) {
    while(CoqEvent_rootOpt) {
        // 1. Setter le temps de la tick.
        EventTimeCapture_update();
        ChronoChecker cc = chronochecker_startNew();
        // 2. Traiter tous les events (keydown, touchDown, ...)
        CoqEvent_processAllQueuedRootEvents();
        // 3. Traiter les callback de timers, i.e. physique de collisions.
        Timer_check();
        // 5. Sleep s'il reste du temps.
        chronochecker_sleepRemaining(cc, EventTimeCapture.deltaTMS, false);
    }
    return NULL;
}
void* thread_drawPngTextures_(void* UNUSED()) {
    Texture_drawMissingPngs();
    return NULL;
}

// MARK: - Quad de rendering (pour multi passes)
// Quad : "cadre" [-1, 1] x [-1, 1] pour la deuxième passe : framebuffer -> écran.
static GLuint CoqGL_renderQuad_vertexArrayId = 0;
static GLuint CoqGL_renderQuad_vertexBufferId = 0;
void CoqGL_initRendererQuad_(void) {
    if(CoqGL_renderQuad_vertexArrayId) return;
    // (x,y), (u,v).
    const float vertices[] = {
        -1.0,-1.0, 0.0, 0.0,
        -1.0, 1.0, 0.0, 1.0,
         1.0,-1.0, 1.0, 0.0,
         1.0,-1.0, 1.0, 0.0,
         1.0, 1.0, 1.0, 1.0,
        -1.0, 1.0, 0.0, 1.0,
    };
    
    // Les vertex à utiliser (buffer des données brut)
    glGenBuffers(1, &CoqGL_renderQuad_vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, CoqGL_renderQuad_vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Vertex Array Object (VAO) : info pour savoir où lire les vertex 
    // avec leurs position, coord., couleurs...
    // Ça prend un VAO par mesh.
    glGenVertexArrays(1, &CoqGL_renderQuad_vertexArrayId);
    glBindVertexArray(CoqGL_renderQuad_vertexArrayId);
    // Ici, `glVertexAttribPointer` va lier les infos pour lire "vec2 pos" (location 0)
    // dans le shader avec le buffer présentement bindé `mesh.vertexBufferId`.
    // I.e. c'est les infos pour savoir où lire les positions.
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),            
                          (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}
void CoqGL_glBindRenderQuadVertexArray(void) {
    glBindVertexArray(CoqGL_renderQuad_vertexArrayId);
}

GLFWwindow* CoqGLFW_and_Coqlib_initAndGetWindow(int const width, int const height,
                char const*const fontsPath, char const*const defaultFontName,
                char const*const emojiFontName)
{
    glfwSetErrorCallback(CoqGlfw_errorCallback_);
    if(!glfwInit()) {
        printerror("Cannot init GLFW.");
        return NULL;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, COQGL_OPENGL_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, COQGL_OPENGL_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    char const* app_name = CoqSystem_appDisplayNameOpt() ? 
        CoqSystem_appDisplayNameOpt() : "Testing OpenGL";
    GLFWwindow *window = glfwCreateWindow(width, height, 
                                          app_name, NULL, NULL);
    if(window == NULL) {
        glfwTerminate();
        printerror("Cannot create GLFW Window.");
        return NULL;
    }
    glfwMakeContextCurrent(window);
    // Init de OpenGL dans linux... (après le context GLFW)
    #ifdef __linux__
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    	glfwTerminate();
        printerror("Cannot create GLFW Window.");
        return NULL;
    }
    #endif
    CoqGL_initRendererQuad_();
    
    CoqFont_freetype_init_(fontsPath, defaultFontName, emojiFontName);
    
    Texture_init_();
    
    // Obtenir les dimensions de la window.
    int x, y;
    int widthPt, heightPt;
    int widthPx, heightPx;
    glfwGetWindowPos(window, &x, &y);
    glfwGetWindowSize(window, &widthPt, &heightPt);
    glfwGetFramebufferSize(window, &widthPx, &heightPx);
    // Init de CoqSystem : OS version, App version, language,... 
    CoqSystem_init((ViewSizeInfo) {
        .margins = {},
        .framePt =  {{ x, y, widthPt, heightPt }},
        .frameSizePx =  {{ widthPx, heightPx }},
    });
    
    return window;
}

static pthread_t threadEvent;
static pthread_t threadDraw;
void CoqGLFW_setEventsAndStart(GLFWwindow*const window) {
    // Les callback d'events pour glfw...
    glfwSetKeyCallback(window, glfw_key_callback_);
    glfwSetScrollCallback(window, glfw_scroll_callback_);
    glfwSetCursorPosCallback(window, glfw_cursorPos_callback_);
    glfwSetMouseButtonCallback(window, glfw_mouseButton_callback_);
    glfwSetFramebufferSizeCallback(window, glfw_framebufferSize_callback_);
    
    // Démarrer...
    if(!CoqEvent_rootOpt) { printerror("Project root not set."); return; }
    ChronoApp_setPaused(false);
    pthread_create(&threadEvent, NULL, thread_processTick, NULL);
}

void CoqGLFW_endAndDestroyWindow(GLFWwindow*const window) {
    // Jeter la root est signe que c'est fini.
    noderef_destroyAndNull((Node**)&CoqEvent_rootOpt);
    pthread_join(threadEvent, NULL);
    pthread_join(threadDraw, NULL);
    Node_render_burnDownGarbage();
    Node_render_burnDownGarbage();
    
    CoqFont_freetype_quit_();
    
    glDeleteBuffers(1,      &CoqGL_renderQuad_vertexBufferId);
    glDeleteVertexArrays(1, &CoqGL_renderQuad_vertexArrayId);
    
    glfwDestroyWindow(window);
    glfwTerminate();
}

void CoqGL_render_checksAfterRendererDraw(void) {
    // Dans la thread de rendering...
    // S'il faut dessiner, partir une thread pour dessiner les pngs.
    if(Texture_needToDrawPngs()) {
        pthread_create(&threadDraw, NULL, thread_drawPngTextures_, NULL);
    }
    // De temps en temps vider les poubelles et libérer les pngs inutilisés.
    if(RendererTimeCapture.tick % 2) return;
    Node_render_burnDownGarbage();
    if(RendererTimeCapture.tick % 100) return;
    Texture_render_releaseUnusedPngs();
}

// MARK: Création d'un programme OpenGL (pipeline)
void print_GLShaderLog_(GLuint const shaderId) {
    GLint   info_length;
    GLchar* info_str;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetShaderInfoLog(shaderId, info_length, NULL, info_str);
        printerror("Error with shader: %s.", info_str);
        free(info_str);
    } else {
        printok("Shader loaded.");
    }
}
void print_GLProgramLog_(GLuint const programId) {
    GLint   info_length;
    GLchar* info_str;
    glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetProgramInfoLog(programId, info_length, NULL, info_str);
        printerror("GL Program: %s.", info_str);
        free(info_str);
    } else {
        printok("Program OK.");
    }
}
GLuint CoqGLProgram_createFromShaders(const char*const vertexShaderFileName,
                                   const char*const fragmentShaderFileName) 
{
    // Vertex Shader
    char* shader_path = FileManager_getResourcePath();
    String_pathAdd(shader_path, vertexShaderFileName, "glsl", NULL);
    const char* shader_content = FILE_stringContentOptAt(shader_path, false);
    if(!shader_content) {
        printerror("Cannot load %s.", vertexShaderFileName);
        return 0;
    }
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &shader_content, NULL);
    glCompileShader(vertexShaderId);
    print_GLShaderLog_(vertexShaderId);

    // Fragment Shader
    shader_path = FileManager_getResourcePath();
    String_pathAdd(shader_path, fragmentShaderFileName, "glsl", NULL);
    shader_content = FILE_stringContentOptAt(shader_path, false);
    if(!shader_content) {
        printerror("Cannot load %s.", fragmentShaderFileName);
        return 0;
    }
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &shader_content, NULL);
    glCompileShader(fragmentShaderId);
    print_GLShaderLog_(fragmentShaderId);
    FILE_freeBuffer();

    // Création du program
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    print_GLProgramLog_(programId);
    GLint   programOk;
    glGetProgramiv(programId, GL_LINK_STATUS, &programOk);
    if(!programOk) {
        printerror("Cannot link glsl program.");
        return 0;
    }
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);
    return programId;
}

// MARK: Création d'un frame buffer
static const GLuint GL_ColorAttIds[] = {
    GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, 
    GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7, GL_COLOR_ATTACHMENT8,
    GL_COLOR_ATTACHMENT9, GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
    GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14,
    GL_COLOR_ATTACHMENT15,
};
FramebufferWithColors CoqGLFramebuffer_create(
    uint32_t const colorAttachmentCount,
    uint32_t const width, uint32_t const height)
{
    if(colorAttachmentCount > 16) {
        printerror("Max 16 color attch.");
        uint_initConst(&colorAttachmentCount, 16);
    }
    FramebufferWithColors fbwc = { 
        .colorAttachmentCount = colorAttachmentCount
    };
    glGenFramebuffers(1, &fbwc.framebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, fbwc.framebufferId);
    // Texture des color attachment du framebuffer
    glGenTextures(colorAttachmentCount, fbwc.colorAttachmentIds);
    for(uint32_t i = 0; i < colorAttachmentCount; i++) {
        glBindTexture(GL_TEXTURE_2D, fbwc.colorAttachmentIds[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
            width, height, 
            0, GL_RGB, GL_UNSIGNED_BYTE, NULL
        );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // Attacher la texture au framebuffer.
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_ColorAttIds[i], 
            GL_TEXTURE_2D, fbwc.colorAttachmentIds[i], 0);
    }
    // Activer la liste des color attachements où on doit écrire lors de la first pass.
    glDrawBuffers(colorAttachmentCount, GL_ColorAttIds);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        printok("Framebuffer init.");
    } else {
        printerror("Problem with framebuffer %d.", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }   
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbwc;
}

void CoqGLframebuffer_initColorAttProgramLocations(GLuint programId, char const**const textureNames, uint32_t colorAttCount) {
    // Binder les locations des textures de la passe `programId`.
    // Si on a OpenGL 4.2, ça devient superflu... On peut juste utiliser 
    // layout(binding = ...) dans les glsl.
    for(uint32_t i = 0; i < colorAttCount; i++) {
        GLuint textureLoc = glGetUniformLocation(programId, textureNames[i]);
        glUniform1i(textureLoc, i);
    }
}
void CoqGLframebuffer_bindColorAttachsTex(FramebufferWithColors fbwc) {
    // Setter les textures pour la seconde passe.
    // (Textures où on a dessiné précédemment et que l'on va utiliser pour faire une autre passe.
    for(uint32_t i = 0; i < fbwc.colorAttachmentCount; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, fbwc.colorAttachmentIds[i]);
    }
}

void CoqGLFrameBuffer_delete(FramebufferWithColors fbwc) {
    glDeleteTextures(fbwc.colorAttachmentCount, fbwc.colorAttachmentIds);
    glDeleteFramebuffers(1, &fbwc.framebufferId);
}

// MARK: Instance uniforms buffer, tempon pour passer un packet de InstanceUniforms.
#define IUB_MaxInstances 500
static GLuint IUB_uniformBlockId_ = 0;
static GLuint IUB_bindingPoint_ = 0;
static GLuint IUB_bufferId_ = 0;
void IUsBuffer_opengl_init(GLuint const programId) {
    IUB_uniformBlockId_ = glGetUniformBlockIndex(programId, "InstanceBufferType");
    glUniformBlockBinding(programId, IUB_uniformBlockId_, IUB_bindingPoint_);
    glGenBuffers(1, &IUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(InstanceUniforms)*IUB_MaxInstances, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, IUB_bindingPoint_, IUB_bufferId_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
void IUsBuffer_opengl_deinit(void) {
    glDeleteBuffers(1, &IUB_bufferId_);
}

/// MARK: - Dessin d'une instance (groupe d'instances) OpenGL -
/*-- Mesh et texture presentement utilisees --*/
static Texture const* currentTexture_ = NULL;
static Mesh const*    currentMesh_ = NULL;
static MeshToDraw     currentMeshToDraw_;
static size_t         currentInstanceCount_ = 1;
static InstanceUniforms const* currentIUs_ = NULL;

void rendering_opengl_initForDrawing(void) {
    currentMesh_ = NULL;
    currentTexture_ = NULL;
    currentInstanceCount_ = 1;
}
void rendering_opengl_drawingEnded(void) {
    currentMesh_ = NULL;
    currentTexture_ = NULL;
    currentInstanceCount_ = 1;
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void rendering_opengl_setCurrentMesh(Mesh*const mesh) {
    if(mesh == currentMesh_) return;
    currentMesh_ = mesh;
    mesh_render_checkMeshInit(mesh);
    mesh_render_checkMeshUpdate(mesh);
    currentMeshToDraw_ = mesh_render_getToDraw(currentMesh_);
    glBindVertexArray(currentMeshToDraw_.glVertexArrayId);
}
void rendering_opengl_setCurrentMeshNoCheck(Mesh const*const mesh) {
    if(mesh == currentMesh_) return;
    currentMesh_ = mesh;
    currentMeshToDraw_ = mesh_render_getToDraw(currentMesh_);
    glBindVertexArray(currentMeshToDraw_.glVertexArrayId);
}
void rendering_opengl_setCurrentTexture(Texture*const tex) {
    if(tex == currentTexture_) return;
    currentTexture_ = tex;
    texture_render_checkTexture(tex);
    TextureToDraw currentTextureToDraw_ = texture_render_getToDraw(tex);
    glBindTexture(GL_TEXTURE_2D, currentTextureToDraw_.glTextureId);
}
void rendering_opengl_setCurrentTextureNoCheck(Texture*const tex) {
    if(tex == currentTexture_) return;
    currentTexture_ = tex;
    TextureToDraw currentTextureToDraw_ = texture_render_getToDraw(tex);
    glBindTexture(GL_TEXTURE_2D, currentTextureToDraw_.glTextureId);
}
void rendering_opengl_setIU(InstanceUniforms const* iu) {
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(InstanceUniforms), iu);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    currentInstanceCount_ = 1;
}
void rendering_opengl_setIUs(IUsBuffer const*const ius) {
    currentInstanceCount_ = ius->actual_count;
    currentIUs_ = ius->iusOpt;
    if(currentInstanceCount_ > IUB_MaxInstances) {
        printwarning("To many instance %zu, max is %d.", currentInstanceCount_, IUB_MaxInstances);
        currentInstanceCount_ = IUB_MaxInstances;
    }
    if(!ius->actual_count || !ius->iusOpt) {
        printwarning("IUs missing.");
        currentInstanceCount_ = 0;
        currentIUs_ = NULL;
    }
    glBindBuffer(GL_UNIFORM_BUFFER, IUB_bufferId_);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,
        currentInstanceCount_ * sizeof(InstanceUniforms), currentIUs_);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void rendering_opengl_drawWithCurrents(void) {
    if(0 == currentInstanceCount_) return;
    if(1 == currentInstanceCount_) {
        if(currentMeshToDraw_.indexCount) {
            // (Indices déjà settés...)
            glDrawElements(GL_TRIANGLES, currentMeshToDraw_.indexCount,
                GL_UNSIGNED_SHORT, NULL);
        } else {
            glDrawArrays(currentMeshToDraw_.glPrimitiveType, 0,
                currentMeshToDraw_.vertexCount);
        }
        return;
    }
    // Cas multi-instances...
    // (au moins OpenGL 3.1 pour les instances ??)
    if(currentMeshToDraw_.indexCount) {
        glDrawElementsInstanced(GL_TRIANGLES, currentMeshToDraw_.indexCount,
                    GL_UNSIGNED_SHORT, NULL, (GLsizei)currentInstanceCount_);
    } else {
        glDrawArraysInstanced(currentMeshToDraw_.glPrimitiveType, 0,
                    currentMeshToDraw_.vertexCount, (GLsizei)currentInstanceCount_);
    }
}


#ifdef __APPLE__
#pragma clang diagnostic pop
#endif
