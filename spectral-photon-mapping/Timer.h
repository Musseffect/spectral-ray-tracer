#pragma once
#include <windows.h>
#undef min
#undef max


template<class FloatType>
class Timer
{
	LARGE_INTEGER startingTime;
	LARGE_INTEGER frequency;
public:
	Timer();
	void restart();
	FloatType elapsed();
	FloatType elapsedAndRestart();
};

template<class FloatType>
Timer<FloatType>::Timer()
{
	restart();
}

template<class FloatType>
void Timer<FloatType>::restart()
{
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&startingTime);
}

template<class FloatType>
FloatType Timer<FloatType>::elapsed()
{
	LARGE_INTEGER endingTime;
	QueryPerformanceCounter(&endingTime);
	LARGE_INTEGER elapsedMicroseconds;
	elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
	FloatType time = FloatType(elapsedMicroseconds.QuadPart) / frequency.QuadPart;
	return time;
}

template<class FloatType>
FloatType Timer<FloatType>::elapsedAndRestart()
{
	FloatType elapsedTime = elapsed();
	restart();
	return elapsedTime;
}