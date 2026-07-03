/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Waveform Source File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.6.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    03-07-2026                                              * //
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
void AudioWizardWaveform::StartWaveformAnalysis(const metadb_handle_list & tracks, int pointsPerSec, bool downmixToMono) {
	if (state.isAnalyzing.load()) {
		StopWaveformAnalysis();
	}

	// Initialize per-track storage
	state.trackWaveforms.clear();
	state.trackWaveforms.resize(tracks.get_count());

	int clampedPointsPerSec = std::clamp(
		pointsPerSec, Config::MIN_POINTS_PER_SEC, Config::MAX_POINTS_PER_SEC
	);

	state.downmixToMono.store(downmixToMono, std::memory_order_release);

	AWHDebug::DebugLog("StartWaveformAnalysis: Initialized ",
		tracks.get_count(), " tracks at ", clampedPointsPerSec, " points/sec",
		downmixToMono ? " (downmix to mono)" : ""
	);

	for (t_size i = 0; i < tracks.get_count(); ++i) {
		auto& track = state.trackWaveforms[i];
		track.handle = tracks[i];
		track.duration = tracks[i]->get_length();
		track.reset();

		// Pre-reserve: divide by channel count when downmixing (1 channel output)
		const size_t effectiveChannels = 1; // always 1 here for reserve; channels set by ProcessWaveformMetrics
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
		SAFEARRAYBOUND bound = { 0, 0 };
		*data = SafeArrayCreate(VT_VARIANT, 1, &bound);
		return;
	}

	const auto& track = state.trackWaveforms[trackIndex];
	const unsigned channels = track.channels;
	const size_t metricsPC = Config::WAVEFORM_CHUNK_ELEMENTS; // 5
	const size_t step = channels * metricsPC;
	const size_t total = track.samples.size();
	const size_t numPoints = (step > 0 && total >= step) ? (total / step) : 0;

	SAFEARRAYBOUND outerBound = { channels, 0 };
	SAFEARRAY* outerArray = SafeArrayCreate(VT_VARIANT, 1, &outerBound);
	if (!outerArray) {
		FB2K_console_formatter() << "Audio Wizard => GetWaveformData: Failed to create outer SAFEARRAY";
		*data = nullptr;
		return;
	}

	for (unsigned c = 0; c < channels; ++c) {
		std::vector<double> chData;
		chData.reserve(numPoints * metricsPC);
		const size_t offset = c * metricsPC;

		for (size_t t = 0; t < numPoints; ++t) {
			const size_t base = t * step + offset;
			chData.insert(chData.end(),
				track.samples.begin() + base,
				track.samples.begin() + base + metricsPC);
		}

		SAFEARRAY* innerArray = AWHCOM::CreateSafeArrayFromData(
			chData.begin(), chData.end(), "GetWaveformData"
		);

		if (!innerArray) {
			FB2K_console_formatter() << "Audio Wizard => GetWaveformData: Failed inner SAFEARRAY for ch " << c;
			continue;
		}

		VARIANT v;
		VariantInit(&v);
		V_VT(&v) = VT_ARRAY | VT_R4;
		V_ARRAY(&v) = innerArray;
		auto idx = static_cast<LONG>(c);

		HRESULT hr = SafeArrayPutElement(outerArray, &idx, &v); // deep-copies innerArray
		if (FAILED(hr)) {
			FB2K_console_formatter() << "Audio Wizard => GetWaveformData: SafeArrayPutElement failed ch " << c << " hr=" << hr;
		}
		SafeArrayDestroy(innerArray); // free local reference

		AWHDebug::DebugLog("GetWaveformData[", trackIndex, "] ch", c, ": ", chData.size(), " values");
	}

	*data = outerArray;
}

void AudioWizardWaveform::GetWaveformDataInfo(size_t trackIndex, bool hasTrackIndex, pfc::string8& json) const {
	std::ostringstream oss;
	oss << std::setprecision(17);

	oss << "{"
		<< R"("componentVersion":")" << ::AW_COMPONENT_VERSION << "\","
		<< "\"waveformDataVersion\":" << Config::WAVEFORM_DATA_VERSION;

	if (hasTrackIndex && trackIndex < state.trackWaveforms.size()) {
		pfc::string8 path;
		double duration = 0.0;
		GetWaveformTrackInfo(trackIndex, path, duration);
		const unsigned channels = GetWaveformTrackChannels(trackIndex);

		oss << ",\"channels\":" << channels
			<< R"(,"path":")" << AWHString::EscapeJsonString(path.c_str()) << "\""
			<< ",\"duration\":" << duration;
	}

	oss << ",\"metricsPerChannel\":" << Config::WAVEFORM_CHUNK_ELEMENTS << ","
		<< "\"metrics\":[";

	for (size_t i = 0; i < Config::WAVEFORM_METRIC_NAMES.size(); ++i) {
		if (i > 0) oss << ",";
		oss << "\"" << Config::WAVEFORM_METRIC_NAMES[i] << "\"";
	}

	oss << "],"
		<< "\"pointsPerSecond\":" << state.pointsPerSecond.load(std::memory_order_relaxed)
		<< "}";

	json = oss.str().c_str();

	AWHDebug::DebugLog(
		"GetWaveformDataInfo[", hasTrackIndex ? trackIndex : 0, "]: ",
		hasTrackIndex ? "per-track" : "component", " info, ", json.get_length(), " bytes"
	);
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
	const bool downmix = state.downmixToMono.load(std::memory_order_relaxed);
	const unsigned effectiveChannels = downmix ? 1u : data.channels;

	// 1. Track Initialization
	if (track.channels != effectiveChannels) {
		track.channels = effectiveChannels;
		track.activeRMSPeaks.assign(effectiveChannels, -100.0);
	}

	// 2. Setup Processing Constants
	const int pointsPerSecond = state.pointsPerSecond.load(std::memory_order_relaxed);
	const size_t safePPS = (pointsPerSecond > 0) ? static_cast<size_t>(pointsPerSecond) : 1;
	const size_t framesPerResolution = std::max<size_t>(1, static_cast<size_t>(data.sampleRate / safePPS));
	const size_t numChunks = (data.frames + framesPerResolution - 1) / framesPerResolution;
	const size_t elementsPerChunk = effectiveChannels * Config::WAVEFORM_CHUNK_ELEMENTS;
	const size_t outputStart = track.samples.size();
	track.samples.resize(outputStart + numChunks * elementsPerChunk);

	// 3. Hoist Allocations — sized for effectiveChannels
	std::vector<double> sumSquares(effectiveChannels);
	std::vector<double> maxAbs(effectiveChannels);
	std::vector<double> minSample(effectiveChannels);
	std::vector<double> maxSample(effectiveChannels);

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
		if (downmix && data.channels > 1) {
			// Downmix path: average all input channels into a single mono sample
			const double invChannels = 1.0 / static_cast<double>(data.channels);
			for (size_t f = 0; f < chunkSize; ++f) {
				double monoSample = 0.0;
				for (size_t c = 0; c < data.channels; ++c, ++dataIdx) {
					monoSample += data.data[dataIdx];
				}
				monoSample *= invChannels;
				const double absSample = std::abs(monoSample);
				sumSquares[0] += monoSample * monoSample;
				maxAbs[0] = std::max(maxAbs[0], absSample);
				minSample[0] = std::min(minSample[0], monoSample);
				maxSample[0] = std::max(maxSample[0], monoSample);
			}
		}
		else {
			// Original per-channel path
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
		}

		// Finalize metrics and store
		size_t outIdx = outputStart + chunkIdx * elementsPerChunk;
		double chunkGlobalMax = 0.0;

		for (size_t c = 0; c < effectiveChannels; ++c) {
			// Calculate metrics
			const double rms = std::sqrt(sumSquares[c] * invChunkSize);
			const double rmsDb = (rms > 0.0) ? std::clamp(AWHAudio::LinearToDb(rms), -100.0, 0.0) : -100.0;
			const double samplePeakDb = (maxAbs[c] > 0.0) ? std::clamp(AWHAudio::LinearToDb(maxAbs[c]), -100.0, 0.0) : -100.0;

			// RMS peak hold/decay
			double& rmsPeakDb = track.activeRMSPeaks[c];
			rmsPeakDb = std::clamp((rmsDb > rmsPeakDb) ? rmsDb : rmsPeakDb - decayAmount, -100.0, 0.0);

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
