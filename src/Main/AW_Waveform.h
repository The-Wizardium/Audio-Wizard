/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Waveform Header File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
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
		RMS, Peak, RMSPeak, WaveformPeak
	};

	struct Config {
		static constexpr WaveformMetric DEFAULT_METRIC = WaveformMetric::WaveformPeak;
		static constexpr size_t WAVEFORM_CHUNK_ELEMENTS = 4; // RMS, RMS_peak, Peak, Waveform Peak
		static constexpr int DEF_POINTS_PER_SEC = 20;
		static constexpr int MIN_POINTS_PER_SEC = 1;
		static constexpr int MAX_POINTS_PER_SEC = 1000;
		static constexpr size_t MAX_SAMPLES = 1000000;
		static constexpr double NORMALIZATION_FACTOR = 1.0;
	};

	// * WAVEFORM STATE * //
	struct TrackWaveform {
		std::vector<double> samples;
		metadb_handle_ptr handle;
		double duration = 0.0;
		double lastSampleTime = 0.0;
		double maxAmplitude = 0.0;

		void reset() {
			samples.clear();
			lastSampleTime = 0.0;
			maxAmplitude = 0.0;
		}
	};

	struct State {
		std::atomic<WaveformMetric> metric = Config::DEFAULT_METRIC;
		std::atomic<bool> isAnalyzing = false;
		std::atomic<bool> isAnalysisComplete = false;
		std::atomic<int> pointsPerSecond = Config::DEF_POINTS_PER_SEC;
		std::atomic<size_t> currentTrackIndex = 0;
		std::vector<TrackWaveform> trackWaveforms;
	}; State state;

	AudioWizardWaveform();
	~AudioWizardWaveform();

	// * PUBLIC MAIN METHODS * //
	void StartWaveformAnalysis(const metadb_handle_list& tracks, int pointsPerSecond);
	void StopWaveformAnalysis();

	// * PUBLIC API METHODS * //
	bool IsWaveformAnalysisComplete(double trackDurationSec) const;
	void GetWaveformData(size_t trackIndex, SAFEARRAY** data) const;
	size_t GetWaveformTrackCount() const;
	void GetWaveformTrackInfo(size_t trackIndex, pfc::string8& path, double& duration) const;
	void SetWaveformMetric(WaveformMetric metric);

	// * PUBLIC WAVEFORM METRICS PROCESSING * //
	void ProcessWaveformMetrics(const ChunkData& data);
};
#pragma endregion
