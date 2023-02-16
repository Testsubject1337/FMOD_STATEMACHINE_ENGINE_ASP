// FMOD_STATEMACHINE_ENGINE.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include "AudioEngine.h"
#include <chrono>
#include <thread>


int main()
{
    std::cout << "Hello World!\n";


    Vector3 soundPos;
    soundPos.x = 6; soundPos.y = 3; soundPos.z = 6;

    int currentID = NULL;

    AudioEngine* audioEngine;

    audioEngine = new AudioEngine();
    audioEngine->Initialize();



    //******REGISTER OF SOUNDS******

    //ID 0
    SoundData testSound2D;
    testSound2D.is3D = false;
    testSound2D.isLoop = true;
    testSound2D.volume = 5;
    testSound2D.isStream = false;
    testSound2D.virtualDistance = 100;
    testSound2D.minDistance = 0;
    testSound2D.maxDistance = 10000000;

    testSound2D.fileName = "music.wav";
    audioEngine->RegisterSound(testSound2D, true);


    //ID 2
    SoundData testSound3D;
    testSound3D.is3D = true;
    testSound3D.minDistance = 1;
    testSound3D.maxDistance = 5;
    testSound3D.volume = 4;
    testSound3D.isLoop = true;

    testSound3D.fileName = "tada.wav";

    currentID = audioEngine->RegisterSound(testSound3D, true);


    //******END OF REGISTRATION******

    audioEngine->Update(0.0f);

    system("pause");

    currentID = audioEngine->PlayAudio(1, soundPos, testSound3D.volume);
    
    //audioEngine->SetAudioChannelVolume(0, 10);

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        audioEngine->Update(0.01);
    }
    
    //currentID = audioEngine->PlayAudio(0, soundPos, testSound2D.volume);

    //std::cout << currentID << std::endl;

    //audioEngine->PlayAudio(2, { 0,0,0 }, 60.0f);

    system("pause");



}

