/*
 * common.h
 *
 *  Created on: 13 jan 2014
 *      Author: mattias
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <alsa/seq_event.h>

typedef snd_seq_event_t event_t;

#ifdef __ANDROID__

typedef float sample_t;
#define _POSIX_SOURCE //Prevent redefinition of variables in alsa headers


#include <android/log.h>
#define debug_print(x)__android_log_write(ANDROID_LOG_INFO, "microsynth", x)

#else

#include <jack/jack.h>
#define sample_t jack_default_audio_sample_t

#define debug_print(x) printf(x)

#endif
extern int SampleRate;
extern int BufferSize;

const float pi = 3.1415926535897932384626433832795028841971693;
const float pi2 = pi * 2;

#endif /* COMMON_H_ */
