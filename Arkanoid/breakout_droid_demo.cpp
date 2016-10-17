#include <jni.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android/sensor.h>
#include <android/log.h>
#include <../../../../android/native_app_glue/android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

struct Paddle
{
    float x;
    float y;
};

static const float PADDLE_HALF_WIDTH = 0.15f;
static const float PADDLE_HALF_HEIGHT = 0.025f;
static const float PADDLE_START_X = 0.0f;
static const float PADDLE_START_Y = -0.775f;
static const float PADDLE_LEFT_BOUND = (-1.0f + PADDLE_HALF_HEIGHT);
static const float PADDLE_RIGHT_BOUND = (1.0f + PADDLE_HALF_WIDTH);

static const float BALL_HALF_WIDTH = 0.025f;
static const float BALL_HALF_HEIGHT = 0.025f;
static const float BALL_START_X = 0.0f;
static const float BALL_START_Y = 0.0f;
static const float BALL_VELOCITY = 0.015;
static const float BALL_LEFT_BOUND = (-1.0f + BALL_HALF_WIDTH);
static const float BALL_RIGHT_BOUND = (1.0f + BALL_HALF_WIDTH);
static const float BALL_TOP_BOUND = (1.0f + BALL_HALF_HEIGHT);
static const float BALL_BOTTOM_BOUND = (-1.0f + BALL_HALF_HEIGHT);

static const float BLOCK_HALF_WIDTH = 0.15f;
static const float BLOCK_WIDTH = BLOCK_HALF_WIDTH * 2.0f;
static const float BLOCK_HALF_HEIGHT = 0.05;
static const float BLOCK_HEIGHT = BLOCK_HALF_HEIGHT * 2.0f;
static const float BLOCK_START_POSITION = -0.62f;
static const float BLOCK_HORIZONTAL_GAP = 0.01f;

static const int32_t NUM_BLOCKS = 15;
static const int32_t NUM_BLOCKS_ROW = 5;

static const int32_t VERTEX_SIZE = sizeof(GLfloat) * 7;
static const int32_t POSITION_PARAMETER_INDEX = 0;
static const int32_t POSITION_NUM_ELEMENTS = 3;
static const int32_t COLOR_PARAMETER_INDEX = 1;
static const int32_t COLOR_NUM_ELEMENTS = 4;

static const int32_t TRIANGLE_NUM_VERTICES = 3;
static const int32_t QUAD_NUM_VERTICES = 6;

struct block
{
    float x;
    float y;
    bool isActive;
};

struct engine
{
    struct android_app* app;

    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t programObject;
    float width;
    float height;
    float touchX;
    float touchY;
    float playerX;
    float playerY;
    float ballX;
    float ballY;
    float ballVelocityX;
    float ballVelocityY;
    bool touchIsDown;
    block blocks[NUM_BLOCKS];
};

static void engine_init_blocks(struct engine* engine)
{
    float blockYPos[] = { 0.8f, 0.675f, 0.55f };

    for (int32_t i = 0; i < NUM_BLOCKS; ++i)
    {
        engine->blocks[i].x = BLOCK_START_POSITION + ((BLOCK_WIDTH + BLOCK_HORIZONTAL_GAP) * (i % NUM_BLOCKS_ROW));
        engine->blocks[i].y = blockYPos[i / NUM_BLOCKS_ROW];
        engine->blocks[i].isActive = true;
    }
}

