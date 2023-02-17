// FMOD_STATEMACHINE_ENGINE.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include "AudioEngine.h"
#include <chrono>
#include <thread>
#include <cmath>




int main()
{
    std::cout << "Hello World!\n";
    std::cout << "This program just demonstrates the State-Machine working. ";


    Vector3 soundPos1;
    soundPos1.x = 0; soundPos1.y = 20; soundPos1.z = 0;

    Vector3 soundPos2;
    soundPos2.x = -4; soundPos2.y = 0; soundPos2.z = -5;


    Vector3 soundPosVirtualTest;
    soundPosVirtualTest.x = 105; soundPosVirtualTest.y = 0; soundPosVirtualTest.z = 0;

    int currentID = NULL;
    int currentChannelID = NULL;

    AudioEngine* audioEngine;

    audioEngine = new AudioEngine();
    audioEngine->Initialize();



    //******REGISTER OF SOUNDS******

    //ID 0
    SoundData testSound3D;
    testSound3D.is3D = true;
    testSound3D.minDistance = 3;
    testSound3D.maxDistance = 90;
    testSound3D.volume = 1;
    testSound3D.isLoop = true;
    testSound3D.virtualDistance = 100;
    

    testSound3D.fileName = "test.wav";

    currentID = audioEngine->RegisterSound(testSound3D, true);


    //******END OF REGISTRATION******

    audioEngine->Update(0.0f);

    system("pause");

    currentChannelID = audioEngine->PlayAudio(0, soundPos1, testSound3D.volume);
    
    audioEngine->Update(1);
    audioEngine->Update(1);

    system("pause");

    system("CLS");

    std::cout << "Changing Position, as soon as button is pressed\n";
    system("pause");
    system("CLS");
    std::cout << "Calling Update...\n";
    audioEngine->moveSoundToPosition(0, soundPos2, 1);
    audioEngine->Update(1);


    system("pause");
    system("CLS");
    std::cout << "Do the -moveSoundInCircle-Trick- as soon as button is pressed \n";
    system("pause");
    audioEngine->moveSoundInCircle(0, 3, 9);



    system("pause");
    system("CLS");
    std::cout << "Testing Virtualization. Current VirtualSetting->Distance is: " << testSound3D.virtualDistance << "\n";
    std::cout << "MOVE SOUND TO VECTOR " << soundPosVirtualTest.x << ", " << soundPosVirtualTest.y << ", " << soundPosVirtualTest.z << std::endl;
    system("pause");
    audioEngine->moveSoundToPosition(0, soundPosVirtualTest, 3);
    std::cout << "Current State is " << audioEngine->getChannelState(0) << std::endl;
    std::cout << "Current Volume is " << audioEngine->getChannelVolume(0) << std::endl;

    system("pause");
    system("CLS");
    std::cout << "Testing DE-Virtualization. Current VirtualSetting->Distance is: " << testSound3D.virtualDistance << "\n";
    std::cout << "MOVE SOUND TO VECTOR " << soundPos2.x << ", " << soundPos2.y << ", " << soundPos2.z << std::endl;
    system("pause");
    audioEngine->moveSoundToPosition(0, soundPos1, 3);
    audioEngine->isChannelVirtual(0);
    std::cout << "Current State is " << audioEngine->getChannelState(0) << std::endl;
    std::cout << "Channels ConfigVolume is " << audioEngine->getChannelVolume(0) << std::endl;

    system("pause");
    system("CLS");
    std::cout << "End of Program. \n";
    audioEngine->StopAudioChannel(0);
    audioEngine->ShutDown();


    system("pause");

}



