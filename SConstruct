import os
import sys

OBJ_DIR = os.path.join(os.getcwd(), 'obj/');
env = Environment(ENV=os.environ,
                  CCFLAGS=['-O3','-Wall', '-Wextra'],
                  OBJPREFIX = OBJ_DIR)

#Windblows
if sys.platform == 'win32':
    Tool('mingw')(env)
    env.Append(LIBS = ['mingw32','SDL2main', 'SDL2', 'OpenGL32',
                       'imagehlp', 'dxguid', 'dxerr8',
                       'oleaut32', 'shell32', 'version', 'uuid',
                       'openal32', 'vorbisfile', 'ole32', 'imm32', 'user32',
                       'gdi32', 'winmm', 'dinput8', 'vorbis', 'ogg', 'ws2_32', 'vulkan'])
else: #Mac OS X
    if sys.platform == 'darwin':
        env['FRAMEWORKS'] = ['Cocoa', 'SDL2', 'OPENGL', 'OpenAL','vorbis', 'Ogg']
        env.Append(LIBS = [ 'iconv', 'pthread'])
    else: #Linux
        env.Append(LIBS = ['SDL2','GL', 'GLU', 'openal', 'vorbisfile', 'ogg', 'vulkan'])


env.Program(target = "CartridgeQuest", 
            source = ["src/mainer.cpp",
                      "src/Dude.cpp",
                      "src/MapGenerator.cpp",
                      "src/map.cpp",
                      "src/bullet.cpp",
                      "src/Decal.cpp",
                      "src/BulletContainer.cpp",
                      "src/maplist.cpp",
                      "src/gui/SelectMenu.cpp",
                      "src/OSTools.cpp",
                      "src/gui/Text.cpp",
                      "src/gui/Slider.cpp",
                      "src/gui/EditBox.cpp",
                      "src/Intro.cpp",
                      "src/SysConfig.cpp",
                      "src/network/Socket.cpp",
                      "src/network/Client.cpp",
                      "src/network/Server.cpp",
                      "src/Vectors.cpp",
                      "src/Xml.cpp",
                      "src/Matrix.cpp",
                      "src/Usefull.cpp",
                      "src/SpriteBatcher.cpp",
                      "src/Image.cpp",
                      "src/VulkanVideo.cpp",
                      "src/SDLVideo.cpp",
                      "src/Extensions.cpp",
                      "src/ShaderProgram.cpp",
                      "src/Shader.cpp",
                      "src/audio/OggStream.cpp",
                      "src/audio/SoundSystem.cpp",
                      "src/Game.cpp",
                      "src/Particles2D.cpp",
                      "src/SaveGame.cpp",
                      "src/GameData.cpp",
                      "src/Collection.cpp",
                      "src/Inventory.cpp"
                      ]
            )

