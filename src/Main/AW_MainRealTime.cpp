/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Real-Time Source File                 * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_MainRealTime.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardMainRealTime::AudioWizardMainRealTime() {
}

AudioWizardMainRealTime::~AudioWizardMainRealTime() {
	StopRealTimeAudioProcessor();
}
#pragma endregion


///////////////////////////////////
// * PUBLIC PROCESSING CONTROL * //
///////////////////////////////////
#pragma region Public Processing Control
void AudioWizardMainRealTime::ResetMetrics() {
	metrics.momentaryLUFS = -INFINITY;
	metrics.shortTermLUFS = -INFINITY;
	metrics.leftRMS = -INFINITY;
	metrics.rightRMS = -INFINITY;
	metrics.leftSamplePeak = -INFINITY;
	metrics.rightSamplePeak = -INFINITY;
	metrics.truePeak = -INFINITY;
	metrics.RMS = -INFINITY;
	metrics.PSR = -INFINITY;
	metrics.PLR = -INFINITY;
	metrics.DR = -INFINITY;
	metrics.PD = -INFINITY;
	metrics.crestFactor = -INFINITY;
	metrics.phaseCorrelation = -INFINITY;
	metrics.stereoWidth = -INFINITY;
}

void AudioWizardMainRealTime::SetMonitoringChunkDuration(int chunkDurationMs) {
	chunkDurationMs = std::clamp(chunkDurationMs, Config::MIN_CHUNK_DURATION_MS, monitor.monitorRefreshRateMs.load(std::memory_order_seq_cst));
	monitor.monitorChunkDurationMs.store(chunkDurationMs, std::memory_order_release);

	HWND hWnd = realTimeDialogHwnd.load(std::memory_order_seq_cst);
	if (hWnd) ::PostMessage(hWnd, Config::WM_CONFIG_CHANGED, 0, 0);
}

void AudioWizardMainRealTime::SetMonitoringRefreshRate(int refreshRateMs) {
	refreshRateMs = std::clamp(refreshRateMs, Config::MIN_REFRESH_RATE_MS, Config::MAX_REFRESH_RATE_MS);
	monitor.monitorRefreshRateMs.store(refreshRateMs, std::memory_order_release);
	AudioWizardSettings::monitorDisplayRefreshRate = monitor.monitorRefreshRateMs;

	HWND hWnd = realTimeDialogHwnd.load(std::memory_order_seq_cst);
	if (hWnd) ::PostMessage(hWnd, Config::WM_CONFIG_CHANGED, 0, 0);
}

void AudioWizardMainRealTime::StartRealTimeMonitoring(int refreshRateMs, int chunkDurationMs) {
	if (monitor.isRealTimeActive.load()) return;

	analysis.realTimeData = AudioWizardAnalysisRealTime::RealTimeData();

	monitor.wasPeakmeterActiveBefore.store(monitor.isPeakmeterActive.load(), std::memory_order_release);
	monitor.isRealTimeActive.store(true, std::memory_order_release);
	monitor.isPeakmeterActive.store(false, std::memory_order_release);
	monitor.isUIMessagePending.store(false, std::memory_order_release);

	SetMonitoringRefreshRate(refreshRateMs);
	SetMonitoringChunkDuration(chunkDurationMs);
	StartRealTimeAudioProcessor(refreshRateMs);
}

void AudioWizardMainRealTime::StopRealTimeMonitoring() {
	if (!monitor.isRealTimeActive.load()) return;

	monitor.isRealTimeActive.store(false, std::memory_order_release);
	monitor.isPeakmeterActive.store(monitor.wasPeakmeterActiveBefore.load(), std::memory_order_release);
	monitor.wasPeakmeterActiveBefore.store(false, std::memory_order_release);
	monitor.isUIMessagePending.store(false, std::memory_order_release);

	if (!IsRealTimeAudioProcessorActive()) {
		StopRealTimeAudioProcessor();
	}
}

