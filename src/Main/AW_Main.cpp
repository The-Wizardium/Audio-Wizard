/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Source File                           * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardMain::AudioWizardMain() {
	mainFullTrack = std::make_unique<AudioWizardMainFullTrack>();
	mainRealTime = std::make_unique<AudioWizardMainRealTime>();

	mainRealTime->monitor.lastRealtimeUpdate.store(std::chrono::steady_clock::now().time_since_epoch().count());
	mainRealTime->monitor.lastMonitoringUpdate.store(std::chrono::steady_clock::now().time_since_epoch().count());
}

AudioWizardMain::~AudioWizardMain() = default;
#pragma endregion


////////////////////////////////////
// * PUBLIC API - CONFIGURATION * //
////////////////////////////////////
#pragma region Public API - Configuration
void AudioWizardMain::SetFullTrackChunkDuration(int chunkDurationMs) {
	mainFullTrack->SetFullTrackChunkDuration(chunkDurationMs);
}

void AudioWizardMain::SetMonitoringChunkDuration(int chunkDurationMs) {
	mainRealTime->SetMonitoringChunkDuration(chunkDurationMs);
}

void AudioWizardMain::SetMonitoringRefreshRate(int refreshRateMs) {
	mainRealTime->SetMonitoringRefreshRate(refreshRateMs);
}
#pragma endregion


////////////////////////////////////////////////////
// * PUBLIC API - FULL-TRACK ANALYSIS CALLBACKS * //
////////////////////////////////////////////////////
#pragma region Public API - Full Track Analysis Callbacks
void AudioWizardMain::SetFullTrackAnalysisCallback(const VARIANT* callback) {
	AWHCOM::CreateCallback(callbacks.fullTrackAnalysisCallback, callback, "FullTrackAnalysis");
}

void AudioWizardMain::SetFullTrackWaveformCallback(const VARIANT* callback) {
	AWHCOM::CreateCallback(callbacks.fullTrackWaveformCallback, callback, "FullTrackWaveform");
}
#pragma endregion


//////////////////////////////////////////////////
// * PUBLIC API - FULL-TRACK ANALYSIS CONTROL * //
//////////////////////////////////////////////////
#pragma region Public API - Full-Track Analysis Control
void AudioWizardMain::StartFullTrackAnalysis(const metadb_handle_list& metadata, int chunkDurationMs) {
	if (metadata.get_count() == 0) {
		FB2K_console_formatter() << "Audio Wizard => StartFullTrackAnalysis: No tracks selected, cannot start analysis.";
		AWHCOM::FireCallback(callbacks.fullTrackAnalysisCallback, false);
		return;
	}

	mainFullTrack->StartFullTrackAnalysis(metadata, chunkDurationMs);
}

void AudioWizardMain::StopFullTrackAnalysis() {
	mainFullTrack->StopFullTrackAnalysis();
}

void AudioWizardMain::StartFullTrackWaveform(const metadb_handle_list& metadata, int chunkDurationMs) {
	if (metadata.get_count() == 0) {
		FB2K_console_formatter() << "Audio Wizard => StartFullTrackWaveform: No track provided, cannot start analysis.";
		AWHCOM::FireCallback(callbacks.fullTrackWaveformCallback, false);
		return;
	}

	mainFullTrack->StartFullTrackWaveform(metadata, chunkDurationMs);
}

void AudioWizardMain::StopFullTrackWaveform() {
	mainFullTrack->StopFullTrackWaveform();
}

void AudioWizardMain::StopFullTrackAudioProcessor() {
	mainFullTrack->StopFullTrackAnalysis();
	mainFullTrack->StopFullTrackWaveform();
}
#pragma endregion


/////////////////////////////////////////////
// * PUBLIC API - FULL-TRACK DATA ACCESS * //
/////////////////////////////////////////////
#pragma region Public API - Full Track Data Access
bool AudioWizardMain::GetFullTrackAnalysis() const {
	metadb_handle_list tracks;
	static_api_ptr_t<playlist_manager> playlistManager;

	const t_size playlistIndex = playlistManager->get_active_playlist();
	playlistManager->playlist_get_selected_items(playlistIndex, tracks);

	return mainFullTrack->GetFullTrackAnalysisForDialog(tracks);
}

