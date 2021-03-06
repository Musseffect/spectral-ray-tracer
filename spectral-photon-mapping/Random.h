#pragma once
#include <random>

class Random
{
	static std::random_device rd;
	static std::mt19937 gen;
	static std::uniform_real_distribution<float> fdis;
public:
	static float random();
	static float random(float min, float max);
	static float random(float max);
	static int random(int min, int max);
	static int random(int max);
	static void setSeed(int seed);
};