void AudioWizardMainRealTime::StartPeakmeterMonitoring(int refreshRateMs, int chunkDurationMs) {
	if (monitor.isPeakmeterActive.load()) return;

	monitor.isPeakmeterActive.store(true, std::memory_order_release);
	SetMonitoringRefreshRate(refreshRateMs);
	SetMonitoringChunkDuration(chunkDurationMs);

	StartRealTimeAudioProcessor(refreshRateMs);
	AudioWizard::Peakmeter()->ResetPeakmeter();
}

void AudioWizardMainRealTime::StopPeakmeterMonitoring() {
	if (!monitor.isPeakmeterActive.load()) return;

	monitor.isPeakmeterActive.store(false, std::memory_order_release);

	if (!IsRealTimeAudioProcessorActive()) {
		StopRealTimeAudioProcessor();
	}
}

void AudioWizardMainRealTime::StartRawAudioMonitoring(int refreshRateMs, int chunkDurationMs) {
	if (monitor.isRawAudioDataActive.load()) return;

	monitor.isRawAudioDataActive.store(true, std::memory_order_release);
	SetMonitoringRefreshRate(refreshRateMs);
	SetMonitoringChunkDuration(chunkDurationMs);

	StartRealTimeAudioProcessor(refreshRateMs);
}

void AudioWizardMainRealTime::StopRawAudioMonitoring() {
	if (!monitor.isRawAudioDataActive.load()) return;

	monitor.isRawAudioDataActive.store(false, std::memory_order_release);

	if (!IsRealTimeAudioProcessorActive()) {
		StopRealTimeAudioProcessor();
	}
}

void AudioWizardMainRealTime::GetRawAudioData(SAFEARRAY** data) const {
	if (!data) {
		FB2K_console_formatter() << "Audio Wizard => GetRawAudioData: Invalid output parameter";
		return;
	}

	*data = nullptr;

	if (!monitor.isRawAudioDataActive.load()) {
		FB2K_console_formatter() << "Audio Wizard => GetRawAudioData: Raw data processing is disabled.";
		return;
	}

	size_t sample_count = rawAudioData->buffer.capacity();
	std::vector<audioType> temp(sample_count);
	size_t read_count = 0;
	bool has_data = rawAudioData->buffer.read(temp.data(), sample_count, &read_count);

	if (has_data && read_count > 0) {
		*data = AWHCOM::CreateSafeArrayFromData(temp.begin(), temp.begin() + read_count, "GetRawAudioData");
	}
	else {
		*data = AWHCOM::CreateSafeArrayFromData(0, 0.0f, "GetRawAudioData"); // Empty array
	}
}
#pragma endregion


