/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Header File                           * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "AW_MainFullTrack.h"
#include "AW_MainRealTime.h"


///////////////////////////
// * AUDIO WIZARD MAIN * //
///////////////////////////
#pragma region Audio Wizard Main
class AudioWizardMain {
public:
	// * PUBLIC API - FULL-TRACK ANALYSIS CALLBACKS SETUP * //
	struct Callbacks {
		VARIANT fullTrackAnalysisCallback;
		VARIANT fullTrackWaveformCallback;

		Callbacks() {
			VariantInit(&fullTrackAnalysisCallback);
			VariantInit(&fullTrackWaveformCallback);
		}
		~Callbacks() {
			VariantClear(&fullTrackAnalysisCallback);
			VariantClear(&fullTrackWaveformCallback);
		}
	}; Callbacks callbacks;

	// * PROCESSOR INSTANCES * //
	std::unique_ptr<AudioWizardMainFullTrack> mainFullTrack;
	std::unique_ptr<AudioWizardMainRealTime> mainRealTime;

	// * CONSTRUCTOR & DESTRUCTOR * //
	AudioWizardMain();
	~AudioWizardMain();

	// * PUBLIC API - CONFIGURATION * //
	void SetFullTrackChunkDuration(int chunkDurationMs);
	void SetMonitoringChunkDuration(int chunkDurationMs);
	void SetMonitoringRefreshRate(int refreshRateMs);

	// * PUBLIC API - FULL-TRACK ANALYSIS CALLBACKS * //
	void SetFullTrackAnalysisCallback(const VARIANT* callback);
	void SetFullTrackWaveformCallback(const VARIANT* callback);

	// * PUBLIC API - FULL-TRACK ANALYSIS CONTROL * //
	void StartFullTrackAnalysis(int chunkDurationMs);
	void StopFullTrackAnalysis();
	void StartFullTrackWaveform(int chunkDurationMs);
	void StopFullTrackWaveform();
	void StopFullTrackAudioProcessor();

	// * PUBLIC API - FULL-TRACK DATA ACCESS * //
	bool GetFullTrackAnalysis() const;
	void GetFullTrackMetrics(SAFEARRAY** fullTrackMetrics) const;
	double GetMomentaryLUFSFull(LONG trackIndex = 0) const;
	double GetShortTermLUFSFull(LONG trackIndex = 0) const;
	double GetIntegratedLUFSFull(LONG trackIndex = 0) const;
	double GetRMSFull(LONG trackIndex = 0) const;
	double GetSamplePeakFull(LONG trackIndex = 0) const;
	double GetTruePeakFull(LONG trackIndex = 0) const;
	double GetPSRFull(LONG trackIndex = 0) const;
	double GetPLRFull(LONG trackIndex = 0) const;
	double GetCrestFactorFull(LONG trackIndex = 0) const;
	double GetLoudnessRangeFull(LONG trackIndex = 0) const;
	double GetDynamicRangeFull(LONG trackIndex = 0) const;
	double GetPureDynamicsFull(LONG trackIndex = 0) const;
	std::map<std::wstring, double> GetAlbumMetricFull(
		const std::function<double(const AudioWizardAnalysisFullTrack::FullTrackResults&)>& metricAccessor
	) const;

	// * PUBLIC API - REAL-TIME MONITORING CONTROL * //
	void StartRealTimeMonitoring(int refreshRateMs, int chunkDurationMs);
	void StopRealTimeMonitoring();
	void StartPeakmeterMonitoring(int refreshRateMs, int chunkDurationMs);
	void StopPeakmeterMonitoring();
	void StartRawAudioMonitoring(int refreshRateMs, int chunkDurationMs);
	void StopRawAudioMonitoring();
	void StopRealTimeAudioProcessor();

	// * PUBLIC API - REAL-TIME DATA ACCESS * //
	void GetRawAudioData(SAFEARRAY** data) const;
	void GetMomentaryLUFS(double* value) const;
	void GetShortTermLUFS(double* value) const;
	void GetRMS(double* value) const;
	void GetLeftRMS(double* value) const;
	void GetRightRMS(double* value) const;
	void GetLeftSamplePeak(double* value) const;
	void GetRightSamplePeak(double* value) const;
	void GetTruePeak(double* value) const;
	void GetPSR(double* value) const;
	void GetPLR(double* value) const;
	void GetCrestFactor(double* value) const;
	void GetDynamicRange(double* value) const;
	void GetPureDynamics(double* value) const;
	void GetPhaseCorrelation(double* value) const;
	void GetStereoWidth(double* value) const;

private:
	bool IsFullTrackSelected(metadb_handle_ptr& track) const;
	bool ValidateTrackAndAnalysis(metadb_handle_ptr& track, LONG trackIndex = 0) const;
};
#pragma endregion
