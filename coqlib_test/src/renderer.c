#include "renderer.h"
#include "_math/_math_camera.h"

/*-- Dims de la view ---------*/
static int    _Renderer_width = 400;
static int    _Renderer_height = 300;

/*-- Id du programme opengl ---------*/
static GLuint _Renderer_program = 0;

/*-- Index des variable GLSL --------*/
static GLint _frame_projection = 0;
static GLint _frame_time = 0;
static GLint _inst_model = 0;
static GLint _inst_ij = 0;

static Mesh*    _Renderer_current_mesh = NULL;
static Texture* _Renderer_current_texture = NULL;

static Texture* _tex_cat = NULL;
static Texture* _tex_test_frame = NULL;

// void _Renderer_drawDrawable(Drawable* d) {

// }

void Renderer_initWithWindow(GLFWwindow* window) {
    // Taille de la view
    glfwGetFramebufferSize(window, &_Renderer_width, &_Renderer_height);
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
    _inst_ij =          glGetUniformLocation(_Renderer_program, "inst_ij");
    GLint _wh_id =      glGetUniformLocation(_Renderer_program, "tex_wh");
    GLint _mn_id =      glGetUniformLocation(_Renderer_program, "tex_mn");
    
    // _texture_loc =      glGetUniformLocation(_Renderer_program, "tex");
    printdebug("locations %d, %d, %d, %d. tex %d, %d", _frame_projection, _frame_time,
            _inst_model, _inst_ij, _wh_id, _mn_id);

    Mesh_init(_Renderer_program);
    Texture_init(_Renderer_program);

    _tex_cat =        Texture_sharedImageByName("coqlib_digits_black");
    _tex_test_frame = Texture_sharedImageByName("coqlib_test_frame");

    // Matrices pour tester...
    Matrix4 projection, model;
    Camera  camera;
    camera_init(&camera, 10.f);
    matrix4_initAsPerspectiveDeltas(&projection, 0.1f, 5.f, 4.f, 2.f, 2.f);
    matrix4_initAsLookAtWithCameraAndYshift(&model, &camera, 0.f);
    glUniformMatrix4fv(_frame_projection, 1, GL_FALSE, projection.m);
    glUniformMatrix4fv(_inst_model, 1, GL_FALSE, model.m);
    glUniform2f(_inst_ij, 0, 0);
}

void         Renderer_drawView(GLFWwindow* window) {
    if(!mesh_sprite) { printerror("Mesh not init."); return; }
    // Clear
    glViewport(0, 0, _Renderer_width, _Renderer_height);
    glClearColor(0.f, 0.5f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Dessiner les objets
    _Renderer_current_mesh = mesh_sprite;

    // TODO boucle sur la root...
    mesh_glBind(_Renderer_current_mesh);
    texture_glBind(_tex_test_frame);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    texture_glBind(_tex_cat);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    // ->boucle sur root...

    // Présenter...
    glfwSwapBuffers(window);
}

void         Renderer_resize(GLFWwindow* window, int width, int height) {
    _Renderer_width =  width;
    _Renderer_height = height;
}

GLuint const Renderer_glProgram(void) {
    return _Renderer_program;
}