////////////////////////////////////////////
// * PRIVATE REAL-TIME AUDIO PROCESSING * //
////////////////////////////////////////////
#pragma region Private Real-Time Audio Processing
void AudioWizardMainRealTime::RealTimeAudioProcessor() {
	bool initialized = false;

	double nextFetchTime = 0.0;
	double chunkDurationSec = 0.0;
	int lastChunkMs = 0;
	int lastRefreshMs = 0;

	auto lastNotify = std::chrono::steady_clock::now();
	auto nextWakeup = lastNotify;

	while (monitor.isFetching.load(std::memory_order_acquire)) {
		double currTime = 0.0;
		const int chunkMs = monitor.monitorChunkDurationMs.load(std::memory_order_relaxed);
		const int refreshMs = monitor.monitorRefreshRateMs.load(std::memory_order_relaxed);

		// 1. Update Timing Configuration
		if (chunkMs != lastChunkMs || refreshMs != lastRefreshMs) {
			lastChunkMs = chunkMs;
			lastRefreshMs = refreshMs;
			chunkDurationSec = AWHConvert::MsToSec(chunkMs);

			if (monitor.isRealTimeActive || monitor.isPeakmeterActive) {
				chunkDurationSec = std::min(chunkDurationSec, AWHConvert::MsToSec(refreshMs));
			}
		}

		// 2. Validate Stream
		if (!visStream.is_valid() || !visStream->get_absolute_time(currTime)) {
			initialized = false;
			std::this_thread::sleep_for(std::chrono::milliseconds(refreshMs));
			continue;
		}

		// 3. Synchronization & Discontinuity
		if (!initialized || std::abs(currTime - nextFetchTime) > Config::DISCONTINUITY_THRESHOLD) {
			nextFetchTime = currTime;
			initialized = true;
		}

		// 4. Sequential Fetching
		AWHAudioData::Chunk chunk;

		for (int i = 0; i < Config::MAX_CATCHUP_CHUNKS && nextFetchTime < currTime
			&& monitor.isFetching.load(std::memory_order_relaxed); ++i) {

			if (!visStream->get_chunk_absolute(*chunk.chunk, nextFetchTime, chunkDurationSec)) {
				break;
			}

			chunk.metadata.timestamp.store(nextFetchTime, std::memory_order_release);
			ChunkData data(*chunk.chunk);

			if (monitor.isRawAudioDataActive) ProcessRawAudioDataCapture(data);

			if (monitor.isRealTimeActive) {
				ProcessRealTimeMetrics(data);
				monitor.lastRealtimeUpdate.store(std::chrono::steady_clock::now().time_since_epoch().count(), std::memory_order_release);
			}
			else if (monitor.isPeakmeterActive) {
				ProcessPeakmeterMetrics(data);
				monitor.lastMonitoringUpdate.store(std::chrono::steady_clock::now().time_since_epoch().count(), std::memory_order_release);
			}

			nextFetchTime += chunkDurationSec;
		}

		// 5. UI Notification - only notify UI when new data was actually processed this iteration
		const auto now = std::chrono::steady_clock::now();
		const HWND hWnd = realTimeDialogHwnd.load(std::memory_order_acquire);
		const int dynamicInterval = std::clamp(refreshMs, Config::MIN_REFRESH_UI_MS, Config::MAX_REFRESH_UI_MS);

		if (hWnd && (now - lastNotify >= std::chrono::milliseconds(dynamicInterval))) {
			bool expected = false;
			if (monitor.isUIMessagePending.compare_exchange_strong(expected, true,
				std::memory_order_acq_rel)) {
				if (::PostMessage(hWnd, Config::WM_UPDATE_METRICS, 0, 0)) {
					lastNotify = now;
				}
				else {
					monitor.isUIMessagePending.store(false, std::memory_order_release);
				}
			}
		}

		// 6. Precision Sleep
		nextWakeup = std::max(now, nextWakeup + std::chrono::milliseconds(refreshMs));
		std::this_thread::sleep_until(nextWakeup);
	}
}

void AudioWizardMainRealTime::StartRealTimeAudioProcessor(int fetchRateMs) {
	if (!visStream.is_valid()) {
		visualisation_manager::get()->create_stream(visStream, visualisation_manager::KStreamFlagNewFFT);
	}
	if (!monitor.isFetching.exchange(true)) {
		if (monitor.realTimeThread.joinable()) {
			monitor.realTimeThread.join();
		}
		monitor.monitorRefreshRateMs = std::clamp(fetchRateMs, Config::MIN_CHUNK_DURATION_MS, Config::MAX_REFRESH_RATE_MS);
		monitor.realTimeThread = std::thread(&AudioWizardMainRealTime::RealTimeAudioProcessor, this);
	}
}

void AudioWizardMainRealTime::StopRealTimeAudioProcessor() {
	monitor.isFetching.store(false, std::memory_order_seq_cst);

	if (monitor.realTimeThread.joinable()) {
		monitor.realTimeThread.join();
	}

	visStream.reset();
}

