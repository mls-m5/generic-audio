/*
 * jackengine.h
 *
 *  Created on: 13 jan 2014
 *      Author: mattias
 */

#ifndef JACKENGINE_H_
#define JACKENGINE_H_

#include <list>
#include <string>

#include "common.h"
#include "element.h"

class SoundEngine {
public:
	SoundEngine();
	virtual ~SoundEngine();

	static bool Init(std::string name = "");
	static bool Activate();
	static bool Close();
	static int GetBufferSize();
	static void AddElement(Element *e);
	static void RemoveElement(Element *e);
	static void SetVolume(double v);

#ifdef __ANDROID__
	static int processAndroid (sample_t *buffer, int bufferSize);
#else
	jack_port_t *inputPorts;
	jack_port_t *outputPorts;
	jack_client_t *client;

protected:
	static int process (jack_nframes_t nframes, void *arg);
#endif

	bool init(std::string name);

	bool canCapture;
	sample_t *dummyInputSample;
	std::list<Element*> elementList;
	double masterVolume;
};

#endif /* JACKENGINE_H_ */
