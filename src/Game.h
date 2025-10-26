#pragma once

#ifndef __ANDROID__
#include "audio/SoundSystem.h"
#include "audio/OggStream.h"
#else
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#include "audio/AndroidSoundSystem.h"
#endif
#include "ActionKeys.h"
#include "Vectors.h"
#include "TouchData.h"
#include "SysConfig.h"
#include "ShaderProgram.h"
#include "gui/SelectMenu.h"
#include "gui/Slider.h"
#include "gui/EditBox.h"
#include "Intro.h"
#include "network/Server.h"
#include "network/Client.h"
#include "BulletContainer.h"
#include "NetworkCommands.h"
#include "Inventory.h"
#include "Collection.h"
#include "GameData.h"
#include "map.h"
#include "maplist.h"






enum GameState
{
    GAMESTATE_TITLE,
    GAMESTATE_INTRO,
    GAMESTATE_HELP,
    GAMESTATE_GAME,
    GAMESTATE_ENDING
};

enum MultiplayerModes
{
    MPMODE_COOP,
    MPMODE_DEATHMATCH
};

enum NetworkingModes
{
    NETMODE_NONE,
    NETMODE_SERVER,
    NETMODE_CLIENT
};


struct ClientLoot
{
    DArray<int> cartridges;
};

class Game
{

    float   OrthoMatrix[16];
    float   ViewMatrix[16];

    ShaderProgram        defaultShader;
    ShaderProgram        colorShader;

    MapList mapai;
    CMap mapas;

    ScroollControl SfxVolumeC;
    ScroollControl MusicVolumeC;
    EditBox ipedit;

    Intro intro;

    DArray<int> loot;

    DArray<ClientLoot> clientLoot; // for server

    DArray<int> stash;
    Inventory inventory;

    Collection cartridgeCollection;

    GameData gameData;

    Client client;
    Server serveris;

    SystemConfig sys;
    OggStream music;
    CBulletContainer bulbox;
    SpriteBatcher pics;


    SelectMenu mainmenu;
    SelectMenu netmenu;
    SelectMenu netgame;
    SelectMenu options;
    SelectMenu mapmenu;

    unsigned int imgCount;
    unsigned int maxwavs;


    int frags; //frags of current player
    DArray<int> fragTable;
    int timeleft;
    bool showdebugtext;
    bool FirstTime;
    bool gameOver;
    bool noAmmo;
    int ms;


    int otherClientCount; //  This number is received from server, for server itself it is 0

    int itmtim;
    int fadeTimer;
    int slimeTimer;

    NetworkingModes netMode;

    int clientInfoSendCounter;

    int  clientMyIndex;
    int door_tim;
    int objectivetim;
    bool Client_GotMapData;
    bool showMiniMap;
    bool doFadein;
    bool godmode;
    bool slimeswap;


public:

    static const int GameKeyCount = 20;

    GameState state;
    MultiplayerModes netGameState;

    char EditText[255];
    DArray<int> clientIds; // how it is in server, info for the new client

    TouchData   touches;

    int DebugMode;
    Vector3D gamepadLAxis;
    Vector3D OldGamepadLAxis;

    Vector3D gamepadRAxis;
    Vector3D OldGamepadRAxis;

    bool doRumble;

    VkDevice         * vulkanDevice;
    VkPhysicalDevice * vkPhysicalDevice;
    VkRenderPass     * vkRenderPass;
    VkCommandBuffer  * vkCmd;
    VkCommandPool    * vkCommandPool;
    VkQueue          * vkGraphicsQueue;

    float MouseX;
    float MouseY;

    float OldMouseX;
    float OldMouseY;

    float RelativeMouseX;
    float RelativeMouseY;
    unsigned char globalKEY;
    unsigned char globalKeyUp;
    unsigned char OldKeys[GameKeyCount]; //  old game keys
    unsigned char Keys[GameKeyCount]; //  game keys
    char DocumentPath[255];
    bool Works;
    bool windowed;
    int renderer; //renderer index from the config file
    unsigned ScreenWidth;
    unsigned ScreenHeight;
    float TimeTicks;
    float DeltaTime;
    float DT;
    float Accumulator;

public:

    Game();

