// FMOD_STATEMACHINE_ENGINE.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include "AudioEngine.h"



int main()
{
    std::cout << "Hello World!\n";
    std::cout << "This program just demonstrates the State-Machine working. ";

    //Set Testvector-Kit
    Vector3 soundPos0;
    soundPos0.x = 0; soundPos0.y = 0; soundPos0.z = 0;

    Vector3 soundPos1;
    soundPos1.x = 0; soundPos1.y = 20; soundPos1.z = 0;

    Vector3 soundPos2;
    soundPos2.x = -4; soundPos2.y = 0; soundPos2.z = -5;

    Vector3 soundPosVirtualTest;
    soundPosVirtualTest.x = 105; soundPosVirtualTest.y = 0; soundPosVirtualTest.z = 0;


    //Create AudioEngine
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
    testSound3D.fileName = "3D.mp3";
    

    audioEngine->RegisterSound(testSound3D, true);


    //ID 1
    SoundData testSound2D;
    testSound2D.is3D = false;
    testSound2D.isLoop = true;
    testSound2D.fileName = "2D.mp3";

    audioEngine->RegisterSound(testSound2D, true);


    //******END OF REGISTRATION******

    audioEngine->Update(0.0f);

    system("pause");


    //Test 3D-Sound
    audioEngine->PlayAudio(0, soundPos1, testSound3D.volume);
    
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


    //Test Virtualization
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
    audioEngine->StopAudioChannel(0);
    audioEngine->Update(0.1);
    audioEngine->Update(0.1);
    std::cout << "Stopped 3D Audio. Continue with 2D-Test\n";

    //Testing 2D-Audio-Source
    system("pause");
    system("CLS");
    audioEngine->PlayAudio(1, soundPos0, testSound2D.volume);
    audioEngine->Update(1);
    audioEngine->Update(1);
    std::cout << "Playing 2D Audio....\n";
    

    system("pause");
    system("CLS");
    std::cout << "End of Program. \n";
    audioEngine->StopAudioChannel(0);
    audioEngine->ShutDown();


    system("pause");

}