bool AudioWizardMainRealTime::IsRealTimeAudioProcessorActive() const {
	return monitor.isPeakmeterActive || monitor.isRealTimeActive || monitor.isRawAudioDataActive;
}
#pragma endregion


//////////////////////////////////////////////
// * PRIVATE REAL-TIME METRICS PROCESSING * //
//////////////////////////////////////////////
#pragma region Private Real-Time Metrics Processing
void AudioWizardMainRealTime::ProcessRealTimeMetrics(const ChunkData& data) {
	AudioWizardAnalysisRealTime::ProcessRealtimeChunk(data, analysis.realTimeData);

	// Continuous/Slow Metrics
	metrics.momentaryLUFS.store(analysis.realTimeData.momentaryLUFS, std::memory_order_release);
	metrics.shortTermLUFS.store(analysis.realTimeData.shortTermLUFS, std::memory_order_release);
	metrics.RMS.store(analysis.realTimeData.RMS, std::memory_order_release);
	metrics.phaseCorrelation.store(analysis.realTimeData.phaseCorrelation, std::memory_order_release);
	metrics.stereoWidth.store(analysis.realTimeData.stereoWidth, std::memory_order_release);
	metrics.DR.store(analysis.realTimeData.dynamicRange, std::memory_order_release);
	metrics.PD.store(analysis.realTimeData.pureDynamics, std::memory_order_release);

	// Dynamics/Ratios
	metrics.PSR.store(analysis.realTimeData.PSR, std::memory_order_release);
	metrics.PLR.store(analysis.realTimeData.PLR, std::memory_order_release);
	metrics.crestFactor.store(analysis.realTimeData.crestFactor, std::memory_order_release);

	// Transient Peaks
	UpdateLatchedMetric(metrics.leftRMS, analysis.realTimeData.leftRMS);
	UpdateLatchedMetric(metrics.rightRMS, analysis.realTimeData.rightRMS);
	UpdateLatchedMetric(metrics.leftSamplePeak, analysis.realTimeData.leftSamplePeak);
	UpdateLatchedMetric(metrics.rightSamplePeak, analysis.realTimeData.rightSamplePeak);
	UpdateLatchedMetric(metrics.truePeak, analysis.realTimeData.truePeak);

	AudioWizard::Peakmeter()->UpdatePeakmeter();
}

void AudioWizardMainRealTime::ProcessPeakmeterMetrics(const ChunkData& data) {
	if (!monitor.isPeakmeterActive) return;

	auto [leftRMS, rightRMS] = AudioWizardAnalysisRealTime::ProcessFrameRMS(data);
	auto [leftSamplePeak, rightSamplePeak] = AudioWizardAnalysisRealTime::ProcessFramePeaks(data);
	metrics.leftRMS.store(leftRMS);
	metrics.rightRMS.store(rightRMS);
	metrics.leftSamplePeak.store(leftSamplePeak);
	metrics.rightSamplePeak.store(rightSamplePeak);

	AudioWizard::Peakmeter()->UpdatePeakmeter();
}

void AudioWizardMainRealTime::ProcessRawAudioDataCapture(const ChunkData& data) {
	if (!monitor.isRawAudioDataActive) return;

	size_t sample_count = data.frames * data.channels;

	if (sample_count == 0 || sample_count > rawAudioData->buffer.capacity()) {
		return;
	}

	rawAudioData->buffer.write(data.data, sample_count);
}
#pragma endregion


/////////////////////////
// * PRIVATE HELPERS * //
/////////////////////////
#pragma region Private Helpers
void AudioWizardMainRealTime::UpdateLatchedMetric(std::atomic<double>& metric, double newValue) const {
	double current = metric.load(std::memory_order_relaxed);

	while (newValue > current) {
		if (metric.compare_exchange_weak(
				current, newValue, std::memory_order_release, std::memory_order_relaxed)
			) {
			break;
		}
		// If CAS fails, 'current' is updated with the latest value, loop continues
	}
}
#pragma endregion
