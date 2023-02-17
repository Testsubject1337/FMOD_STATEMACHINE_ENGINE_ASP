#include "AudioEngine.h"
#include "AudioChannel.h"
#include "AudioCore.h"


// ***** create Pointer to make AudioCore usable ***** //
AudioCore* core = nullptr; 

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
		"Current STATE " << static_cast<int>(sampledata->second->state) << "\n"
		"---------------\n";

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
#ifdef _DEBUG
		std::cout << "AUDIO ENGINE: Channel found. Setting new Position of ChannelID **" << channelID <<  "** to\n";
		std::cout << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
		std::cout << "isRelative = " << isRelative << std::endl;
		std::cout << "---------------\n";
#endif
		auto& channel = exists->second;
		channel->SetUpdateFlag(AudioChannel::UpdateFlag::POSITION, true);
		if (isRelative) {
			Vector3 newPos =
			{
				channel->position.x + pos.x,
				channel->position.y + pos.y,
				channel->position.z + pos.z
			};
			channel->position = newPos;
		}
		else if (!isRelative)
		{
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
#ifdef _DEBUG

		std::cout << "Audio Engine: Setting virtual Flag = true for ChannelID " << channelID << std::endl;
		std::cout << "---------------\n";
#endif
		exists->second->virtualFlag = true;
	}
}

void AudioEngine::DeVirtualizeAudioChannel(int channelID)
{
#ifdef _DEBUG
	std::cout << "AudioEngine: Calling Devirtualiziation on ChannelID " << channelID << std::endl;
	std::cout << "---------------\n";
#endif
	auto exists = core->channelMap.find(channelID);
	if (exists != core->channelMap.end()) 
	{
		exists->second->virtualFlag = false;
	}

}

bool AudioEngine::isChannelVirtual(int channelID)
{
	//Get Channel
	auto exists = core->channelMap.find(channelID);

	if (exists->second->state == AudioChannel::State::VIRTUAL)
	{
#ifdef _DEBUG
		std::cout << "Channel is Virtual.\n";
		std::cout << "---------------\n";
#endif // _DEBUG

		return true;
	}
	else
	{
#ifdef _DEBUG
		std::cout << "Channel is NOT Virtual.\n";
		std::cout << "---------------\n";
#endif // _DEBUG
		return false;
	}
}

std::string AudioEngine::getChannelState(int channelID)
{
	//Get Channel
	auto exists = core->channelMap.find(channelID);

	int state = static_cast<int>(exists->second->state);

	switch (state)
	{
	case 0:
		return "INIT";
	case 1:
		return "TOPLAY";
	case 2:
		return "PLAYING";
	case 3:
		return "LOADING";
	case 4:
		return "PREPLAYING";
	case 5:
		return "VIRTUALIZING";
	case 6:
		return "VIRTUAL";
	case 7:
		return "STOPPING";
	case 8:
		return "STOPPED";


	}


}

float AudioEngine::getChannelVolume(int channelID)
{
	//Get Channel
	auto exists = core->channelMap.find(channelID);

	return exists->second->volume;
}

void AudioEngine::moveSoundInCircle(int channelID, int durationInSeconds, int radius)
{
	const int StepsPerSecond = 30; // Steps per second
	const float StepAngle = 2 * 3.14159265359 / (StepsPerSecond * durationInSeconds); // Angle per step
	const int TotalSteps = StepsPerSecond * durationInSeconds; // Total number of steps
	const float FinalAngle = 2 * 3.14159265359; // Final angle to reach start position



	// Initial position of the vector
	Vector3 startPosition = { 0.0f, 0.0f, radius };
	Vector3 positionCircle = startPosition;

	//Move there
	moveSoundToPosition(channelID, startPosition, 0.5f);

	// Loop for circular motion
	for (int step = 0; step < TotalSteps; step++)
	{
		//Feed Console
		system("CLS");
		std::cout << "MOVE SOUND IN CIRCLE" << std::endl;

		// Compute the new position of the vector
		float x = std::cos(StepAngle * step) * startPosition.x - std::sin(StepAngle * step) * startPosition.z;
		float z = std::sin(StepAngle * step) * startPosition.x + std::cos(StepAngle * step) * startPosition.z;
		positionCircle = { x, 0.0f, z };

		// Set the new position of the sound source
		SetAudioChannelPosition(channelID, positionCircle, false);
		Update(1);

		// Wait for the next step
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / StepsPerSecond));

		//Feed Console
		std::cout << "---------------\n\n";

	}

	// Move back to the start position
	system("CLS");
	std::cout << "MOVE SOUND IN CIRCLE" << std::endl;
	std::cout << "---------------\n\n";
	SetAudioChannelPosition(channelID, startPosition, false);
	Update(1);

}


void AudioEngine::moveSoundToPosition(int channelID, Vector3 targetPosition, int durationInSeconds)
{
	const int StepsPerSecond = 30; // Steps per second
	const int TotalSteps = StepsPerSecond * durationInSeconds; // Total number of steps

	// Get the starting position of the sound source
	auto exists = core->channelMap.find(channelID);
	Vector3 startPosition = exists->second->position;

	// Calculate the distance to the target position
	Vector3 direction = targetPosition - startPosition;
	float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);

	// Calculate the step size and direction for each step
	Vector3 stepSize = direction / TotalSteps;

	// Record the start time
	auto startTime = std::chrono::high_resolution_clock::now();

	// Loop for moving to the target position
	for (int step = 0; step < TotalSteps; step++)
	{
		//Reset Console
		system("CLS");
		std::cout << "MOVE SOUND TO VECTOR " << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << std::endl;

		// Calculate the new position of the sound source
		Vector3 newPosition = startPosition + stepSize * step;

		// Set the new position of the sound source
		SetAudioChannelPosition(channelID, newPosition, false);

		// Record the current time
		auto currentTime = std::chrono::high_resolution_clock::now();

		// Calculate the time difference since the start of the loop
		auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

		// Call the Update function with the current time difference
		Update(timeDifference);

		// Wait for the next step
		std::this_thread::sleep_for(std::chrono::milliseconds(1000 / StepsPerSecond));

		//Feed Console
		std::cout << "---------------\n\n";
	}

	// Set the final position of the sound source to the target position
	system("CLS");
	std::cout << "MOVE SOUND TO VECTOR " << targetPosition.x << ", " << targetPosition.y << ", " << targetPosition.z << std::endl;
	std::cout << "---------------\n\n";
	SetAudioChannelPosition(channelID, targetPosition, false);

	// Record the current time
	auto currentTime = std::chrono::high_resolution_clock::now();

	// Calculate the time difference since the start of the loop
	auto timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
}