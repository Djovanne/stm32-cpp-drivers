/*
 * FifoBuffer.hh
 *
 *  Created on: Aug 8, 2026
 *      Author: giova
 */

#ifndef SHARED_FIFOBUFFER_HH_
#define SHARED_FIFOBUFFER_HH_

#include <array>
#include "main.h"

template<typename T, size_t N>
class FifoBuffer : public std::array<T, N>
{
private:

	volatile size_t head;  // onde is dados sao inseridos
	volatile size_t tail;  // onde os dados sao lidos
	//size_t count; // quantos elementos há

public:
	FifoBuffer();

	// Insere um elemento no buffer (falso se estiver cheio)
	bool push(const T& x);

	// Remove e retorna o elemento mais antigo (retorna false se estiver vazio)
	bool pop(T& outValue);

	// Remove uma série de elementos do buffer
	size_t popRange(T* dest, size_t maxLen);

	// Limpa o buffer
	void clear();

	// se estra  vazio
	bool isEmpty() const;
	// se esta cheio
	bool isFull() const;
	// quanto ainda esta livre
	size_t available() const;
	// tamanho maximo
	size_t capacity() const {return N;}
};

template<typename T, size_t N>
FifoBuffer<T, N>::FifoBuffer() : std::array<T, N>(), head(0), tail(0) {}

template<typename T, size_t N>
bool FifoBuffer<T, N>::push(const T& x)
{
    size_t next_head = (head + 1) % N;
    if (next_head == tail) return false; // Buffer cheio

    std::array<T, N>::operator[](head) = x;
    head = next_head;
    return true;
}

template<typename T, size_t N>
bool FifoBuffer<T, N>::pop(T& outValue)
{
    if (head == tail) return false; // Buffer vazio

    outValue = std::array<T, N>::operator[](tail);
    tail = (tail + 1) % N;

    return true;
}

template<typename T, size_t N>
size_t FifoBuffer<T, N>::popRange(T* dest, size_t maxLen)
{
    size_t current_head = head; // Snapshot seguro
    size_t current_tail = tail;

    size_t items_available = (current_head >= current_tail) ?
                             (current_head - current_tail) :
                             (N - current_tail + current_head);

    size_t itemsToRead = (maxLen < items_available) ? maxLen : items_available;

    for (size_t i = 0; i < itemsToRead; i++) {
        dest[i] = std::array<T, N>::operator[](tail);
        tail = (tail + 1) % N;
    }

    return itemsToRead;
}

template<typename T, size_t N>
void FifoBuffer<T, N>::clear()
{
    head = 0;
    tail = 0;
}

template<typename T, size_t N>
bool FifoBuffer<T, N>::isEmpty() const
{
    return (head == tail);
}

template<typename T, size_t N>
bool FifoBuffer<T, N>::isFull() const
{
    return (((head + 1) % N) == tail);
}

template<typename T, size_t N>
size_t FifoBuffer<T, N>::available() const
{
    size_t current_head = head;
    size_t current_tail = tail;
    return (current_head >= current_tail) ? (current_head - current_tail) : (N - current_tail + current_head);
}
#endif /* SHARED_FIFOBUFFER_HH_ */
