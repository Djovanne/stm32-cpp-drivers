/*
 * Timer.hh
 *
 *  Created on: Sep 27, 2025
 *      Author: giova
 */

#include "main.h"

#ifdef HAL_TIM_MODULE_ENABLED

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

class Timer
{
public:

    Timer(TIM_HandleTypeDef* htim);
    ~Timer();

    HAL_StatusTypeDef start();
	HAL_StatusTypeDef stop();
	void reset();

	//consegue o tempo atual e passado desde um tick específico, nessa ordem
	uint32_t get_tick();
	uint32_t get_elapsed_time(uint32_t start_tick);

	//converte o tempo atual
    float ticks_ms(uint32_t ticks);
    float ticks_us(uint32_t ticks);

    //conta o tempo de forma bloqueante
    void delay_ms(uint32_t ms);
    void delay_us(uint32_t us);

    uint32_t get_timer_freq();

private:
    TIM_HandleTypeDef* htim;    // Ponteiro para o handle do timer HAL
    uint32_t timer_freq_hz;     // Frequência de contagem do timer (calculada)

    uint32_t calculate_timer_freq();
};

#endif /* INC_HARDWARETIMER_H_ */

#endif