    void init(bool useVulkan);
    void render(bool useVulkan);
    void network();
    void logic();
    void destroy();
    void loadConfig();

private:
    void DrawTitleScreen();
    void DrawEndScreen();
    void DrawGameplay();
    void MoveDude();
    void SendWarpMessage();
    void ItemPickup();
    void SendItemCRemove(int itemIndex);
    void goToEnding();
    void GoToLevel(int currentHp, int currentAmmo, int level, int otherplayer);
    void SendItemSRemove(int ItemIndex, int clientIndex, bool playerTaked);
    //mapo pavadinimas, klientu skaicius
    void SendMapInfo(int clientIndex, CMap& map);
    //monster races amd item positions
    void SendMapData(int clientIndex, CMap& map);
    void InitServer();
    void StopServer();
    bool JoinServer(const char* ip, unsigned port);
    void QuitServer();
    void SendServerDoorState(unsigned int clientIndex, int doorx,int doory, unsigned char doorframe);
    void DoorsInteraction();
    void HandleInteractionsWithDeadPlayers();
    void CheckForExit();
    int  slimeReaction(int index);
    void SendServerEquipedCartridgeToAllClients(unsigned dudeIdx, int equipedGame);
    void SendClientEquipedCartridgeIdxToServer(unsigned index);
    void SendClientMeleeImpulseToServer(int victimID, int hp);
    void SendClientShootImpulseToServer(WeaponTypes weaponType);
    void SendClientCoords();
    void SendClientDoorState(int doorx,int doory, unsigned char doorframe);
    void SendResurrectMessageToClient(unsigned clientIdx, unsigned playerIdx);
    void SendResurrectMessageToServer();
    void SendGameOverMessageToClient(unsigned clientIndex);
    void AdaptMapView();
    void DrawMissionObjectives();
    void MonsterAI(int index);
    void BeatEnemy(int aID, int damage);
    void SendServerMeleeImpulseToClient(unsigned int clientIndex, int victim, int hp, int attacker);
    void SendBulletImpulse(int monsterindex, int ammo, int clientIndex, unsigned char weaponType);
    void GenerateTheMap(int level, int currentHp, int currentAmmo);
    void LoadTheMap(const char* name, bool createItems, int otherPlayers, int currentHp);
    void LoadFirstMap();
    void TitleMenuLogic();
    void HandleBullets();
    void AnimateSlime();
    void ResetVolume();
    bool InitAudio();
    void ParseMessagesServerGot();
    void ParseMessagesClientGot();
    void GetClientAtackImpulse(const unsigned char* buf, unsigned * index, int ClientIndex);
    bool OnHit(Bullet& bul);
    void DrawStats();
    void SendPlayerInfoToClient(int clientindex);
    void GetMapInfo(const unsigned char* bufer, int* index);
    void GetServerResurrectMsg(const unsigned char* buffer, int* index);
    void KillPlayer(int index);
    void GetAttackImpulseFromServer(const unsigned char* buf, int* index);
    void SendItemCreation(float x, float y, int value, unsigned int clientIndex);
    void GetNewItemInfo(unsigned char* bufer, int* index);
    void GetMapData(const unsigned char* bufer, int* index);
    void populateClientDudes(int oldClientCount);
    void DrawMap(float r,float g, float b);
    void GetDoorInfo(const unsigned char* bufer, unsigned * index, int* dx, int* dy, unsigned char* frame);
    void KillEnemy(unsigned ID);
    int FPS();
    void DrawSomeText();
    void ServerParseClientGameEquip(const unsigned char* buffer, unsigned* bufferindex, int clientIndex);
    void ServerParseCharacterData(const unsigned char* bufer, unsigned * buferindex, int clientIndex);
    void ServerParseWeaponShot(const unsigned char* buffer, unsigned * bufferindex, int clientIndex);
    void ServerParseClientResurrect(unsigned* bufferindex, int clientIndex);
    void ServerSendTimerSync(unsigned clientIdx);
    void GetServerTimeMsg(const unsigned char* buffer, int * bufferindex);
    void GetServerCoopGameOverMsg(int* bufferindex);
    void GetServerEquipedGame(const unsigned char* buffer, int * bufferindex);

    void GetCharData(const unsigned char* bufer, int bufersize, int* index);

    void equipCartridge(Dude* dude, int game);

    void DrawMiniMap(int x, int y);
    void LoadMap(const char* mapname, int otherplayers);
    void HelpScreenLogic();
    void EndingLogic();
    int PlayerCount();
    void LoadShader(ShaderProgram* shader, const char* name, bool useVulkan, bool useUVS);
    void IntroScreenLogic();
    void PlayNewSong(const char* songName);

    void SendKillCommandToClient(unsigned clientIdx, int victimID);
    void SendFragsToClient(int clientIdx, int frags);

    void DeleteAudio();
    void HandlePlayerAttacks(Dude* player, int clientIndex);
    void CoreGameLogic();
    void DrawNumber(int x, int y, int num);
    void DrawHelp();
};


