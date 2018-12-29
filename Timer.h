#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>


class Timer
{
private:
	double		m_countsPerSecond, m_frameTime;
	__int64		m_counterStart, m_frameTimeOld;

	int			m_frameCount, m_fps;

public:
	Timer();
	~Timer();
	void StartTimer();
	void UpdateTimer();
	double GetTime();
	double GetFrameTime();
	int GetFPS() { return m_fps; }

};

