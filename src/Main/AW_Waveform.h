/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Waveform Header File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
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
		static constexpr int DEFAULT_SAMPLES_PER_SEC = 20;
		static constexpr int MIN_SAMPLES_PER_SEC = 1;
		static constexpr int MAX_SAMPLES_PER_SEC = 1000;
		static constexpr size_t MAX_SAMPLES = 1000000;
		static constexpr double NORMALIZATION_FACTOR = 1.0;
	};

	// * WAVEFORM STATE * //
	struct State {
		std::atomic<WaveformMetric> metric = Config::DEFAULT_METRIC;
		std::atomic<bool> isAnalyzing = false;
		std::atomic<bool> isAnalysisComplete = false;
		std::atomic<int> samplesPerSecond = Config::DEFAULT_SAMPLES_PER_SEC;
		std::vector<double> waveformSamples;
		std::vector<double> completedWaveformSamples;
		double lastSampleTime = 0.0;
		double maxAmplitude = 0.0;
	}; State state;

	AudioWizardWaveform();
	~AudioWizardWaveform();

	// * PUBLIC MAIN METHODS * //
	void StartWaveformAnalysis(int samplesPerSec);
	void StopWaveformAnalysis();

	// * PUBLIC API METHODS * //
	bool IsAnalysisComplete(double trackDurationSec) const;
	void GetWaveformData(SAFEARRAY** data) const;
	void SetWaveformMetric(WaveformMetric metric);

	// * PUBLIC WAVEFORM METRICS PROCESSING * //
	void ProcessWaveformMetrics(const ChunkData& data);
};
#pragma endregion
