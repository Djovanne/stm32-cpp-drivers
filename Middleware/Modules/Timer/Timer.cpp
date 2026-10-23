/*
 * TImer.cpp
 *
 *  Created on: Jul 27, 2026
 *      Author: giova
 */

#include "Timer.h"

Timer::Timer(uint32_t interval)
{
	this->interval = interval;
}

// Seta novo intervalo de tempo
void Timer::setInterval(uint32_t newInterval)
{
	this->interval = newInterval;
}

// Resgata o tempo atual
uint32_t Timer::getInterval()
{
	return interval;
}

// Checa se o intervalo passou e reinicia caso tenha
bool Timer::checkAndReset()
{
	uint32_t now = HAL_GetTick();
	if(now - lastTime >= interval)
	{
		lastTime = now;
		return true;
	}
	return false;
}

// Checa se o intervalo passo, não reinicia
bool Timer::hasElapsed() const
{
	uint32_t now = HAL_GetTick();
	return (bool)(now - lastTime >= interval);
}
bool Timer::hasElapsed(uint32_t ms) const
{
	uint32_t now = HAL_GetTick();
	return (bool)(now - lastTime >= ms);
}

// Reinicia o Timer
void Timer::reset()
{
	this->lastTime = HAL_GetTick();
}
