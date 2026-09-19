/*
 * RingBuffer.hh
 *
 *  Created on: Jul 20, 2026
 *      Author: giova
 *
 *      A proposta dessa classe é a implementação de forma mais intuitiva de um Buffer Circular,
 *      Com o bonus da operação:
 *
 *      - Produto Interno (dot)
 *      - Convolução (conv)
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

// Libs
#include <array>   // Se baseia em array
#include "main.h"  // para tipo size_t e afins

/*
 * T: Tipo de dado do buffer
 * N: Tamanho do Buffer em sí
 *
 * Por Exemplo:
 *
 * typename T = float;
 * size_t N = 16
 *
 * RingBuffer<T, N> buffer;
 * cria um buffer de floats, de tamanho 16
 *
 * e por herdar do array, o operando de acesso direto a posição é valido
 * pro exemplo:
 *
 * T data = buffer[N]
 *
 * acessa o elemento 16 do buffer
 *
 */
template<typename T, size_t N>
class RingBuffer : public std::array<T, N>
{
	protected:
		// Guarda o indice de será gravado o proximo dado
		size_t first;
		// Indica se o Buffer está cheio
		bool full;
	public:
		// Consrutor
		RingBuffer();
		// Adiciona o dado no buffer circular
		void push(const T& x);
		// Retorna o valor no indice referente ao indice lógico circular, nao ao fisico da ordem
		inline T at(std::size_t i);
		// mesma ideia do de cima, mas o operador direto: Buffer[0]
		inline T operator[](std::size_t i);
		inline const T operator[](std::size_t i) const;
		// Calcula o produto escalar dentre o buffer atual e o de entrada
		template<typename vect> T dot(const vect& u, size_t n = N);
		// Calcula a convolucao com o de vetor de entrada, na janela n
		template<typename vect> T conv(const vect& u, size_t n = N);
		// Indica se o Buffer se encheu desde sua criacao
		bool isFull();
};

template<typename T, size_t N>
RingBuffer<T, N>::RingBuffer() : std::array<T, N>(), first(0), full(false){
}

template<typename T, size_t N>
void RingBuffer<T, N>::push(const T& x){
  std::array<T, N>::operator[](first) = x;
  first = (first + 1)%N;
  if(first == 0 && !full)
    full = true;
}

template<typename T, size_t N>
inline T RingBuffer<T, N>::at(std::size_t i){
  return std::array<T, N>::at((first + i)%N);
}

template<typename T, size_t N>
inline T RingBuffer<T, N>::operator[](std::size_t i){
  return std::array<T, N>::operator[]((first + i)%N);
}

template<typename T, size_t N>
inline const T RingBuffer<T, N>::operator[](std::size_t i) const{
  return std::array<T, N>::operator[]((first + i)%N);
}

template<typename T, size_t N>
template<typename vect>
T RingBuffer<T, N>::dot(const vect& u, size_t n){
  T prod = 0;
  for(size_t i = 0; i < n; i++)
    prod += operator[](i)*u[i];
  return prod;
}

template<typename T, size_t N>
template<typename vect>
T RingBuffer<T, N>::conv(const vect& u, size_t n)
{
	T prod = 0;
	for(size_t i = 0; i < n; i++)
		prod += operator[](N-1-i)*u[i];
	return prod;
}

template<typename T, size_t N>
bool RingBuffer<T, N>::isFull(){
  return full;
}

#endif

