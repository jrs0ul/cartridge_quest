cmake_minimum_required(VERSION 3.4.1)

project(CartridgeQuest)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

set(SRCDIR ../src/)

add_library(app-glue STATIC
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

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



find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log
            )



target_include_directories(game PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${ANDROID_NDK}/sources/android/native_app_glue)




target_link_libraries(
                       game
                       android
                       EGL
                       GLESv2
                       app-glue
                       OpenSLES
                       #Vulkan::Vulkan


                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )
