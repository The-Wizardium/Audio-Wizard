/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Waveform Source File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
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
void AudioWizardWaveform::StartWaveformAnalysis(int samplesPerSec) {
	if (state.isAnalyzing.load()) {
		StopWaveformAnalysis();
	}

	state.waveformSamples.clear();
	state.completedWaveformSamples.clear();
	state.waveformSamples.reserve(Config::MAX_SAMPLES);
	state.lastSampleTime = 0.0;
	state.maxAmplitude = 0.0;
	state.isAnalysisComplete.store(false, std::memory_order_release);
	state.isAnalyzing.store(false, std::memory_order_release);

	int clampedResolution = std::clamp(samplesPerSec, Config::MIN_SAMPLES_PER_SEC, Config::MAX_SAMPLES_PER_SEC);
	state.samplesPerSecond.store(clampedResolution);
	FB2K_console_formatter() << "Audio Wizard => SetResolution: " << clampedResolution << " samples/s";
	state.isAnalyzing.store(true, std::memory_order_release);

	AudioWizard::Main()->SetMonitoringChunkDuration(1000 / samplesPerSec);
	AudioWizard::Main()->StartFullTrackWaveform(1000 / samplesPerSec);
}

void AudioWizardWaveform::StopWaveformAnalysis() {
	if (!state.isAnalyzing.load()) return;

	state.isAnalyzing.store(false, std::memory_order_release);
	state.completedWaveformSamples = state.waveformSamples;
	state.isAnalysisComplete.store(true, std::memory_order_release);

	AudioWizard::Main()->StopFullTrackWaveform();
}
#pragma endregion


////////////////////////////
// * PUBLIC API METHODS * //
////////////////////////////
#pragma region Public API Methods
bool AudioWizardWaveform::IsAnalysisComplete(double trackDurationSec) const {
	double resolutionSec = 1.0 / state.samplesPerSecond.load();
	double expectedSamples = (trackDurationSec / resolutionSec) * Config::WAVEFORM_CHUNK_ELEMENTS;
	bool complete = state.isAnalysisComplete.load(std::memory_order_acquire) &&
		state.completedWaveformSamples.size() >= static_cast<size_t>(expectedSamples * 0.95
	);

	FB2K_console_formatter() << "Audio Wizard => IsAnalysisComplete: " << state.completedWaveformSamples.size()
		<< " samples, Expected: " << expectedSamples
		<< ", Last time: " << state.lastSampleTime
		<< "s, Complete: " << (complete ? "Yes" : "No");

	return complete;
}

void AudioWizardWaveform::GetWaveformData(SAFEARRAY** data) const {
	if (!data) {
		FB2K_console_formatter() << "Audio Wizard => GetWaveformData: Invalid parameters";
		return;
	}

	std::vector<double> samples = state.isAnalysisComplete.load(std::memory_order_acquire)
		? state.completedWaveformSamples
		: state.waveformSamples;

	*data = AWHCOM::CreateSafeArrayFromData(samples.begin(), samples.end(), "GetWaveformData");

	if (*data) {
		FB2K_console_formatter() << "Audio Wizard => GetWaveformData: Successfully created SAFEARRAY with " << samples.size() << " samples";
	}
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

	// Define constants for clarity and safety
	const int samplesPerSecond = state.samplesPerSecond.load();
	const double resolutionSec = 1.0 / static_cast<double>(samplesPerSecond);
	const size_t framesPerResolution = std::max<size_t>(1, static_cast<size_t>(data.sampleRate / samplesPerSecond));
	const double rmsDivisor = data.channels > 1 ? 2.0 : 1.0;

	// Reserve exact space for output vector to avoid reallocations
	std::vector<double> output;
	output.reserve((data.frames + framesPerResolution - 1) / framesPerResolution * AudioWizardWaveform::Config::WAVEFORM_CHUNK_ELEMENTS);

	double currentTime = state.lastSampleTime;
	double maxSquaredSampleSum = 0.0;
	double lastWaveformPeak = 0.0;

	// Process audio data in chunks
	for (size_t framesProcessed = 0; framesProcessed < data.frames; framesProcessed += framesPerResolution) {
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

	// Update waveform state safely
	auto& samples = state.waveformSamples;
	const size_t maxSamples = AudioWizardWaveform::Config::MAX_SAMPLES;
	if (samples.size() + output.size() > maxSamples) {
		const size_t excess = samples.size() + output.size() - maxSamples;
		samples.erase(samples.begin(), samples.begin() + excess);
	}
	samples.insert(samples.end(), output.begin(), output.end());
	state.lastSampleTime = currentTime;

	if (!output.empty()) {
		state.maxAmplitude = std::max(state.maxAmplitude, *std::max_element(output.begin(), output.end()));
	}

	// Log processing details
	//FB2K_console_formatter() << "Audio Wizard => ProcessWaveformMetrics: Generated "
	//	<< output.size() / AudioWizardWaveform::Config::WAVEFORM_CHUNK_ELEMENTS << " chunks for " << data.frames
	//	<< " frames at resolution " << samplesPerSecond << " samples/s, "
	//	<< "Last sample time: " << currentTime
	//	<< "s, Expected chunk duration: " << (data.frames / data.sampleRate) << "s"
	//	<< ", Sample waveformPeak: " << lastWaveformPeak;
}
#pragma endregion
