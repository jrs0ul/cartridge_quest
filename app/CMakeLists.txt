cmake_minimum_required(VERSION 3.4.1)

project(CartridgeQuest LANGUAGES CXX)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

set(SRCDIR ../src/)
find_package(game-activity REQUIRED CONFIG)

#find_package(Vulkan REQUIRED)

add_library(game SHARED

             ${SRCDIR}VulkanVideo.cpp
             ${SRCDIR}game_android.cpp
             ${SRCDIR}Game.cpp
             ${SRCDIR}SpriteBatcher.cpp
             ${SRCDIR}audio/AndroidSoundSystem.cpp
             ${SRCDIR}Dude.cpp
             ${SRCDIR}MapGenerator.cpp
             ${SRCDIR}map.cpp
             ${SRCDIR}bullet.cpp
             ${SRCDIR}Decal.cpp
             ${SRCDIR}Intro.cpp
             ${SRCDIR}BulletContainer.cpp
             ${SRCDIR}maplist.cpp
             ${SRCDIR}gui/SelectMenu.cpp
             ${SRCDIR}OSTools.cpp
             ${SRCDIR}Inventory.cpp
             ${SRCDIR}Particles2D.cpp
             ${SRCDIR}gui/Text.cpp
             ${SRCDIR}gui/Slider.cpp
             ${SRCDIR}gui/EditBox.cpp
             ${SRCDIR}Collection.cpp
             ${SRCDIR}GameData.cpp
             ${SRCDIR}SaveGame.cpp
             ${SRCDIR}SysConfig.cpp
             ${SRCDIR}network/Socket.cpp
             ${SRCDIR}network/Client.cpp
             ${SRCDIR}network/Server.cpp
             ${SRCDIR}Vectors.cpp
             ${SRCDIR}Xml.cpp
             ${SRCDIR}Matrix.cpp
             ${SRCDIR}Usefull.cpp
             ${SRCDIR}SpriteBatcher.cpp
             ${SRCDIR}Image.cpp
             ${SRCDIR}RenderTexture.cpp
             ${SRCDIR}ShaderProgram.cpp
             ${SRCDIR}Shader.cpp
             )


target_include_directories(game PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           )
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u \
    Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

target_link_libraries(game PUBLIC

                       android
                       EGL
                       GLESv2
                       OpenSLES
                       game-activity::game-activity_static
                       #Vulkan::Vulkan
                       log )
