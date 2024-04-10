#include "renderer.h"


/*-- Dims de la view ---------*/
int           Renderer_width = 800;
int           Renderer_height = 500;

//-- Version récente ? 
static bool isOpenGL_3_1_ = false;

/*-- Index des variable GLSL --------*/
static GLint _frame_projection = 0;
static GLint _frame_time = 0;

/*-- Mesh et texture presentement utilisees --*/
static Mesh*    _current_mesh = NULL;
static GLenum   _current_mesh_primitive = GL_TRIANGLE_STRIP;
static uint32_t _current_mesh_index_count = 0;
static uint32_t _current_mesh_vertex_count = 0;
static Texture* _current_texture = NULL;

/*-- deltaT (temps entre deux frame) --*/
static Chrono   _chrono_deltaT;
static FluidPos _fluid_deltaT;


void   Renderer_drawDrawable_(Drawable* d) {
    // 1. Mise a jour de la mesh ?
    if(_current_mesh != d->_mesh) {
        _current_mesh = d->_mesh;
        _current_mesh_vertex_count = mesh_vertexCount(d->_mesh);
        _current_mesh_index_count = mesh_indexCount(d->_mesh);
        _current_mesh_primitive = mesh_primitiveType(d->_mesh);
        mesh_glBind(_current_mesh);
    }
    // 2. Mise a jour de la texture ?
    if(_current_texture != d->_tex) {
        _current_texture = d->_tex;
        texture_glBind(_current_texture);
    }
    // 3. Cas standard (une instance)
    DrawableMulti* dm = node_asDrawableMultiOpt(&d->n);
    if(!dm) {
      perinstanceuniform_glBind(&d->n._piu);
      if(_current_mesh_index_count) {
        glDrawElements(GL_TRIANGLES, _current_mesh_index_count, GL_UNSIGNED_SHORT, 0);
      } else {
        glDrawArrays(_current_mesh_primitive, 0, _current_mesh_vertex_count);
      }
      return;
    }
    // 4. Cas multi-instance
    uint32_t instanceCount = dm->piusBuffer.actual_count;
    if(instanceCount == 0) {
        printwarning("No instance to draw.");
        return;
    }
    // (au moins 3.1 pour les instances ??)
    if(isOpenGL_3_1_) { 
      piusbuffer_glBind(&dm->piusBuffer);
      if(_current_mesh_index_count) {
        glDrawElementsInstanced(GL_TRIANGLES, _current_mesh_index_count, GL_UNSIGNED_SHORT,
                                0, instanceCount);
      } else {
        glDrawArraysInstanced(_current_mesh_primitive, 0, 
                              _current_mesh_vertex_count, instanceCount);
      }
      return;
    }
    // Sinon boucle pour dessiner toutes les instances... :(  ??
    // Il doit il y avoir une meilleure solution... ?
    const PerInstanceUniforms* piu =        dm->piusBuffer.pius;
    const PerInstanceUniforms* const end = &dm->piusBuffer.pius[dm->piusBuffer.actual_count];
    while(piu < end) {
        perinstanceuniform_glBind(piu);
        if(_current_mesh_index_count) {
          glDrawElements(GL_TRIANGLES, _current_mesh_index_count, GL_UNSIGNED_SHORT, 0);
        } else {
          glDrawArrays(_current_mesh_primitive, 0, _current_mesh_vertex_count);
        }
        piu++;
    }
}

