/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Real-Time Source File                 * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
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

	SetMonitoringRefreshRate(std::max(refreshRateMs, 17));
	SetMonitoringChunkDuration(chunkDurationMs);
	StartRealTimeAudioProcessor(refreshRateMs);
}

void AudioWizardMainRealTime::StopRealTimeMonitoring() {
	if (!monitor.isRealTimeActive.load()) return;

	monitor.isRealTimeActive.store(false, std::memory_order_release);
	monitor.isPeakmeterActive.store(monitor.wasPeakmeterActiveBefore.load(), std::memory_order_release);
	monitor.wasPeakmeterActiveBefore.store(false, std::memory_order_release);

	if (!IsRealTimeAudioProcessorActive()) {
		StopRealTimeAudioProcessor();
	}
}

void AudioWizardMainRealTime::StartPeakmeterMonitoring(int refreshRateMs, int chunkDurationMs) {
	if (monitor.isPeakmeterActive.load()) return;

	monitor.isPeakmeterActive.store(true, std::memory_order_release);
	SetMonitoringRefreshRate(std::max(refreshRateMs, 17));
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
	auto nextFetchTime = std::chrono::steady_clock::now();
	static auto lastNotify = std::chrono::steady_clock::now();
	int lastChunkDurationMs = 0;
	int lastRefreshRateMs = 0;
	double chunkDurationSec = 0.0;
	double currentTime = 0.0;

	auto shouldUpdate = [](const auto& now, const std::atomic<int64_t>& lastUpdate, int refreshRateMs) {
		auto last = std::chrono::steady_clock::time_point(
			std::chrono::nanoseconds(lastUpdate.load(std::memory_order_acquire))
		);
		return (now - last) >= std::chrono::milliseconds(refreshRateMs);
	};

	while (monitor.isFetching.load(std::memory_order_acquire)) {
		const int chunkDurationMs = monitor.monitorChunkDurationMs.load(std::memory_order_acquire);
		const int refreshRateMs = monitor.monitorRefreshRateMs.load(std::memory_order_acquire);

		if (chunkDurationMs != lastChunkDurationMs || refreshRateMs != lastRefreshRateMs) {
			lastChunkDurationMs = chunkDurationMs;
			lastRefreshRateMs = refreshRateMs;
			chunkDurationSec = AWHConvert::MsToSec(chunkDurationMs);

			if (monitor.isRealTimeActive || monitor.isPeakmeterActive) {
				chunkDurationSec = std::min(chunkDurationSec, AWHConvert::MsToSec(refreshRateMs));
			}
		}

		AWHAudioData::Chunk chunk;
		chunk.metadata.isValid = visStream.is_valid() &&
			visStream->get_absolute_time(currentTime) &&
			visStream->get_chunk_absolute(*chunk.chunk, currentTime, chunkDurationSec);

		if (chunk.metadata.isValid) {
			ChunkData data(*chunk.chunk);
			const auto now = std::chrono::steady_clock::now();
			chunk.metadata.timestamp.store(currentTime, std::memory_order_release);

			if (monitor.isRawAudioDataActive) {
				ProcessRawAudioDataCapture(data);
			}

			if (monitor.isRealTimeActive && shouldUpdate(now, monitor.lastRealtimeUpdate, refreshRateMs)) {
				ProcessRealTimeMetrics(data);
				monitor.lastRealtimeUpdate.store(now.time_since_epoch().count(), std::memory_order_release);
			}
			else if (monitor.isPeakmeterActive && shouldUpdate(now, monitor.lastMonitoringUpdate, refreshRateMs)) {
				ProcessPeakmeterMetrics(data);
				monitor.lastMonitoringUpdate.store(now.time_since_epoch().count(), std::memory_order_release);
			}

			if (const HWND hWnd = realTimeDialogHwnd.load(std::memory_order_acquire);
				hWnd && (now - lastNotify >= std::chrono::milliseconds(Config::UI_NOTIFICATION_INTERVAL_MS))) {
				::PostMessage(hWnd, Config::WM_UPDATE_METRICS, 0, 0);
				lastNotify = now;
			}
		}

		nextFetchTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(refreshRateMs);
		std::this_thread::sleep_until(nextFetchTime);
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

	metrics.momentaryLUFS.store(analysis.realTimeData.momentaryLUFS);
	metrics.shortTermLUFS.store(analysis.realTimeData.shortTermLUFS);
	metrics.RMS.store(analysis.realTimeData.RMS);
	metrics.leftRMS.store(analysis.realTimeData.leftRMS);
	metrics.rightRMS.store(analysis.realTimeData.rightRMS);
	metrics.leftSamplePeak.store(analysis.realTimeData.leftSamplePeak);
	metrics.rightSamplePeak.store(analysis.realTimeData.rightSamplePeak);
	metrics.truePeak.store(analysis.realTimeData.truePeak);
	metrics.PSR.store(analysis.realTimeData.PSR);
	metrics.PLR.store(analysis.realTimeData.PLR);
	metrics.crestFactor.store(analysis.realTimeData.crestFactor);
	metrics.DR.store(analysis.realTimeData.dynamicRange);
	metrics.PD.store(analysis.realTimeData.pureDynamics);
	metrics.phaseCorrelation.store(analysis.realTimeData.phaseCorrelation);
	metrics.stereoWidth.store(analysis.realTimeData.stereoWidth);

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
