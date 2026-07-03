/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Full-Track Header File                * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.6.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    03-07-2026                                              * //
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
		static constexpr int FULL_METRICS_DATA_VERSION = 1; // NOTE: bump whenever FULL_METRICS_PER_TRACK or FULL_METRIC_NAMES changes.
		static constexpr size_t FULL_METRICS_PER_TRACK = 12; // M LUFS, S LUFS, I LUFS, RMS, SP, TP, PSR, PLR, CF, LRA, DR, PD

		struct MetricEntry {
			std::string_view name;
			double (*accessor)(const FullTrackData&);
		};
		static constexpr std::array<MetricEntry, FULL_METRICS_PER_TRACK> FULL_METRICS = {{
			{ "M LUFS", &AudioWizardAnalysisFullTrack::GetMomentaryLUFSFull  },
			{ "S LUFS", &AudioWizardAnalysisFullTrack::GetShortTermLUFSFull  },
			{ "I LUFS", &AudioWizardAnalysisFullTrack::GetIntegratedLUFSFull },
			{ "RMS",    &AudioWizardAnalysisFullTrack::GetRMSFull            },
			{ "SP",     &AudioWizardAnalysisFullTrack::GetSamplePeakFull     },
			{ "TP",     &AudioWizardAnalysisFullTrack::GetTruePeakFull       },
			{ "PSR",    &AudioWizardAnalysisFullTrack::GetPSRFull            },
			{ "PLR",    &AudioWizardAnalysisFullTrack::GetPLRFull            },
			{ "CF",     &AudioWizardAnalysisFullTrack::GetCrestFactorFull    },
			{ "LRA",    &AudioWizardAnalysisFullTrack::GetLoudnessRangeFull  },
			{ "DR",     &AudioWizardAnalysisFullTrack::GetDynamicRangeFull   },
			{ "PD",     &AudioWizardAnalysisFullTrack::GetPureDynamicsFull   }
		}};

		static constexpr int DEF_CHUNK_DURATION_MS = 200;
		static constexpr int MIN_CHUNK_DURATION_MS = 10;
		static constexpr int MAX_CHUNK_DURATION_MS = 1000;
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
		std::atomic<int> monitorChunkDurationMs = Config::DEF_CHUNK_DURATION_MS;
		std::atomic<int> waveformChunkDurationMs = Config::MAX_CHUNK_DURATION_MS;
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
	void GetFullTrackMetricsDataInfo(pfc::string8& json) const;
	void SetFullTrackChunkDuration(int chunkDurationMs);
	void StartFullTrackAnalysis(const metadb_handle_list& tracks, int chunkDurationMs);
	void StopFullTrackAnalysis();
	void StartFullTrackWaveform(const metadb_handle_list& tracks, int chunkDurationMs);
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
