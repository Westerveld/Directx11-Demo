#include "Timer.h"



Timer::Timer()
{
	m_countsPerSecond = 0.0;
	m_counterStart = 0;
	m_frameCount = 0;
	m_fps = 0;
	m_frameTimeOld = 0;
}


Timer::~Timer()
{
}

void Timer::StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	m_countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	m_counterStart = frequencyCount.QuadPart;
}

void Timer::UpdateTimer()
{
	m_frameCount++;
	if (GetTime() > 1.0f)
	{
		m_fps = m_frameCount;
		m_frameCount = 0;
		StartTimer();
	}

}

double Timer::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - m_counterStart) / m_countsPerSecond;
}

double Timer::GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;

	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - m_frameTimeOld;
	m_frameTimeOld = currentTime.QuadPart;
	if (tickCount < 0.0f)
		tickCount = 0.0f;
	return float(tickCount) / m_countsPerSecond;
}