/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Main Full-Track Source File                * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_MainFullTrack.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardMainFullTrack::AudioWizardMainFullTrack() {
}
#pragma endregion


///////////////////////////////////
// * PUBLIC PROCESSING CONTROL * //
///////////////////////////////////
#pragma region Public Processing Control
bool AudioWizardMainFullTrack::GetFullTrackAnalysisForDialog(const metadb_handle_list& tracks) {
	if (tracks.get_count() == 0) {
		fb2k::inMainThread([] {
			uMessageBox(core_api::get_main_window(), "No track selected in the playlist.", "Analysis Error", MB_OK);
		});

		return false;
	}

	fetcher.isFullTrackFetching.store(true, std::memory_order_release);

	AnalysisResult result;
	auto startTime = std::chrono::steady_clock::now();
	bool anyResults = false;
	bool isCanceled = false;
	pfc::string8 errors;

	auto callback = threaded_process_callback_lambda::create([this, &result, &isCanceled, &tracks, &errors]
	(threaded_process_status& status, abort_callback const& abort) {
		status.set_title(tracks.get_count() == 1 ? "Analyzing audio file..." : "Analyzing audio files...");
		try {
			ProcessFullTracksForDialog(tracks, abort, status, result.results, result.totalDuration);
		}
		catch (const foobar2000_io::exception_aborted&) {
			isCanceled = true;
			errors.add_string("Analysis was canceled by the user.\n");
		}
		catch (const std::exception& e) {
			pfc::string8 msg;
			msg << "Critical failure: " << e.what() << "\n";
			errors.add_string(msg);
		}
		catch (...) {
			errors.add_string("Unknown critical failure\n");
		}
	});

	threaded_process::get()->run_modal(callback,
		threaded_process::flag_show_abort | threaded_process::flag_show_progress | threaded_process::flag_show_item,
		core_api::get_main_window(), "Audio Analysis Progress"
	);

	anyResults = !result.results.empty();
	ProcessFullTrackResultsForDialog(std::move(result.results), result.totalDuration, startTime);

	fetcher.isFullTrackFetching.store(false, std::memory_order_release);

	if (!errors.is_empty()) {
		fb2k::inMainThread([errors]() {
			uMessageBox(core_api::get_main_window(), errors, "Analysis Results", MB_OK);
		});
	}

	return !isCanceled && anyResults;
}

