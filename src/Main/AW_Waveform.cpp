/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Waveform Source File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    27-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_Main.h"
#include "AW_Waveform.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardWaveform::AudioWizardWaveform() = default;
AudioWizardWaveform::~AudioWizardWaveform() = default;
#pragma endregion


////////////////////////
// * PUBLIC METHODS * //
////////////////////////
#pragma region Public Methods
void AudioWizardWaveform::StartWaveformAnalysis(const metadb_handle_list & tracks, int pointsPerSec) {
	if (state.isAnalyzing.load()) {
		StopWaveformAnalysis();
	}

	// Initialize per-track storage
	state.trackWaveforms.clear();
	state.trackWaveforms.resize(tracks.get_count());

	int clampedPointsPerSec = std::clamp(
		pointsPerSec, Config::MIN_POINTS_PER_SEC, Config::MAX_POINTS_PER_SEC
	);

	AWHDebug::DebugLog("StartWaveformAnalysis: Initialized ",
		tracks.get_count(), " tracks at ", clampedPointsPerSec, " points/sec"
	);

	for (t_size i = 0; i < tracks.get_count(); ++i) {
		auto& track = state.trackWaveforms[i];
		track.handle = tracks[i];
		track.duration = tracks[i]->get_length();
		track.reset();

		// Pre-reserve based on expected size
		auto expectedSamples = static_cast<size_t>(
			track.duration * clampedPointsPerSec * Config::WAVEFORM_CHUNK_ELEMENTS
		);
		track.samples.reserve(std::min(expectedSamples, Config::MAX_SAMPLES));
	}

	state.currentTrackIndex.store(0, std::memory_order_release);
	state.pointsPerSecond.store(clampedPointsPerSec, std::memory_order_release);
	state.isAnalysisComplete.store(false, std::memory_order_release);
	state.isAnalyzing.store(true, std::memory_order_release);

	int chunkDurationMs = 1000 / clampedPointsPerSec;
	AudioWizard::Main()->StartFullTrackWaveform(tracks, chunkDurationMs);
}

void AudioWizardWaveform::StopWaveformAnalysis() {
	if (!state.isAnalyzing.load()) return;

	state.isAnalyzing.store(false, std::memory_order_release);
	state.isAnalysisComplete.store(true, std::memory_order_release);

	AWHDebug::DebugLog("StopWaveformAnalysis: Completed with ", state.trackWaveforms.size(), " tracks");
	AudioWizard::Main()->StopFullTrackWaveform();
}
#pragma endregion


////////////////////////////
// * PUBLIC API METHODS * //
////////////////////////////
#pragma region Public API Methods
bool AudioWizardWaveform::IsWaveformAnalysisComplete(double trackDurationSec) const {
	if (!state.isAnalysisComplete.load(std::memory_order_acquire)) {
		return false;
	}

	double resolutionSec = 1.0 / state.pointsPerSecond.load();
	double expectedPoints = (trackDurationSec / resolutionSec) * Config::WAVEFORM_CHUNK_ELEMENTS;

	for (const auto& track : state.trackWaveforms) {
		if (track.samples.size() >= static_cast<size_t>(expectedPoints * 0.95)) {
			return true;
		}
	}

	return false;
}

void AudioWizardWaveform::GetWaveformData(size_t trackIndex, SAFEARRAY** data) const {
	if (!data) return;

	if (trackIndex >= state.trackWaveforms.size()) {
		FB2K_console_formatter() << "Audio Wizard => GetWaveformData: Invalid index " << trackIndex;

		*data = AWHCOM::CreateSafeArrayFromData(
			std::vector<double>{}.begin(), std::vector<double>{}.end(), "GetWaveformData"
		);

		return;
	}

	const auto& track = state.trackWaveforms[trackIndex];
	*data = AWHCOM::CreateSafeArrayFromData(track.samples.begin(), track.samples.end(), "GetWaveformData");

	AWHDebug::DebugLog("GetWaveformData[", trackIndex, "]: ", track.samples.size(), " samples");
}

unsigned AudioWizardWaveform::GetWaveformTrackChannels(size_t trackIndex) const {
	if (trackIndex < state.trackWaveforms.size()) {
		return state.trackWaveforms[trackIndex].channels;
	}
	return 0;
}

size_t AudioWizardWaveform::GetWaveformTrackCount() const {
	return state.trackWaveforms.size();
}

void AudioWizardWaveform::GetWaveformTrackInfo(size_t trackIndex, pfc::string8& path, double& duration) const {
	if (trackIndex >= state.trackWaveforms.size()) {
		path = "";
		duration = 0.0;
		return;
	}

	const auto& track = state.trackWaveforms[trackIndex];

	if (track.handle.is_valid()) {
		path = track.handle->get_path();
	}
	else {
		path = "";
	}

	duration = track.duration;
}

void AudioWizardWaveform::SetWaveformMetric(WaveformMetric metric) {
	state.metric.store(metric);
}
#pragma endregion


