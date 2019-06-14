#pragma once
#include <stdio.h>      
#include <time.h>       
#include <math.h> 

class Performance
{
public:
	float CurrentTime();
	void StartTime();
	void Reset();
	float GetFPS();
	float GetAvgFPS();

	float sTime;
	int fpsTimes;
	float totalTime;
};