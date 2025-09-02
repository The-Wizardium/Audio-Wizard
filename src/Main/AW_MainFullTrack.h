/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Full-Track Header File                * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


/////////////////////////
// * MAIN FULL-TRACK * //
/////////////////////////
#pragma region Main Full-Track
class AudioWizardMainFullTrack {
public:
	// * TYPE ALIASES * //
	using ChunkData = AWHAudioData::ChunkData;
	using FullTrackResults = AudioWizardAnalysisFullTrack::FullTrackResults;
	using FullTrackData = AudioWizardAnalysisFullTrack::FullTrackData;

	// * MAIN CONFIG * //
	struct Config {
		static constexpr int MIN_CHUNK_DURATION_MS = 17;
		static constexpr int MAX_FULL_TRACK_CHUNK_MS = 200;
	};

	// * ANALYSIS RESULT * //
	struct AnalysisResult {
		double totalDuration = 0.0;
		std::vector<FullTrackResults> results;
	};

	// * ANALYSIS STATE * //
	struct AnalysisState {
		metadb_handle_ptr lastAnalyzedTrack = nullptr;
		metadb_handle_list lastAnalyzedTracks;
		std::vector<std::vector<std::unique_ptr<FullTrackData>>> fullTrackData;
		std::atomic<bool> isBatchProcessing = false;
		std::atomic<int> fullTrackIndex = 0;
		std::atomic<int> writeIndex = 0;
		AnalysisState() { fullTrackData.resize(2); }
	}; AnalysisState analysis;

	// * MONITORING STATE * //
	struct MonitorState {
		std::atomic<bool> isFullTrackMetricsComplete = false;
		std::atomic<bool> isFullTrackMetricsActive = false;
		std::atomic<bool> isFullTrackWaveformActive = false;
		std::atomic<int> monitorChunkDurationMs = Config::MAX_FULL_TRACK_CHUNK_MS;
	}; MonitorState monitor;

	// * FETCHER STATE * //
	struct FetcherState {
		std::atomic<bool> isFullTrackFetching = false;
		std::future<void> fullTrackFetcherFuture;
	}; FetcherState fetcher;

	// * CONSTRUCTOR & DESTRUCTOR * //
	AudioWizardMainFullTrack();
	~AudioWizardMainFullTrack() = default;

	// * PUBLIC PROCESSING CONTROL * //
	bool GetFullTrackAnalysisForDialog(const metadb_handle_list& tracks);
	void GetFullTrackMetrics(SAFEARRAY** fullTrackMetrics) const;
	void SetFullTrackChunkDuration(int chunkDurationMs);
	void StartFullTrackAnalysis(int chunkDurationMs, const metadb_handle_list& tracks);
	void StopFullTrackAnalysis();
	void StartFullTrackWaveform(int chunkDurationMs, const metadb_handle_ptr& track);
	void StopFullTrackWaveform();

private:
	// * PRIVATE AUDIO PROCESSING * //
	void FullTrackAudioDecoder(const metadb_handle_ptr& track, FullTrackData& ftData, FullTrackResults* results,
		abort_callback& abort, bool processMetrics = false, bool processWaveform = false, threaded_process_status* status = nullptr
	) const;
	void FullTrackAudioProcessor(const metadb_handle_ptr& track, FullTrackResults* results = nullptr, threaded_process_status* status = nullptr);

	// * PRIVATE AUDIO PROCESSING ANALYSIS DIALOG * //
	void ProcessFullTracksForDialog(const metadb_handle_list& tracks, abort_callback const& abort,
		threaded_process_status& status, std::vector<FullTrackResults>& results, double& totalDuration
	);
	void ProcessFullTrackResultsForDialog(std::vector<FullTrackResults>&& results, double totalDuration,
		const std::chrono::steady_clock::time_point& startTime
	) const;
};
#pragma endregion
