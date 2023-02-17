#include "AudioChannel.h"
#include "AudioEngine.h"
#include "AudioCore.h"

AudioChannel::AudioChannel(AudioCore& audioCore, int soundID, SoundData* soundData, VirtualSetting virtSetting, Vector3& pos, float vol) : 
	core(audioCore), soundID(soundID), soundData(soundData), virtualSetting(virtSetting), virtualDistance(soundData->virtualDistance), position(pos)
{
	volume = vol;
}

AudioChannel::~AudioChannel()
{}

void AudioChannel::SetUpdateFlag(UpdateFlag updateFlag, bool flag)
{
	updateFlags.set(static_cast<size_t>(updateFlag), flag);
}

void AudioChannel::Update(float deltaTime)
{
	virtualTimer += deltaTime;
	auto exists = core.sounds.find(soundID);;
	

	//====================================================================================================
	//=====================================BEGIN OF STATEMACHINE==========================================
	//====================================================================================================
	switch (state)
	{
		//======================================INIT===========================================
	case AudioChannel::State::INIT:
		state = State::TOPLAY;

		break;


		//======================================TOPLAY===========================================
	case AudioChannel::State::TOPLAY:



		// ******************************** TOPLAY -> STOPPING 
		if (stopRequested) {
#ifdef _DEBUG
			std::cout << "STATEMACHINE STOP REQUESTED! \n";
#endif
			state = State::STOPPING;
			return;
		}



		// ******************************** TOPLAY -> STOPPING/VIRTUAL
		// If the sound is a one shot, stop the sound. Otherwise virtualise the sound.
		if (VirtualCheck(true, deltaTime))
		{
			if (IsOneShot()) {
				state = State::STOPPING;
			}
			else
				state = State::VIRTUAL;
			return;
		}



		//  ********************************  TOPLAY -> LOADING
		// If the sound isnt loaded and needs to be 
		if (!core.CheckLoaded(soundID)) {
#ifdef _DEBUG
			std::cout << "STATEMACHINE CALLED LOAD SOUND! \n";
#endif
			core.LoadSound(soundID);
			state = State::LOADING;
			return;
		}



		//  ********************************  TOPLAY -> PLAYING
		// If there are no special cases, continue and play the sound
		channel = nullptr;
		exists = core.sounds.find(soundID);
		if (exists != core.sounds.end()) {
#ifdef _DEBUG //If Debug *is* defined, create FMOD-Result for making Debugging easier
			FMOD_RESULT result;
			result = core.system->playSound(exists->second, nullptr, true, &channel);
			if (result != FMOD_OK)
			{
				std::cout << "FMOD ERROR IN STATEMACHINE FOR CHANNEL " << channel << ": " << FMOD_ErrorString(result) << std::endl;
			}
#endif
#ifndef _DEBUG //If Debug is *not* defined, just play the sound.
			core.system->playSound(exists->second, nullptr, true, &channel);
#endif
		}
#ifdef _DEBUG
		else { std::cout << "STATEMACHINE ERROR: SOUND NOT IN SOUNDMAP! \n"; }
#endif

		//If Channel exists, set State to "Playing" and set Channel-Attributes, else set State to "Stopping"
		if (channel) {
			state = State::PLAYING;
			FMOD_VECTOR pos{ position.x, position.y, position.z };
			channel->set3DAttributes(&pos, nullptr);
			channel->set3DMinMaxDistance(soundData->minDistance, soundData->maxDistance);
			channel->setMode(FMOD_3D_LINEARSQUAREROLLOFF);
			channel->setVolume(volume);
			channel->setPaused(false);
		}
		else
			//  ********************************  TOPLAY -> STOPPING
			state = State::STOPPING;
		break;


		//======================================PLAYING===========================================
	case AudioChannel::State::PLAYING:

		UpdateParams();

		//  ********************************  PLAYING -> STOPPING
		// Should Stop?
		if (stopRequested) 
		{
			state = State::STOPPING;
		}

		//  ********************************  PLAYING -> VIRTUALIZING
		// Should be virtualized?
		if (VirtualCheck(false, deltaTime)) {
#ifdef _DEBUG
			std::cout << "STATEMACHINE: Setting State to VIRTUALISING\n";
#endif
			state = State::VIRTUALIZING;
		}


		break;


		//======================================LOADING===========================================
	case AudioChannel::State::LOADING:
		// === LOADING ---> TOPLAY === //
		//Check if the sound is loaded, if it is play it
		// =========================== //
		if (core.CheckLoaded(soundID)) {
#ifdef _DEBUG

			std::cout << "STATEMACHINE CHECKING FOR LOADING! \n";

#endif
			//  ********************************  LOADING -> TOPLAY
			state = State::TOPLAY;
		}
		// =========================== //

		break;


		//======================================PREPLAYING===========================================
	case AudioChannel::State::PREPLAYING:
		// This state is active when the sound needs to be preplayed to
		// run the fade in effect.
		RunFadeIn(deltaTime);

		break;


		//======================================VIRTUALIZING===========================================
	case AudioChannel::State::VIRTUALIZING:
#ifdef _DEBUG
		std::cout << "STATEMACHINE: CHANNEL IS VIRTUALISING\n";
#endif
		RunFadeOut(deltaTime);
		UpdateParams();

		if (!VirtualCheck(false, deltaTime)) {
			state = State::PREPLAYING;
		}
		break;


		//======================================VIRTUAL===========================================
	case AudioChannel::State::VIRTUAL:
#ifdef _DEBUG
		std::cout << "STATEMACHINE: CHANNEL IS NOW VIRTUAL\n";
#endif
		if (stopRequested) {
			state = State::STOPPING;
		}
		else if (!VirtualCheck(false, deltaTime)) {
#ifdef _DEBUG
			std::cout << "STATEMACHINE: CHANNEL SHOULD NOT BE VIRTUAL, SINCE VIRTUAL-CHECK IS FALSE BASED ON POSITION!";
#endif
			if (virtualSetting == VirtualSetting::RESTART) {
				state = State::INIT;
			}
			else if (virtualSetting == VirtualSetting::PAUSE) {
				channel->setPaused(false);
				state = State::PREPLAYING;
			}
			else {
				state = State::PREPLAYING;
			}
		}
		break;


		//======================================STOPPING===========================================
	case AudioChannel::State::STOPPING:
		//Run fade out logic and stop the channel
#ifdef _DEBUG
		
			std::cout << "STATEMACHINE CALLED STOPPING! \n";
		
#endif

		RunFadeOut(deltaTime);
		UpdateParams();
		if (stopRequested || volume == 0.0f) {
			channel->stop();
			state = State::STOPPED;
			return;
		}
		break;


		//======================================STOPPED===========================================
	case AudioChannel::State::STOPPED:
		break;
	default:
		break;
	}

//====================================================================================================
//======================================END OF STATEMACHINE===========================================
//====================================================================================================
}





