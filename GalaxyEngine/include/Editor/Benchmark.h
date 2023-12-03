#pragma once
#include "GalaxyAPI.h"
namespace GALAXY 
{
	namespace Editor
	{
		struct BenchmarkSettings
		{
			float time = 0.0f;
			float updateFrequency = 0.0f;
		};

		enum class BenchmarkState
		{
			None,
			Running,
			Finished
		};

		class Benchmark
		{
		public:
			Benchmark() = default;

			void StartBenchmark();
			void UpdateBenchmark(float deltaTime);

			inline bool IsRunning() const { return m_benchmarkState == BenchmarkState::Running; }
			inline float GetAverageFPS() const { return m_averageFPS; }

			void SetBenchmarkSettings(const BenchmarkSettings& settings) { m_settings = settings; }
		private:
			void EndBenchmark();

		private:
			BenchmarkSettings m_settings;
			std::vector<float> m_benchmarkFps;

			float m_currentTime = 0.0f;
			float m_averageFPS = 0.0f;

			size_t m_averageFpsIndex = 0;

			BenchmarkState m_benchmarkState = BenchmarkState::Finished;
		};
	}
}
