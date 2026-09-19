/*
 * NLMSFilter.h
 *
 *  Created on: May 1, 2026
 *      Author: giova
 */

#ifndef APP_FILTER_NLMSFILTER_H_
#define APP_FILTER_NLMSFILTER_H

#include "main.h"
#include <array>
#include <cstddef>
#include "RingBuffer.hh"

/* Um filtro adaptativo tem o formato de um regressor linear multidimensional FIR (como ilustrado abaixo)
 *
 * Y(n) = w0*u(n) + w1*u(n-1) + ... + w(N-1)*u(n-N+1) + w(N)*u(n-N)
 * sendo Y(n) a saída estimada pelo filtro
 * w(i) a matriz de pesos do filtro
 * u(n) a N-ésima entrada do filtro (do presente n para o passado N)
 * N tamanho do buffer/quuantidade de amostras passadas/ Ordem do filtro
*/
// T: Tipo de dado, N Ordem do filtro
template <size_t N, typename T = float>

class NLMSFilter
{
private:
	std::array<T, N> weights;    // Matriz de pesos do filtro (w)
	//std::array<T, N> delay_line; // Buffer de entrada (u) - Comtem dados do presente e passado apenas
	RingBuffer<T, N> delay_line;   // Buffer de entrada (u) - Comtem dados do presente e passado apenas


	T mu;    // Taxa de Aprendizado (igual neurônio artificial)
	T delta; // Fator de Regularizção (para não dividir por zero na normalização)

	T last_error;
	T last_output;

	//size_t head_intex; // indexador do buffer circular


public:
	/*
	 * @brief Construtod do Filtro NLMS
	 * @param learning_rate: Passo de adaptação (dentre 0 e 1)
	 * @param regularization: Evitar divizão por zero
	 */
	NLMSFilter(T learning_rate = 0.1f, T regularization = 1e-6f);
	//virtual ~NLMSFilter();

	/**
	 * @brief Processa a amostra e atualiza os pesos adaptativos
	 * @param input_sample Amostra inserida na linha de atraso
	 * @param desired_sample Valor desejado (referência de erro)
	 * @return O valor estimado/filtrado (y)
	 */
	T update(T input_sample, T desired_sample);

	/**
	 * @brief Reinicia os pesos e zera os buffers
	 * @param default_weight Valor inicial para os pesos
	 */
	void reset(T default_weight = 0.0f, T default_buffer_value = 0.0f);

	// ==========================================
	// Setters para calibração
	// ==========================================
	void setLearningRate(T new_mu);
	void setRegularization(T new_delta);

	// ==========================================
	// Getters para monitoramento / Filtro Casado
	// ==========================================
	T getLastError() const;
	T getLastOutput() const;

	std::array<T, N> getOrderedBuffer() const;
};

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template <size_t N, typename T>
NLMSFilter<N, T>::NLMSFilter(T learning_rate, T regularization)
{
	this->mu = learning_rate;
	this->delta = regularization;
	this->last_error = 0.0f;
	this->last_output = 0.0f;
	//this->head_intex = 0;

	reset(0.0f); //Pesos com valor 0 inicialmente
}

////////////////////////////////////////////////////////////////////////////////////////
// Atualização dos pesos do filtro, retorna y
// input_sample = entrada atual
// desired_sample = saida alvo (ideal)

template <size_t N, typename T>
T NLMSFilter<N, T>::update(T input_sample, T desired_sample)
{
	//% Atualiza o vetor de entrada, inserindo o novo dado na pilha
	//delay_line[head_intex] = input_sample;
	delay_line.push(input_sample);

	T y = 0.0f;  		// Saida que será estimada
	T norm_sq = 0.0f;   // Normalizacao da entrada

    // Calcula o valor da saida com base nos pesos (convolucao):
    // Y(n) = Wo*u(n) + ... + WN*u(n-N)

	y = delay_line.conv(weights);

    // A Energia do valor de entrada
    // E = u(n)*u(n)
	norm_sq = delay_line.dot(delay_line);

	// Erro da predição dentre o estimado e real
	T error = desired_sample - y;
	// Agora vem o filtro NLMS em sí:
	// Calcula o fator de aprendizado
	T update_factor = (mu*error)/(norm_sq + delta);

	// Atualização dos pesos do filtro
	for (size_t i = 0; i < N; i++)
		weights[i] += update_factor * delay_line[N-1-i];


	// atualiza as variáveis de estado
	last_error = error;
	last_output = y;

	return y;
}

// Reseta todos os pesos e o Buffer
template <size_t N, typename T>
void NLMSFilter<N, T>::reset(T default_weight_value, T default_buffer_value)
{
    for (size_t i = 0; i < N; ++i)
    {
    	delay_line.push(default_buffer_value);
    	weights[i] = default_weight_value;
    }

    last_error = 0.0f;
    last_output = 0.0f;
}

template <size_t N, typename T>
void NLMSFilter<N, T>::setLearningRate(T new_mu) {
    mu = new_mu;
}

template <size_t N, typename T>
void NLMSFilter<N, T>::setRegularization(T new_delta) {
    delta = new_delta;
}

template <size_t N, typename T>
T NLMSFilter<N, T>::getLastError() const {
    return last_error;
}

////////////////////// Ultima saida do filtro ///////////////////////////

template <size_t N, typename T>
T NLMSFilter<N, T>::getLastOutput() const {
    return last_output;
}

///////////////// Retorna a matriz de pesos do filtro ////////////////////

template <size_t N, typename T>
std::array<T, N> NLMSFilter<N, T>::getOrderedBuffer() const
{
    std::array<T, N> ordered;

    for (size_t i = 0; i < N; ++i)
    	ordered[i] = delay_line[i];

    return ordered;
}

#endif /* APP_FILTER_NLMSFILTER_H_ */
