#pragma once
#include <string>

#include <chrono>
#define CHRONO          (Chrono::GetInstance())
#define DELTA_TIME      (CHRONO->GetDeltaTime())
#define TOTAL_TIME      (CHRONO->GetTotalTime())

class Chrono
{
private:
    static Chrono* s_pInstance;

private:
    std::chrono::steady_clock::time_point m_StartPoint; // Time point of the last Restart/Creation of Chrono
    std::chrono::steady_clock::time_point m_LastPoint;  // Time point of the last frame
    double m_DeltaTime;                                 // Time between previous frame and now
    std::chrono::steady_clock::time_point m_BeginFPS;   // Time point of the previous FPS result
    int m_Frame;                                        // Number of frame processed in 1.0s
    float m_FPS;                                        // Number of FPS
    double m_CapFPS;

private:
    Chrono();
    ~Chrono();

public:
    static Chrono* GetInstance();
    static void Release();

    void Restart();

    void Update();

    float GetDeltaTime() const { return m_DeltaTime; }
    float GetTotalTime() const { return std::chrono::duration<float>(m_LastPoint - m_StartPoint).count(); }
    
    std::string GetCurrentTimeAsString() const;
    std::string GetCurrentTimeAsStringWithMS() const;

    void SetFPSCap(float maxFPS = -1) { m_CapFPS = 1.f / maxFPS; }
    std::string ShowFPS() const { return "FPS: " + std::to_string(m_FPS); }
    float GetFPS() const { return m_FPS; }
};