void AudioWizardMainFullTrack::GetFullTrackMetrics(SAFEARRAY** fullTrackMetrics) const {
	if (!fullTrackMetrics) {
		FB2K_console_formatter() << "Audio Wizard => GetFullTrackMetrics: Invalid metrics pointer";
		return;
	}

	const auto& tracks = analysis.lastAnalyzedTracks;
	const size_t numTracks = tracks.get_count();
	const size_t metricsPerTrack = 12; // M LUFS, S LUFS, I LUFS, RMS, SP, TP, PSR, PLR, CF, LRA, DR, PD
	std::vector<float> allMetrics(numTracks * metricsPerTrack, -INFINITY);

	if (numTracks == 0) {
		FB2K_console_formatter() << "Audio Wizard => GetFullTrackMetrics: No tracks analyzed";
		*fullTrackMetrics = AWHCOM::CreateSafeArrayFromData(allMetrics.begin(), allMetrics.end(), "GetFullTrackMetrics");
		return;
	}

	if (!monitor.isFullTrackMetricsComplete.load(std::memory_order_acquire)) {
		FB2K_console_formatter() << "Audio Wizard => GetFullTrackMetrics: Analysis not complete";
		*fullTrackMetrics = AWHCOM::CreateSafeArrayFromData(allMetrics.begin(), allMetrics.end(), "GetFullTrackMetrics");
		return;
	}

	int readIndex = analysis.fullTrackIndex.load(std::memory_order_acquire);
	if (numTracks != analysis.fullTrackData[readIndex].size()) {
		FB2K_console_formatter() << "Audio Wizard => GetFullTrackMetrics: Mismatch between analyzed tracks count and data size";
		*fullTrackMetrics = AWHCOM::CreateSafeArrayFromData(allMetrics.begin(), allMetrics.end(), "GetFullTrackMetrics");
		return;
	}

	for (t_size i = 0; i < numTracks; ++i) {
		const auto& data = *analysis.fullTrackData[readIndex][i];
		size_t offset = i * metricsPerTrack;
		allMetrics[offset + 0] = static_cast<float>(AudioWizardAnalysisFullTrack::GetMomentaryLUFSFull(data));
		allMetrics[offset + 1] = static_cast<float>(AudioWizardAnalysisFullTrack::GetShortTermLUFSFull(data));
		allMetrics[offset + 2] = static_cast<float>(AudioWizardAnalysisFullTrack::GetIntegratedLUFSFull(data));
		allMetrics[offset + 3] = static_cast<float>(AudioWizardAnalysisFullTrack::GetRMSFull(data));
		allMetrics[offset + 4] = static_cast<float>(AudioWizardAnalysisFullTrack::GetSamplePeakFull(data));
		allMetrics[offset + 5] = static_cast<float>(AudioWizardAnalysisFullTrack::GetTruePeakFull(data));
		allMetrics[offset + 6] = static_cast<float>(AudioWizardAnalysisFullTrack::GetPSRFull(data));
		allMetrics[offset + 7] = static_cast<float>(AudioWizardAnalysisFullTrack::GetPLRFull(data));
		allMetrics[offset + 8] = static_cast<float>(AudioWizardAnalysisFullTrack::GetCrestFactorFull(data));
		allMetrics[offset + 9] = static_cast<float>(AudioWizardAnalysisFullTrack::GetLoudnessRangeFull(data));
		allMetrics[offset + 10] = static_cast<float>(AudioWizardAnalysisFullTrack::GetDynamicRangeFull(data));
		allMetrics[offset + 11] = static_cast<float>(AudioWizardAnalysisFullTrack::GetPureDynamicsFull(data));
	}

	*fullTrackMetrics = AWHCOM::CreateSafeArrayFromData(allMetrics.begin(), allMetrics.end(), "GetFullTrackMetrics");
}

void AudioWizardMainFullTrack::SetFullTrackChunkDuration(int chunkDurationMs) {
	int clampedDuration = std::clamp(chunkDurationMs, Config::MIN_CHUNK_DURATION_MS, Config::MAX_CHUNK_DURATION_MS);
	monitor.monitorChunkDurationMs.store(clampedDuration, std::memory_order_release);
	AWHDebug::DebugLog("SetFullTrackChunkDuration: ", clampedDuration, "ms");
}

void AudioWizardMainFullTrack::StartFullTrackAnalysis(const metadb_handle_list& tracks, int chunkDurationMs) {
	if (fetcher.isFullTrackFetching.load(std::memory_order_acquire)) {
		AWHDebug::DebugLog("StartFullTrackAnalysis: Analysis already in progress, skipping.");
		return;
	}

	if (monitor.isFullTrackMetricsActive.load()) {
		AWHDebug::DebugLog("StartFullTrackAnalysis: Analysis active, stopping and restarting.");
		StopFullTrackAnalysis();
	}

	fetcher.isFullTrackFetching.store(true, std::memory_order_release);
	monitor.isFullTrackMetricsActive.store(true, std::memory_order_release);

	int writeIndex = (analysis.fullTrackIndex.load(std::memory_order_acquire) + 1) % 2;
	analysis.fullTrackData[writeIndex].clear();
	analysis.fullTrackData[writeIndex].reserve(tracks.get_count());
	for (t_size i = 0; i < tracks.get_count(); ++i) {
		analysis.fullTrackData[writeIndex].emplace_back(std::make_unique<FullTrackData>());
	}
	analysis.fullTrackIndex.store(writeIndex, std::memory_order_release);
	SetFullTrackChunkDuration(chunkDurationMs);

	fetcher.fullTrackFetcherFuture = std::async(std::launch::async, [this, tracks, writeIndex] {
		bool success = true;
		try {
			abort_callback_impl abort;
			bool fullTrackMetricsActive = monitor.isFullTrackMetricsActive.load();

			for (t_size i = 0; i < tracks.get_count(); ++i) {
				FullTrackAudioDecoder(tracks[i], *analysis.fullTrackData[writeIndex][i], nullptr, abort, fullTrackMetricsActive, false);
				abort.check();
			}

			if (fullTrackMetricsActive) {
				analysis.lastAnalyzedTracks = tracks;
				monitor.isFullTrackMetricsComplete.store(true, std::memory_order_release);
				AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackAnalysisCallback, true);
				monitor.isFullTrackMetricsActive.store(false, std::memory_order_release);
			}
		}
		catch (const std::exception& e) {
			FB2K_console_formatter() << "Audio Wizard => Full-track multi-track analysis failed: " << e.what();
			monitor.isFullTrackMetricsComplete.store(false, std::memory_order_release);
			success = false;
		}
		fetcher.isFullTrackFetching.store(false, std::memory_order_release);

		if (!success) { // Fire failure for relevant callbacks
			AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackAnalysisCallback, false);
		}
	});
}