void   Renderer_initWithWindow(SDL_Window* window, const char* font_path, 
                               const char* font_name)
 {
    // Version 
    
    const char* version_str = (const char*)glGetString(GL_VERSION);
    float       version_float;
    sscanf(version_str, "%f", &version_float);
    isOpenGL_3_1_ = version_float > 3.0;
    printdebug("Is at least OpenGL 3.1: %s.", isOpenGL_3_1_ ? "yes ✅" : "no ⚠️");
    if(!isOpenGL_3_1_) {
        printwarning("Cannot draw multiple instance with openGL %s ?", version_str);
    }
    // Shaders
    // Vertex shader
    GLint info_length;
    GLchar* info_str;
    const char* glsl_version = isOpenGL_3_1_ ? "\n\n#version 410\n\n" : "\n\n#version 300 es\n\n";
    const char* shader_path = FileManager_getResourcePathOpt("vertex_shader", "glsl", NULL);
    const char* shader_content = FILE_stringContentOptAt(shader_path);
    const char* vertexShader_content = String_createCat(glsl_version, shader_content);
    // printf(vertexShader_content);
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderId, 1, &vertexShader_content, NULL);
    coq_free((char*)vertexShader_content);
    glCompileShader(vertexShaderId);
    glGetShaderiv(vertexShaderId, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetShaderInfoLog(vertexShaderId, info_length, NULL, info_str);
        printerror("Vertex shader: %s", info_str);
        free(info_str);
    } else {
        printdebug("Vertex shader OK ✅.");
    }
    // Fragment shader
    shader_path = FileManager_getResourcePathOpt("fragment_shader", "glsl", NULL);
    shader_content = FILE_stringContentOptAt(shader_path);
    const char* fragmentShader_content = String_createCat(glsl_version, shader_content);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderId, 1, &fragmentShader_content, NULL);
    coq_free((char*)fragmentShader_content);
    glCompileShader(fragmentShaderId);
    glGetShaderiv(fragmentShaderId, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetShaderInfoLog(fragmentShaderId, info_length, NULL, info_str);
        printerror("Fragment shader: %s", info_str);
        free(info_str);
    } else {
        printdebug("Fragment shader OK ✅.");
    }
    // Program
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    // Juste un program... on peut le setter tout suite (pas de changement)
    glUseProgram(programId);
    // Blending ordinaire...
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Index des variable GLSL
    _frame_projection = glGetUniformLocation(programId, "frame_projection");
    _frame_time =       glGetUniformLocation(programId, "frame_time");

    Mesh_init(programId);
    Texture_GLinit(programId, font_path, font_name);
    PIUsBuffer_GLinit(programId);
}
void   Renderer_drawView(SDL_Window* window, Root* root) {
    if(!mesh_sprite) { printerror("Mesh not init."); return; }
    if(!root) { printerror("No root."); return; }
    // 1. Check le chrono/sleep.
    float deltaTMS = fminf(60.f, fmaxf(3.f, 
        (float)chrono_elapsedMS(&_chrono_deltaT)));
    chrono_start(&_chrono_deltaT);
    fl_set(&_fluid_deltaT, deltaTMS);
    deltaTMS = fl_pos(&_fluid_deltaT);
    int64_t deltaTMSint = rand_float_toInt(deltaTMS);
    ChronoRender_update(deltaTMSint);

    // 2. Per frame uniform (pfu)
    pfu_default.time = ChronoRender_elapsedAngleSec();
    matrix4_initProjectionWithRoot(&pfu_default.projection, root);
    glUniformMatrix4fv(_frame_projection, 1, GL_FALSE, pfu_default.projection.f_arr);
    glUniform1f(_frame_time, pfu_default.time);
    // matrix4_print(&pfu_default.projection);

    // 3. Set viewport, et clear color (cc) red, green, blue, alpha.
    glViewport(0, 0, Renderer_width, Renderer_height);
    Vector4 cc = fl_array_toVec4(root->back_RGBA);
    glClearColor(cc.r, cc.g, cc.b, cc.a);
    glClear(GL_COLOR_BUFFER_BIT);

    // 4. Dessiner tous les objets...
    Drawable* (*updateModel)(Node*) = root->updateModelAndGetDrawable;
    Squirrel sq;
    sq_init(&sq, &root->n, sq_scale_ones);
    do {
        Drawable* d = updateModel(sq.pos);
        if(d) Renderer_drawDrawable_(d);
    } while(sq_goToNextToDisplay(&sq));

    // Unbind (superflu?)
    _current_mesh = NULL;
    _current_texture = NULL;
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    // 5. Fini, présenter...
    SDL_GL_SwapWindow(window);
}

