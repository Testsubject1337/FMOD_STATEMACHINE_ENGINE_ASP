#pragma once

#include "fmod.h"
#include <fmod.hpp>
#include <vector>
#include <map>
#include <iostream>
#include "Vector3.h"


// ********************************* Audio Engine ********************************* //
//This Audio Engine is a state machine oriented System for application
//in games, 3D or 2D. Consisting of AudioCore, housing the state machine 
//architecture and core functionality, and the AudioEngine where all of this 
//is wrapped and applied. The engine is designed to be expandable and maintainable,
//with its state machine design meaning new features in the form of states can be
//added at the developers desire. 
// ******************************************************************************** //


struct SoundData {
	std::string fileName;
	float volume;
	float minDistance;
	float maxDistance;
	float virtualDistance;
	bool is3D;
	bool isLoop;
	bool isStream;
};

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
};