void AudioWizardMainFullTrack::StopFullTrackAnalysis() {
	if (!monitor.isFullTrackMetricsActive.load()) return;

	monitor.isFullTrackMetricsActive.store(false, std::memory_order_release);

	if (fetcher.fullTrackFetcherFuture.valid()) {
		fetcher.fullTrackFetcherFuture.wait();
	}
}

void AudioWizardMainFullTrack::StartFullTrackWaveform(const metadb_handle_list& tracks, int chunkDurationMs) {
	if (tracks.get_count() == 0) {
		FB2K_console_formatter() << "Audio Wizard => StartFullTrackWaveform: No tracks provided";
		AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackWaveformCallback, false);
		return;
	}

	if (monitor.isFullTrackWaveformActive.load()) {
		AWHDebug::DebugLog("StartFullTrackWaveform: Waveform active, stopping and restarting");
		StopFullTrackWaveform();
	}

	if (fetcher.isFullTrackFetching.load(std::memory_order_acquire)) {
		AWHDebug::DebugLog("StartFullTrackWaveform: Fetching already in progress, skipping");
		return;
	}

	fetcher.isFullTrackFetching.store(true, std::memory_order_release);
	monitor.isFullTrackWaveformActive.store(true, std::memory_order_release);
	monitor.waveformChunkDurationMs.store(chunkDurationMs, std::memory_order_release);

	AWHDebug::DebugLog("StartFullTrackWaveform: Processing ", tracks.get_count(),
		" tracks at ", chunkDurationMs, "ms chunks");

	// Prepare dummy data structures
	int writeIndex = (analysis.fullTrackIndex.load(std::memory_order_acquire) + 1) % 2;
	analysis.fullTrackData[writeIndex].clear();
	analysis.fullTrackData[writeIndex].reserve(tracks.get_count());
	for (t_size i = 0; i < tracks.get_count(); ++i) {
		analysis.fullTrackData[writeIndex].emplace_back(std::make_unique<FullTrackData>());
	}
	analysis.fullTrackIndex.store(writeIndex, std::memory_order_release);
	analysis.lastAnalyzedTracks = tracks;

	fetcher.fullTrackFetcherFuture = std::async(std::launch::async, [this, tracks, writeIndex] {
		bool success = true;
		try {
			abort_callback_impl abort;
			auto* waveform = AudioWizard::Waveform();

			// Process each track sequentially
			for (t_size i = 0; i < tracks.get_count(); ++i) {
				if (!monitor.isFullTrackWaveformActive.load(std::memory_order_acquire)) {
					AWHDebug::DebugLog("StartFullTrackWaveform: Aborted at track ", i);
					break;
				}

				waveform->state.currentTrackIndex.store(i, std::memory_order_release);

				AWHDebug::DebugLog("StartFullTrackWaveform: Processing track ", i, " - ", tracks[i]->get_path());

				// Decode and process this track
				FullTrackAudioDecoder(tracks[i], *analysis.fullTrackData[writeIndex][i], nullptr, abort, false, true);
				abort.check();
			}

			if (monitor.isFullTrackWaveformActive.load(std::memory_order_acquire)) {
				monitor.isFullTrackMetricsComplete.store(true, std::memory_order_release);
				AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackWaveformCallback, true, [] {
					AudioWizard::Waveform()->StopWaveformAnalysis();
				});
				monitor.isFullTrackWaveformActive.store(false, std::memory_order_release);
			}
		}
		catch (const std::exception& e) {
			FB2K_console_formatter() << "Audio Wizard => Waveform batch failed: " << e.what();
			success = false;
		}
		fetcher.isFullTrackFetching.store(false, std::memory_order_release);

		if (!success) {
			AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackWaveformCallback, false, [] {
					AudioWizard::Waveform()->StopWaveformAnalysis();
				});
			}
		});
}

