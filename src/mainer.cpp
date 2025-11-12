#ifdef _WIN32
    #ifdef  _MSC_VER
        #define _CRT_SECURE_NO_DEPRECATE 1
        #pragma comment(lib,"SDL2.lib")
        #pragma comment(lib,"vulkan-1.lib")
        #pragma comment(lib,"SDL2main.lib")
        #pragma comment(lib,"OpenGl32.lib")
        #pragma comment(lib,"openal32.lib")
        #pragma comment(lib,"Ws2_32.lib") 
        #pragma comment(lib,"Crypt32.lib")
        #pragma comment(lib,"Wldap32.lib")
        #pragma comment(lib,"Normaliz.lib")
        
        #ifdef _DEBUG
            #pragma comment(lib,"libogg_d.lib")
            #pragma comment(lib,"libvorbis_d.lib")
            #pragma comment(lib,"libvorbisfile_d.lib")
        #else
            #pragma comment(lib,"libogg.lib")
            #pragma comment(lib,"libvorbis.lib")
            #pragma comment(lib,"libvorbisfile.lib")
        #endif
    #endif
#endif



#include <ctime>
#include "Game.h"
#include "SDLVideo.h"
#include "OSTools.h"
#ifdef __APPLE__
#include <limits.h>
#include <unistd.h>
#include <CoreFoundation/CoreFoundation.h>
#endif
#include <string>
#include <thread>


SDLVideo SDL;
SDL_Joystick *Joy = 0;
SDL_GameControllerType controllerType = SDL_CONTROLLER_TYPE_UNKNOWN;
SDL_GameController* gamepad  = 0;

int JoyX = 0;
int JoyY = 0;
int MouseX, MouseY; //relative mouse coords
int _MouseX, _MouseY;
unsigned long tick;

static std::string buffer;

const char* GamePadTypes[] = {"Unknown", "XBOX 360", "XBOX One", "Playstation 3",
                              "Playstation 4", "Nintendo Switch PRO", "Virtual",
                              "Playstation 5", "Amazon Luna", "Google Stadia",
                              "NVIDIA Shield", "Nintendo Switch Joycon LEFT",
                              "Nintendo Switch Joycon RIGHT", "Nintendo Switch Joycon PAIR"};

Game game;

void ConfigureGraphicsLib(bool useVulkan)
{
    game.init(useVulkan);
}
//-----------------
void RenderScreen(bool useVulkan)
{
    if (useVulkan)
    {
        SDL.VkAcquireNextImage();
        SDL.VkResetCommandBuffer();
        SDL.VkBeginCommandBuffer();
    }

    game.renderToFBO(useVulkan);

    if (useVulkan)
    {
        SDL.VkBeginRenderPass({0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0});
    }

    game.renderFBO(useVulkan);

    SDL.swap(useVulkan);
}
//-----------------
void Logic()
{
    game.logic();
}
//-----------------
static void  process_events(){
    
    SDL_Event event;
    float scaleX = 1.f;
    float scaleY = 1.f;

    while( SDL_PollEvent( &event ) ) 
    {

        switch( event.type ) 
        {

        case SDL_TEXTINPUT:
        {
            strcpy(game.EditText, event.text.text);
        } break;

        case SDL_KEYUP:
        {
            game.globalKEY = 0;
            game.globalKeyUp = (char)event.key.keysym.scancode;
        } break;

        case SDL_KEYDOWN:{

            game.globalKEY = (char)event.key.keysym.scancode;
            switch( event.key.keysym.sym ) 
            {
                default:{} break;
                case SDLK_F1: {++game.DebugMode; if (game.DebugMode > 1) game.DebugMode = 0;} 
            }
        } break;
        case SDL_MOUSEBUTTONUP:{
            Vector3D pos(event.button.x * scaleX, event.button.y * scaleY, 0);
            //printf("up x:%f y:%f\n", pos.x() , pos.y());
            game.touches.up.add(pos);
            game.touches.allfingersup = true;
        } break;
        case SDL_MOUSEBUTTONDOWN:{
            Vector3D pos(event.button.x * scaleX, event.button.y * scaleY, 0);
            //printf("down x:%f y:%f\n", pos.x() , pos.y());
            game.touches.down.add(pos);
            game.touches.allfingersup = false;

        } break;

        case SDL_MOUSEWHEEL:
        {
            game.Keys[7] = 1;
        } break;

        case SDL_MOUSEMOTION:{
            if(SDL_GetMouseState(0, 0)&SDL_BUTTON_LMASK){
                Vector3D pos(event.button.x * scaleX, event.button.y * scaleY, 0);
                //printf("motion x:%f y:%f\n", pos.x() , pos.y());
                game.touches.move.add(pos);
                game.touches.allfingersup = false;
            }
        }break;


        case SDL_QUIT:{
            game.Works = false;
        }break;
    
        }

    }
}
//--------------------
void CheckKeys()
{
    const Uint8* keys = SDL_GetKeyboardState(NULL);

    SDL_GetRelativeMouseState ( &MouseX, &MouseY );
    SDL_GetMouseState(&_MouseX, &_MouseY);


    game.RelativeMouseX = MouseX;
    game.RelativeMouseY = MouseY;

    game.MouseX = _MouseX;
    game.MouseY = _MouseY;


    memcpy(game.OldKeys, game.Keys, Game::GameKeyCount);
    memset(game.Keys, 0, Game::GameKeyCount);

    if ( keys[SDL_SCANCODE_W] )     game.Keys[0] = 1;
    if ( keys[SDL_SCANCODE_S] )     game.Keys[1] = 1;
    if ( keys[SDL_SCANCODE_A] )     game.Keys[3] = 1;
    if ( keys[SDL_SCANCODE_D] )     game.Keys[2] = 1;

    if ( keys[SDL_SCANCODE_UP] )    game.Keys[0] = 1;
    if ( keys[SDL_SCANCODE_DOWN])   game.Keys[1] = 1;
    if ( keys[SDL_SCANCODE_LEFT])   game.Keys[3] = 1;
    if ( keys[SDL_SCANCODE_RIGHT])  game.Keys[2] = 1;

    if ( keys[SDL_SCANCODE_SPACE])  game.Keys[4] = 1;
    if ( keys[SDL_SCANCODE_RETURN]) game.Keys[4] = 1;
    if ( keys[SDL_SCANCODE_ESCAPE]) game.Keys[5] = 1;
    if ( keys[SDL_SCANCODE_LCTRL])  game.Keys[6] = 1;
    if ( keys[SDL_SCANCODE_TAB])    game.Keys[8] = 1;
    if ( keys[SDL_SCANCODE_I])      game.Keys[9] = 1;


    if (gamepad)
    {
        const int DEADZONE = 10;

        SDL_GameControllerUpdate();

        game.gamepadLAxis.x = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX) / 1000;

        if (game.gamepadLAxis.x > 0 && game.gamepadLAxis.x < DEADZONE)
        {
            game.gamepadLAxis.x = 0;
        }

        if (game.gamepadLAxis.x < 0 && game.gamepadLAxis.x > -DEADZONE)
        {
            game.gamepadLAxis.x = 0;
        }


        game.gamepadLAxis.y = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY) / 1000;

        if (game.gamepadLAxis.y > 0 && game.gamepadLAxis.y < DEADZONE)
        {
            game.gamepadLAxis.y = 0;
        }

        if (game.gamepadLAxis.y < 0 && game.gamepadLAxis.y > -DEADZONE)
        {
            game.gamepadLAxis.y = 0;
        }


        game.gamepadRAxis.x = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) / 1000;

        if (game.gamepadRAxis.x > 0 && game.gamepadRAxis.x < DEADZONE)
        {
            game.gamepadRAxis.x = 0;
        }

        if (game.gamepadRAxis.x < 0 && game.gamepadRAxis.x > -DEADZONE)
        {
            game.gamepadRAxis.x = 0;
        }



        game.gamepadRAxis.y = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) / 1000;

        if (game.gamepadRAxis.y > 0 && game.gamepadRAxis.y < DEADZONE)
        {
            game.gamepadRAxis.y = 0;
        }

        if (game.gamepadRAxis.y < 0 && game.gamepadRAxis.y > -DEADZONE)
        {
            game.gamepadRAxis.y = 0;
        }


        if (game.doRumble)
        {
            SDL_GameControllerRumble(gamepad, 0x8888, 0x8888, 250);
            game.doRumble = false;
        }

        if (SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 1000 > 0) game.Keys[6] = 1;
        if (SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 1000 > 0) game.Keys[6] = 1;

        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_A))             game.Keys[4] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_B))             game.Keys[5] = 1;

        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))  game.Keys[7] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) game.Keys[7] = 1;

        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP))       game.Keys[0] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN))     game.Keys[1] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))    game.Keys[2] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT))     game.Keys[3] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_START))         game.Keys[8] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_X))             game.Keys[9] = 1;

    }
}

