/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Waveform Header File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.6.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    03-07-2026                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


//////////////////
// * WAVEFORM * //
//////////////////
#pragma region Waveform
class AudioWizardWaveform {
public:
	// * TYPE ALIASES * //
	using ChunkData = AWHAudioData::ChunkData;

	enum class WaveformMetric {
		RMS, RMSPeak, SamplePeak, Waveform
	};

	struct Config {
		static constexpr int WAVEFORM_DATA_VERSION = 1; // NOTE: bump whenever WAVEFORM_CHUNK_ELEMENTS or WAVEFORM_METRIC_NAMES changes.
		static constexpr size_t WAVEFORM_CHUNK_ELEMENTS = 5; // RMS, RMSPeak, SamplePeak, Min, Max
		static constexpr std::array<std::string_view, WAVEFORM_CHUNK_ELEMENTS> WAVEFORM_METRIC_NAMES = {
			"rms", "rms_peak", "sample_peak", "min", "max"
		};
		static constexpr WaveformMetric DEFAULT_METRIC = WaveformMetric::Waveform;
		static constexpr int DEF_POINTS_PER_SEC = 20;
		static constexpr int MIN_POINTS_PER_SEC = 1;
		static constexpr int MAX_POINTS_PER_SEC = 1000;
		static constexpr size_t MAX_SAMPLES = 1000000;
		static constexpr double NORMALIZATION_FACTOR = 1.0;
	};

	// * WAVEFORM STATE * //
	struct TrackWaveform {
		std::vector<double> samples;
		std::vector<double> activeRMSPeaks;
		metadb_handle_ptr handle;
		unsigned channels = 0;
		double duration = 0.0;
		double lastSampleTime = 0.0;
		double maxAmplitude = 0.0;

		void reset() {
			samples.clear();
			channels = 0;
			lastSampleTime = 0.0;
			maxAmplitude = 0.0;
		}
	};

	struct State {
		std::atomic<WaveformMetric> metric = Config::DEFAULT_METRIC;
		std::atomic<bool> isAnalyzing = false;
		std::atomic<bool> isAnalysisComplete = false;
		std::atomic<bool> downmixToMono = false;
		std::atomic<int> pointsPerSecond = Config::DEF_POINTS_PER_SEC;
		std::atomic<size_t> currentTrackIndex = 0;
		std::vector<TrackWaveform> trackWaveforms;
	}; State state;

	AudioWizardWaveform();
	~AudioWizardWaveform();

	// * PUBLIC MAIN METHODS * //
	void StartWaveformAnalysis(const metadb_handle_list& tracks, int pointsPerSecond, bool downmixToMono = false);
	void StopWaveformAnalysis();

	// * PUBLIC API METHODS * //
	bool IsWaveformAnalysisComplete(double trackDurationSec) const;
	void GetWaveformData(size_t trackIndex, SAFEARRAY** data) const;
	void GetWaveformDataInfo(size_t trackIndex, bool hasTrackIndex, pfc::string8& json) const;
	unsigned GetWaveformTrackChannels(size_t trackIndex) const;
	size_t GetWaveformTrackCount() const;
	void GetWaveformTrackInfo(size_t trackIndex, pfc::string8& path, double& duration) const;
	void SetWaveformMetric(WaveformMetric metric);

	// * PUBLIC WAVEFORM METRICS PROCESSING * //
	void ProcessWaveformMetrics(const ChunkData& data);
};
#pragma endregion
