#include "TimeHandler.h"

TimeHandler::TimeHandler()
{
	m_countsPerSecond = 0.0;
	m_counterStart = 0;
	m_frameCount = 0;
	m_fps = 0;
	m_frameTimeOld = 0;
}

TimeHandler::~TimeHandler()
{
}

void TimeHandler::StartTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	m_countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	m_counterStart = frequencyCount.QuadPart;
}

void TimeHandler::UpdateTimer()
{
	m_frameCount++;
	if (GetTime() > 1.0f)
	{
		m_fps = m_frameCount;
		m_frameCount = 0;
		StartTimer();
	}
	m_frameTime = GetFrameTime();

}

double TimeHandler::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - m_counterStart) / m_countsPerSecond;
}

double TimeHandler::GetFrameTime()
{
	LARGE_INTEGER currentTime;
	__int64 tickCount;

	QueryPerformanceCounter(&currentTime);

	tickCount = currentTime.QuadPart - m_frameTimeOld;
	m_frameTimeOld = currentTime.QuadPart;
	if (tickCount < 0.0f)
		tickCount = 0;
	return float(tickCount) / m_countsPerSecond;
}