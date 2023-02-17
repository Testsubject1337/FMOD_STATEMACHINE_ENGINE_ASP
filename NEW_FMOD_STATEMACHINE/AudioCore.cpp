#include "AudioCore.h"
#include "AudioEngine.h"
#include "AudioChannel.h"

// ****** ****** Function Definitions ****** ****** //
// ****** AudioCore Constructor ****** //

AudioCore::AudioCore() : earPos{ 0.0f, 0.0f, 0.0f }, nextSoundID(0), nextAudioChannelID(0)
{
	system = nullptr;
	FMOD::System_Create(&system);
	FMOD::Debug_Initialize(FMOD_DEBUG_LEVEL_LOG);
	system->init(128, FMOD_INIT_CHANNEL_LOWPASS, nullptr);
	FMOD_VECTOR listenerPos = { earPos.x, earPos.y, earPos.z };
	FMOD_VECTOR listenerVel = { 0.0f, 0.0f, 0.0f };
	system->set3DListenerAttributes(0, &listenerPos, &listenerVel, nullptr, nullptr);
	system->setGeometrySettings(1000.0f);
}

AudioCore::~AudioCore()
{
	//Release all sounds from AudioEngine
	for (auto& allSounds : sounds) 
	{
		allSounds.second->release();
	}
	sounds.clear();
	system->release();
	system = nullptr;
}

void AudioCore::SetEarPos(Vector3& pos, bool isRelative, Vector3 forward, Vector3 up)
{
	FMOD_VECTOR listenerPos;
	FMOD_VECTOR listenerVel;
	if (isRelative) {
		listenerPos = { earPos.x + pos.x,
					   earPos.y + pos.y,
					   earPos.z + pos.z };
		listenerVel = { 0.0f, 0.0f, 0.0f };
	}
	else {
		listenerPos = { pos.x, pos.y, pos.z };
		listenerVel = { 0.0f, 0.0f, 0.0f };
	}
	earPos = { listenerPos.x, listenerPos.y, listenerPos.z };
	FMOD_VECTOR fvecForward{};
	fvecForward.x = forward.x;
	fvecForward.y = forward.y;
	fvecForward.z = forward.z;

	FMOD_VECTOR fvecUp{};
	fvecUp.x = up.x;
	fvecUp.y = up.y;
	fvecUp.z = up.z;

	system->set3DListenerAttributes(0, &listenerPos, &listenerVel, &fvecForward, &fvecUp);
}

void AudioCore::Update(float deltaTime)
{
	std::vector<AudioChannelMap::iterator> stoppedChannels;
	//Itterate through each channel and call the channel specific update function via the state machine
	for (auto iterator = channelMap.begin(), end = channelMap.end(); iterator != end; iterator++) {
		iterator->second->Update(deltaTime);
		float directOcclusion = 0.0f; float reverbOcclusion = 0.0f;

		FMOD_VECTOR earPosVector = { earPos.x, earPos.y, earPos.z };
		FMOD_VECTOR channelPos = { iterator->second->position.x, iterator->second->position.y, iterator->second->position.y };

		system->getGeometryOcclusion(&earPosVector, &channelPos, &directOcclusion, &reverbOcclusion);

		//If the channels state is set to STOPPED, push the channel to stopped channels
		if (iterator->second->state == AudioChannel::State::STOPPED)
			stoppedChannels.push_back(iterator);
	}
	//Delete stopped channels
	for (auto& iterator : stoppedChannels) {
		channelMap.erase(iterator);
	}

	//Call FMOD system update
	system->update();
}

bool AudioCore::CheckLoaded(int soundID)
{
	auto exists = sounds.find(soundID);
	return !(exists == sounds.end());
}

void AudioCore::LoadSound(int soundID)
{
	//Ensure sound is registered
	auto data = soundDataMap.find(soundID);
	if (data == soundDataMap.end())
	{
		std::cerr << "CREATE SOUND ERROR: SOUND NOT REGISTERED\n ";
		return;
	}
	
#ifdef _DEBUG
	std::cout << "Loading Sound with ID " << soundID << std::endl;
#endif


	//Generate FMOD MODE based on the parameters passed in, applying those params to the sound 
	FMOD_MODE mode = FMOD_NONBLOCKING;

	mode |= data->second->is3D ? FMOD_3D : FMOD_2D;
	mode |= data->second->isLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
	mode |= data->second->isStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

	//Create and register the sound
	FMOD::Sound* sound = nullptr;

	FMOD_RESULT result;
	result = system->createSound(data->second->fileName.c_str(), mode, nullptr, &sound);

#ifdef _DEBUG
	std::cout << "createSound Result: " << result << std::endl;
	std::cout << "Sound: " << sound << " with filename: " << data->second->fileName << std::endl;

	if (result != FMOD_OK)
	{
		std::cerr << "CREATING SOUND ERROR: " << result << std::endl;
	}

#endif


	if (sound) 
	{
		sounds[soundID] = sound;
	}
}