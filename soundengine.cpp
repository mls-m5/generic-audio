/*
 * jackengine.cpp
 *
 *  Created on: 13 jan 2014
 *      Author: mattias
 */

#include "soundengine.h"
#include <stdio.h>
#include <errno.h>
#include <jack/jack.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <list>

static SoundEngine *globalEngine = 0;
int SampleRate = 44100; //Temporary value
int BufferSize = 100; //Temporary value

SoundEngine::SoundEngine():
		client(0),
		inputPorts(0),
		outputPorts(0),
		canCapture(true),
		dummyInputSample(0),
		masterVolume(1) {

}

SoundEngine::~SoundEngine() {
	if (dummyInputSample){
		delete dummyInputSample;
	}
}



/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void
jack_shutdown (void *arg)
{
	exit (1);
}

bool SoundEngine::Init(std::string name) {
	if (globalEngine){
		return true;
	}
	else{
		globalEngine = new SoundEngine();
		return globalEngine->init(name);
	}

}

void SoundEngine::AddElement(Element* e) {
	globalEngine->elementList.push_back(e);
}

void SoundEngine::RemoveElement(Element* e) {
	globalEngine->elementList.remove(e);
}

int
SoundEngine::process (jack_nframes_t nframes, void *arg)
{
	jack_default_audio_sample_t *in, *out;

	if (globalEngine->canCapture){
		in = (jack_default_audio_sample_t*)jack_port_get_buffer (globalEngine->inputPorts, nframes);
	}
	else{
		if (globalEngine->dummyInputSample == 0){
			//Create empty dummy buffer
			globalEngine->dummyInputSample = new jack_default_audio_sample_t[nframes];
		}
		in = globalEngine->dummyInputSample;
		for (int i = 0; i < nframes; ++i){
			in[i] = 0;
		}
	}
	out = (jack_default_audio_sample_t*)jack_port_get_buffer (globalEngine->outputPorts, nframes);

	bool firstElement = true;
	for (auto element: globalEngine->elementList){
		if (firstElement){
			firstElement = 0;
		}
		else{
			memcpy(in, out,
					sizeof (jack_default_audio_sample_t) *  nframes);
		}
		element->process(in, out, nframes);
	}
	if (globalEngine->masterVolume != 1){
		const auto volume = globalEngine->masterVolume;
		for (int i = 0; i < nframes; ++i){
			out[i] *= volume;
		}
	}

	for (auto element: globalEngine->elementList){
		element->controlSignal();
	}
	return 0;
}

bool SoundEngine::init(std::string name) {
	const char **ports;
	const char *server_name = NULL;
	std::string clientName = name;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	/* open a client connection to the JACK server */

	client = jack_client_open (clientName.c_str(), options, &status, server_name);
	if (client == NULL) {
		fprintf (stderr, "jack_client_open() failed, "
				"status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf (stderr, "Unable to connect to JACK server\n");
		}
		exit (1);
	}
	if (status & JackServerStarted) {
		fprintf (stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		clientName = jack_get_client_name(client);
		fprintf (stderr, "unique name `%s' assigned\n", clientName.c_str());
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	 */

	jack_set_process_callback (globalEngine->client, process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	 */

	jack_on_shutdown (globalEngine->client, jack_shutdown, 0);

	/* display the current sample rate.
	 */

	//	printf ("engine sample rate: %" PRIu32 "\n",
	//		jack_get_sample_rate (client));

	/* create two ports */

	inputPorts = jack_port_register (globalEngine->client, "input",
			JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsInput, 0);
	inputPorts = jack_port_register (globalEngine->client, "input2",
			JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsInput, 0);
	outputPorts = jack_port_register (globalEngine->client, "output",
			JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsOutput, 0);
	outputPorts = jack_port_register (globalEngine->client, "output2",
			JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsOutput, 0);

	if ((inputPorts == NULL) || (outputPorts == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		exit (1);
	}

	SampleRate = jack_get_sample_rate(globalEngine->client);
	BufferSize = jack_get_buffer_size(globalEngine->client);

	return true;
}

bool SoundEngine::Activate() {
	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

//	jack_set_buffer_size(globalEngine->client, 256);

	if (jack_activate (globalEngine->client)) {
		fprintf (stderr, "cannot activate client");
		exit (1);
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	auto ports = jack_get_ports (globalEngine->client, NULL, NULL,
			JackPortIsPhysical|JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		globalEngine->canCapture = false;
//		exit (1);
	}
	else{
		if (jack_connect (globalEngine->client, ports[0], jack_port_name (globalEngine->inputPorts))) {
			fprintf (stderr, "cannot connect input port 1\n");
		}
		free (ports);
	}

	ports = jack_get_ports (globalEngine->client, NULL, NULL,
			JackPortIsPhysical|JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		exit (1);
	}

	if (jack_connect (globalEngine->client, jack_port_name (globalEngine->outputPorts), ports[0])) {
		fprintf (stderr, "cannot connect output port 1\n");
	}

	if (jack_connect (globalEngine->client, jack_port_name (globalEngine->outputPorts), ports[1])) {
		fprintf (stderr, "cannot connect output port 2\n");
	}

	free (ports);

	/* keep running until stopped by the user */
}

int SoundEngine::GetBufferSize() {
	return BufferSize;
}

bool SoundEngine::Close() {
	/* this is never reached but if the program
	   had some other way to exit besides being killed,
	   they would be important to call.
	 */
	jack_client_close (globalEngine->client);
}

void SoundEngine::SetVolume(double v) {
	if (globalEngine){
		globalEngine->masterVolume = v;
	}
}
