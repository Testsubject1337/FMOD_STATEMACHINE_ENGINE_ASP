#pragma once

#include "Vector3.h"
#include "fmod.h"
#include "fmod.hpp"
#include <bitset>

class AudioEngine;
class AudioCore;

//Defines the virtual settings a channel can use to select its virtualisation preference
enum class VirtualSetting
{
	RESTART, PAUSE, MUTE
};


// ================= FMOD Channel Wrapper ======================= //
	// == The AudioChannel wraps the FMOD::Channel so that it     == //
	// == can be used as a part of the state machine, containing  == //
	// == its channel, sound data and unique ID to keep track of  == //
	// == each channel as it is used through the state machine    == //

struct AudioChannel
{
	AudioChannel(class AudioCore& audioCore, int soundID, struct SoundData* soundData, VirtualSetting virtSetting, Vector3& pos, float vol);
	~AudioChannel();

	//Defines the different states of the state machine
	enum class State {
		INIT,
		TOPLAY,
		PLAYING,
		LOADING,
		PREPLAYING,
		VIRTUALISING,
		VIRTUAL,
		STOPPING,
		STOPPED
	};

	enum class UpdateFlag : short {
		POSITION = 0,
		VOLUME
	};

	//Channel Wrapper Data Variables
	class AudioCore& core;
	FMOD::Channel* channel = nullptr;
	SoundData* soundData;
	int soundID;

	//Virtualisation Settings
	VirtualSetting virtualSetting;

	bool virtualFlag = false;
	float virtualDistance;
	float virtualTimer = 0.0f;
	float virtualCheckPlayPeriod = 0.5f;
	float virtualCheckVirtualPeriod = 1.0f;
	float virtualFadeInTime = 0.2f;
	float virtualFadeOutTime = 0.2f;
	float stopFadeOutTime = 4.0f;

	State state = State::INIT;
	float volume = 1.0f;
	bool stopRequested = false;
	Vector3 position;

	//Virtual Flags
	bool isVirtFlagEffective = true;
	bool virtFlag = false;
	std::bitset<8> updateFlags;

	void SetUpdateFlag(UpdateFlag updateFlag, bool flag);
	void Update(float deltaTime);
	void UpdateParams();
	void RunFadeIn(float deltaTime);
	void RunFadeOut(float deltaTime);
	bool VirtualCheck(bool allowOneShot, float deltaTime);
	bool IsPlaying();
	bool IsOneShot();

};

