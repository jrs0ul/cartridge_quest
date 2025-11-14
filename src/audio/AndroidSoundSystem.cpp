#include <cstdio>
#include "AndroidSoundSystem.h"
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))


ResourseDescriptor loadResourceDescriptor(const char* path, AAssetManager* man){
    AAsset* asset = AAssetManager_open(man, path, AASSET_MODE_UNKNOWN);
    ResourseDescriptor resourceDescriptor;
    resourceDescriptor.decriptor = AAsset_openFileDescriptor(asset, &resourceDescriptor.start, &resourceDescriptor.length);
    AAsset_close(asset);
    return resourceDescriptor;
}

//-----------------------------------------
SoundBuffer* loadSoundFile(const char* filename, AAssetManager* man){
    SoundBuffer* result = new SoundBuffer();
    LOGI("Opening %s\n", filename);
    AAsset* asset = AAssetManager_open(man, filename, AASSET_MODE_UNKNOWN);
    off_t length = AAsset_getLength(asset);
    result->length = length - sizeof(WAVHeader);
    result->header = new WAVHeader();
    result->buffer = new char[result->length];
    //LOGI("length %d\n", result->length);
    AAsset_read(asset, result->header, sizeof(WAVHeader));
    AAsset_read(asset, result->buffer, result->length);
    AAsset_close(asset);
    return result;
}
//---------------------------

