/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Waveform Source File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
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
	if (!AudioWizard::Waveform() || !state.isAnalyzing.load()) {
		return;
	}

	size_t currentIndex = state.currentTrackIndex.load(std::memory_order_acquire);

	if (currentIndex >= state.trackWaveforms.size()) {
		AWHDebug::DebugLog("ProcessWaveformMetrics: Invalid track index ", currentIndex);
		return;
	}

	// Define constants for clarity and safety
	auto& track = state.trackWaveforms[currentIndex];
	const int pointsPerSecond = state.pointsPerSecond.load();
	const double resolutionSec = 1.0 / static_cast<double>(pointsPerSecond);
	const size_t framesPerResolution = std::max<size_t>(1, static_cast<size_t>(data.sampleRate / pointsPerSecond));
	const double rmsDivisor = data.channels > 1 ? 2.0 : 1.0;

	// Reserve exact space for output vector to avoid reallocations
	std::vector<double> output;
	output.reserve((data.frames + framesPerResolution - 1) / framesPerResolution * Config::WAVEFORM_CHUNK_ELEMENTS);

	double currentTime = track.lastSampleTime;
	double maxSquaredSampleSum = 0.0;
	double lastWaveformPeak = 0.0;

	// Process audio data in chunks
	for (size_t framesProcessed = 0; framesProcessed < data.frames;
		framesProcessed += framesPerResolution) {

		const size_t chunkSize = std::min(framesPerResolution, data.frames - framesProcessed);
		const double invChunkSize = 1.0 / static_cast<double>(chunkSize);

		double sumSquaresLeft = 0.0;
		double sumSquaresRight = 0.0;
		double maxAbsLeft = 0.0;
		double maxAbsRight = 0.0;
		double maxSignedSample = 0.0;

		// Accumulate metrics for the chunk
		for (size_t i = framesProcessed; i < framesProcessed + chunkSize; ++i) {
			const size_t baseIdx = i * data.channels;
			for (size_t c = 0; c < data.channels; ++c) {
				const double sample = data.data[baseIdx + c];
				const double sampleSquared = sample * sample;
				const double absSample = std::abs(sample);

				if (c == 0) {
					sumSquaresLeft += sampleSquared;
					maxAbsLeft = std::max(maxAbsLeft, absSample);
				}
				else if (c == 1 && data.channels > 1) {
					sumSquaresRight += sampleSquared;
					maxAbsRight = std::max(maxAbsRight, absSample);
				}

				if (absSample > std::abs(maxSignedSample)) {
					maxSignedSample = sample;
				}
			}
		}

		// Compute waveform metrics
		const double rmsLeft = std::sqrt(sumSquaresLeft * invChunkSize);
		const double rmsRight = data.channels > 1 ? std::sqrt(sumSquaresRight * invChunkSize) : rmsLeft;
		const double rms = (rmsLeft + rmsRight) / rmsDivisor;
		const double peak = std::max(maxAbsLeft, maxAbsRight);

		const double squaredSampleSum = rms * rms * static_cast<double>(chunkSize);
		maxSquaredSampleSum = std::max(maxSquaredSampleSum, squaredSampleSum);
		const double rmsPeak = std::sqrt(maxSquaredSampleSum * invChunkSize);
		lastWaveformPeak = std::clamp(maxSignedSample, -1.0, 1.0);

		// Convert to dB with safety checks
		const double rms_dB = rms > 0.0 ? std::min(AWHAudio::LinearToDb(rms), 0.0) : -100.0;
		const double peak_dB = peak > 0.0 ? std::min(AWHAudio::LinearToDb(peak), 0.0) : -100.0;
		const double rmsPeak_dB = rmsPeak > 0.0 ? std::min(AWHAudio::LinearToDb(rmsPeak), 0.0) : -100.0;

		// Store metrics in output
		output.push_back(std::round(rms_dB));
		output.push_back(std::round(rmsPeak_dB));
		output.push_back(std::round(peak_dB));
		output.push_back(lastWaveformPeak);

		currentTime += resolutionSec;
	}

	// Append to current track's samples
	track.samples.insert(track.samples.end(), output.begin(), output.end());
	track.duration = currentTime;
	track.lastSampleTime = currentTime;

	if (!output.empty()) {
		track.maxAmplitude = std::max(track.maxAmplitude, *std::max_element(output.begin(), output.end()));
	}
}
#pragma endregion
