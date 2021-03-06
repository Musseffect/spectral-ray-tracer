#include "Random.h"

float Random::random()
{
	return fdis(gen);
}

float Random::random(float min, float max)
{
	return fdis(gen) * (max - min) + min;
}

float Random::random(float max)
{
	return fdis(gen) * max;
}

int Random::random(int min, int max)
{
	return min + (int)round((max - min) * random());
}

int Random::random(int max)
{
	return (int)round(max*random());
}

void Random::setSeed(int seed)
{
	gen.seed(seed);
}

std::random_device Random::rd;
std::mt19937 Random::gen = std::mt19937(Random::rd());
std::uniform_real_distribution<float> Random::fdis(0.0f, 1.0f);