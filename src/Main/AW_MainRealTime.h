/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Real-Time Header File                 * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


////////////////////////
// * MAIN REAL-TIME * //
////////////////////////
#pragma region Main Real-Time
class AudioWizardMainRealTime {
public:
	// * TYPE ALIASES * //
	using ChunkData = AWHAudioData::ChunkData;
	using DoubleBuffer = AWHAudioBuffer::DoubleBuffer<audioType>;
	static inline service_ptr_t<visualisation_stream_v3>& visStream = AWHAudioData::visStream;

	// * REAL-TIME DIALOG WINDOW HANDLE * //
	std::atomic<HWND> realTimeDialogHwnd = nullptr;

	// * CONFIG * //
	struct Config {
		static constexpr UINT WM_UPDATE_METRICS = WM_USER + 1;
		static constexpr UINT WM_CONFIG_CHANGED = WM_USER + 2;
		static constexpr size_t DEFAULT_CHANNELS = 2;
		static constexpr size_t DEFAULT_SAMPLE_RATE = 44100;
		static constexpr double MAX_CHUNK_DURATION_MS = 100.0;
		static constexpr int DEFAULT_REFRESH_RATE_MS = 17;
		static constexpr int MIN_REFRESH_RATE_MS = 17;
		static constexpr int DEFAULT_CHUNK_DURATION_MS = 50;
		static constexpr int MAX_REFRESH_RATE_MS = 1000;
		static constexpr int MIN_CHUNK_DURATION_MS = 17;
		static constexpr int UI_NOTIFICATION_INTERVAL_MS = 100;
	};

	// * AUDIO METRICS * //
	struct Metrics {
		std::atomic<double> momentaryLUFS = -INFINITY;
		std::atomic<double> shortTermLUFS = -INFINITY;
		std::atomic<double> RMS = -INFINITY;
		std::atomic<double> leftRMS = -INFINITY;
		std::atomic<double> rightRMS = -INFINITY;
		std::atomic<double> leftSamplePeak = -INFINITY;
		std::atomic<double> rightSamplePeak = -INFINITY;
		std::atomic<double> truePeak = -INFINITY;
		std::atomic<double> PSR = -INFINITY;
		std::atomic<double> PLR = -INFINITY;
		std::atomic<double> crestFactor = -INFINITY;
		std::atomic<double> DR = -INFINITY;
		std::atomic<double> PD = -INFINITY;
		std::atomic<double> phaseCorrelation = -INFINITY;
		std::atomic<double> stereoWidth = -INFINITY;
	}; Metrics metrics;

	// * RAW AUDIO DATA * //
	struct RawAudioData {
		DoubleBuffer buffer;
		explicit RawAudioData(size_t sample_rate, double max_chunk_duration_ms, size_t channels) :
			buffer(static_cast<size_t>(sample_rate* (max_chunk_duration_ms / 1000.0)* channels)) {}
	};
	std::unique_ptr<RawAudioData> rawAudioData = std::make_unique<RawAudioData>(
		Config::DEFAULT_SAMPLE_RATE, Config::MAX_CHUNK_DURATION_MS, Config::DEFAULT_CHANNELS
	);

	// * ANALYSIS STATE * //
	struct AnalysisState {
		AudioWizardAnalysisRealTime::RealTimeData realTimeData;
	}; AnalysisState analysis;

	// * MONITORING STATE * //
	struct MonitorState {
		std::atomic<int> monitorRefreshRateMs = Config::DEFAULT_REFRESH_RATE_MS;
		std::atomic<int> monitorChunkDurationMs = Config::DEFAULT_CHUNK_DURATION_MS;
		std::atomic<bool> wasPeakmeterActiveBefore = false;
		std::atomic<bool> isPeakmeterActive = false;
		std::atomic<bool> isRealTimeActive = false;
		std::atomic<bool> isRawAudioDataActive = false;
		std::atomic<int64_t> lastRealtimeUpdate = 0;
		std::atomic<int64_t> lastMonitoringUpdate = 0;
		std::atomic<bool> isFetching = false;
		std::thread realTimeThread;
	}; MonitorState monitor;

	// * CONSTRUCTOR & DESTRUCTOR * //
	AudioWizardMainRealTime();
	~AudioWizardMainRealTime();

	// * PUBLIC PROCESSING CONTROL * //
	void ResetMetrics();
	void SetMonitoringChunkDuration(int chunkDurationMs);
	void SetMonitoringRefreshRate(int refreshRateMs);
	void StartRealTimeMonitoring(int refreshRateMs, int chunkDurationMs);
	void StopRealTimeMonitoring();
	void StartPeakmeterMonitoring(int refreshRateMs, int chunkDurationMs);
	void StopPeakmeterMonitoring();
	void StartRawAudioMonitoring(int refreshRateMs, int chunkDurationMs);
	void StopRawAudioMonitoring();
	void GetRawAudioData(SAFEARRAY** data) const;

private:
	// * PRIVATE REAL-TIME AUDIO PROCESSING * //
	void RealTimeAudioProcessor();
	void StartRealTimeAudioProcessor(int fetchRateMs);
	void StopRealTimeAudioProcessor();
	bool IsRealTimeAudioProcessorActive() const;

	// * PRIVATE REAL-TIME METRICS PROCESSING * //
	void ProcessRealTimeMetrics(const ChunkData& data);
	void ProcessPeakmeterMetrics(const ChunkData& data);
	void ProcessRawAudioDataCapture(const ChunkData& data);
};
#pragma endregion
