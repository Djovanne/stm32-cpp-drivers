/*
 * Timer.cpp
 *
 *  Created on: Jan 7, 2026
 *      Author: giova
 */

#include <Timer.hh>

#ifdef HAL_TIM_MODULE_ENABLED

Timer::Timer(TIM_HandleTypeDef* htim)
{
	this->htim = htim;
	this->timer_freq_hz = get_timer_freq();
	stop();
}

Timer::~Timer() {}

HAL_StatusTypeDef Timer::start()
{
	return HAL_TIM_Base_Start(htim);
}

HAL_StatusTypeDef Timer::stop()
{
	return HAL_TIM_Base_Stop(htim);
}

void Timer::reset()
{
    __HAL_TIM_SET_COUNTER(htim, 0);
}

uint32_t Timer::get_tick()
{
    return __HAL_TIM_GET_COUNTER(htim);
}

uint32_t Timer::get_elapsed_time(uint32_t start_tick)
{
    uint32_t current = __HAL_TIM_GET_COUNTER(htim);
    if (current >= start_tick) {
        return current - start_tick;
    } else {
        return (current + htim->Instance->ARR + 1) - start_tick;
    }
}

float Timer::ticks_ms(uint32_t ticks)
{
    if (timer_freq_hz == 0) return 0.0f;
    return ((float)ticks / timer_freq_hz) * 1000.0f;
}

float Timer::ticks_us(uint32_t ticks)
{
    if (timer_freq_hz == 0) return 0.0f;
    return ((float)ticks / timer_freq_hz) * 1000000.0f;
}

void Timer::delay_ms(uint16_t ms)
{
	if (ms == 0) return;
	    uint32_t start = get_tick();
	    uint32_t wait_ticks = (uint32_t)(((uint64_t)ms * timer_freq_hz) / 1000);

	    while (get_elapsed_time(start) < wait_ticks);
}

void Timer::delay_us(uint32_t us)
{
    if (us == 0) return;
    uint32_t start = get_tick();
    uint32_t wait_ticks = (uint32_t)(((uint64_t)us * timer_freq_hz) / 1000000);

    if (wait_ticks == 0) wait_ticks = 1;

    while (get_elapsed_time(start) < wait_ticks);
}

uint32_t Timer::get_timer_freq()
{
    return this->timer_freq_hz;
}

uint32_t Timer::calculate_timer_freq()
{
    TIM_TypeDef* tim = htim->Instance;
    uint32_t pclk = 0;
    uint32_t timer_clock = 0;

    pclk = HAL_RCC_GetPCLK1Freq();
	//apb_prescaler = (RCC->CFGR & RCC_CFGR_PPRE1);

	if (pclk != HAL_RCC_GetHCLKFreq())
		timer_clock = pclk * 2;
	else
		timer_clock = pclk;
	uint32_t prescaler_value = tim->PSC;

	return timer_clock / (prescaler_value + 1);
}

#endif
