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
    int currentChannelID = NULL;

    AudioEngine* audioEngine;

    audioEngine = new AudioEngine();
    audioEngine->Initialize();



    //******REGISTER OF SOUNDS******



    //ID 0
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

    currentChannelID = audioEngine->PlayAudio(0, soundPos, testSound3D.volume);
    
    audioEngine->Update(1);
    audioEngine->Update(1);

    system("pause");

    system("CLS");

    std::cout << "Virtualising Channel, as soon as button is pressed\n";
    system("pause");
    system("CLS");
    std::cout << "Calling Update...\n";
    audioEngine->VirtualizeAudioChannel(currentChannelID);


    system("pause");
    system("CLS");
    std::cout << "Calling Update...\n";
    audioEngine->Update(1);


    system("pause");
    system("CLS");
    std::cout << "Calling Update...\n";
    audioEngine->Update(1);


    system("pause");
    system("CLS");
    std::cout << "Calling Update...\n";
    audioEngine->Update(1);

    system("pause");
    system("CLS");
    std::cout << "Calling Update...\n";
    audioEngine->Update(1); //Should show "CHANNEL IS NOW VIRTUAL"


    system("pause");
    system("CLS");
    std::cout << "End of Program. \n";
    system("pause");

}