void AudioWizardMainFullTrack::StopFullTrackWaveform() {
	if (!monitor.isFullTrackWaveformActive.load()) return;

	monitor.isFullTrackWaveformActive.store(false, std::memory_order_release);

	if (fetcher.fullTrackFetcherFuture.valid()) {
		fetcher.fullTrackFetcherFuture.wait();
	}
}
#pragma endregion


//////////////////////////////////
// * PRIVATE AUDIO PROCESSING * //
//////////////////////////////////
#pragma region Private Audio Processing
void AudioWizardMainFullTrack::FullTrackAudioDecoder(const metadb_handle_ptr& track, FullTrackData& ftData, FullTrackResults* results,
	abort_callback& abort, bool processMetrics, bool processWaveform, threaded_process_status* status) const {

	service_ptr_t<input_decoder> decoder;
	service_ptr_t<file> fileHandle;
	const auto& location = track->get_location();
	input_entry::g_open_for_decoding(decoder, fileHandle, location.get_path(), abort, false);
	decoder->initialize(location.get_subsong_index(), 0, abort);

	audio_chunk_impl chunk;
	ftData.handle = track;

	std::vector<audioType> audioBuffer;
	unsigned channels = 0;
	double sampleRate = 0.0;
	t_size currentFrames = 0;
	t_size targetFrames = 0;
	double totalFrames = 0.0;
	double processedDuration = 0.0;
	double trackDuration = track->get_length();

	int chunkDurationMs = monitor.monitorChunkDurationMs.load(std::memory_order_acquire);
	if (monitor.isFullTrackWaveformActive.load(std::memory_order_acquire)) {
		chunkDurationMs = monitor.waveformChunkDurationMs.load(std::memory_order_acquire);
		AWHDebug::DebugLog("FullTrackAudioDecoder: Using waveform-specific duration ", chunkDurationMs, "ms");
	}

	bool fullTrackMetricsActive = processMetrics && monitor.isFullTrackMetricsActive.load(std::memory_order_acquire);
	bool fullTrackWaveformActive = processWaveform && monitor.isFullTrackWaveformActive.load(std::memory_order_acquire);

	auto processChunk = [&](t_size framesToProcess) {
		if (framesToProcess == 0) return;

		std::vector<audioType> processSamples(
			audioBuffer.begin(), audioBuffer.begin() + framesToProcess * channels
		);

		AWHAudioData::ChunkData processData;
		processData.setOwnedData(std::move(processSamples));
		processData.channels = channels;
		processData.frames = framesToProcess;
		processData.sampleRate = sampleRate;

		totalFrames += processData.frames;
		processedDuration += processData.frames / sampleRate;

		if (fullTrackMetricsActive || results) {
			AudioWizardAnalysisFullTrack::ProcessFullTrackChunk(processData, ftData);
		}
		if (fullTrackWaveformActive) {
			AudioWizard::Waveform()->ProcessWaveformMetrics(processData);
		}
	};

	while (decoder->run(chunk, abort)) {
		if (channels == 0) {
			channels = chunk.get_channels();
			sampleRate = chunk.get_srate();
			targetFrames = static_cast<t_size>(sampleRate * (chunkDurationMs / 1000.0));
			if (targetFrames < 1) targetFrames = 1;
			audioBuffer.reserve(targetFrames * channels * 2); // Pre-reserve lightly
		}

		const auto* chunkData = chunk.get_data();
		t_size chunkFrames = chunk.get_sample_count();
		audioBuffer.insert(audioBuffer.end(), chunkData, chunkData + chunkFrames * channels);
		currentFrames += chunkFrames;

		while (currentFrames >= targetFrames) {
			processChunk(targetFrames);
			audioBuffer.erase(audioBuffer.begin(), audioBuffer.begin() + targetFrames * channels);
			currentFrames -= targetFrames;

			if (status && trackDuration > 0 && !analysis.isBatchProcessing.load(std::memory_order_acquire)) {
				double progress = processedDuration / trackDuration;
				status->set_progress_float(progress);
				status->force_update();
			}
			abort.check();
		}
	}

	// Drain remainder
	processChunk(currentFrames);

	// Final processing
	if (fullTrackMetricsActive || results) {
		AudioWizardAnalysisFullTrack::ProcessOriginalBlocks(ftData);
		AudioWizardAnalysisFullTrack::ProcessDynamicsFactors(ftData);
	}

	if (results) {
		AudioWizardAnalysisFullTrack::ProcessFullTrackResults(track, ftData, *results);
		AudioWizardAnalysisFullTrack::ResetFullTrackData(ftData);
	}

	AWHDebug::DebugLog("ProcessAudioChunks: Completed, duration: ", (totalFrames / sampleRate), "s, frames: ", totalFrames);
}