////////////////////////////////////////////
// * PUBLIC WAVEFORM METRICS PROCESSING * //
////////////////////////////////////////////
#pragma region Public Waveform Metrics Processing
void AudioWizardWaveform::ProcessWaveformMetrics(const ChunkData& data) {
	if (!AudioWizard::Waveform() || !state.isAnalyzing.load(std::memory_order_relaxed)) {
		return;
	}

	const size_t currentIndex = state.currentTrackIndex.load(std::memory_order_acquire);
	if (currentIndex >= state.trackWaveforms.size()) {
		AWHDebug::DebugLog("ProcessWaveformMetrics: Invalid track index ", currentIndex);
		return;
	}

	auto& track = state.trackWaveforms[currentIndex];

	// 1. Track Initialization
	if (track.channels != data.channels) {
		track.channels = data.channels;
		track.activeRMSPeaks.assign(data.channels, -100.0);
	}

	// 2. Setup Processing Constants
	const int pointsPerSecond = state.pointsPerSecond.load(std::memory_order_relaxed);
	const size_t safePPS = (pointsPerSecond > 0) ? static_cast<size_t>(pointsPerSecond) : 1;

	const size_t framesPerResolution = std::max<size_t>(1, static_cast<size_t>(data.sampleRate / safePPS));
	const size_t numChunks = (data.frames + framesPerResolution - 1) / framesPerResolution;
	const size_t elementsPerChunk = data.channels * Config::WAVEFORM_CHUNK_ELEMENTS;

	const size_t outputStart = track.samples.size();
	track.samples.resize(outputStart + numChunks * elementsPerChunk);

	// 3. Hoist Allocations - Reuse vectors for every chunk
	std::vector<double> sumSquares(data.channels);
	std::vector<double> maxAbs(data.channels);
	std::vector<double> minSample(data.channels);
	std::vector<double> maxSample(data.channels);

	double currentTime = track.lastSampleTime;

	// 4. Main Processing Loop
	for (size_t chunkStart = 0, chunkIdx = 0; chunkStart < data.frames;
		chunkStart += framesPerResolution, ++chunkIdx) {

		const size_t chunkSize = std::min(framesPerResolution, data.frames - chunkStart);
		const double chunkTimeSec = static_cast<double>(chunkSize) / data.sampleRate;
		const double decayAmount = 20.0 * chunkTimeSec;
		const double invChunkSize = 1.0 / static_cast<double>(chunkSize);

		// Reset accumulators
		std::fill(sumSquares.begin(), sumSquares.end(), 0.0);
		std::fill(maxAbs.begin(), maxAbs.end(), 0.0);
		std::fill(minSample.begin(), minSample.end(), std::numeric_limits<double>::infinity());
		std::fill(maxSample.begin(), maxSample.end(), -std::numeric_limits<double>::infinity());

		// Process frames - sequential memory access
		size_t dataIdx = chunkStart * data.channels;
		for (size_t f = 0; f < chunkSize; ++f) {
			for (size_t c = 0; c < data.channels; ++c, ++dataIdx) {
				const double sample = data.data[dataIdx];
				const double absSample = std::abs(sample);

				sumSquares[c] += sample * sample;
				maxAbs[c] = std::max(maxAbs[c], absSample);
				minSample[c] = std::min(minSample[c], sample);
				maxSample[c] = std::max(maxSample[c], sample);
			}
		}

		// Finalize metrics and store
		size_t outIdx = outputStart + chunkIdx * elementsPerChunk;
		double chunkGlobalMax = 0.0;

		for (size_t c = 0; c < data.channels; ++c) {
			// Calculate metrics
			const double rms = std::sqrt(sumSquares[c] * invChunkSize);
			const double rmsDb = (rms > 0.0) ? std::max(AWHAudio::LinearToDb(rms), -100.0) : -100.0;
			const double samplePeakDb = (maxAbs[c] > 0.0) ? std::max(AWHAudio::LinearToDb(maxAbs[c]), -100.0) : -100.0;

			// RMS peak hold/decay
			double& rmsPeakDb = track.activeRMSPeaks[c];
			rmsPeakDb = (rmsDb > rmsPeakDb) ? rmsDb : std::max(-100.0, rmsPeakDb - decayAmount);

			// Store metrics in order: rms, rms_peak, sample_peak, min, max
			track.samples[outIdx++] = std::round(rmsDb);
			track.samples[outIdx++] = std::round(rmsPeakDb);
			track.samples[outIdx++] = std::round(samplePeakDb);
			track.samples[outIdx++] = std::clamp(minSample[c], -1.0, 1.0);
			track.samples[outIdx++] = std::clamp(maxSample[c], -1.0, 1.0);

			chunkGlobalMax = std::max(chunkGlobalMax, maxAbs[c]);
		}

		track.maxAmplitude = std::max(track.maxAmplitude, chunkGlobalMax);
		currentTime += chunkTimeSec;
	}

	track.duration = currentTime;
	track.lastSampleTime = currentTime;
}
#pragma endregion