GLuint LoadShader(const char* shaderSrc, GLenum type)
{
    GLuint shader;
    GLint compiled;

    shader = glCreateShader(type);

    if (shader != 0)
    {
        glShaderSource(shader, 1, &shaderSrc, NULL);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            GLint info_length = 0;

            if (info_length > 1)
            {
                char* info_log = new char[info_length];
                glGetShaderInfoLog(shader, info_length, NULL, info_log);
                LOGW("ERROR: Shader Compilation Failure: %s\n", info_log);
                delete[] info_log;
            }
        }

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

static int engine_init_display(struct engine* engine)
{
    const EGLint attribs[] =
            {
                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                    EGL_BLUE_SIZE, 8,
                    EGL_GREEN_SIZE, 8,
                    EGL_RED_SIZE, 8,
                    EGL_NONE
            };

    EGLint w, h, dummy, format;
    EGLint num_configs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, NULL, NULL);

    eglChooseConfig(display, attribs, &config, 1, &num_configs);

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);

    EGLint contextAttribs[] =
            {
                    EGL_CONTEXT_CLIENT_VERSION, 2,
                    EGL_NONE
            };

    context = eglCreateContext(display, config, NULL, contextAttribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        LOGW("Unable To eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->touchX = 0.0f;
    engine->touchY = 0.0f;
    engine->touchIsDown = false;
    engine->playerX = PADDLE_START_X;
    engine->playerY = PADDLE_START_Y;
    engine->ballX = BALL_START_X;
    engine->ballY = BALL_START_Y;
    engine->ballVelocityX = BALL_VELOCITY;
    engine->ballVelocityY = BALL_VELOCITY;

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    char vShaderStr[] =
            "attribute vec4 a_vPosition;	        \n"
                    "attribute vec4 a_vColor;		\n"
                    "varying vec4 v_vColor; 		\n"
                    "void main()					\n"
                    "{								\n"
                    "	gl_Position = a_vPosition;	\n"
                    "	v_vColor = a_vColor;		\n"
                    "}								\n";

    char fShaderStr[] =
            "precision mediump float;		        \n"
                    "varying vec4 v_vColor;			\n"
                    "void main()					\n"
                    "{								\n"
                    "	gl_FragColor = v_vColor;	\n"
                    "}								\n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    vertexShader = LoadShader(vShaderStr, GL_VERTEX_SHADER);
    fragmentShader = LoadShader(fShaderStr, GL_FRAGMENT_SHADER);

    engine->programObject = glCreateProgram();

    if (engine->programObject == 0) return -1;

    glAttachShader(engine->programObject, vertexShader);
    glAttachShader(engine->programObject, fragmentShader);

    glBindAttribLocation(engine->programObject, POSITION_PARAMETER_INDEX, "a_vPosition");
    glBindAttribLocation(engine->programObject, COLOR_PARAMETER_INDEX, "a_vColor");

    glLinkProgram(engine->programObject);

    glGetProgramiv(engine->programObject, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint info_length = 0;
        glGetProgramiv(engine->programObject, GL_INFO_LOG_LENGTH, &info_length);

        if (info_length > 1)
        {
            char* info_log = new char[info_length];
            glGetProgramInfoLog(engine->programObject, info_length, NULL, info_log);
            LOGW("ERROR: Failure While Linking: %s\n", info_log);

            delete [] info_log;
        }

        glDeleteProgram(engine->programObject);
        return -1;
    }

    engine_init_blocks(engine);

    return 0;
}

static void engine_update_frame(struct engine* engine)
{
    if (engine->touchIsDown)
    {
        if (engine->touchX < 0.15f && engine->touchY < 0.2f)
        {
            engine->playerX -= 0.015f;

            if (engine->playerX < PADDLE_LEFT_BOUND)
            {
                engine->playerX = PADDLE_LEFT_BOUND;
            }
        }
        else if (engine->touchX > 0.85f && engine->touchY < 0.2f)
        {
            engine->playerX += 0.015f;

            if (engine->playerX > PADDLE_RIGHT_BOUND)
            {
                engine->playerX = PADDLE_RIGHT_BOUND;
            }
        }
    }

    engine->ballX += engine->ballVelocityX;

    if (engine->ballX < BALL_LEFT_BOUND || engine->ballX > BALL_RIGHT_BOUND)
    {
        engine->ballVelocityX = -engine->ballVelocityX;
    }

    engine->ballX += engine->ballVelocityY;

    if (engine->ballY > BALL_TOP_BOUND)
    {
        engine->ballVelocityX = -engine->ballVelocityY;
    }

    if (engine->ballY < BALL_BOTTOM_BOUND)
    {
        if (engine->ballVelocityY < 0.0f)
        {
            engine->ballVelocityY = -engine->ballVelocityY;
        }

        engine->ballX = BALL_START_X;
        engine->ballY = BALL_START_Y;

        engine_init_blocks(engine);
    }

    float ballXPlusVelocity = engine->ballX + engine->ballVelocityX;
    float ballYPlusVelocity = engine->ballY + engine->ballVelocityY;

    const float ballLeft     = ballXPlusVelocity - BALL_HALF_WIDTH;
    const float ballRight    = ballXPlusVelocity + BALL_HALF_WIDTH;
    const float ballTop      = ballYPlusVelocity + BALL_HALF_HEIGHT;
    const float ballBottom   = ballYPlusVelocity - BALL_HALF_HEIGHT;
    const float paddleLeft   = engine->playerX - PADDLE_HALF_WIDTH;
    const float paddleRight  = engine->playerX + PADDLE_HALF_WIDTH;
    const float paddleTop    = engine->playerY + PADDLE_HALF_HEIGHT;
    const float paddleBottom = engine->playerY - PADDLE_HALF_HEIGHT;

    if (!((ballRight < paddleLeft) || (ballLeft > paddleRight) || (ballBottom > paddleTop) || (ballTop < paddleBottom)))
    {
        if (engine->ballVelocityY < 0.0f)
        {
            engine->ballVelocityY = -engine->ballVelocityY;
        }
    }

    bool anyBlockActive = false;

    for (int32_t i = 0; i < NUM_BLOCKS; ++i)
    {
        block& currentBlock = engine->blocks[i];

        if (currentBlock.isActive)
        {
            const float blockLeft   = currentBlock.x - BLOCK_HALF_WIDTH;
            const float blockRight  = currentBlock.x + BLOCK_HALF_WIDTH;
            const float blockTop    = currentBlock.y + BLOCK_HALF_HEIGHT;
            const float blockBottom = currentBlock.y - BLOCK_HALF_HEIGHT;

            if (!((ballRight < blockLeft) || (ballLeft > blockRight) || (ballRight < blockLeft) || (ballTop < blockBottom) || (ballBottom > blockTop)))
            {
                if (ballLeft < blockLeft || ballRight > blockRight)
                {
                    engine->ballVelocityX = -engine->ballVelocityX;
                }

                currentBlock.isActive = false;
            }
            anyBlockActive = true;
        }
    }

    if (!anyBlockActive)
    {
        engine_init_blocks(engine);
    }
}

static void engine_draw_frame(struct engine* engine)
{
    if (engine->display == NULL) return;

    glViewport(0, 0, static_cast<int32_t>(engine->width), static_cast<int32_t>(engine->height));

    glClearColor(0.95f, 0.95f, 0.95f, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(engine->programObject);

    glEnableVertexAttribArray(POSITION_PARAMETER_INDEX);
    glEnableVertexAttribArray(COLOR_PARAMETER_INDEX);

    const float z = 0.0f;
    const float buttonColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };

    GLfloat leftButton[] = {
            -0.85f, 0.75f, z,
            buttonColor[0], buttonColor[1], buttonColor[2], buttonColor[3],
            -0.9f, 0.8f, z,
            buttonColor[0], buttonColor[1], buttonColor[2], buttonColor[3],
            -0.85f, 0.85f, z,
            buttonColor[0], buttonColor[1], buttonColor[2], buttonColor[3]
    };

    glVertexAttribPointer(POSITION_PARAMETER_INDEX, POSITION_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, leftButton);
    glVertexAttribPointer(COLOR_PARAMETER_INDEX, COLOR_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, &leftButton[3]);
    glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_NUM_VERTICES);

    GLfloat rightButton[] = {
            0.85f, 0.75f, z,
            buttonColor[0], buttonColor[1], buttonColor[2], buttonColor[3],
            0.9f, 0.8f, z,
            buttonColor[0], buttonColor[1], buttonColor[2], buttonColor[3],
            0.85f, 0.85f, z,
            buttonColor[0], buttonColor[1], buttonColor[2], buttonColor[3],
    };

    glVertexAttribPointer(POSITION_PARAMETER_INDEX, POSITION_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, rightButton);
    glVertexAttribPointer(COLOR_PARAMETER_INDEX, COLOR_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, &rightButton[3]);
    glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_NUM_VERTICES);

    float left = engine->playerX - PADDLE_HALF_WIDTH;
    float right = engine->playerX + PADDLE_HALF_WIDTH;
    float top = engine->playerY - PADDLE_HALF_HEIGHT;
    float bottom = engine->playerY + PADDLE_HALF_HEIGHT;
    const float paddleColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };

    GLfloat paddle[] = {
            left, top, z,
            paddleColor[0], paddleColor[1], paddleColor[2], paddleColor[3],
            left, bottom, z,
            paddleColor[0], paddleColor[1], paddleColor[2], paddleColor[3],
            right, top, z,
            paddleColor[0], paddleColor[1], paddleColor[2], paddleColor[3],
            right, top, z,
            paddleColor[0], paddleColor[1], paddleColor[2], paddleColor[3],
            left, bottom, z,
            paddleColor[0], paddleColor[1], paddleColor[2], paddleColor[3],
            right, bottom, z,
            paddleColor[0], paddleColor[1], paddleColor[2], paddleColor[3]
    };

    glVertexAttribPointer(POSITION_PARAMETER_INDEX, POSITION_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, paddle);
    glVertexAttribPointer(COLOR_PARAMETER_INDEX, COLOR_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, &paddle[3]);
    glDrawArrays(GL_TRIANGLES, 0, QUAD_NUM_VERTICES);

    left = engine->ballX - BALL_HALF_WIDTH;
    right = engine->ballX + BALL_HALF_WIDTH;
    top = engine->ballY + BLOCK_HALF_HEIGHT;
    bottom = engine->ballY - BLOCK_HALF_HEIGHT;
    const float ballColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };

    GLfloat ball[] = {
            left, top, z,
            ballColor[0], ballColor[1], ballColor[2], ballColor[3],
            left, bottom, z,
            ballColor[0], ballColor[1], ballColor[2], ballColor[3],
            right, top, z,
            ballColor[0], ballColor[1], ballColor[2], ballColor[3],
            right, top, z,
            ballColor[0], ballColor[1], ballColor[2], ballColor[3],
            left, bottom, z,
            ballColor[0], ballColor[1], ballColor[2], ballColor[3],
            right, bottom, z,
            ballColor[0], ballColor[1], ballColor[2], ballColor[3]
    };

    glVertexAttribPointer(POSITION_PARAMETER_INDEX, POSITION_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, ball);
    glVertexAttribPointer(COLOR_PARAMETER_INDEX, COLOR_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, &ball[3]);
    glDrawArrays(GL_TRIANGLES, 0, QUAD_NUM_VERTICES);

    GLfloat blockColors[][4] = { { 0.0f, 1.0f, 0.0f, 1.0f } , { 0.0f, 0.0f, 1.0f, 1.0f } };

    for (int32_t i = 0; i < NUM_BLOCKS; ++i)
    {
        block& currentBlock = engine->blocks[i];

        if (currentBlock.isActive)
        {
            const int32_t colorIndex = i % 2;
            const float r = blockColors[colorIndex][0];
            const float g = blockColors[colorIndex][1];
            const float b = blockColors[colorIndex][2];
            const float a = blockColors[colorIndex][3];
            const float left = currentBlock.x - BLOCK_HALF_WIDTH;
            const float right = currentBlock.x - BLOCK_HALF_WIDTH;
            const float top = currentBlock.y + BLOCK_HALF_HEIGHT;
            const float bottom = currentBlock.y + BLOCK_HALF_HEIGHT;

            GLfloat block[] = {
                    left, top, z,
                    r, g, b, a,
                    right, bottom, z,
                    r, g, b, a,
                    right, bottom, z,
                    r, g, b, a,
                    left, bottom, z,
                    r, g, b, a,
                    right, bottom, z,
                    r, g, b, a,
            };

            glVertexAttribPointer(POSITION_PARAMETER_INDEX, POSITION_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, block);
            glVertexAttribPointer(COLOR_PARAMETER_INDEX, COLOR_NUM_ELEMENTS, GL_FLOAT, GL_FALSE, VERTEX_SIZE, &block[3]);
            glDrawArrays(GL_TRIANGLES, 0, QUAD_NUM_VERTICES);
        }
    }

    glDisableVertexAttribArray(POSITION_PARAMETER_INDEX);
    glDisableVertexAttribArray(COLOR_PARAMETER_INDEX);

    eglSwapBuffers(engine->display, engine->surface);
}