void AudioWizardMainFullTrack::FullTrackAudioProcessor(const metadb_handle_ptr& track, FullTrackResults* results, threaded_process_status* status) {
	if (results) { // Processing for analysis dialog
		FullTrackData ftData = {};
		try {
			abort_callback_impl abort;
			FullTrackAudioDecoder(track, ftData, results, abort, false, false, status);
		}
		catch (const std::exception& e) {
			FB2K_console_formatter() << "Audio Wizard => Full-track batch analysis failed: " << e.what();
		}
	}
	else { // Processing for external API usage
		fetcher.isFullTrackFetching.store(true, std::memory_order_release);
		int writeIndex = (analysis.fullTrackIndex.load(std::memory_order_acquire) + 1) % 2;
		analysis.fullTrackIndex.store(writeIndex, std::memory_order_release);
		analysis.fullTrackData[writeIndex].clear();
		analysis.fullTrackData[writeIndex].emplace_back(std::make_unique<FullTrackData>());

		fetcher.fullTrackFetcherFuture = std::async(std::launch::async, [this, track, writeIndex] {
			bool success = true;
			try {
				abort_callback_impl abort;
				bool fullTrackMetricsActive = monitor.isFullTrackMetricsActive.load();
				bool fullTrackWaveformActive = monitor.isFullTrackWaveformActive.load();

				FullTrackAudioDecoder(track, *analysis.fullTrackData[writeIndex][0], nullptr, abort, fullTrackMetricsActive, fullTrackWaveformActive);

				if (fullTrackWaveformActive) {
					AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackWaveformCallback, true, [] {
						AudioWizard::Waveform()->StopWaveformAnalysis();
					});
					monitor.isFullTrackWaveformActive.store(false, std::memory_order_release);
				}

				if (fullTrackMetricsActive) {
					analysis.fullTrackIndex.store(writeIndex, std::memory_order_release);
					monitor.isFullTrackMetricsComplete.store(true, std::memory_order_release);
					analysis.lastAnalyzedTrack = track;
					AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackAnalysisCallback, true);
					monitor.isFullTrackMetricsActive.store(false, std::memory_order_release);
				}
			}
			catch (const std::exception& e) {
				FB2K_console_formatter() << "Audio Wizard => Full-track real-time analysis failed: " << e.what();
				monitor.isFullTrackMetricsComplete.store(false, std::memory_order_release);
				success = false;
			}
			fetcher.isFullTrackFetching.store(false, std::memory_order_release);

			if (!success) { // Fire failure for relevant callbacks
				if (monitor.isFullTrackWaveformActive.load()) {
					AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackWaveformCallback, false, [] {
						AudioWizard::Waveform()->StopWaveformAnalysis();
					});
				}
				if (monitor.isFullTrackMetricsActive.load()) {
					AWHCOM::FireCallback(AudioWizard::Main()->callbacks.fullTrackAnalysisCallback, false);
				}
			}
		});
	}
}
#pragma endregion


