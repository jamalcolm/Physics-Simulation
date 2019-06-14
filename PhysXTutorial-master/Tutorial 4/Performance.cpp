#include "Performance.h"

float Performance::CurrentTime()
{
	clock_t t;
	t = clock();
	return (float(t));
}

void Performance::StartTime()
{
	sTime = CurrentTime();
}

void Performance::Reset()
{
	fpsTimes = 0;
	totalTime = 0.f;
}

float Performance::GetFPS()
{
	float duration = 1000.f /( CurrentTime() - sTime);
	fpsTimes++;
	totalTime += duration;
	return duration;
}

float Performance::GetAvgFPS()
{
	return (totalTime / (float)fpsTimes);
}

