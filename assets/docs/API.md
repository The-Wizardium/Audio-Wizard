# Audio Wizard - API Reference

*Version 0.1 - Last Updated: 03.08.2025*

Audio Wizard provides a JavaScript API for real-time audio analysis and visualization in foobar2000, accessible via a COM/ActiveX interface in scripting environments like [Spider Monkey Panel](https://github.com/TheQwertiest/foo_spider_monkey_panel) or [JSplitter](https://foobar2000.ru/forum/viewtopic.php?t=6378).
This document is for foobar2000 JavaScript developers, offering practical examples and a complete API reference.

<br>
<br>

## Usage Examples

Instantiate the `AudioWizard` ActiveX object in foobar2000:

```javascript
const AudioWizard = new ActiveXObject('AudioWizard');
```

The following examples demonstrate how to use Audio Wizard for real-time monitoring, peakmeter visualization, raw audio processing,
full-track analysis, album-level analysis, and waveform generation. Refer to the [API Reference](#api-reference) for available properties and methods.

<br>
<br>

### Real-Time Monitoring

Retrieve and log real-time audio metrics:

```javascript
/**
 * Starts real-time audio monitoring and logs various audio metrics.
 */
function startRealTimeMonitoring() {
	if (!AudioWizard) return;

	AudioWizard.StartRealTimeMonitoring(17, 50); // 17ms refresh rate, 50ms chunk duration

	console.log('Real-time - Momentary LUFS:', AudioWizard.MomentaryLUFS.toFixed(2));
	console.log('Real-time - Short Term LUFS:', AudioWizard.ShortTermLUFS.toFixed(2));
	console.log('Real-time - RMS:', AudioWizard.RMS.toFixed(2));
	console.log('Real-time - Left RMS:', AudioWizard.LeftRMS.toFixed(2));
	console.log('Real-time - Right RMS:', AudioWizard.RightRMS.toFixed(2));
	console.log('Real-time - Left Sample Peak:', AudioWizard.LeftSamplePeak.toFixed(2));
	console.log('Real-time - Right Sample Peak:', AudioWizard.RightSamplePeak.toFixed(2));
	console.log('Real-time - True Peak:', AudioWizard.TruePeak.toFixed(2));
	console.log('Real-time - PSR:', AudioWizard.PSR.toFixed(2));
	console.log('Real-time - PLR:', AudioWizard.PLR.toFixed(2));
	console.log('Real-time - Crest Factor:', AudioWizard.CrestFactor.toFixed(2));
	console.log('Real-time - DR:', AudioWizard.DynamicRange.toFixed(2));
	console.log('Real-time - PD:', AudioWizard.PureDynamics.toFixed(2));
	console.log('Real-time - Phase Correlation:', AudioWizard.PhaseCorrelation.toFixed(2));
	console.log('Real-time - Stereo Width:', AudioWizard.StereoWidth.toFixed(2), '\n');
}
```

**Notes**:
- Adjust `refreshRate` and `chunkDuration` for performance.
- Use `toFixed(2)` for readable output.
- Call `StopRealTimeMonitoring()` to free resources.

<br>
<br>

### Peakmeter Monitoring

Retrieve and log adjusted RMS and sample peak levels for visual display:

```javascript
/**
 * Starts peakmeter monitoring and logs adjusted RMS and sample peak levels.
 */
function startPeakmeterMonitoring() {
	if (!AudioWizard) return;

	AudioWizard.StartPeakmeterMonitoring(17, 50); // 17ms refresh rate, 50ms chunk duration

	console.log('Peakmeter - Adjusted Left RMS:', AudioWizard.PeakmeterAdjustedLeftRMS.toFixed(2));
	console.log('Peakmeter - Adjusted Right RMS:', AudioWizard.PeakmeterAdjustedRightRMS.toFixed(2));
	console.log('Peakmeter - Adjusted Left Sample Peak:', AudioWizard.PeakmeterAdjustedLeftSamplePeak.toFixed(2));
	console.log('Peakmeter - Adjusted Right Sample Peak:', AudioWizard.PeakmeterAdjustedRightSamplePeak.toFixed(2));
	console.log('Peakmeter - Offset:', AudioWizard.PeakmeterOffset.toFixed(2), '\n');

	// Call AudioWizard.StopPeakmeterMonitoring() when done
}
```

**Notes**:
- Peakmeter properties return adjusted dBFS values (typically -5 to +5 dB), not raw RMS or sample peak values.
- Use `PeakmeterOffset` to adjust gain (set as an integer, e.g., `-20 to +20` dB).
- Call `StopPeakmeterMonitoring()` to free resources.
- Adjust `refreshRate` and `chunkDuration` for responsiveness.

<br>
<br>

### Raw Audio Data

Access and analyze raw PCM audio samples:

```javascript
/**
 * Starts raw audio monitoring and processes raw PCM audio samples.
 */
function startRawAudioMonitoring() {
	if (!AudioWizard) return;

	AudioWizard.StartRawAudioMonitoring(17, 50); // 17ms refresh rate, 50ms chunk duration

	try {
		const startTime = Date.now();
		const rawData = AudioWizard.RawAudioData;

		if (rawData) {
			const accessTime = Date.now() - startTime;

			console.log('RawData length:', rawData.length);
			console.log('RawData first element:', rawData[0]);
			console.log('RawData middle element:', rawData[Math.floor(rawData.length / 2)]);
			console.log('RawData current element:', rawData[rawData.length - 1]);
			console.log('Array access time (ms):', accessTime.toFixed(2));
			console.log('Last 5 samples:', rawData.slice(-5).join(', '));

			const rmsStartTime = Date.now();
			const windowSize = 100;
			const startIndex = Math.max(0, rawData.length - windowSize);

			const leftSamples = rawData.filter((_, i) => i >= startIndex && i % 2 === 0);
			const rightSamples = rawData.filter((_, i) => i >= startIndex && i % 2 !== 0);
			const sumSquaresLeft = leftSamples.reduce((sum, x) => sum + x * x, 0);
			const sumSquaresRight = rightSamples.reduce((sum, x) => sum + x * x, 0);
			const rmsLeft = leftSamples.length > 0 ? Math.sqrt(sumSquaresLeft / leftSamples.length) : 0;
			const rmsRight = rightSamples.length > 0 ? Math.sqrt(sumSquaresRight / rightSamples.length) : 0;
			const rmsTime = Date.now() - rmsStartTime;

			console.log(`RMS left channel (last ~${leftSamples.length} samples):`, rmsLeft.toFixed(6));
			console.log(`RMS right channel (last ~${rightSamples.length} samples):`, rmsRight.toFixed(6));
			console.log('RMS calculation time (ms):', rmsTime.toFixed(2));
		} else {
			console.log('Audio Wizard => RawData is empty');
		}
	}
	catch (e) {
		console.log('Audio Wizard => Error accessing RawAudioData:', e.message);
	}

	// Call AudioWizard.StopRawAudioMonitoring() when done
}
```

**Notes**:
- Use `try-catch` for error handling.
- Compares calculated RMS with `PeakmeterAdjustedLeftRMS` (adjusted, not raw).
- Call `StopRawAudioMonitoring()` to free resources.

<br>
<br>

### Full-Track Analysis

Asynchronously analyze selected tracks:

```javascript
/**
 * Starts analyzing asynchronously selected tracks and logs detailed audio metrics.
 */
async function startFullTrackAnalysis() {
	if (!AudioWizard) return;

	console.log("Audio Wizard => Starting full-track metrics analysis...");

	try {
		await new Promise((resolve, reject) => {
			const onComplete = () => {
				console.log("Audio Wizard => Analysis complete!");

				const metricsPerTrack = 12; // M LUFS, S LUFS, I LUFS, RMS, SP, TP, PSR, PLR, CF, LRA, DR, PD
				const metrics = AudioWizard.GetFullTrackMetrics();

				const selectedTracks = plman.GetPlaylistSelectedItems(plman.ActivePlaylist);
				const tfArtist = fb.TitleFormat("%artist%");
				const tfAlbum = fb.TitleFormat("%album%");
				const tfTitle = fb.TitleFormat("%title%");

				console.log(`Audio Wizard => Analyzed ${selectedTracks.Count} track(s):`);

				for (let i = 0; i < selectedTracks.Count; i++) {
					const track = selectedTracks[i];
					const artist = tfArtist.EvalWithMetadb(track);
					const album = tfAlbum.EvalWithMetadb(track);
					const title = tfTitle.EvalWithMetadb(track);
					const offset = i * metricsPerTrack;

					console.log(`Audio Wizard => GetFullTrackMetrics => Track ${i + 1}: ${artist} - ${album} - ${title}`);
					console.log(`  M LUFS: ${metrics[offset + 0].toFixed(2)}`);
					console.log(`  S LUFS: ${metrics[offset + 1].toFixed(2)}`);
					console.log(`  I LUFS: ${metrics[offset + 2].toFixed(2)}`);
					console.log(`  RMS: ${metrics[offset + 3].toFixed(2)}`);
					console.log(`  Sample Peak: ${metrics[offset + 4].toFixed(2)}`);
					console.log(`  True Peak: ${metrics[offset + 5].toFixed(2)}`);
					console.log(`  PSR: ${metrics[offset + 6].toFixed(2)}`);
					console.log(`  PLR: ${metrics[offset + 7].toFixed(2)}`);
					console.log(`  CF: ${metrics[offset + 8].toFixed(2)}`);
					console.log(`  LRA: ${metrics[offset + 9].toFixed(2)}`);
					console.log(`  DR: ${metrics[offset + 10].toFixed(2)}`);
					console.log(`  PD: ${metrics[offset + 11].toFixed(2)}`);

					console.log(`Audio Wizard => Individual Metrics => Track ${i + 1}: ${artist} - ${album} - ${title}`);
					console.log(`  M LUFS: ${AudioWizard.GetMomentaryLUFSFull(i).toFixed(2)}`);
					console.log(`  S LUFS: ${AudioWizard.GetShortTermLUFSFull(i).toFixed(2)}`);
					console.log(`  I LUFS: ${AudioWizard.GetIntegratedLUFSFull(i).toFixed(2)}`);
					console.log(`  RMS: ${AudioWizard.GetRMSFull(i).toFixed(2)}`);
					console.log(`  Sample Peak: ${AudioWizard.GetSamplePeakFull(i).toFixed(2)}`);
					console.log(`  True Peak: ${AudioWizard.GetTruePeakFull(i).toFixed(2)}`);
					console.log(`  PSR: ${AudioWizard.GetPSRFull(i).toFixed(2)}`);
					console.log(`  PLR: ${AudioWizard.GetPLRFull(i).toFixed(2)}`);
					console.log(`  CF: ${AudioWizard.GetCrestFactorFull(i).toFixed(2)}`);
					console.log(`  LRA: ${AudioWizard.GetLoudnessRangeFull(i).toFixed(2)}`);
					console.log(`  DR: ${AudioWizard.GetDynamicRangeFull(i).toFixed(2)}`);
					console.log(`  PD: ${AudioWizard.GetPureDynamicsFull(i).toFixed(2)}`);

					console.log("\n");
				}

				// Compute and log DR and PD album metrics
				const albums = new Map();
				for (let i = 0; i < selectedTracks.Count; i++) {
					const track = selectedTracks[i];
					const album = tfAlbum.EvalWithMetadb(track);
					const artist = tfArtist.EvalWithMetadb(track);
					if (album && !albums.has(album)) {
						albums.set(album, artist || "Unknown Artist");
					}
				}

				console.log(`Audio Wizard => Analyzed ${albums.size} album(s):`);

				for (const [album, artist] of albums.entries()) {
					const dr = AudioWizard.GetDynamicRangeAlbumFull(album);
					const pd = AudioWizard.GetPureDynamicsAlbumFull(album);
					console.log(`Audio Wizard => Album metrics: ${artist} - ${album}`);
					console.log(`  DR-A: ${dr === -Infinity ? '-inf' : dr.toFixed(2)}`);
					console.log(`  PD-A: ${pd === -Infinity ? '-inf' : pd.toFixed(2)}`);
				}

				resolve();
			};

			try {
				AudioWizard.SetFullTrackAnalysisCallback(onComplete);
				AudioWizard.StartFullTrackAnalysis(100);
			} catch (e) {
				reject(new Error(`Audio Wizard => Failed to start full-track metrics analysis: ${e.description}`));
			}
		});
	}
	catch (e) {
		console.log(`Audio Wizard => Error in full-track metrics analysis: ${e.message}`);
	}
}
```

**Notes**:
- Requires `plman` and `fb` objects.
- Uses `Promise` for asynchronous analysis.
- Specify track index in individual getters.
- Call `StopFullTrackAnalysis()` to abort early.

<br>
<br>

### Waveform Analysis

Generate waveform data:

```javascript
/**
 * Starts waveform analysis and logs the resulting waveform data.
 */
function startWaveformAnalysis() {
	if (!AudioWizard) return;

	console.log("Audio Wizard => Starting waveform analysis...");

	AudioWizard.WaveformMetric = 0; // 0 = RMS, 2 = RMS_Peak, 3 = Peak, 4 = Waveform_Peak
	AudioWizard.StartWaveformAnalysis(100); // 100 ms resolution

	AudioWizard.SetFullTrackWaveformCallback(() => {
		console.log('Audio Wizard => Waveform analysis complete!');
		console.log('Waveform Data:', AudioWizard.WaveformData);
		AudioWizard.StopWaveformAnalysis();
	});
}
```

**Notes**:
- Set `WaveformMetric` before analysis.
- Use `SetFullTrackWaveformCallback` for completion.
- Call `StopWaveformAnalysis()` when done.

<br>
<br>

### Callbacks

Demonstrates how to manage Audio Wizard monitoring with foobar2000 playback callbacks in
[Spider Monkey Panel](https://github.com/TheQwertiest/foo_spider_monkey_panel) or [JSplitter](https://foobar2000.ru/forum/viewtopic.php?t=6378).

```javascript
function on_playback_new_track(metadb) {
	if (!AudioWizard) return;

	AudioWizard.StartRealTimeMonitoring(17, 50); // 17ms refresh rate, 50ms chunk duration
	AudioWizard.StartPeakmeterMonitoring(17, 50);
	AudioWizard.StartRawAudioMonitoring(17, 50);
}

function on_playback_stop(reason) {
	if (!AudioWizard) return;

	if (reason !== 2) {
		AudioWizard.StopRealTimeMonitoring();
		AudioWizard.StopPeakmeterMonitoring();
		AudioWizard.StopRawAudioMonitoring();
	}
}
```

**Notes**:
- Ensure `AudioWizard` is instantiated globally before use, as shown in Usage Examples -> `const AudioWizard = new ActiveXObject('AudioWizard');`
- The `reason` parameter in `on_playback_stop` indicates why playback stopped: 0 (user stop), 1 (track end), 2 (end of playlist),
  or 3 (starting another track). Monitoring is typically stopped for all reasons except 2 to avoid interrupting analysis across playlist transitions.
- This example enables all monitoring types (real-time, peakmeter, raw audio) for demonstration. Adjust by including only the necessary
  Start* and Stop* methods (e.g., StartRealTimeMonitoring and StopRealTimeMonitoring) based on your use case to optimize performance.
  See Real-Time Monitoring, Peakmeter Monitoring, or Raw Audio Data for details.

<br>
<br>

## API Reference

The **API Reference** provides tables listing all properties and methods of the `AudioWizard` ActiveX object, with notes for additional context where needed.
Refer to [Usage Examples](#usage-examples) for practical applications.

### Properties

| Name                              | Type                 | Access     | Description                                                                 |
|-----------------------------------|----------------------|------------|-----------------------------------------------------------------------------|
| MomentaryLUFS                     | number               | Read-only  | Momentary loudness in LUFS.                                                 |
| ShortTermLUFS                     | number               | Read-only  | Short-Term loudness in LUFS.                                                |
| RMS                               | number               | Read-only  | Overall RMS level in dBFS.                                                  |
| LeftRMS                           | number               | Read-only  | RMS level for the left channel in dBFS.                                     |
| RightRMS                          | number               | Read-only  | RMS level for the right channel in dBFS.                                    |
| LeftSamplePeak                    | number               | Read-only  | Sample peak for the left channel in dBFS.                                   |
| RightSamplePeak                   | number               | Read-only  | Sample peak for the right channel in dBFS.                                  |
| TruePeak                          | number               | Read-only  | True peak level in dBTP.                                                    |
| PSR                               | number               | Read-only  | Peak to Short-Term Loudness Ratio in dB.                                    |
| PLR                               | number               | Read-only  | Peak to Long-Term Loudness Ratio in dB.                                     |
| CrestFactor                       | number               | Read-only  | Crest factor (ratio of peak to RMS).                                        |
| DynamicRange                      | number               | Read-only  | Dynamic Range in dB.                                                        |
| PureDynamics                      | number               | Read-only  | Pure Dynamics in dB.                                                        |
| PhaseCorrelation                  | number               | Read-only  | Phase correlation between channels (-1 to 1).                               |
| StereoWidth                       | number               | Read-only  | Stereo width metric (0 to 1).                                               |
| PeakmeterOffset                   | number               | Read/Write | Gain offset in dB applied to peakmeter measurements (-20 to +20 dB).        |
| PeakmeterAdjustedLeftRMS          | number               | Read-only  | Adjusted RMS level for the left channel in dBFS, optimized for display.     |
| PeakmeterAdjustedRightRMS         | number               | Read-only  | Adjusted RMS level for the right channel in dBFS, optimized for display.    |
| PeakmeterAdjustedLeftSamplePeak   | number               | Read-only  | Adjusted sample peak for the left channel in dBFS, optimized for display.   |
| PeakmeterAdjustedRightSamplePeak  | number               | Read-only  | Adjusted sample peak for the right channel in dBFS, optimized for display.  |
| RawAudioData                      | Array                | Read-only  | PCM audio samples for the current chunk.                                    |
| WaveformMetric                    | number               | Read/Write | Metric for analysis (0 = RMS, 1 = RMS_Peak, 2 = Peak, 3 = Waveform_Peak).   |
| WaveformData                      | Array                | Read-only  | Waveform data points.                                                       |

- **Peakmeter Monitoring**:
  - `PeakmeterOffset`: Adjusts gain for peakmeter measurements. Set as an integer (e.g., `AudioWizard.PeakmeterOffset = 5` for 5 dB); returns a float when read.
  - `PeakmeterAdjustedLeftRMS`, `PeakmeterAdjustedRightRMS`: Adjusted RMS levels (~-5 to +5 dBFS), processed with dynamic gain offset and fade-in effects, not raw RMS.
  - `PeakmeterAdjustedLeftSamplePeak`, `PeakmeterAdjustedRightSamplePeak`: Adjusted sample peaks (~-5 to +5 dBFS), processed with dynamic gain offset and smoothing.

- **Raw Audio Data**:
  - `RawAudioData`: Raw PCM samples; see [Raw Audio Data](#raw-audio-data) example.

- **Waveform Analysis**:
  - `WaveformMetric`: Set to 0 (RMS), 1 (RMS_Peak), 2 (Peak), or 3 (Waveform_Peak) before analysis.
  - `WaveformData`: Waveform data points; see [Waveform Analysis](#waveform-analysis) example.

<br>
<br>

### Methods

| Name                            | Signature                                               | Description                                                      |
|---------------------------------|---------------------------------------------------------|------------------------------------------------------------------|
| StartRealTimeMonitoring         | (refreshRate: number, chunkDuration: number) -> void    | Starts real-time monitoring.                                     |
| StopRealTimeMonitoring          | () -> void                                              | Stops real-time monitoring.                                      |
| StartPeakmeterMonitoring        | (refreshRate: number, chunkDuration: number) -> void    | Starts peakmeter monitoring.                                     |
| StopPeakmeterMonitoring         | () -> void                                              | Stops peakmeter monitoring.                                      |
| StartRawAudioMonitoring         | (refreshRate: number, chunkDuration: number) -> void    | Starts raw audio data capture.                                   |
| StopRawAudioMonitoring          | () -> void                                              | Stops raw audio data capture.                                    |
| StartWaveformAnalysis           | (resolution: number) -> void                            | Starts asynchronous waveform analysis.                           |
| StopWaveformAnalysis            | () -> void                                              | Stops waveform analysis.                                         |
| SetFullTrackWaveformCallback    | (callback: Function) -> void                            | Sets the callback for waveform analysis completion.              |
| StartFullTrackAnalysis          | (chunkDuration: number) -> void                         | Starts asynchronous analysis of selected tracks.                 |
| StopFullTrackAnalysis           | () -> void                                              | Stops full-track analysis.                                       |
| SetFullTrackAnalysisCallback    | (callback: Function) -> void                            | Sets the callback for analysis completion.                       |
| GetFullTrackMetrics             | () -> Array                                             | Returns all metrics for all analyzed tracks.                     |
| GetMomentaryLUFSFull            | ([index: number]) -> number                             | Returns Momentary LUFS for the specified track (default: 0).     |
| GetShortTermLUFSFull            | ([index: number]) -> number                             | Returns Short Term LUFS for the specified track (default: 0).    |
| GetIntegratedLUFSFull           | ([index: number]) -> number                             | Returns Integrated LUFS for the specified track (default: 0).    |
| GetRMSFull                      | ([index: number]) -> number                             | Returns RMS for the specified track (default: 0).                |
| GetSamplePeakFull               | ([index: number]) -> number                             | Returns Sample Peak for the specified track (default: 0).        |
| GetTruePeakFull                 | ([index: number]) -> number                             | Returns True Peak for the specified track (default: 0).          |
| GetPSRFull                      | ([index: number]) -> number                             | Returns PSR for the specified track (default: 0).                |
| GetPLRFull                      | ([index: number]) -> number                             | Returns PLR for the specified track (default: 0).                |
| GetCrestFactorFull              | ([index: number]) -> number                             | Returns Crest Factor for the specified track (default: 0).       |
| GetLoudnessRangeFull            | ([index: number]) -> number                             | Returns Loudness Range for the specified track (default: 0).     |
| GetDynamicRangeFull             | ([index: number]) -> number                             | Returns Dynamic Range for the specified track (default: 0).      |
| GetPureDynamicsFull             | ([index: number]) -> number                             | Returns Pure Dynamics for the specified track (default: 0).      |
| GetDynamicRangeAlbumFull        | (albumName: string) -> number                           | Returns Dynamic Range album metric for the specified album.      |
| GetPureDynamicsAlbumFull        | (albumName: string) -> number                           | Returns Pure Dynamics album metric for the specified album.      |

- **Real-Time Monitoring**:
  - `StartRealTimeMonitoring`: Set `refreshRate` (ms) and `chunkDuration` (ms) for update frequency and data granularity.

- **Peakmeter Monitoring**:
  - `StartPeakmeterMonitoring`: Adjust parameters for visualization responsiveness.

- **Raw Audio Monitoring**:
  - `StartRawAudioMonitoring`: Use cautiously due to high data volume.

- **Full-Track Analysis**:
  - `StartFullTrackAnalysis`: Set `chunkDuration` (ms) for analysis granularity.
  - `StopFullTrackAnalysis`: Use to abort early.
  - `SetFullTrackAnalysisCallback`: Provide a JavaScript function for async completion.
  - `GetFullTrackMetrics`: Returns array of metrics (LUFS, LRA, True Peak, PSR, PLR, DR) per track.
  - `GetIntegratedLUFSFull`, `GetLoudnessRangeFull`, `GetTruePeakFull`, `GetPSRFull`, `GetPLRFull`, `GetDynamicRangeFull`: Use track index (default: 0).

- **Full-Album Analysis**:
  - `GetDynamicRangeAlbumFull`: Use album name (string) to retrieve Dynamic Range album metric.
  - `GetPureDynamicsAlbumFull`: Use album name (string) to retrieve Pure Dynamics album metric.

- **Waveform Analysis**:
  - `StartWaveformAnalysis`: Set `resolution` (ms) for data granularity.
  - `SetFullTrackWaveformCallback`: Provide a JavaScript function for async completion.

<br>
<br>

## Performance Considerations

- **Refresh Rate and Chunk Duration**:
  - Lower `refreshRate` (e.g., 10 ms) increases CPU usage but improves responsiveness. Use 17-50 ms for a balance.
- **Raw Audio Data**:
  - At 44.1 kHz, a 50 ms `chunkDuration` generates ~2205 samples per channel. Use smaller chunks (e.g., 10 ms) for lower memory usage.
- **Waveform Analysis**:
  - Higher `resolution` (e.g., 10 ms) increases data points but may slow processing. Use 50-100 ms for most applications.
- **Full-Track Analysis**:
  - Larger `chunkDuration` (e.g., 500 ms) reduces processing time but lowers granularity.

<br>
<br>

## Error Handling

Common errors include:
- `E_POINTER`: Invalid pointer (e.g., null `value` in getters). Ensure valid arguments.
- `E_UNEXPECTED`: Peakmeter unavailable (e.g., not initialized). Check `AudioWizard` state.
- `E_INVALIDARG`: Invalid argument (e.g., out-of-range values for size or position, or invalid album name). Check parameter type and ranges.
- For async full-track analysis operations and 'maybe' retrieving the raw audio samples, use `try-catch` blocks in JavaScript to be on the safe side.

<br>
<br>

## Terminology

| Term                                    | Definition                                                                                                 |
|-----------------------------------------|------------------------------------------------------------------------------------------------------------|
| Momentary LUFS                          | Momentary loudness in LUFS, typically -70 to 0, measured over ~400 ms.                                     |
| Short-Term LUFS                         | Short-Term loudness in LUFS, typically -70 to 0, averaged over ~3 seconds.                                 |
| Integrated LUFS                         | Integrated loudness in LUFS, typically -70 to 0, averaged over ~3 seconds.                                 |
| RMS                                     | Root Mean Square, the overall RMS level in dBFS.                                                           |
| Left RMS                                | RMS level for the left channel in dBFS.                                                                    |
| Right RMS                               | RMS level for the right channel in dBFS.                                                                   |
| Left Sample Peak                        | Sample peak for the left channel in dBFS.                                                                  |
| Right Sample Peak                       | Sample peak for the right channel in dBFS.                                                                 |
| Sample Peak                             | The maximum absolute sample value in dBFS.                                                                 |
| True Peak                               | True peak level in dBTP, accounting for inter-sample peaks.                                                |
| PSR                                     | Peak to Short-Term Loudness Ratio in dB.                                                                   |
| PLR                                     | Peak to Long-Term Loudness Ratio in dB.                                                                    |
| Crest Factor                            | Ratio of peak to RMS level, unitless.                                                                      |
| LRA                                     | Loudness Range, the dynamic range of loudness in LU.                                                       |
| DR                                      | Dynamic Range, in dB, measuring the difference between loudest and quietest parts.                         |
| PD                                      | Pure Dynamics, in dB, measured by psychoaccoustic principles.                                              |
| DR-A                                    | Average Dynamic Range across all tracks in an album, in dB.                                                |
| PD-A                                    | Average Pure Dynamics across all tracks in an album, in dB, adjusted for perceptual dynamics.              |
| Phase Correlation                       | Correlation between left and right channels, ranging from -1 to 1.                                         |
| Stereo Width                            | Metric of stereo image width, ranging from 0 to 1.                                                         |
| Peakmeter Adjusted Left RMS             | Adjusted RMS level for the left channel in dBFS, typically -5 to +5, optimized for display.                |
| Peakmeter Adjusted Right RMS            | Adjusted RMS level for the right channel in dBFS, typically -5 to +5, optimized for display.               |
| Peakmeter Adjusted Left Sample Peak     | Adjusted sample peak for the left channel in dBFS, typically -5 to +5, optimized for display.              |
| Peakmeter Adjusted Right Sample Peak    | Adjusted sample peak for the right channel in dBFS, typically -5 to +5, optimized for display.             |
| Peakmeter Offset                        | Gain offset in dB applied to peakmeter measurements, adjustable from -20 to +20.                           |
| Raw Audio Data                          | Raw PCM audio samples for the current chunk, unitless.                                                     |
| Waveform Data                           | Data points for waveform analysis, depending on the selected metric (e.g., RMS, Peak).                     |
| Waveform Metric                         | The metric used for waveform analysis (0 = RMS, 1 = RMS_Peak, 2 = Peak, 3 = Waveform_Peak).                |
| dBFS                                    | Decibels relative to Full Scale, a measure of amplitude relative to the maximum possible digital level.    |
| dBTP                                    | Decibels True Peak, accounting for inter-sample peaks in digital audio.                                    |

<br>
<br>

## Support

- [Audio Wizard on Github](https://github.com/TT-ReBORN/foo_audio_wizard)
