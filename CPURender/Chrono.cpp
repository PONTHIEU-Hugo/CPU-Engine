#include "Chrono.h"
#include <sstream>
#include <iomanip>

Chrono* Chrono::s_pInstance = nullptr;

Chrono::Chrono() 
{ 
	Restart();
}

Chrono* Chrono::GetInstance()
{
	if (s_pInstance == nullptr) s_pInstance = new Chrono();
	return s_pInstance;
}

void Chrono::Release()
{
	if (s_pInstance != nullptr)
	{
		delete s_pInstance;
		s_pInstance = nullptr;
	}
}

Chrono::~Chrono()
{
	s_pInstance = nullptr;
}

void Chrono::Restart()
{
	m_StartPoint = std::chrono::steady_clock::now();
	m_LastPoint = m_StartPoint;
	m_DeltaTime = 0.f;
	m_BeginFPS = std::chrono::steady_clock::now();
	m_Frame = 0;
	m_FPS = 0.f;
}

void Chrono::Update()
{
	auto now = std::chrono::steady_clock::now();
	m_DeltaTime = std::chrono::duration<float>(now - m_LastPoint).count();
	m_LastPoint = now;

	m_Frame++;
	float elapsed = std::chrono::duration<float>(now - m_BeginFPS).count();
	if (elapsed >= 1.0f)
	{
		m_FPS = m_Frame / elapsed;
		m_Frame = 0;
		m_BeginFPS = now;
	}
}

std::string Chrono::GetCurrentTimeAsString() const
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	tm time{};
	
	std::ostringstream oss;
	if (localtime_s(&time, &now_c) == 0)
	{
		oss << '['
			<< std::setfill('0') << std::setw(2) << time.tm_hour << ':'
			<< std::setfill('0') << std::setw(2) << time.tm_min << ':'
			<< std::setfill('0') << std::setw(2) << time.tm_sec << ']';
	}
	else
	{
		oss << "[--:--:--]";
	}
	return oss.str();
}

std::string Chrono::GetCurrentTimeAsStringWithMS() const
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	tm time;	

	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

	std::ostringstream oss;
	if (localtime_s(&time, &now_c) == 0)
	{
		oss << '['
			<< std::setfill('0') << std::setw(2) << time.tm_hour << ':'
			<< std::setfill('0') << std::setw(2) << time.tm_min << ':'
			<< std::setfill('0') << std::setw(2) << time.tm_sec << ':'
			<< std::setfill('0') << std::setw(3) << millis << ']';
	}
	else
	{
		oss << "[--:--:--:--]";
	}
	return oss.str();
}