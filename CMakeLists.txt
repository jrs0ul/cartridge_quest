cmake_minimum_required(VERSION 3.1)

project(CartridgeQuest LANGUAGES CXX)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(CartridgeQuest
                    src/Dude.cpp
                    src/MapGenerator.cpp
                    src/map.cpp
                    src/bullet.cpp
                    src/Decal.cpp
                    src/Intro.cpp
                    src/BulletContainer.cpp
                    src/maplist.cpp
                    src/gui/SelectMenu.cpp
                    src/OSTools.cpp
                    src/Inventory.cpp
                    src/Particles2D.cpp
                    src/gui/Text.cpp
                    src/gui/Slider.cpp
                    src/gui/EditBox.cpp
                    src/Collection.cpp
                    src/GameData.cpp
                    src/SaveGame.cpp
                    src/SysConfig.cpp
                    src/network/Socket.cpp
                    src/network/Client.cpp
                    src/network/Server.cpp
                    src/Vectors.cpp
                    src/Xml.cpp
                    src/Matrix.cpp
                    src/Usefull.cpp
                    src/SpriteBatcher.cpp
                    src/Image.cpp
                    src/RenderTexture.cpp
                    src/VulkanVideo.cpp
                    src/SDLVideo.cpp
                    src/Extensions.cpp
                    src/ShaderProgram.cpp
                    src/Shader.cpp
                    src/audio/OggStream.cpp
                    src/audio/SoundSystem.cpp
                    src/Game.cpp
                    src/mainer.cpp
                    )

                target_link_libraries(CartridgeQuest 
                        SDL2
                        GL
                        openal
                        vorbisfile
                        vulkan
                        ogg
                     )

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/shaders/justcolor_vert.spv
    COMMAND glslangValidator ${CMAKE_SOURCE_DIR}/shaders/vk_justcolor.vert --target-env vulkan1.1 -o ${CMAKE_SOURCE_DIR}/shaders/justcolor_vert.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/vk_justcolor.vert
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/shaders/justcolor_frag.spv
    COMMAND glslangValidator ${CMAKE_SOURCE_DIR}/shaders/vk_justcolor.frag --target-env vulkan1.1 -o ${CMAKE_SOURCE_DIR}/shaders/justcolor_frag.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/vk_justcolor.frag
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/shaders/default_vert.spv
    COMMAND glslangValidator ${CMAKE_SOURCE_DIR}/shaders/vk_default.vert --target-env vulkan1.1 -o ${CMAKE_SOURCE_DIR}/shaders/default_vert.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/vk_default.vert
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/shaders/default_frag.spv
    COMMAND glslangValidator ${CMAKE_SOURCE_DIR}/shaders/vk_default.frag --target-env vulkan1.1 -o ${CMAKE_SOURCE_DIR}/shaders/default_frag.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/vk_default.frag
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/shaders/filmGrain_vert.spv
    COMMAND glslangValidator ${CMAKE_SOURCE_DIR}/shaders/vk_filmGrain.vert --target-env vulkan1.1 -o ${CMAKE_SOURCE_DIR}/shaders/filmGrain_vert.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/vk_filmGrain.vert
)

add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/shaders/filmGrain_frag.spv
    COMMAND glslangValidator ${CMAKE_SOURCE_DIR}/shaders/vk_filmGrain.frag --target-env vulkan1.1 -o ${CMAKE_SOURCE_DIR}/shaders/filmGrain_frag.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/shaders/vk_filmGrain.frag
)




add_custom_target(CompileShaders ALL DEPENDS
    ${CMAKE_SOURCE_DIR}/shaders/justcolor_vert.spv
    ${CMAKE_SOURCE_DIR}/shaders/justcolor_frag.spv
    ${CMAKE_SOURCE_DIR}/shaders/default_vert.spv
    ${CMAKE_SOURCE_DIR}/shaders/default_frag.spv
    ${CMAKE_SOURCE_DIR}/shaders/filmGrain_vert.spv
    ${CMAKE_SOURCE_DIR}/shaders/filmGrain_frag.spv
)


