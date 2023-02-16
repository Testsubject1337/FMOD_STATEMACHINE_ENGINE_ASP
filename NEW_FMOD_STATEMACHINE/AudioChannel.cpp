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


	// ********************************* //
	// ******** State machine ********** //
	// ********************************* //
	switch (state)
	{
	case AudioChannel::State::INIT:
		//[[fallthrough]]; 
		state = State::TOPLAY;
		break;
	case AudioChannel::State::TOPLAY:
		// === TOPLAY ---> STOPPING === //
		// ============================ //
		if (stopRequested) {

#ifdef _DEBUG
			std::cout << "STATEMACHINE STOP REQUESTED! \n";
#endif

			state = State::STOPPING;
			return;
		}
		// ============================ //

		// === TOPLAY ---> STOPPING/VIRTUAL === //
		// If the sound is a one shot, stop the sound
		// Otherwise virtualise the sound
		// ==================================== //
		if (VirtualCheck(true, deltaTime))
		{
			if (IsOneShot()) {
				state = State::STOPPING;
			}
			else
				state = State::VIRTUAL;
			return;
		}
		// =================================== //

		// === TOPLAY ---> LOADING === //
		// If the sound isnt loaded and needs to be 
		// =========================== //
		if (!core.CheckLoaded(soundID)) {
#ifdef _DEBUG

			std::cout << "STATEMACHINE CALLED LOAD SOUND! \n";

#endif
			core.LoadSound(soundID);
			state = State::LOADING;
			return;
		}
		// =========================== //

		// === TOPLAY ---> PLAYING === //
		// If there are no special cases,
		// continue and play the sound
		// =========================== //
		channel = nullptr;
		exists = core.sounds.find(soundID);
		if (exists != core.sounds.end()) {
#ifdef _DEBUG
			FMOD_RESULT result;
			result = core.system->playSound(exists->second, nullptr, true, &channel);
			if (result != FMOD_OK)
			{
				std::cout << "FMOD ERROR IN STATEMACHINE FOR CHANNEL " << channel << ": " << FMOD_ErrorString(result) << std::endl;
			}
#endif
#ifndef _DEBUG
			core.system->playSound(exists->second, nullptr, true, &channel);
#endif
		}
#ifdef _DEBUG
		else
		{
			std::cout << "STATEMACHINE ERROR: SOUND NOT IN SOUNDMAP! \n";
		}
#endif
		if (channel) {
			state = State::PLAYING;
			FMOD_VECTOR pos{ position.x, position.y, position.z };
			channel->set3DAttributes(&pos, nullptr);
			channel->set3DMinMaxDistance(soundData->minDistance, soundData->maxDistance);
			channel->setVolume(volume);

			channel->setPaused(false);
		}
		else
			state = State::STOPPING;
		// =========================== //

		break;
	case AudioChannel::State::PLAYING:

		UpdateParams();

		// === PLAYING ---> STOPPING === //
		// Does the sound need to be stopped?
		// =========================== //
		if (/*!IsPlaying() ||*/ stopRequested) {
			state = State::STOPPING;
		}
		// =========================== //

		// === PLAYING ---> VIRTUALISING === //
		// ================================= //
		/*if (virtualFlag == true) {
			state = State::VIRTUALISING;
		}*/
		if (VirtualCheck(false, deltaTime)) {
			state = State::VIRTUALISING;
		}

		// ================================= //

		break;
	case AudioChannel::State::LOADING:
		// === LOADING ---> TOPLAY === //
		//Check if the sound is loaded, if it is play it
		// =========================== //
		if (core.CheckLoaded(soundID)) {
#ifdef _DEBUG

			std::cout << "STATEMACHINE CHECKING FOR LOADING! \n";

#endif
			state = State::TOPLAY;
		}
		// =========================== //

		break;
	case AudioChannel::State::PREPLAYING:
		// This state is active when the sound needs to be preplayed to
		// run the fade in effect.
		RunFadeIn(deltaTime);

		break;
	case AudioChannel::State::VIRTUALISING:

		RunFadeOut(deltaTime);
		UpdateParams();
		if (!VirtualCheck(false, deltaTime)) {
			state = State::PREPLAYING;
		}

		break;
	case AudioChannel::State::VIRTUAL:

		if (stopRequested) {
			state = State::STOPPING;
		}
		else if (!VirtualCheck(false, deltaTime)) {
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
	case AudioChannel::State::STOPPED:
		break;
	default:
		break;
	}
}

void AudioChannel::UpdateParams()
{
	//Update virtual flags and parameters 
	if (updateFlags.test(static_cast<size_t>(UpdateFlag::POSITION))) {
		FMOD_VECTOR newPosition = { position.x, position.y, position.z };
		channel->set3DAttributes(&newPosition, nullptr);
		SetUpdateFlag(UpdateFlag::POSITION, false);
	}
}

//Run the fade in logic for a channel
void AudioChannel::RunFadeIn(float deltaTime)
{
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
	else if (state == State::VIRTUALISING) {
		newVolume = currentVolume - deltaTime / virtualFadeOutTime;
		//newVolume = currentVolume - virtualFadeOutTime * deltaTime;
	}
	if (newVolume <= 0.0f) {
		if (state == State::STOPPING) {
			channel->stop();
			state = State::STOPPED;
		}
		else if (state == State::VIRTUALISING) {
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
	/*if (virtualTimer < virtualCheckVirtualPeriod) {
		return (state == State::VIRTUALISING || state == State::VIRTUAL);
	}*/
	if (isVirtFlagEffective) {
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