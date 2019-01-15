#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include <d3dx11.h>
#include <windows.h>
#include <dxerr.h>
#include <xnamath.h>


class TimeHandler
{
private:
	double		m_countsPerSecond, m_frameTime;
	__int64		m_counterStart, m_frameTimeOld;

	int			m_frameCount, m_fps;

public:
	TimeHandler();
	~TimeHandler();
	void StartTimer();
	void UpdateTimer();
	double GetTime();
	double GetFrameTime();
	int GetFPS() { return m_fps; }
	double GetDeltaTime() { return m_frameTime; }

};