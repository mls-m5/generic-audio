/*
 * A template project that initializes audio an play a simple sound
 *
 * Mattias Lasersköld
 */

#include "soundengine.h"
#include "common.h"

#include <string>
#include <math.h>
#include <unistd.h> //For sleep()

class Sine: public Element{
public:
	Sine(): phase(0) {

	}
	virtual void process(sample_t* in, sample_t* out, int bufferSize){
		double step = 1. / SampleRate;
		for (int i = 0; i < bufferSize; ++i){
			out[i] = in[i] + sin(phase * 880) / 10;
			phase += step;
		}
	}

	double phase;
};

int
main (int argc, char *argv[])
{
	SoundEngine::Init("generic-audio");

	auto element = new Sine();
	SoundEngine::AddElement(element);

	SoundEngine::Activate();

	sleep(-1);

	SoundEngine::Close();
	exit (0);
}
