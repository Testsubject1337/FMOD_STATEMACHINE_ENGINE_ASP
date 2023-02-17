#pragma once
#include <string>


struct SoundData {
	std::string fileName;
	float volume = 1;

	//When the listener is within the minimum distance of the sound source the 3D volume will be at its maximum. 
	//As the listener moves from the minimum distance to the maximum distance the sound will attenuate following the rolloff curve set. 
	//When outside the maximum distance the sound will no longer attenuate.
	float minDistance = 0;
	float maxDistance = 10;
	float virtualDistance;
	bool is3D;
	bool isLoop;
	bool isStream;
};