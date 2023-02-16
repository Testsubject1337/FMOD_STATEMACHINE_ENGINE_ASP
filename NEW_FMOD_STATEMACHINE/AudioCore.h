#pragma once

#include <map>
#include <vector>
#include <iterator>
#include "Vector3.h"
#include "fmod.h"
#include <fmod.hpp>
#include <fmod_errors.h>
#include <bitset>
#include <memory>

struct AudioChannel;
class AudioEngine;

class AudioCore
{
public:
	AudioCore();
	~AudioCore();
	void SetEarPos(Vector3& pos, bool isRelative, Vector3 forward, Vector3 up);
	void Update(float deltaTime);
	bool CheckLoaded(int soundID);
	void LoadSound(int soundID);

	//Fmod system definition
	FMOD::System* system;

	Vector3 earPos;

	int nextSoundID;
	int nextAudioChannelID;

	//Define the maps for data to be stored in for the engine
	typedef std::map<int, FMOD::Sound*> SoundMap;
	typedef std::map<int, struct SoundData*> SoundDataMap;
	typedef std::map<int, std::unique_ptr<struct AudioChannel>> AudioChannelMap;

	SoundMap sounds;
	SoundDataMap soundDataMap;
	AudioChannelMap channelMap;

private:
};

