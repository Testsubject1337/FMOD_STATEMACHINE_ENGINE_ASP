#include "AudioEngine.h"
#include "AudioChannel.h"
#include "AudioCore.h"


// ***** create Pointer to make AudioCore usable ***** //

AudioCore* core = nullptr; 


// ***** Constructor ***** //

AudioEngine::AudioEngine()
{
}


// ***** Destructor ***** //

AudioEngine::~AudioEngine()
{
}


// ******  Core System Function Definitions  ****** //

void AudioEngine::Initialize()
{
	core = new AudioCore();
}

void AudioEngine::Update(float timeDeltaTime)
{
	core->Update(timeDeltaTime);
}

int AudioEngine::ErrorCheck(FMOD_RESULT result)
{
	if (result != FMOD_OK) 
	{
		std::cout << "FMOD ERROR " << result << std::endl;
		return 1;
	}
	return 0;
}

void AudioEngine::SetEarPos(Vector3 pos, bool isRelative, Vector3 forward, Vector3 up)
{
	core->SetEarPos(pos, isRelative, forward, up);
}

void AudioEngine::ShutDown()
{
	delete core;
}


// ******  SoundData Management Function Definitions  ****** //

bool AudioEngine::CheckLoaded(int soundID)
{
	return core->CheckLoaded(soundID);
}

void AudioEngine::LoadSound(int soundID)
{
	core->LoadSound(soundID);
}

void AudioEngine::UnloadSound(int soundID)
{
	//If the sound is loaded, unload it. If it isnt then do nothing
	auto exists = core->sounds.find(soundID);
	if (exists != core->sounds.end()) {
		//Unload the sound
		exists->second->release();
		core->sounds.erase(exists);
	}
}

int AudioEngine::RegisterSound(SoundData& soundData, bool load) //Registers a sound to a unique ID, giving the option to load it directly after
{
	int soundID = core->nextSoundID;
	core->nextSoundID++;
	core->soundDataMap[soundID] = &soundData;

	if (load) {
		core->LoadSound(soundID);
	}
	return soundID;
}


// ****** Channel Manipulation ****** //

int AudioEngine::PlayAudio(int soundID, Vector3 pos, float volume)
{
	FMOD_RESULT result;

	int audioChannelID = core->nextAudioChannelID; //get next ChannelID
	core->nextAudioChannelID++; //Count up for next Audio
	auto found = core->sounds.find(soundID);

#ifdef _DEBUG
	FMOD_OPENSTATE openstate;
	unsigned int bufferedinpercent;
	found->second->getOpenState(&openstate, &bufferedinpercent, NULL, NULL);
	std::cout << "OPENSTATE: " << openstate << " BUFFERED " << bufferedinpercent << std::endl;

#endif

	//If Audio is not loaded, fail
	if (found == core->sounds.end())
	{
		std::cerr << "PLAY SOUND ERROR: AUDIO NOT LOADED\n";
		return audioChannelID;
	}

	//Create AudioChannel, ***MUTED*** to prevent Artifacts
	core->channelMap[audioChannelID] = std::make_unique<AudioChannel>(*core, soundID, core->soundDataMap[soundID], VirtualSetting::MUTE, pos, volume);
	
#ifdef _DEBUG
	auto sampledata = core->channelMap.find(audioChannelID);
	std::cout << "Play Audio: AudioChannel with ID " << audioChannelID << " is " << sampledata->second << std::endl;
	std::cout << "Channel Details: \nisPlaying:" << sampledata->second->IsPlaying() << "\n"
		"Current virtflag " << sampledata->second->virtFlag << "\n" <<
		"Current position " << sampledata->second->position.x << ", " << sampledata->second->position.y << ", " << sampledata->second->position.z << "\n" <<
		"Current STATE " << static_cast<int>(sampledata->second->state) << "\n";

#endif

	return audioChannelID;
}

void AudioEngine::SetAudioChannelVolume(int channelID, float volume)
{
	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end()) {
		exists->second->volume = volume;
		exists->second->channel->setVolume(volume);
	}
}

void AudioEngine::SetSoundDirection(int channelID, Vector3 direction, Vector3 coneSettings)
{
	FMOD_VECTOR coneDirection = { direction.x, direction.y, direction.z };
	FMOD_VECTOR settings = { coneSettings.x, coneSettings.y, coneSettings.z };

	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end()) 
	{
		exists->second->channel->set3DConeOrientation(&coneDirection);
		//inside angle, outside angle, outsidevolume
		exists->second->channel->set3DConeSettings(settings.x, settings.y, settings.z);
	}
}

void AudioEngine::ChangeVirtualSetting(int channelID, int setting)
{
	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end()) {
		if (setting == 0) {
			exists->second->virtualSetting = VirtualSetting::RESTART;
		}
		else if (setting == 1) {
			exists->second->virtualSetting = VirtualSetting::PAUSE;
		}
		else if (setting == 2) {
			exists->second->virtualSetting = VirtualSetting::MUTE;
		}
		else {
			//If an incorrect number was provided, default to MUTE
			exists->second->virtualSetting = VirtualSetting::MUTE;
		}

	}
}

void AudioEngine::SetAudioChannelPosition(int channelID, Vector3 pos, bool isRelative)
{
	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end())
	{
		auto& channel = exists->second;
		channel->SetUpdateFlag(AudioChannel::UpdateFlag::POSITION, true);
		if (isRelative) {
			Vector3 newPos =
			{
				channel->position.x + pos.x,
				channel->position.y + pos.y,
				channel->position.z + pos.z
			};
			channel->position = pos;
		}
	}
}

void AudioEngine::StopAudioChannel(int channelID)
{
	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end()) {
		exists->second->stopRequested = true;
	}
}

void AudioEngine::VirtualizeAudioChannel(int channelID)
{
	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end()) 
	{
		exists->second->virtualFlag = true;
	}
}

void AudioEngine::DeVirtualizeAudioChannel(int channelID)
{
	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end()) 
	{
		exists->second->virtualFlag = false;
	}

}