static void engine_term_display(struct engine* engine)
{
    if (engine->display != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (engine->context != EGL_NO_CONTEXT)
        {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }

    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    struct engine* engine = (struct engine*)app->userData;

    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
        int32_t ret = 0;

        int32_t action = AMotionEvent_getAction(event);

        if (action == AMOTION_EVENT_ACTION_DOWN)
        {
            engine->touchIsDown = true;
            ret = 1;
        }
        else if (action == AMOTION_EVENT_ACTION_UP)
        {
            engine->touchIsDown = false;
            ret = 1;
        }

        if (ret)
        {
            engine->touchX = static_cast<float>(AMotionEvent_getRawX(event, 0)) / engine->width;
            engine->touchY = static_cast<float>(AMotionEvent_getRawY(event, 0)) / engine->height;
        }

        return ret;
    }

    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    struct engine* engine = static_cast<struct engine*>(app->userData);

    switch (cmd)
    {
        case APP_CMD_INIT_WINDOW:
        {
            if (engine->app->window != NULL)
            {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
        } break;
        case APP_CMD_TERM_WINDOW:
        {
            engine_term_display(engine);
        } break;
    }
}

void android_main(struct android_app* state)
{
    struct engine engine;

    app_dummy();

    memset(&engine, 0, sizeof(engine));

    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    engine.app = state;

    while (1)
    {
        int i_dent;
        int events;
        struct android_poll_source* source;

        while ((i_dent = ALooper_pollAll(0, NULL, &events, reinterpret_cast<void**>(&source))) >= 0)
        {
            if (source != NULL)
            {
                source->process(state, source);
            }

            if (state->destroyRequested != 0)
            {
                engine_term_display(&engine);
                return;
            }
        }

        engine_update_frame(&engine);
        engine_draw_frame(&engine);
    }
}