#include "pch.h"
#include "Editor/Benchmark.h"

#include <numeric>
namespace GALAXY
{
	namespace Editor
	{
		void Benchmark::StartBenchmark()
		{
			m_benchmarkState = BenchmarkState::Running;
			m_currentTime = 0;
			m_benchmarkFps.clear();
			m_averageFPS = 0;
		}

		void Benchmark::UpdateBenchmark(const float deltaTime)
		{
			if (m_benchmarkState == BenchmarkState::Finished)
				return;

			m_currentTime += deltaTime;

			if (std::fmod(m_currentTime, m_settings.updateFrequency) >= deltaTime)
				return;

			m_benchmarkFps.push_back(1.f / deltaTime);
			auto const count = static_cast<float>(m_benchmarkFps.size());
			m_averageFPS = std::reduce(m_benchmarkFps.begin(), m_benchmarkFps.end()) / count;

			if (m_benchmarkFps.size() - m_averageFpsIndex >= 100)
			{
				const size_t newCount = m_benchmarkFps.size() - m_averageFpsIndex;
				const float averageFPS = std::reduce(m_benchmarkFps.begin() + m_averageFpsIndex, m_benchmarkFps.end()) / newCount;
				m_benchmarkFps.erase(m_benchmarkFps.begin() + m_averageFpsIndex, m_benchmarkFps.end());
				m_benchmarkFps.push_back(averageFPS);
				m_averageFpsIndex++;
			}

			if (m_currentTime >= m_settings.time)
			{
				EndBenchmark();
			}
		}

		void Benchmark::EndBenchmark()
		{
			m_benchmarkState = BenchmarkState::Finished;
			PrintLog("Benchmark ended with average of %f FPS", m_averageFPS);
		}
	}
}
