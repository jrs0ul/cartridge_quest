#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#include "Game.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))



struct engine {
    struct android_app* app{};


    int animating{};
    EGLDisplay display{};
    EGLSurface surface{};
    EGLContext context{};
    EGLConfig config{};
    Vector3D oldDown;
    int32_t width{};
    int32_t height{};
    int32_t fingersOnscreen{};
    Game* game{};
    bool loaded{};
    bool resetMovement{};
    bool backPressed{};

};


//-------------------------------------------
long getTicks() {
    struct timespec now{};
    clock_gettime(CLOCK_MONOTONIC, &now);
    long msecs = now.tv_nsec / 1000000;
    return (long)now.tv_sec * 1000 + msecs;
}
//-------------------------------------------------
static int engine_init_display(struct engine* engine) {

    LOGI("initializing the display\n");
    const EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
    EGLDisplay display;

    if (!engine->loaded) {
        LOGI("Let's make a new display\n");
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(display, 0, 0);
    }
    else{
        LOGI("Let's use an existing display\n");
        display = engine->display;
    }
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    if (!engine->loaded) {
        LOGI("Let's create a new context\n");
        EGLint AttribList[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_NONE
        };
        context = eglCreateContext(display, config, NULL, AttribList);

    }
    else {
        LOGI("Let's use an existing context\n");
        context = engine->context;
    }

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    if (!engine->loaded) {
        eglQuerySurface(display, surface, EGL_WIDTH, &w);
        eglQuerySurface(display, surface, EGL_HEIGHT, &h);
        engine->width = w;
        engine->height = h;
    }

    engine->display = display;
    engine->context = context;
    engine->surface = surface;



    glDisable(GL_DEPTH_TEST);


    if (!engine->loaded) {
        if (engine->game) {
            engine->game->loadConfig();
            auto* sys = engine->game->getSysConfig();
            if (sys->ScreenWidth * sys->screenScaleX > engine->width)
            {
                sys->screenScaleX = engine->width / sys->ScreenWidth;
                sys->screenScaleY = sys->screenScaleX;
                engine->game->ScreenHeight = sys->ScreenHeight * sys->screenScaleY;
                engine->game->ScreenWidth = sys->ScreenWidth * sys->screenScaleX;

            }


            engine->game->init(false);
            engine->game->TimeTicks = getTicks();
            engine->loaded = true;
            memset(engine->game->Keys, 0, Game::GameKeyCount);
        }
    }

    engine->animating = 1;

    return 0;
}
//-----------------------------------------------------
//
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        return;
    }

    if (engine->game) {
        if (getTicks() > engine->game->tick) {

            engine->game->DeltaTime = (getTicks() - engine->game->TimeTicks) / 1000.0f;
            engine->game->TimeTicks = getTicks();

            memcpy(engine->game->OldKeys, engine->game->Keys, Game::GameKeyCount);
            memset(engine->game->Keys, 0, Game::GameKeyCount);

            if (engine->backPressed)
            {
                engine->backPressed = false;
                engine->game->Keys[5] = 1;
            }

            const float widthFactor = (float)engine->game->ScreenWidth / (float)engine->game->getSysConfig()->ScreenWidth;
            const float heightFactor = (float)engine->game->ScreenHeight / (float)engine->game->getSysConfig()->ScreenHeight;

            if (engine->game->touches.allfingersup)
            {
                engine->game->gamepadLAxis.x = 0;
                engine->game->gamepadLAxis.y = 0;
                engine->resetMovement = true;
            }

            if (!engine->game->touches.down.empty())
            {
                if (engine->resetMovement)
                {
                    engine->oldDown = engine->game->touches.down[0];
                    engine->resetMovement = false;
                }
              /*
                engine->game->MouseX = engine->game->touches.down[mouseIdx].x / widthFactor;
                engine->game->MouseY = engine->game->touches.down[mouseIdx].y / heightFactor;*/
            }

            if (!engine->game->touches.move.empty()) {

                Vector3D diff = engine->game->touches.move[0] - engine->oldDown;

                engine->game->gamepadLAxis.x = diff.x;
                engine->game->gamepadLAxis.y = diff.y;
                engine->game->gamepadRAxis.x = diff.x;
                engine->game->gamepadRAxis.y = diff.y;

                
                diff.normalize();
                if (diff.y > 0.25f)
                {
                    engine->game->Keys[0] = 1;
                }

                if (diff.y < -0.25f)
                {
                    engine->game->Keys[1] = 1;
                }

                if (diff.x < -0.25f)
                {
                    engine->game->Keys[2] = 1;
                }

                if (diff.y > 0.25f)
                {
                    engine->game->Keys[3] = 1;
                }

/*
                engine->game->MouseX = engine->game->touches.move[mouseIdx].x / widthFactor;
                engine->game->MouseY = engine->game->touches.move[mouseIdx].y / heightFactor;*/
            }

            engine->game->Accumulator += engine->game->DeltaTime;


            while (engine->game->Accumulator >= engine->game->DT)
            {


                engine->game->logic();
                engine->game->Accumulator -= engine->game->DT;
            }


            engine->game->renderToFBO(false);
            engine->game->renderFBO(false);

            eglSwapBuffers(engine->display, engine->surface);

            engine->game->tick = getTicks() + 1000/70;
        }

    }


}
//-------------------------------------------------------
/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    //if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        //if (engine->context != EGL_NO_CONTEXT) {
        //    eglDestroyContext(engine->display, engine->context);
        //}
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        //eglTerminate(engine->display);


    //}
    //engine->display = EGL_NO_DISPLAY;
    //engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
    engine->animating = 0;

}
//-------------------------------------
/**
 * Process input.
 */