//--------------------
int main(int argc, char* argv[])
{

    if (argc)
    {
        printf("arg: %s/n", argv[0]);
    }

    srand(time(0));

    char buf[128];
    GetHomePath(buf);
    sprintf(game.DocumentPath, "%s.warezdude3", buf);
    MakeDir(game.DocumentPath);
    game.loadConfig();

    printf("%d %d\n", game.ScreenWidth, game.ScreenHeight);
    SDL.setMetrics(game.ScreenWidth, game.ScreenHeight);


    const char* title = "WD40";

    const bool USE_VULKAN = (bool)game.renderer;

    if (!SDL.initWindow(title, "icon1.bmp", game.windowed, USE_VULKAN))
    {
        game.Works = false;
    }

    game.vulkanDevice     = SDL.getVkDevice();
    game.vkPhysicalDevice = SDL.getVKPhysicalDevice();
    game.vkCmd            = SDL.getVkCmd();
    game.vkRenderPass     = SDL.getVkRenderPass();
    game.vkCommandPool    = SDL.getVkCommandPool();
    game.vkGraphicsQueue  = SDL.getVkGraphicsQueue();
    game.vkSwapChainImageCount = SDL.getVkSwapChainImageCount();

    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    if(SDL_NumJoysticks() > 0)
    {
        Joy = SDL_JoystickOpen(0);

        if (SDL_IsGameController(0))
        {
            controllerType = SDL_GameControllerTypeForIndex(0);
            gamepad = SDL_GameControllerOpen(0);
            printf("game controller type: %s\n", GamePadTypes[controllerType]);
        }
        else
        {
            printf("Generic DirectX Joystick\n");
        }
    }

    ConfigureGraphicsLib(USE_VULKAN);

    game.TimeTicks = SDL_GetTicks();

    SDL_ShowCursor(false);


    while (game.Works)
    {
        if ((SDL_GetTicks() > tick))
        {

            game.DeltaTime = (SDL_GetTicks() - game.TimeTicks) / 1000.0f;
            game.TimeTicks = SDL_GetTicks();

            game.Accumulator += game.DeltaTime;

            while (game.Accumulator >= game.DT)
            {
                Logic();
                game.Accumulator -= game.DT;
            }

            CheckKeys();
            RenderScreen(USE_VULKAN);

            tick = SDL_GetTicks() + 1000 / 61;
        }

        game.network();

        SDL_Delay(0.6);

        process_events();

    }
    printf("QUITING!\n");

    game.destroy();



    SDL.quit(USE_VULKAN);

    return 0;
}