void SoundSystem::playMusic(const char* path, AAssetManager* man){


    ResourseDescriptor rd = loadResourceDescriptor(path, man);
    SLDataLocator_AndroidFD locatorIn = {
            SL_DATALOCATOR_ANDROIDFD,
            rd.decriptor,
            rd.start,
            rd.length
    };

    SLDataFormat_MIME dataFormat = {
            SL_DATAFORMAT_MIME,
            NULL,
            SL_CONTAINERTYPE_UNSPECIFIED
    };

    SLDataSource audioSrc = {&locatorIn, &dataFormat};

    SLDataLocator_OutputMix dataLocatorOut = {
            SL_DATALOCATOR_OUTPUTMIX,
            outPutMixObj
    };

    SLDataSink audioSnk = {&dataLocatorOut, NULL};
    const SLInterfaceID pIDs[2] = {SL_IID_PLAY, SL_IID_SEEK};
    const SLboolean pIDsRequired[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

    SLresult result = (*engine)->CreateAudioPlayer(engine, &musicPlayerObj, &audioSrc, &audioSnk, 2, pIDs, pIDsRequired);
    if (result != SL_RESULT_SUCCESS) {
        LOGI("Failed to create audio player for %s\n", path);
        return;
    }
    result = (*musicPlayerObj)->Realize(musicPlayerObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
        return;

    SLPlayItf player;
    result = (*musicPlayerObj)->GetInterface(musicPlayerObj, SL_IID_PLAY, &player);
    if (result != SL_RESULT_SUCCESS)
        return;
    SLSeekItf seek;
    result = (*musicPlayerObj)->GetInterface(musicPlayerObj, SL_IID_SEEK, &seek);
    if (result != SL_RESULT_SUCCESS)
        return;
    (*seek)->SetLoop(
            seek,
            SL_BOOLEAN_TRUE,
            0,
            SL_TIME_UNKNOWN
    );
    (*player)->SetPlayState(player, SL_PLAYSTATE_PLAYING);
}
//------------------------------------------------
void SoundSystem::stopMusic(){
    if (musicPlayerObj){
        (*musicPlayerObj)->Destroy(musicPlayerObj);
        musicPlayerObj = 0;
    }
}




//---------------------------------------------------
void SoundSystem::loadFiles(const char* BasePath, const char* list, AAssetManager * man){


   /* buffers.add(loadSoundFile("sfx/reflect.wav", man));
    buffers.add(loadSoundFile("sfx/bang.wav", man));
    buffers.add(loadSoundFile("sfx/byeball.wav", man));
    buffers.add(loadSoundFile("sfx/gunfire.wav", man));
    buffers.add(loadSoundFile("sfx/prize.wav", man));
    buffers.add(loadSoundFile("sfx/reflect2.wav", man));

    LOGI("Loaded %u buffers\n", buffers.count());*/

}

//----------------------------------------------------------
static void PlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {

}
//---------------------------------
bool SoundSystem::init(){
    LOGI("Starting sound service");

    SLresult result;

    // engine
    const SLuint32 engineMixIIDCount = 1;
    const SLInterfaceID engineMixIIDs[] = {SL_IID_ENGINE};
    const SLboolean engineMixReqs[] = {SL_BOOLEAN_TRUE};

    // create engine
    result = slCreateEngine(&engineObj, 0, NULL,
    engineMixIIDCount, engineMixIIDs, engineMixReqs);
    if (result != SL_RESULT_SUCCESS)
        return false;
    result = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
        return false;
    // get interfaces
    result = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engine);
    if (result != SL_RESULT_SUCCESS)
        return false;
    // mixed output
    const SLuint32 outputMixIIDCount = 0;
    const SLInterfaceID outputMixIIDs[] = {};
    const SLboolean outputMixReqs[] = {};

    // create output
    result = (*engine)->CreateOutputMix(engine, &outPutMixObj,
    outputMixIIDCount, outputMixIIDs, outputMixReqs);
    if (result != SL_RESULT_SUCCESS)
        return false;
    result = (*outPutMixObj)->Realize(outPutMixObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
        return false;

    //audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 1};
    SLDataFormat_PCM format_pcm = {
            SL_DATAFORMAT_PCM,
            1,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_CENTER,
            SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outPutMixObj};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[2] = {SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
    const SLboolean req[2] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engine)->CreateAudioPlayer(engine, &playerObj, &audioSrc, &audioSnk, 2, ids, req);
    if (result != SL_RESULT_SUCCESS)
        return false;

    result = (*playerObj)->Realize(playerObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS)
        return false;
    result = (*playerObj)->GetInterface(playerObj, SL_IID_PLAY, &soundPlayer);
    if (result != SL_RESULT_SUCCESS)
        return false;
    result = (*playerObj)->GetInterface(playerObj, SL_IID_BUFFERQUEUE,
                                             &bufferQueueObj);
    if (result != SL_RESULT_SUCCESS)
        return false;
    result = (*bufferQueueObj)->RegisterCallback(bufferQueueObj, PlayerCallback, NULL);
    if (result != SL_RESULT_SUCCESS)
        return false;

    result = (*playerObj)->GetInterface(playerObj, SL_IID_VOLUME, &playerVolume);
    if (result != SL_RESULT_SUCCESS)
        return false;
    result = (*soundPlayer)->SetPlayState(soundPlayer, SL_PLAYSTATE_PLAYING);
    if (result != SL_RESULT_SUCCESS)
        return false;

   
    return true;

}


//------------------------------------------------
void SoundSystem::playSound(unsigned int index, bool loop){

    //if (_bufferActive) {
    //    return;
    //}


    SLresult result;

    result = (*bufferQueueObj)->Enqueue(bufferQueueObj, buffers[index]->buffer, buffers[index]->length);
    if (result != SL_RESULT_SUCCESS) {
        return;
    }

}




//-----------------------------------------------
void SoundSystem::exit(){

    for (unsigned i = 0; i < buffers.count(); i++){
        delete buffers[i];
    }
    buffers.destroy();

    audioInfo.destroy();

    if (playerObj){
        (*playerObj)->Destroy(playerObj);
        playerObj = 0;
    }

    LOGI("Destroying sound output");
    if (outPutMixObj){
        (*outPutMixObj)->Destroy(outPutMixObj);
        outPutMixObj = 0;
    }

    LOGI("Destroy sound engine");
    if (engineObj){
        (*engineObj)->Destroy(engineObj);
        engineObj = 0;
        engine = 0;
    }
}
