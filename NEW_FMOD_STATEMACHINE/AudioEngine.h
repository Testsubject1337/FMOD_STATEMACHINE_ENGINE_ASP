#pragma once

#include "fmod.h"
#include <fmod.hpp>
#include <vector>
#include <map>
#include <iostream>
#include "Vector3.h"
#include <chrono>
#include <thread>
#include <cmath>
#include "SoundData.h"

// ********************************* Audio Engine ********************************* //
// This is a system designed for use in games, both 2D and 3D, that is made up of two 
// main parts : the AudioCore, which handles the core functionalityand state machine 
// architecture, and the AudioEngine, which combinesand applies these components.
// The state machine design allows for easy expansionand maintenance, 
// as new features can be added in the form of new states as desired by developers.
// ******************************************************************************** //



class AudioEngine
{
public:
	AudioEngine();
	~AudioEngine();

	// *** Core System Functions *** // 
	void Initialize();
	void Update(float timeDeltaTime);
	int ErrorCheck(FMOD_RESULT result);
	void SetEarPos(Vector3 pos, bool isRelative, Vector3 forward, Vector3 up);
	void ShutDown();


	// *** Sound Data Management *** //
	bool CheckLoaded(int soundID);
	void LoadSound(int soundID);
	void UnloadSound(int soundID);
	int RegisterSound(SoundData& soundData, bool load); //Registers a sound to a unique ID, giving the option to load it directly after


	// *** Channel Manipulation *** //
	int PlayAudio(int soundID, Vector3 pos, float volume); 
	void SetAudioChannelVolume(int channelID, float volume); 
	void SetSoundDirection(int channelID, Vector3 direction, Vector3 coneSettings);
	void ChangeVirtualSetting(int channelID, int setting);
	void SetAudioChannelPosition(int channelID, Vector3 pos, bool isRelative);
	void StopAudioChannel(int channelID);
	void VirtualizeAudioChannel(int channelID);
	void DeVirtualizeAudioChannel(int channelID);

	// *** Effects *** // 
	FMOD::Reverb3D* CreateReverb(Vector3 position, FMOD_REVERB_PROPERTIES properties);
	void SetReverbActive(FMOD::Reverb3D* reverb, bool state);
	void CreateFmodGeometry(FMOD::Geometry* geometry, int maxPoligons, int maxVertices);
	void GetOcclusion(FMOD_VECTOR* listenerPos, FMOD_VECTOR* sourcePos, float directOcclusion, float reverbOcclusion);
	void SetOcclusion(int channelID, float directOcclusion, float reverbOcclusion);

	// *** FUN AND DEBUG-FEATURES *** //
	void moveSoundInCircle(int channelID, int durationInSeconds, int radius); //Time is not considered precisely - only an approximate value.
	void moveSoundToPosition(int channelID, Vector3 targetPosition, int durationInSeconds); //Time is not considered precisely - only an approximate value.
	bool isChannelVirtual(int channelID); //Debug
	std::string getChannelState(int channelID); //Debug
	float getChannelVolume(int channelID); //Debug

};