//////////////////////////////////////////////////
// * PRIVATE AUDIO PROCESSING ANALYSIS DIALOG * //
//////////////////////////////////////////////////
#pragma region Private Audio Processing Analysis Dialog
void AudioWizardMainFullTrack::ProcessFullTracksForDialog(const metadb_handle_list& tracks, abort_callback const& abort,
	threaded_process_status& status, std::vector<FullTrackResults>& results, double& totalDuration) {

	bool multiTracksSelected = tracks.get_count() > 1;
	analysis.isBatchProcessing.store(multiTracksSelected, std::memory_order_release);
	std::deque<std::pair<t_size, std::future<void>>> activeFutures;
	t_size nextTrack = 0;
	t_size completedTracks = 0;
	const t_size totalTracks = tracks.get_count();
	const t_size maxConcurrent = std::max(t_size{ 1 }, t_size{ std::thread::hardware_concurrency() });

	results.clear();
	results.resize(totalTracks);
	status.set_progress(0, totalTracks);
	status.set_item("Processing tracks...");
	status.force_update();
	totalDuration = 0.0;

	auto updateProgressBar = [&status, totalTracks, &tracks](t_size finishedTracks, t_size index) {
		status.set_progress(finishedTracks, totalTracks);
		status.set_item_path(tracks[index]->get_path());
		status.force_update();
	};

	while (completedTracks < totalTracks) {
		if (activeFutures.size() < maxConcurrent && nextTrack < totalTracks) {
			abort.check();
			t_size currentIndex = nextTrack;
			activeFutures.emplace_back(currentIndex, std::async(std::launch::async, [this, track = tracks[currentIndex], &results, currentIndex, &status]{
				FullTrackAudioProcessor(track, &results[currentIndex], &status);
			}));
			++nextTrack;
		}

		for (auto it = activeFutures.begin(); it != activeFutures.end(); ) {
			if (it->second.wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
				t_size index = it->first;
				try {
					it->second.get();
					totalDuration += tracks[index]->get_length();
					completedTracks++;
					updateProgressBar(completedTracks, index);
				}
				catch (const std::exception& e) {
					FB2K_console_formatter() << "Error processing track " << tracks[index]->get_path() << ": " << e.what();
				}
				it = activeFutures.erase(it);
			}
			else {
				++it;
			}
		}
	}

	analysis.isBatchProcessing.store(false, std::memory_order_release);
}

void AudioWizardMainFullTrack::ProcessFullTrackResultsForDialog(std::vector<FullTrackResults>&& results,
	double totalDuration, const std::chrono::steady_clock::time_point& startTime) const {
	if (results.empty()) return;

	// Compute album-metrics
	auto dynamicRangeAlbum = AudioWizardAnalysisFullTrack::GetAlbumMetricFull(
		results, [](const FullTrackResults& r) { return r.dynamicRange; }
	);
	auto pureDynamicsAlbum = AudioWizardAnalysisFullTrack::GetAlbumMetricFull(
		results, [](const FullTrackResults& r) { return r.pureDynamics; }
	);
	for (auto& result : results) {
		result.dynamicRangeAlbum = dynamicRangeAlbum[result.album];
		result.pureDynamicsAlbum = pureDynamicsAlbum[result.album];
	}

	std::wstring timeStr = AWHString::GetProcessingTime(startTime, 2);
	std::wstring speedStr = AWHString::GetProcessingSpeed(totalDuration, startTime, 2);

	fb2k::inMainThread([capturedResults = std::move(results), capturedTimeStr = std::move(timeStr),
		capturedSpeedStr = std::move(speedStr)]() mutable {
		AudioWizard::SetFullTrackDialog(std::make_unique<AudioWizardDialogFullTrack>(
			std::move(capturedResults), capturedTimeStr.c_str(), capturedSpeedStr.c_str()
		));
		AudioWizard::DialogFullTrack()->Create(core_api::get_main_window());
		AudioWizard::DialogFullTrack()->CenterWindow();
		AudioWizard::DialogFullTrack()->ShowWindow(SW_SHOW);
	});
}
#pragma endregion
