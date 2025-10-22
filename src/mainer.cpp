#ifdef _WIN32
    #ifdef  _MSC_VER
        #define _CRT_SECURE_NO_DEPRECATE 1
        #pragma comment(lib,"SDL2.lib")
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

Game Game;


void ConfigureGraphicsLib(bool useVulkan)
{
    Game.init(useVulkan);
}
//-----------------
void RenderScreen(bool useVulkan)
{
    SDL.beginRenderPass(useVulkan);

    Game.render(useVulkan);

    SDL.swap(useVulkan);
}
//-----------------
void Logic()
{
    Game.logic();
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
            strcpy(Game.EditText, event.text.text);
        } break;

        case SDL_KEYUP:
        {
            Game.globalKEY = 0;
            Game.globalKeyUp = (char)event.key.keysym.scancode;
        } break;

        case SDL_KEYDOWN:{

            Game.globalKEY = (char)event.key.keysym.scancode;
            switch( event.key.keysym.sym ) 
            {
                default:{} break;
                case SDLK_F1: {++Game.DebugMode; if (Game.DebugMode > 1) Game.DebugMode = 0;} 
            }
        } break;
        case SDL_MOUSEBUTTONUP:{
            Vector3D pos(event.button.x * scaleX, event.button.y * scaleY, 0);
            //printf("up x:%f y:%f\n", pos.x() , pos.y());
            Game.touches.up.add(pos);
            Game.touches.allfingersup = true;
        } break;
        case SDL_MOUSEBUTTONDOWN:{
            Vector3D pos(event.button.x * scaleX, event.button.y * scaleY, 0);
            //printf("down x:%f y:%f\n", pos.x() , pos.y());
            Game.touches.down.add(pos);
            Game.touches.allfingersup = false;

        } break;

        case SDL_MOUSEWHEEL:
        {
            Game.Keys[7] = 1;
        } break;

        case SDL_MOUSEMOTION:{
            if(SDL_GetMouseState(0, 0)&SDL_BUTTON_LMASK){
                Vector3D pos(event.button.x * scaleX, event.button.y * scaleY, 0);
                //printf("motion x:%f y:%f\n", pos.x() , pos.y());
                Game.touches.move.add(pos);
                Game.touches.allfingersup = false;
            }
        }break;


        case SDL_QUIT:{
            Game.Works = false;
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


    Game.RelativeMouseX = MouseX;
    Game.RelativeMouseY = MouseY;

    Game.MouseX = _MouseX;
    Game.MouseY = _MouseY;


    memcpy(Game.OldKeys, Game.Keys, Game::GameKeyCount);
    memset(Game.Keys, 0, Game::GameKeyCount);

    if ( keys[SDL_SCANCODE_W] )     Game.Keys[0] = 1;
    if ( keys[SDL_SCANCODE_S] )     Game.Keys[1] = 1;
    if ( keys[SDL_SCANCODE_A] )     Game.Keys[3] = 1;
    if ( keys[SDL_SCANCODE_D] )     Game.Keys[2] = 1;

    if ( keys[SDL_SCANCODE_UP] )    Game.Keys[0] = 1;
    if ( keys[SDL_SCANCODE_DOWN])   Game.Keys[1] = 1;
    if ( keys[SDL_SCANCODE_LEFT])   Game.Keys[3] = 1;
    if ( keys[SDL_SCANCODE_RIGHT])  Game.Keys[2] = 1;

    if ( keys[SDL_SCANCODE_SPACE])  Game.Keys[4] = 1;
    if ( keys[SDL_SCANCODE_RETURN]) Game.Keys[4] = 1;
    if ( keys[SDL_SCANCODE_ESCAPE]) Game.Keys[5] = 1;
    if ( keys[SDL_SCANCODE_LCTRL])  Game.Keys[6] = 1;
    if ( keys[SDL_SCANCODE_TAB])    Game.Keys[8] = 1;
    if ( keys[SDL_SCANCODE_I])      Game.Keys[9] = 1;


    if (gamepad)
    {
        const int DEADZONE = 10;

        SDL_GameControllerUpdate();

        Game.gamepadLAxis.x = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX) / 1000;

        if (Game.gamepadLAxis.x > 0 && Game.gamepadLAxis.x < DEADZONE)
        {
            Game.gamepadLAxis.x = 0;
        }

        if (Game.gamepadLAxis.x < 0 && Game.gamepadLAxis.x > -DEADZONE)
        {
            Game.gamepadLAxis.x = 0;
        }


        Game.gamepadLAxis.y = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY) / 1000;

        if (Game.gamepadLAxis.y > 0 && Game.gamepadLAxis.y < DEADZONE)
        {
            Game.gamepadLAxis.y = 0;
        }

        if (Game.gamepadLAxis.y < 0 && Game.gamepadLAxis.y > -DEADZONE)
        {
            Game.gamepadLAxis.y = 0;
        }


        Game.gamepadRAxis.x = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) / 1000;

        if (Game.gamepadRAxis.x > 0 && Game.gamepadRAxis.x < DEADZONE)
        {
            Game.gamepadRAxis.x = 0;
        }

        if (Game.gamepadRAxis.x < 0 && Game.gamepadRAxis.x > -DEADZONE)
        {
            Game.gamepadRAxis.x = 0;
        }



        Game.gamepadRAxis.y = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) / 1000;

        if (Game.gamepadRAxis.y > 0 && Game.gamepadRAxis.y < DEADZONE)
        {
            Game.gamepadRAxis.y = 0;
        }

        if (Game.gamepadRAxis.y < 0 && Game.gamepadRAxis.y > -DEADZONE)
        {
            Game.gamepadRAxis.y = 0;
        }


        if (Game.doRumble)
        {
            SDL_GameControllerRumble(gamepad, 0x8888, 0x8888, 250);
            Game.doRumble = false;
        }

        if (SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 1000 > 0) Game.Keys[6] = 1;
        if (SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 1000 > 0) Game.Keys[6] = 1;

        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_A))             Game.Keys[4] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_B))             Game.Keys[5] = 1;

        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER))  Game.Keys[7] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) Game.Keys[7] = 1;

        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_UP))       Game.Keys[0] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_DOWN))     Game.Keys[1] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT))    Game.Keys[2] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_DPAD_LEFT))     Game.Keys[3] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_START))         Game.Keys[8] = 1;
        if (SDL_GameControllerGetButton(gamepad, SDL_CONTROLLER_BUTTON_X))             Game.Keys[9] = 1;

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
    sprintf(Game.DocumentPath, "%s.warezdude3", buf);
    MakeDir(Game.DocumentPath);
    Game.loadConfig();

    printf("%d %d\n", Game.ScreenWidth, Game.ScreenHeight);
    SDL.setMetrics(Game.ScreenWidth, Game.ScreenHeight);


    const char* title = "WD40";

    const bool USE_VULKAN = true;

    if (!SDL.initWindow(title, "icon1.bmp", Game.windowed, USE_VULKAN))
    {
        Game.Works = false;
    }

    Game.vulkanDevice     = SDL.getVkDevice();
    Game.vkPhysicalDevice = SDL.getVKPhysicalDevice();
    Game.vkCmd            = SDL.getVkCmd();
    Game.vkRenderPass     = SDL.getVkRenderPass();
    Game.vkCommandPool    = SDL.getVkCommandPool();
    Game.vkGraphicsQueue  = SDL.getVkGraphicsQueue();

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

    Game.TimeTicks = SDL_GetTicks();
    

    SDL_ShowCursor(false);


    while (Game.Works)
    {
        if ((SDL_GetTicks() > tick))
        {

            Game.DeltaTime = (SDL_GetTicks() - Game.TimeTicks) / 1000.0f;
            Game.TimeTicks = SDL_GetTicks();

            Game.Accumulator += Game.DeltaTime;

            while (Game.Accumulator >= Game.DT)
            {
                Logic();
                Game.Accumulator -= Game.DT;
            }

            CheckKeys();
            RenderScreen(USE_VULKAN);

            tick = SDL_GetTicks() + 1000 / 61;
        }

        Game.network();

        SDL_Delay(0.6);

        process_events();

    }
    printf("QUITING!\n");

    Game.destroy();



    SDL.quit(USE_VULKAN);

    return 0;
}