void AudioWizardMain::GetFullTrackMetrics(SAFEARRAY** fullTrackMetrics) const {
	mainFullTrack->GetFullTrackMetrics(fullTrackMetrics);
}

double AudioWizardMain::GetMomentaryLUFSFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetMomentaryLUFSFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetShortTermLUFSFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetShortTermLUFSFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetIntegratedLUFSFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetIntegratedLUFSFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetRMSFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetRMSFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetSamplePeakFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetSamplePeakFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetTruePeakFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetTruePeakFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetPSRFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetPSRFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetPLRFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetPLRFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetCrestFactorFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetCrestFactorFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetLoudnessRangeFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetLoudnessRangeFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetDynamicRangeFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetDynamicRangeFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

double AudioWizardMain::GetPureDynamicsFull(LONG trackIndex) const {
	metadb_handle_ptr track;
	if (!ValidateTrackAndAnalysis(track, trackIndex)) return -INFINITY;

	int readIndex = mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire);
	return AudioWizardAnalysisFullTrack::GetPureDynamicsFull(*mainFullTrack->analysis.fullTrackData[readIndex][trackIndex]);
}

std::map<std::wstring, double> AudioWizardMain::GetAlbumMetricFull(
	const std::function<double(const AudioWizardAnalysisFullTrack::FullTrackResults&)>& metricAccessor) const {

	auto index = static_cast<size_t>(mainFullTrack->analysis.fullTrackIndex.load(std::memory_order_acquire));
	if (index >= mainFullTrack->analysis.fullTrackData.size()) {
		return {};
	}

	const auto& tracks = mainFullTrack->analysis.fullTrackData[index];
	if (tracks.empty()) {
		return {};
	}

	std::vector<AudioWizardAnalysisFullTrack::FullTrackResults> results;
	results.reserve(tracks.size());

	for (const auto& track : tracks) {
		if (track) {
			AudioWizardAnalysisFullTrack::FullTrackResults result;
			AudioWizardAnalysisFullTrack::ProcessFullTrackResults(track->handle, *track, result);
			results.push_back(result);
		}
	}

	return AudioWizardAnalysisFullTrack::GetAlbumMetricFull(results, metricAccessor);
}
#pragma endregion


///////////////////////////////////////////////////
// * PUBLIC API - REAL-TIME MONITORING CONTROL * //
///////////////////////////////////////////////////
#pragma region Public API - Real-Time Monitoring Control
void AudioWizardMain::StartRealTimeMonitoring(int refreshRateMs, int chunkDurationMs) {
	mainRealTime->StartRealTimeMonitoring(refreshRateMs, chunkDurationMs);
}

void AudioWizardMain::StopRealTimeMonitoring() {
	mainRealTime->StopRealTimeMonitoring();
}

void AudioWizardMain::StartPeakmeterMonitoring(int refreshRateMs, int chunkDurationMs) {
	mainRealTime->StartPeakmeterMonitoring(refreshRateMs, chunkDurationMs);
}

void AudioWizardMain::StopPeakmeterMonitoring() {
	mainRealTime->StopPeakmeterMonitoring();
}

void AudioWizardMain::StartRawAudioMonitoring(int refreshRateMs, int chunkDurationMs) {
	mainRealTime->StartRawAudioMonitoring(refreshRateMs, chunkDurationMs);
}

void AudioWizardMain::StopRawAudioMonitoring() {
	mainRealTime->StopRawAudioMonitoring();
}

void AudioWizardMain::StopRealTimeAudioProcessor() {
	mainRealTime->StopPeakmeterMonitoring();
	mainRealTime->StopRealTimeMonitoring();
	mainRealTime->StopRawAudioMonitoring();
}
#pragma endregion


