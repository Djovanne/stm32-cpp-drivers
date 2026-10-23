/*
 * ALEFilter.h
 *
 *  Created on: Aug 4, 2026
 *      Author: giova
 */

#ifndef FILTER_ALEFILTER_H_
#define FILTER_ALEFILTER_H_

#include "NLMSFilter.h"
#include "RingBuffer.hh"
#include <cstddef>

template <size_t N, size_t DL, typename T = float>
class ALEFilter
{
private:
	// filtro adaptativo NLMS

    NLMSFilter<N, T> nlms;
	/*
	 * Buffer de linha de atraso da entrada, do modo:
	 * u(n), u(n-1), u(n-2),...,u(n-N+1),u(n-DL)
	 * lembrando que ESSE buffer apenas guarda os dados atrasados, nao os dados do filtro
	 * por exemplo, se colocarmos DL = 5, entao o real do filtro ser:
	 * y(n) = w0*u(n-5) + w1(n-6) ... + w(N-DL-1)*u(n-N+1) + w(N-DL)*u(n-N)
	 */

    RingBuffer<T, DL> delay_line;

    T f_data;

public:
    ALEFilter(T learning_rate, T regularization) : nlms(learning_rate, regularization)
    {
    	//reinicia os pesos
        reset(0.0f);
    }

    void reset(T base_value)
    {
    	// reseta o filtro adaptativo com o valor de tara
        nlms.reset(1.0f / N, base_value);

        // 0 todo o array de pesos
        for (size_t i = 0; i < DL; ++i) delay_line.push(base_value);
    }

    T update(T input_sample)
    {
        // Pega a amostra atrasada
        T delayed_sample = delay_line[0];
        // insere a atual no buffer
        delay_line.push(input_sample);

        // NLMS atualiza usando delayed_sample como entrada u(n) e input_sample como d(n) (referencia)
        return nlms.update(delayed_sample, input_sample);
    }

    void setLearningRate(T mu)
    {
    	nlms.setLearningRate(mu);
    }

    void setRegularization(T delta)
    {
    	nlms.setRegularization(delta);
    }
};



#endif /* FILTER_ALEFILTER_H_ */
