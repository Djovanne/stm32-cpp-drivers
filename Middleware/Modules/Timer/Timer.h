/*
 * TImer.h
 *
 *  Created on: Jul 27, 2026
 *      Author: giova
 */

#ifndef SHARED_TIMER_TIMER_H_
#define SHARED_TIMER_TIMER_H_

#include "main.h"
#include <stdbool.h>

class Timer
{
public:
	Timer(uint32_t interval);

	// Seta novo intervalo de tempo
	void setInterval(uint32_t newInterval);

	// Resgata o tempo atual
	uint32_t getInterval();

	// Checa se o intervalo passou e reinicia caso tenha
	bool checkAndReset();

	// Checa se o intervalo passo, não reinicia
	bool hasElapsed() const;
	bool hasElapsed(uint32_t ms) const;

	// Reinicia o Timer
	void reset();

private:

	uint32_t interval = 0;
	uint32_t lastTime = 0;
};

#endif /* SHARED_TIMER_TIMER_H_ */
