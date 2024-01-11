#include "renderer.h"
#include "coq_nodes.h"

/*-- Dims de la view ---------*/
int           Renderer_width = 800;
int           Renderer_height = 500;

/*-- Id du programme opengl ---------*/
static GLuint _Renderer_program = 0;

/*-- Index des variable GLSL --------*/
static GLint _frame_projection = 0;
static GLint _frame_time = 0;

static GLint _inst_model = 0;
static GLint _inst_color = 0;
static GLint _inst_ij = 0;
static GLint _inst_emph = 0;
static GLint _inst_show = 0;

/*-- Mesh et texture presentement utilisees --*/
static Mesh*    _current_mesh = NULL;
static GLenum   _current_mesh_primitive = GL_TRIANGLE_STRIP;
static uint32_t _current_mesh_index_count = 0;
static uint32_t _current_mesh_vertex_count = 0;
static Texture* _current_texture = NULL;

/*-- deltaT (temps entre deux frame) --*/
static Chrono   _chrono_deltaT;
static FluidPos _fluid_deltaT;


void   _Renderer_drawDrawable(Drawable* d) {
    // 1. Mise a jour de la mesh ?
    if(_current_mesh != d->mesh) {
        _current_mesh = d->mesh;
        _current_mesh_vertex_count = mesh_vertexCount(d->mesh);
        _current_mesh_index_count = mesh_indexCount(d->mesh);
        _current_mesh_primitive = mesh_primitiveType(d->mesh);
        mesh_glBind(_current_mesh);
    }
    // 2. Mise a jour de la texture ?
    if(_current_texture != d->tex) {
        _current_texture = d->tex;

        texture_glBind(_current_texture);
    }
    // 3. Per instance uniforms (piu)
    glUniformMatrix4fv(_inst_model, 1, GL_FALSE, d->n.piu.model.f_arr);
    glUniform4fv(_inst_color, 1, d->n.piu.color.f_arr);
    glUniform2fv(_inst_ij, 1, d->n.piu.tile);
    glUniform1f(_inst_emph, d->n.piu.emph);
    glUniform1f(_inst_show, d->n.piu.show);

    // 4. Dessiner
    if(_current_mesh_index_count) {
        glDrawElements(GL_TRIANGLES, _current_mesh_index_count, GL_UNSIGNED_SHORT, 0);
    } else {
        glDrawArrays(_current_mesh_primitive, 0, _current_mesh_vertex_count);
    }
}

void   Renderer_initWithWindow(GLFWwindow* window, const char* font_path, 
                               const char* font_name)
 {
    // Shaders
    // Vertex shader
    GLint info_length;
    GLchar* info_str;
    const char* vert_path = FileManager_getResourcePathOpt("vertex_shader", "glsl", NULL);
    const char* vert_content = FILE_contentOpt(vert_path);
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vert_content, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetShaderInfoLog(vertex_shader, info_length, NULL, info_str);
        printerror("Vertex shader: %s", info_str);
        free(info_str);
    } else {
        printdebug("Vertex shader OK ✅.");
    }
    // Fragment shader
    const char* frag_path = FileManager_getResourcePathOpt("fragment_shader", "glsl", NULL);
    const char* frag_content = FILE_contentOpt(frag_path);
    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &frag_content, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &info_length);
    if(info_length > 1) {
        info_str = calloc(1, info_length + 1);
        glGetShaderInfoLog(fragment_shader, info_length, NULL, info_str);
        printerror("Fragment shader: %s", info_str);
        free(info_str);
    } else {
        printdebug("Fragment shader OK ✅.");
    }
    // Program
    _Renderer_program = glCreateProgram();
    glAttachShader(_Renderer_program, vertex_shader);
    glAttachShader(_Renderer_program, fragment_shader);
    glLinkProgram(_Renderer_program);
    // Juste un program... on peut le setter tout suite (pas de changement)
    glUseProgram(_Renderer_program);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Index des variable GLSL
    _frame_projection = glGetUniformLocation(_Renderer_program, "frame_projection");
    _frame_time =       glGetUniformLocation(_Renderer_program, "frame_time");
    _inst_model =       glGetUniformLocation(_Renderer_program, "inst_model");
    _inst_color =       glGetUniformLocation(_Renderer_program, "inst_color");
    _inst_ij =          glGetUniformLocation(_Renderer_program, "inst_ij");
    _inst_emph =        glGetUniformLocation(_Renderer_program, "inst_emph");
    _inst_show =        glGetUniformLocation(_Renderer_program, "inst_show");

    Mesh_init(_Renderer_program);
    Texture_init(_Renderer_program, font_path, font_name);
}

void   Renderer_drawView(GLFWwindow* window, Root* root) {
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
        if(d) _Renderer_drawDrawable(d);
    } while(sq_goToNextToDisplay(&sq));

    // Unbind (superflu?)
    _current_mesh = NULL;
    _current_texture = NULL;
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    // 5. Fini, présenter...
    glfwSwapBuffers(window);
}

