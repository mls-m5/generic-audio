/*
 * instrument.h
 *
 *  Created on: 13 jan 2014
 *      Author: mattias
 */

#ifndef INSTRUMENT_H_
#define INSTRUMENT_H_
#include <list>

#include "common.h"

enum PortamentoStyle{
	psMonophonic = 0,
	psPolyphonic = 1
};

class Element{
public:
	Element() {};
	virtual ~Element(){};

	virtual void process(sample_t* in, sample_t* out, int bufferSize) = 0;
	virtual void controlSignal() {}; //hint to send controlMessages
};

class Envelope{
public:
	Envelope():enabled(true){};
	virtual ~Envelope(){};

	//Process
	//Return true is the tone is still alive
	virtual bool process(sample_t *buf, int bufferSize, int state) = 0;
	virtual Envelope *clone() const = 0;
	virtual int getValueCount() const = 0;
	virtual void setValue(int index, double value) = 0;

	bool enabled;
};

#endif /* INSTRUMENT_H_ */