////////////////////////////////////////////
// * PUBLIC API - REAL-TIME DATA ACCESS * //
////////////////////////////////////////////
#pragma region Public API - Real-Time Data Access
void AudioWizardMain::GetRawAudioData(SAFEARRAY** data) const {
	mainRealTime->GetRawAudioData(data);
}

void AudioWizardMain::GetMomentaryLUFS(double* value) const {
	*value = mainRealTime->metrics.momentaryLUFS.load();
}

void AudioWizardMain::GetShortTermLUFS(double* value) const {
	*value = mainRealTime->metrics.shortTermLUFS.load();
}

void AudioWizardMain::GetLeftRMS(double* value) const {
	*value = mainRealTime->metrics.leftRMS.load();
}

void AudioWizardMain::GetRightRMS(double* value) const {
	*value = mainRealTime->metrics.rightRMS.load();
}

void AudioWizardMain::GetLeftSamplePeak(double* value) const {
	*value = mainRealTime->metrics.leftSamplePeak.load();
}

void AudioWizardMain::GetRightSamplePeak(double* value) const {
	*value = mainRealTime->metrics.rightSamplePeak.load();
}

void AudioWizardMain::GetTruePeak(double* value) const {
	*value = mainRealTime->metrics.truePeak.load();
}

void AudioWizardMain::GetRMS(double* value) const {
	*value = mainRealTime->metrics.RMS.load();
}

void AudioWizardMain::GetPSR(double* value) const {
	*value = mainRealTime->metrics.PSR.load();
}

void AudioWizardMain::GetPLR(double* value) const {
	*value = mainRealTime->metrics.PLR.load();
}

void AudioWizardMain::GetCrestFactor(double* value) const {
	*value = mainRealTime->metrics.crestFactor.load();
}

void AudioWizardMain::GetDynamicRange(double* value) const {
	*value = mainRealTime->metrics.DR.load();
}

void AudioWizardMain::GetPureDynamics(double* value) const {
	*value = mainRealTime->metrics.PD.load();
}

void AudioWizardMain::GetPhaseCorrelation(double* value) const {
	*value = mainRealTime->metrics.phaseCorrelation.load();
}

void AudioWizardMain::GetStereoWidth(double* value) const {
	*value = mainRealTime->metrics.stereoWidth.load();
}
#pragma endregion


/////////////////////////
// * PRIVATE METHODS * //
/////////////////////////
#pragma region Private Methods
bool AudioWizardMain::IsFullTrackSelected(metadb_handle_ptr& track) const {
	metadb_handle_list tracks;
	static_api_ptr_t<playlist_manager> playlistManager;

	const t_size playlistIndex = playlistManager->get_active_playlist();
	playlistManager->playlist_get_selected_items(playlistIndex, tracks);

	if (tracks.get_count() == 0) {
		FB2K_console_formatter() << "Audio Wizard => IsFullTrackSelected: No track selected in playlist";
		return false;
	}

	track = tracks[0]; // Use the first selected track
	return true;
}

bool AudioWizardMain::ValidateTrackAndAnalysis(metadb_handle_ptr& track, LONG trackIndex) const {
	metadb_handle_list tracks;
	static_api_ptr_t<playlist_manager> playlistManager;
	const t_size playlistIndex = playlistManager->get_active_playlist();
	playlistManager->playlist_get_selected_items(playlistIndex, tracks);

	if (trackIndex < 0 || static_cast<t_size>(trackIndex) >= tracks.get_count()) {
		FB2K_console_formatter() << "Audio Wizard => ValidateTrackAndAnalysis: Invalid track index " << trackIndex;
		return false;
	}

	if (!mainFullTrack->monitor.isFullTrackMetricsComplete.load(std::memory_order_acquire)) {
		FB2K_console_formatter() << "Audio Wizard => ValidateTrackAndAnalysis: Analysis not complete for track index " << trackIndex;
		return false;
	}

	track = tracks[trackIndex];
	return true;
}
#pragma endregion