static int32_t engine_handle_input(struct android_app* app) {
    auto* engine = (struct engine*)app->userData;

    auto ib = android_app_swap_input_buffers(app);
    if (ib && ib->motionEventsCount)
    {
        for (int i = 0; i < ib->motionEventsCount; i++) {
            auto *event = &ib->motionEvents[i];
            int32_t ptrIdx = 0;
            switch (event->action & AMOTION_EVENT_ACTION_MASK) {

                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                {
                    ptrIdx = (event->action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                            AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                    Vector3D v = Vector3D(GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_X),
                                          GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_Y),
                                          0);
                    engine->game->touches.down.push_back(v);
                } break;
                case AMOTION_EVENT_ACTION_POINTER_UP:
                {
                    ptrIdx = (event->action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                            AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                    Vector3D v = Vector3D(GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_X),
                                          GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_Y),
                                          0);
                    engine->game->touches.up.push_back(v);
                } break;


                case AMOTION_EVENT_ACTION_UP:
                {
                    engine->game->touches.allfingersup = true;

                    Vector3D v = Vector3D(GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_X),
                                          GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_Y),
                                          0);
                    engine->game->touches.up.push_back(v);
                }
                    break;
                case AMOTION_EVENT_ACTION_DOWN : {
                    engine->game->touches.allfingersup = false;
                    Vector3D v = Vector3D(GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_X),
                                          GameActivityPointerAxes_getAxisValue(
                                                  &event->pointers[ptrIdx], AMOTION_EVENT_AXIS_Y),
                                          0);
                    engine->game->touches.down.push_back(v);
                }
                    break;
                case AMOTION_EVENT_ACTION_MOVE: {

                    engine->game->touches.allfingersup = false;
                    for (int j = 0; j < event->pointerCount; ++j)
                    {
                        Vector3D v = Vector3D(GameActivityPointerAxes_getAxisValue(
                                                      &event->pointers[j], AMOTION_EVENT_AXIS_X),
                                              GameActivityPointerAxes_getAxisValue(
                                                      &event->pointers[j], AMOTION_EVENT_AXIS_Y),
                                              0);
                        engine->game->touches.move.push_back(v);
                    }

                }


            }
        }



        android_app_clear_motion_events(ib);
        return 1;
    }
    else if (ib && ib->keyEventsCount)
    {
        for (int i = 0; i < ib->keyEventsCount; i++)
        {
            auto *event = &ib->keyEvents[i];
            switch (event->keyCode)
            {
                case AKEYCODE_BACK:{
                    engine->backPressed = true;
                }
            }
        }
        android_app_clear_key_events(ib);
        return 1;
    }

    return 0;
}
//----------------------------------
/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    auto* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.

            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {

                engine->game->AssetManager = app->activity->assetManager;
                engine_init_display(engine);
                engine_draw_frame(engine);

            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS: engine->animating = 1; break;
        case APP_CMD_LOST_FOCUS: {
            engine->animating = 0;
            //engine->game->ss.stopMusic();
            engine_draw_frame(engine);
        }
            break;
        case APP_CMD_DESTROY:{

            LOGI("Let's destroy this app\n");
            if (engine->game) {
                engine->game->destroy();
            }

            if (engine->surface != EGL_NO_SURFACE) {
                LOGI("Let's destroy the surface\n");
                eglDestroySurface(engine->display, engine->surface);
            }

            if (engine->context != EGL_NO_CONTEXT) {
                LOGI("Let's destroy the context\n");
                eglDestroyContext(engine->display, engine->context);
            }



            eglTerminate(engine->display);

            delete engine->game;
        } break;
    }
}

//-------------------------------------------

void android_main(struct android_app* state) {
    struct engine engine;


    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    engine.app = state;

    android_app_set_key_event_filter(state, NULL);
    android_app_set_motion_event_filter(state, NULL);


    ((struct engine*)(state->userData))->game = new Game();
    ((struct engine*)(state->userData))->loaded = false;

    // loop waiting for stuff to do.
    while (true) {
        int events;
        struct android_poll_source* source;

        while ((ALooper_pollOnce(engine.animating ? 0 : -1, nullptr, &events,
                                      (void**)&source)) >= 0) {

            // Process this event.
            if (source) {
                source->process(source->app, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested) {
                engine_term_display(&engine);
                return;
            }
        }

        engine_handle_input(state);

        if (engine.animating) {
            engine_draw_frame(&engine);
        }
    }
}