void AudioChannel::UpdateParams()
{
#ifdef _DEBUG
	std::cout << "CHANNEL: UpdateParams() called.\n";
#endif
	//Update virtual flags and parameters 
	if (updateFlags.test(static_cast<size_t>(UpdateFlag::POSITION))) {
#ifdef _DEBUG
		std::cout << "CHANNEL: UpdateFlag recognized as true. Updating Position to\n";
		std::cout << position.x << ", " << position.y << ", " << position.z << std::endl;
#endif
		FMOD_VECTOR newPosition = { position.x, position.y, position.z };
		channel->set3DAttributes(&newPosition, nullptr);
		SetUpdateFlag(UpdateFlag::POSITION, false);
	}
}

//Run the fade in logic for a channel
void AudioChannel::RunFadeIn(float deltaTime)
{
	const float maxDeltaTime = virtualFadeInTime;
	if (deltaTime > maxDeltaTime) {
		deltaTime = 0.1f; //limiting delta-time for avoiding failed fade-ins due to performance-issues
	}
	float currentVolume;
	channel->getVolume(&currentVolume);
	float newVolume = currentVolume + deltaTime / virtualFadeInTime;
	if (newVolume >= volume) {
		if (state == State::PREPLAYING) {
			
			state = State::PLAYING;
		}
	}
	else {
		channel->setVolume(newVolume);
	}
}

void AudioChannel::RunFadeOut(float deltaTime)
{
	float currentVolume;

	channel->getVolume(&currentVolume);
	float newVolume = currentVolume;
	if (state == State::STOPPING) {
		newVolume = currentVolume - deltaTime / stopFadeOutTime;
	}
	else if (state == State::VIRTUALIZING) {
		newVolume = currentVolume - deltaTime / virtualFadeOutTime;
		//newVolume = currentVolume - virtualFadeOutTime * deltaTime;
	}
	if (newVolume <= 0.0f) {
		if (state == State::STOPPING) {
			channel->stop();
			state = State::STOPPED;
		}
		else if (state == State::VIRTUALIZING) {
			state = State::VIRTUAL;
			if (virtualSetting == VirtualSetting::RESTART) {
				channel->stop();
			}
			else if (virtualSetting == VirtualSetting::PAUSE) {
				channel->setPaused(true);
			}
			else if (virtualSetting == VirtualSetting::MUTE) {
				channel->setVolume(0.0f);
			}
		}


	}
	else {
		channel->setVolume(newVolume);
	}
}

bool AudioChannel::VirtualCheck(bool allowOneShot, float deltaTime) 
{
	if (!isVirtFlagEffective) {
		return virtualFlag;
	}
	else {
		virtualTimer = 0.0f;
		Vector3& earPosition = core.earPos;
		float deltaX = position.x - earPosition.x;
		float deltaY = position.y - earPosition.y;
		float deltaZ = position.z - earPosition.z;
		float distanceSquared = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
		return(distanceSquared > virtualDistance * virtualDistance);
	}
}

bool AudioChannel::IsPlaying()
{
	bool isPlaying = channel->isPlaying(&isPlaying);

	return isPlaying;
}

bool AudioChannel::IsOneShot()
{

	return false;
}


