/**
* Author: Gianfranco Romani
* Assignment: Pong Clone
* Date due: 2023-10-28, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define LOG(argument) std::cout << argument << '\n'
#define STB_IMAGE_IMPLEMENTATION
#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include <cmath>

const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.0f,
            BG_BLUE = 0.0f,
            BG_GREEN = 0.0f,
            BG_OPACITY = 1.0f;


const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const char PADDLE_SPRITE[] = "white_board.png";
const char BALL_SPRITE[] = "ball.png";
const char BORDER_BOX_SPRITE[] = "border_box.png";


const float ROT_SPEED = 100.0f;


// POSITIONS
const glm::vec3 PADDLE_ONE_INIT_POS = glm::vec3(-4.0f, 0.0f, 0.0f),
                PADDLE_ONE_INIT_SCA = glm::vec3(0.25f, 2.5f, 0.0f);

const glm::vec3 PADDLE_TWO_INIT_POS = glm::vec3(4.5f, 0.0f, 0.0f),
                PADDLE_TWO_INIT_SCA = glm::vec3(0.25f, 2.5f, 0.0f);

const glm::vec3 BALL_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f),
                BALL_INIT_SCA = glm::vec3(1.0f, 1.0f, 0.0f);

// BORDER-BOX
const glm::vec3 BORDER_UP_INIT_POS = glm::vec3(0.0f, 3.65f, 0.0f),
                BORDER_UP_INIT_SCA = glm::vec3(10.0f, 0.25f, 0.0f);

const glm::vec3 BORDER_DOWN_INIT_POS = glm::vec3(0.0f, -3.65f, 0.0f),
                BORDER_DOWN_INIT_SCA = glm::vec3(10.0f, 0.25f, 0.0f);

const glm::vec3 BORDER_LEFT_INIT_POS = glm::vec3(-4.9f, 0.0f, 0.0f),
                BORDER_LEFT_INIT_SCA = glm::vec3(0.25f, 6.0f, 0.0f);

const glm::vec3 BORDER_RIGHT_INIT_POS = glm::vec3(4.9f, 0.0f, 0.0f),
                BORDER_RIGHT_INIT_SCA = glm::vec3(.25f, 10.0f, 0.0f);

// TEXTURES
const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0,
            TEXTURE_BORDER = 0;

const float MILLISECONDS_IN_SECOND = 1000.0;

SDL_Window* g_display_window;
bool g_game_is_running = true;

ShaderProgram g_shader_program;

// OBJECTS IN SCREEN
GLuint        g_paddle_texture_id,
              g_ball_texture_id,
              g_border_texture_id;

glm::mat4 g_view_matrix,
          g_paddle_one_model_matrix,
          g_paddle_two_model_matrix,
          g_border_up_model_matrix,
          g_border_down_model_matrix,
          g_border_left_model_matrix,
          g_border_right_model_matrix,
          g_ball_model_matrix,
          g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_rot_angle = 0.0f;
float g_speed = 1.0f;


glm::vec3 g_paddle_one_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_paddle_one_position = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_paddle_two_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_paddle_two_position = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_movement = glm::vec3(0.0f, 0.0f, 0.0f),
          g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);

bool g_multiplayer_status = false;

bool paddle_one_up_lock = false,
     paddle_one_down_lock = false;

bool paddle_two_up_lock = false,
     paddle_two_down_lock = false;

bool ai_paddle_direction = true;

int BORDER_COLLISION = -1;


GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(image);

    return textureID;
}


void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("User input exercise",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    // ———————————————— PADDLE_ONE ———————————————— //
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_paddle_one_model_matrix = glm::mat4(1.0f);
    g_paddle_one_model_matrix = glm::translate(g_paddle_one_model_matrix, PADDLE_ONE_INIT_POS);
    g_paddle_one_model_matrix = glm::scale(g_paddle_one_model_matrix, PADDLE_ONE_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_paddle_texture_id = load_texture(PADDLE_SPRITE);

    // ———————————————— PADDLE_TWO ———————————————— //
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_paddle_two_model_matrix = glm::mat4(1.0f);
    g_paddle_two_model_matrix = glm::translate(g_paddle_two_model_matrix, PADDLE_TWO_INIT_POS);
    g_paddle_two_model_matrix = glm::scale(g_paddle_two_model_matrix, PADDLE_TWO_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_paddle_texture_id = load_texture(PADDLE_SPRITE);

    // ———————————————— BALL ———————————————— //
    g_ball_model_matrix = glm::mat4(1.0f);
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, BALL_INIT_POS);
    g_ball_model_matrix = glm::scale(g_ball_model_matrix, BALL_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    g_ball_texture_id = load_texture(BALL_SPRITE);

    // ———————————————— BORDER_UP ———————————————— //
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_border_up_model_matrix = glm::mat4(1.0f);
    g_border_up_model_matrix = glm::translate(g_border_up_model_matrix, BORDER_UP_INIT_POS);
    g_border_up_model_matrix = glm::scale(g_border_up_model_matrix, BORDER_UP_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_border_texture_id = load_texture(BORDER_BOX_SPRITE);

    // ———————————————— BORDER_DOWN ———————————————— //
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_border_down_model_matrix = glm::mat4(1.0f);
    g_border_down_model_matrix = glm::translate(g_border_down_model_matrix, BORDER_DOWN_INIT_POS);
    g_border_down_model_matrix = glm::scale(g_border_down_model_matrix, BORDER_DOWN_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_border_texture_id = load_texture(BORDER_BOX_SPRITE);

    // ———————————————— BORDER_LEFT ———————————————— //
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_border_left_model_matrix = glm::mat4(1.0f);
    g_border_left_model_matrix = glm::translate(g_border_left_model_matrix, BORDER_LEFT_INIT_POS);
    g_border_left_model_matrix = glm::scale(g_border_left_model_matrix, BORDER_LEFT_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_border_texture_id = load_texture(BORDER_BOX_SPRITE);

    // ———————————————— BORDER_RIGHT ———————————————— //
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_border_right_model_matrix = glm::mat4(1.0f);
    g_border_right_model_matrix = glm::translate(g_border_right_model_matrix, BORDER_RIGHT_INIT_POS);
    g_border_right_model_matrix = glm::scale(g_border_right_model_matrix, BORDER_RIGHT_INIT_SCA);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());
    g_border_texture_id = load_texture(BORDER_BOX_SPRITE);

    // ———————————————— GENERAL ———————————————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
}

void process_input()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_game_is_running = !g_game_is_running;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_q:
                g_game_is_running = !g_game_is_running;
                break;

            default: break;
            }
        }
    }
    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_W] && !paddle_one_up_lock)
    {
        g_paddle_one_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_S] && !paddle_one_down_lock)
    {
        g_paddle_one_movement.y = -1.0f;
    }
    else if (key_state[SDL_SCANCODE_T]) {
        g_multiplayer_status = !g_multiplayer_status;
    } 
    
    if (key_state[SDL_SCANCODE_UP] && g_multiplayer_status && !paddle_two_up_lock)
    {
        g_paddle_two_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN] && g_multiplayer_status && !paddle_two_down_lock)
    {
        g_paddle_two_movement.y = -1.0f;
    }

    if (glm::length(g_paddle_one_movement) > 1.0f)
    {
        g_paddle_one_movement = glm::normalize(g_paddle_one_movement);
    }

    if (glm::length(g_paddle_two_movement) > 1.0f)
    {
        g_paddle_two_movement = glm::normalize(g_paddle_two_movement);
    }
}

void update()
{

    float collision_factor = 2;

    /** ———— PADDLE_ONE_BORDER COLLISION DETECTION ———— **/
    float x_borderup_distance = fabs(g_paddle_one_position.x - BORDER_UP_INIT_POS.x) - ((PADDLE_ONE_INIT_SCA.x * collision_factor + BORDER_UP_INIT_SCA.x * collision_factor) / 2.0f);
    float y_borderup_distance = fabs(g_paddle_one_position.y - BORDER_UP_INIT_POS.y) - ((PADDLE_ONE_INIT_SCA.y * collision_factor + BORDER_UP_INIT_SCA.y * collision_factor) / 2.0f);

    if (x_borderup_distance < 0.0f && y_borderup_distance < 0.0f)
    {
        paddle_one_up_lock = true;
    }
    else {
        paddle_one_up_lock = false;
    }

    float x_borderdown_distance = fabs(g_paddle_one_position.x - BORDER_DOWN_INIT_POS.x) - ((PADDLE_ONE_INIT_SCA.x * collision_factor + BORDER_DOWN_INIT_SCA.x * collision_factor) / 2.0f);
    float y_borderdown_distance = fabs(g_paddle_one_position.y - BORDER_DOWN_INIT_POS.y) - ((PADDLE_ONE_INIT_SCA.y * collision_factor + BORDER_DOWN_INIT_SCA.y * collision_factor) / 2.0f);

    if (x_borderdown_distance < 0.0f && y_borderdown_distance < 0.0f)
    {
        paddle_one_down_lock = true;
    }
    else {
        paddle_one_down_lock = false;
    }

    /** ———— PADDLE_TWO_BORDER COLLISION DETECTION ———— **/
    float x_two_borderup_distance = fabs(g_paddle_two_position.x - BORDER_UP_INIT_POS.x) - ((PADDLE_TWO_INIT_SCA.x * collision_factor + BORDER_UP_INIT_SCA.x * collision_factor) / 2.0f);
    float y_two_borderup_distance = fabs(g_paddle_two_position.y - BORDER_UP_INIT_POS.y) - ((PADDLE_TWO_INIT_SCA.y * collision_factor + BORDER_UP_INIT_SCA.y * collision_factor) / 2.0f);

    if (x_two_borderup_distance < 0.0f && y_two_borderup_distance < 0.0f)
    {
        ai_paddle_direction = false;
        paddle_two_up_lock = true;
    }
    else {
        paddle_two_up_lock = false;
    }

    float x_two_borderdown_distance = fabs(g_paddle_two_position.x - BORDER_DOWN_INIT_POS.x) - ((PADDLE_TWO_INIT_SCA.x * collision_factor + BORDER_DOWN_INIT_SCA.x * collision_factor) / 2.0f);
    float y_two_borderdown_distance = fabs(g_paddle_two_position.y - BORDER_DOWN_INIT_POS.y) - ((PADDLE_TWO_INIT_SCA.y * collision_factor + BORDER_DOWN_INIT_SCA.y * collision_factor) / 2.0f);

    if (x_two_borderdown_distance < 0.0f && y_two_borderdown_distance < 0.0f)
    {
        ai_paddle_direction = true;
        paddle_two_down_lock = true;
    }
    else {
        paddle_two_down_lock = false;
    }

    if (ai_paddle_direction && !g_multiplayer_status) {
        g_paddle_two_movement.y = 1.0f;
    }
    else if (!ai_paddle_direction && !g_multiplayer_status) {
        g_paddle_two_movement.y = -1.0f;
    }

    /** ———— BALL_BORDER COLLISION DETECTION ———— **/
    
    float border_collision_factor = 1;
    float border_lr_collision_factor = 1.8;

    float x_ball_u_distance = fabs(g_ball_position.x - BORDER_UP_INIT_POS.x) - ((BALL_INIT_SCA.x * border_collision_factor + BORDER_UP_INIT_SCA.x * border_collision_factor) / 2.0f);
    float y_ball_u_distance = fabs(g_ball_position.y - BORDER_UP_INIT_POS.y) - ((BALL_INIT_SCA.y * border_collision_factor + BORDER_UP_INIT_SCA.y * border_collision_factor) / 2.0f);

    if (x_ball_u_distance < 0.0f && y_ball_u_distance < 0.0f) {
        BORDER_COLLISION = 0;
    }

    float x_ball_r_distance = fabs(g_ball_position.x - BORDER_RIGHT_INIT_POS.x) - ((BALL_INIT_SCA.x * border_lr_collision_factor + BORDER_RIGHT_INIT_SCA.x * border_lr_collision_factor) / 2.0f);
    float y_ball_r_distance = fabs(g_ball_position.y - BORDER_RIGHT_INIT_POS.y) - ((BALL_INIT_SCA.y * border_lr_collision_factor + BORDER_RIGHT_INIT_SCA.y * border_lr_collision_factor) / 2.0f);
    if (x_ball_r_distance < 0.0f && y_ball_r_distance < 0.0f) {
        exit(0);
    }

    float x_ball_l_distance = fabs(g_ball_position.x - BORDER_LEFT_INIT_POS.x) - ((BALL_INIT_SCA.x * border_lr_collision_factor + BORDER_LEFT_INIT_SCA.x * border_lr_collision_factor) / 2.0f);
    float y_ball_l_distance = fabs(g_ball_position.y - BORDER_LEFT_INIT_POS.y) - ((BALL_INIT_SCA.y * border_lr_collision_factor + BORDER_LEFT_INIT_SCA.y * border_lr_collision_factor) / 2.0f);
    if (x_ball_l_distance < 0.0f) {
        exit(0);
    }

    float y_ball_d_distance = fabs(g_ball_position.y - BORDER_DOWN_INIT_POS.y) - ((BALL_INIT_SCA.y * border_collision_factor + BORDER_DOWN_INIT_SCA.y * border_collision_factor) / 2.0f);
    if (y_ball_d_distance < 0.0f) {
        BORDER_COLLISION = 3;
    }

    /** ———— BALL_PADDLE COLLISION DETECTION ———— **/
    float paddle_collision_factor = 1.3;
    float paddle_collision_factor_y = 1.0;

    float x_ball_paddle_one_distance = fabs(g_ball_position.x - PADDLE_ONE_INIT_POS.x) - ((BALL_INIT_SCA.x * paddle_collision_factor + PADDLE_ONE_INIT_SCA.x * paddle_collision_factor) / 2.0f);
    float y_ball_paddle_one_distance = fabs(g_ball_position.y - PADDLE_ONE_INIT_POS.y) - ((BALL_INIT_SCA.y * paddle_collision_factor_y + PADDLE_ONE_INIT_SCA.y * paddle_collision_factor_y) / 2.0f);

    if (x_ball_paddle_one_distance < 0.0f && y_ball_paddle_one_distance < 0.0f) {
        BORDER_COLLISION = 2;
    }

    float x_ball_paddle_two_distance = fabs(g_ball_position.x - PADDLE_TWO_INIT_POS.x) - ((BALL_INIT_SCA.x * paddle_collision_factor + PADDLE_TWO_INIT_SCA.x * paddle_collision_factor) / 2.0f);

    if (x_ball_paddle_two_distance < 0.0f) {
        BORDER_COLLISION = 1;
    }

    if (BORDER_COLLISION == 0) {
        g_ball_movement.y = -1.5f;
        g_ball_movement.x = 1.5f;
    }
    else if (BORDER_COLLISION == 1) {
        g_ball_movement.y = -1.5f;
        g_ball_movement.x = -1.5f;
    }
    else if (BORDER_COLLISION == 2) {
        g_ball_movement.y = 0.5f;
        g_ball_movement.x = 1.5f;
    }
    else if (BORDER_COLLISION == 3) {
        g_ball_movement.y = 1.5f;
        g_ball_movement.x = -1.5f;
    }
    else {
        g_ball_movement.y = -0.5f;
        g_ball_movement.x = -1.5f;
    }

    // ———————————————— DELTA TIME CALCULATIONS ———————————————— //
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    // ———————————————— RESETTING MODEL MATRIX ———————————————— //
    g_paddle_one_model_matrix = glm::mat4(1.0f);
    g_paddle_one_model_matrix = glm::translate(g_paddle_one_model_matrix, PADDLE_ONE_INIT_POS);
    g_paddle_one_model_matrix = glm::scale(g_paddle_one_model_matrix, PADDLE_ONE_INIT_SCA);

    g_paddle_two_model_matrix = glm::mat4(1.0f);
    g_paddle_two_model_matrix = glm::translate(g_paddle_two_model_matrix, PADDLE_TWO_INIT_POS);
    g_paddle_two_model_matrix = glm::scale(g_paddle_two_model_matrix, PADDLE_TWO_INIT_SCA);

    g_ball_model_matrix = glm::mat4(1.0f);
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, BALL_INIT_POS);
    g_ball_model_matrix = glm::scale(g_ball_model_matrix, BALL_INIT_SCA);

    // ———————————————— TRANSLATIONS ———————————————— //
    g_paddle_one_position += g_paddle_one_movement * g_speed * delta_time;
    g_paddle_one_model_matrix = glm::translate(g_paddle_one_model_matrix, g_paddle_one_position);
    g_paddle_one_movement = glm::vec3(0.0f, 0.0f, 0.0f);

    g_paddle_two_position += g_paddle_two_movement * g_speed * delta_time;
    g_paddle_two_model_matrix = glm::translate(g_paddle_two_model_matrix, g_paddle_two_position);
    g_paddle_two_movement = glm::vec3(0.0f, 0.0f, 0.0f);

    g_ball_position += g_ball_movement * g_speed * delta_time;
    g_ball_model_matrix = glm::translate(g_ball_model_matrix, g_ball_position);
    g_ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // ———————————————— PADDLE_ONE ———————————————— //
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };

    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_paddle_one_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_paddle_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— PADDLE_TWO ———————————————— //

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_paddle_two_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_paddle_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BALL ———————————————— //
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_ball_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_ball_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BORDER_UP ———————————————— //

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_border_up_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_border_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BORDER_DOWN ———————————————— //

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_border_down_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_border_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BORDER_LEFT ———————————————— //

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_border_left_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_border_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— BORDER_RIGHT ———————————————— //

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    g_shader_program.set_model_matrix(g_border_right_model_matrix);
    glBindTexture(GL_TEXTURE_2D, g_border_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // ———————————————— GENERAL ———————————————— //
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();

    while (g_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
