<div align="center">
  <br>
  <picture>
	<source media="(prefers-color-scheme: dark)" srcset="../images/Audio-Wizard-Logo.svg">
	<img src="../images/Audio-Wizard-Logo.svg" width="400" alt="Audio Wizard Logo">
  </picture>
  <br>
  <br>
</div>

<div align="center">
  <h1>
	<picture>
	  <source media="(prefers-color-scheme: dark)" srcset="../images/Audio-Wizard-Title-Dark.svg">
	  <img src="../images/Audio-Wizard-Title-Light.svg" alt="Audio Wizard Title">
	</picture>
  </h1>
</div>

<div align="center">
  <picture>
	<source media="(prefers-color-scheme: dark)" srcset="../images/Audio-Wizard-Subtitle-Dark.svg">
	<img src="../images/Audio-Wizard-Subtitle-Light.svg" alt="Audio Wizard Subtitle">
  </picture>
</div>

<br>
<br>

Within the blazing heart of the **Rubynar Sanctum**, where the **Ruby Spell** pulses with sonic fire,
the scribes of **The Wizardium** inscribe the chronicles of the **Audio Wizard's** ascent.
Each sigil in this sacred codex unveils incantations — properties to bind, methods to command, and exemplars
to guide — granting worthy scholars dominion over foobar2000's auditory essence,
from its luminous loudness to the primal pulse of its dynamic soul.

<br>

<h3 align="center"><em><span title="The Wisdom Of The Divine Flame">⸺ Sapientia Flamma Divina ⸺</span></em></h3>
<div align="center"><a href="https://github.com/The-Wizardium">A Sacred Chapter Of The Wizardium</a></div>

<br>
<h2></h2>
<br>
<br>

# Audio Wizard - API Reference

*Version 0.6 - Last Updated: 03.07.2026*

Audio Wizard provides a JavaScript API for real-time audio analysis and visualization in foobar2000,
accessible via a COM/ActiveX interface in scripting environments like
[Spider Monkey Panel](https://github.com/TheQwertiest/foo_spider_monkey_panel) or
[JSplitter](https://foobar2000.ru/forum/viewtopic.php?t=6378).
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

### Physical File Path Resolution

Resolves the actual on-disk path for a track in a single call - plain files, tracks embedded in
archives (`.zip`, `.7z`) or CUE sheets via foobar2000's native `<container>|<member>` notation, and
tracks streamed through the Unpack component's `unpack://` protocol. Use this instead of writing your
own path-parsing logic: `GetPhysicalFilePath` already knows how to unwrap every path scheme foobar2000
and its common archive-support components use, so your script never needs to inspect `RawPath` manually
or care whether a track lives in an archive at all.

```javascript
/**
 * Gets the on-disk path for a track - plain file, archive/CUE member, or Unpack-streamed.
 * @param {object} handle - The handle of the track.
 * @returns {string} The physical file path.
 */
function getPhysicalFilePath(handle) {
	if (!AudioWizard) return handle.Path;
	return AudioWizard.GetPhysicalFilePath(handle.RawPath);
}

// No longer needed - superseded by AudioWizard.GetPhysicalFilePath(handle.RawPath)

// /**
//  * Gets the physical file path by stripping 'unpack://' protocols.
//  * @param {object} handle - The handle of the track.
//  * @returns {object} The stripped physical file path.
//  */
// function getPhysicalFilePath(handle) {
// 	// If it's not an archive, return the path as is
// 	if (!handle.RawPath.includes('unpack://')) {
// 		return handle.Path;
// 	}

// 	// Extract path after 'file://'
// 	const filePrefix = 'file://';
// 	const parts = handle.RawPath.split(filePrefix);

// 	// Return the physical path (the first part before the internal pipe)
// 	return (parts.length > 1) ? parts[1].split('|')[0] : handle.Path;
// }
```

**Notes**:
- Prefer `handle.RawPath` over `handle.Path` as input - `RawPath` always carries an explicit scheme
  prefix so plain files and archive members aren't ambiguous.
- Handles every path shape automatically - **no extra code needed on your side**:
  - Plain files - a `file://` prefix (if present) is stripped down to the bare OS path; a path with
    no recognized scheme is returned unchanged.
  - foobar2000's native archive/CUE notation (`<container>|<member>`, e.g. tracks inside a
    natively-supported `.zip`/`.7z`, or CUE sheet entries) - returns the container file's path.
  - The Unpack component's streaming protocol (`unpack://<type>|<n>|file://<container>|<member>`) -
    returns the container archive's path.
- For any track inside an archive or CUE sheet, this returns the path to the container file itself,
  not the virtual member path - what you want for hashing, file-system checks (`FileExists`,
  `GetFileSize`, etc.), or handing a path to external tools that can only work with real files on disk.
- If you've been resolving archive paths yourself with something like this, it can be retired -
  `GetPhysicalFilePath` covers the same `unpack://` case and goes further, since it also resolves
  native archive/CUE paths that never go through `unpack://` at all.
- Extremely rare or nested/unrecognized schemes are returned unchanged rather than resolved, as a last resort.

<br>
<br>

### Real-Time Monitoring

Retrieve and log real-time audio metrics:

```javascript
/**
 * Starts real-time audio monitoring and logs various audio metrics.
 * @param {number} [refreshRate] - The optional refresh rate from 10-1000ms.
 * @param {number} [chunkDuration] - The optional chunk duration from 10-1000ms.
 */
function startRealTimeMonitoring(refreshRate = 33, chunkDuration = 50) {
	if (!AudioWizard) return;

	AudioWizard.StartRealTimeMonitoring(refreshRate, chunkDuration);

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
 * @param {number} [refreshRate] - The optional refresh rate from 10-1000ms.
 * @param {number} [chunkDuration] - The optional chunk duration from 10-1000ms.
 */
function startPeakmeterMonitoring(refreshRate = 33, chunkDuration = 50) {
	if (!AudioWizard) return;

	AudioWizard.StartPeakmeterMonitoring(refreshRate, chunkDuration);

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
 * @param {number} [refreshRate] - The optional refresh rate from 10-1000ms.
 * @param {number} [chunkDuration] - The optional chunk duration from 10-1000ms.
 */
function startRawAudioMonitoring(refreshRate = 33, chunkDuration = 50) {
	if (!AudioWizard) return;

	AudioWizard.StartRawAudioMonitoring(refreshRate, chunkDuration);

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

### Full-Track Analysis (Track Metadata Helper)

Prepares COM-ready track metadata from metadb handle(s):

```javascript
/**
 * Prepares COM-ready track metadata from metadb handle(s), with fallback to selected items if null.
 * @global
 * @param {FbMetadbHandle|FbMetadbHandleList|null} metadb -
 *  - FbMetadbHandle: fb.GetNowPlaying() or fb.GetSelected();
 *  - FbMetadbHandleList: plman.GetPlaylistSelectedItems(plman.ActivePlaylist);
 *  - null: defaults to plman.GetPlaylistSelectedItems(plman.ActivePlaylist).
 * @returns {Object} { handleList: FbMetadbHandleList, metadata: string[], artists: string[], albums: string[], titles: string[] }
 *  - metadata: Array of "path\u001Fsubsong" strings (e.g., "C:\\song.mp3\u001F0"), auto-marshaled to VT_ARRAY | VT_BSTR for COM.
 */
function getMetadata(metadb) {
	const handleData = metadb || plman.GetPlaylistSelectedItems(plman.ActivePlaylist);
	const handleList = new FbMetadbHandleList(handleData);
	const handleArray = handleList.Convert();
	const handleCount = handleArray.length;

	const metadata = new Array(handleCount);
	const artists = new Array(handleCount);
	const albums = new Array(handleCount);
	const titles = new Array(handleCount);

	const sep = '\u001F';
	const combinedTf = fb.TitleFormat(`%artist%${sep}%album%${sep}%title%`);

	for (let i = 0; i < handleCount; i++) {
		const handle = handleArray[i];
		const parts = combinedTf.EvalWithMetadb(handle).split(sep);
		metadata[i] = `${handle.Path}${sep}${handle.SubSong}`;
		artists[i] = parts[0] || "Unknown Artist";
		albums[i]  = parts[1] || "Unknown Album";
		titles[i]  = parts[2] || "Unknown Title";
	}

	return { handleList, metadata, artists, albums, titles };
}
```

**Notes**:
- Metadata format uses Unicode Information Separator One (U+001F) to separate path and subsong.
- Returns all necessary information for track identification and display.
- Automatically marshaled to COM-compatible format (VT_ARRAY | VT_BSTR).

<br>
<br>

### Full-Track Metrics Data Info (Track Metadata Helper)

Lets you discover the actual `GetFullTrackMetrics()` layout instead of hardcoding it.

```javascript
/**
 * Retrieves and parses the full-track metrics schema exposed by Audio Wizard.
 * @global
 * @returns {Object|null} The parsed schema object, or null if AudioWizard is unavailable or the JSON failed to parse.
 */
function getFullTrackMetricsDataInfo() {
	if (!AudioWizard) return null;

	try {
		const infoJson = AudioWizard.GetFullTrackMetricsDataInfo();
		return JSON.parse(infoJson);
	}
	catch (e) {
		console.log(`Audio Wizard => Error parsing GetFullTrackMetricsDataInfo: ${e.message}`);
		return null;
	}
}
```

**Notes**:
- Returns component-level schema info: `componentVersion`, `fullTrackMetricsDataVersion`, `metricsPerTrack`, `metrics`.
- `fullTrackMetricsDataVersion` increments whenever `metricsPerTrack`/`metrics` change - check it (or just re-read `metrics`/`metricsPerTrack` directly) instead of assuming a fixed layout across component versions.
- `metrics[]` gives the ordered metric identifiers backing the flat array from `GetFullTrackMetrics()` - use `metrics.indexOf('PD')` etc. instead of hardcoding an index.

<br>
<br>

### Full-Track Waveform Data Info (Track Metadata Helper)

Lets you discover the actual data layout instead of hardcoding it.

```javascript
/**
 * Retrieves and parses the waveform data schema exposed by Audio Wizard.
 * @global
 * @param {number} [trackIndex] - The optional 0-based track index for per-track info (channels, path, duration).
 * When omitted, only component-level schema info is returned:
 * - componentVersion
 * - waveformDataVersion
 * - metricsPerChannel
 * - metrics
 * - pointsPerSecond
 *
 * NOTE: pointsPerSecond is included at the component level but is NOT fixed schema -
 * it reflects whatever resolution the most recently *started* analysis used (default until one has run).
 * Read it right after the analysis you care about, as startWaveformAnalysis() does, not as a stand-alone
 * "check the schema before doing anything" call.
 * @returns {Object|null} The parsed schema object, or null if AudioWizard is unavailable or the JSON failed to parse.
 */
function getWaveformDataInfo(trackIndex) {
	if (!AudioWizard) return null;

	try {
		const infoJson = trackIndex == null
			? AudioWizard.GetWaveformDataInfo()
			: AudioWizard.GetWaveformDataInfo(trackIndex);

		return JSON.parse(infoJson);
	}
	catch (e) {
		console.log(`Audio Wizard => Error parsing GetWaveformDataInfo: ${e.message}`);
		return null;
	}
}
```

**Notes**:
- Call with no argument for component-level info only (`componentVersion`, `waveformDataVersion`, `metricsPerChannel`, `metrics`, `pointsPerSecond`).
- Call with a track index after that track's waveform analysis has actually completed (i.e. inside the
  `SetFullTrackWaveformCallback` success callback) to also get that track's `channels`, `path`, and `duration`.
  `GetWaveformTrackCount()` becomes non-zero as soon as `StartWaveformAnalysis` is called - that doesn't mean
  per-track data is ready. `duration`/`path` are populated synchronously at start, but `channels` stays `0`
  until the first chunk for that track has been decoded.
- `waveformDataVersion` increments whenever `metricsPerChannel`/`metrics` change - check it (or just re-read `metrics`/`metricsPerChannel` directly) instead of assuming a fixed layout across component versions.

<br>
<br>

### Full-Track Analysis (Batch Retrieval)

Asynchronously analyze tracks using batch metrics retrieval:

```javascript
/**
 * Starts full-track metrics analysis using GetFullTrackMetrics (batch retrieval).
 * @global
 * @param {FbMetadbHandle|FbMetadbHandleList|null} metadb - The metadb handle(s).
 * @param {number} [chunkDuration] - The optional chunk duration from 10-1000ms.
 * @returns {Promise<{success: boolean, metrics?: any}>}
 */
async function startFullTrackMetricsBatch(metadb, chunkDuration = 200) {
	if (!AudioWizard || AudioWizard.FullTrackProcessing) {
		return { success: false };
	}

	console.log("Audio Wizard => Starting full-track metrics batch analysis...");

	try {
		const { handleList, metadata, artists, albums, titles } = getMetadata(metadb);
		console.log(`Audio Wizard => Processing ${artists.length} track(s) via unified format`);

		return await new Promise((resolve) => {
			const onComplete = (success) => {
				try {
					console.log(`Audio Wizard => Batch metrics callback fired, success: ${success}`);

					if (!success) {
						console.log('Audio Wizard => No tracks selected, returning empty batch result');
						resolve({ success: false });
						return;
					}

					console.log("Audio Wizard => Batch metrics analysis complete!");

					const fullMetricsDataInfo = getFullTrackMetricsDataInfo();

					if (!fullMetricsDataInfo) {
						console.log('Audio Wizard => Batch metrics analysis failed: could not retrieve data schema (GetFullTrackMetricsDataInfo)');
						resolve({ success: false });
						return;
					}

					const { metricsPerTrack, metrics: metricNames } = fullMetricsDataInfo;
					const metrics = AudioWizard.GetFullTrackMetrics();

					console.log(`Audio Wizard => Analyzed ${handleList.Count} track(s) with GetFullTrackMetrics:`);

					for (let i = 0; i < handleList.Count; i++) {
						const artist = artists[i];
						const album = albums[i];
						const title = titles[i];
						const offset = i * metricsPerTrack;

						console.log(`Audio Wizard => GetFullTrackMetrics => Track ${i + 1}: ${artist} - ${album} - ${title}`);

						for (let m = 0; m < metricsPerTrack; m++) {
							console.log(`  ${metricNames[m]}: ${metrics[offset + m].toFixed(2)}`);
						}
						console.log("\n");
					}

					resolve({ success: true, metrics, fullMetricsDataInfo });
				}
				catch (e) {
					console.log(`Audio Wizard => Error in batch metrics callback: ${e.message}`);
					resolve({ success: false });
				}
			};

			AudioWizard.SetFullTrackAnalysisCallback(onComplete);
			AudioWizard.StartFullTrackAnalysis(metadata, chunkDuration);
		});
	}
	catch (e) {
		console.log(`Audio Wizard => Unexpected error in full-track metrics batch analysis: ${e.message}`);
		return { success: false };
	}
}
```

**Notes**:
- Requires `plman` and `fb` objects.
- Uses `Promise` for asynchronous analysis with timeout.
- Check `FullTrackProcessing` property before starting to avoid overlapping operations.
- This method allows analyzing ANY tracks using the metadata array overload, not just selected ones.
- Callback receives a `success` boolean parameter indicating completion status.
- Uses `GetFullTrackMetrics()` for efficient batch retrieval of all metrics.
- Uses `GetFullTrackMetricsDataInfo()` to discover `metricsPerTrack` and `metrics[]` instead of hardcoding the stride.

<br>
<br>

### Full-Track Analysis (Individual Getters)

Asynchronously analyze tracks using individual metric getters:

```javascript
/**
 * Starts full-track metrics analysis using single getters (e.g., GetMomentaryLUFSFull).
 * @global
 * @param {FbMetadbHandle|FbMetadbHandleList|null} metadb - The metadb handle(s).
 * @param {number} [chunkDuration] - The optional chunk duration from 10-1000ms.
 * @returns {Promise<{success: boolean, singleMetrics?: any}>}
 */
async function startFullTrackMetricsSingle(metadb, chunkDuration = 200) {
	if (!AudioWizard || AudioWizard.FullTrackProcessing) {
		return { success: false };
	}

	console.log("Audio Wizard => Starting full-track single metrics analysis...");

	try {
		const { handleList, metadata, artists, albums, titles } = getMetadata(metadb);

		if (!handleList || handleList.Count === 0) {
			console.log("Audio Wizard => No tracks to analyze.");
			return { success: false };
		}

		const trackCount = handleList.Count;

		return await new Promise((resolve) => {
			const onComplete = (success) => {
				if (!success) {
					console.log('Audio Wizard => Analysis failed or was cancelled');
					resolve({ success: false, singleMetrics: new Map() });
					return;
				}

				console.log("Audio Wizard => Single metrics analysis complete!");
				console.log(`Audio Wizard => Analyzed ${trackCount} track(s):`);

				const singleMetrics = new Map();

				for (let i = 0; i < trackCount; i++) {
					const key = `${artists[i]} - ${albums[i]} - ${titles[i]}`;

					const metrics = {
						mLufs: AudioWizard.GetMomentaryLUFSFull(i),
						sLufs: AudioWizard.GetShortTermLUFSFull(i),
						iLufs: AudioWizard.GetIntegratedLUFSFull(i),
						rms: AudioWizard.GetRMSFull(i),
						samplePeak: AudioWizard.GetSamplePeakFull(i),
						truePeak: AudioWizard.GetTruePeakFull(i),
						psr: AudioWizard.GetPSRFull(i),
						plr: AudioWizard.GetPLRFull(i),
						cf: AudioWizard.GetCrestFactorFull(i),
						lra: AudioWizard.GetLoudnessRangeFull(i),
						dr: AudioWizard.GetDynamicRangeFull(i),
						pd: AudioWizard.GetPureDynamicsFull(i)
					};

					singleMetrics.set(key, metrics);

					console.log(`Audio Wizard => Track ${i + 1}: ${key}`);
					console.log(`  M LUFS: ${metrics.mLufs.toFixed(2)} | S LUFS: ${metrics.sLufs.toFixed(2)} | I LUFS: ${metrics.iLufs.toFixed(2)}`);
					console.log(`  RMS: ${metrics.rms.toFixed(2)} | Peak: ${metrics.samplePeak.toFixed(2)} | True Peak: ${metrics.truePeak.toFixed(2)}`);
					console.log(`  PSR: ${metrics.psr.toFixed(2)} | PLR: ${metrics.plr.toFixed(2)} | CF: ${metrics.cf.toFixed(2)}`);
					console.log(`  LRA: ${metrics.lra.toFixed(2)} | DR: ${metrics.dr.toFixed(2)} | PD: ${metrics.pd.toFixed(2)}`);
					console.log("");
				}

				resolve({ success: true, singleMetrics });
			};

			AudioWizard.SetFullTrackAnalysisCallback(onComplete);
			AudioWizard.StartFullTrackAnalysis(metadata, chunkDuration);
		});
	}
	catch (e) {
		console.log(`Audio Wizard => Error in full-track single metrics analysis: ${e.message || e}`);
		return { success: false };
	}
}
```

**Notes**:
- Requires `plman` and `fb` objects.
- Uses `Promise` for asynchronous analysis with timeout.
- Check `FullTrackProcessing` property before starting.
- Specify track index in individual getters (e.g., `GetMomentaryLUFSFull(i)`).
- Call `StopFullTrackAnalysis()` to abort early.

<br>
<br>

### Full-Track Analysis (Album-Level)

Asynchronously analyze album-level metrics:

```javascript
/**
 * Starts full-track metrics analysis for album-level metrics.
 * @global
 * @param {FbMetadbHandle|FbMetadbHandleList|null} metadb - The metadb handle(s).
 * @param {number} [chunkDuration] - The optional chunk duration from 10-1000ms.
 * @returns {Promise<{success: boolean, albums?: Map}>}
 */
async function startFullTrackMetricsAlbum(metadb, chunkDuration = 200) {
	if (!AudioWizard || AudioWizard.FullTrackProcessing) {
		return { success: false };
	}

	console.log("Audio Wizard => Starting full-track album metrics analysis...");

	try {
		const { handleList, metadata, artists, albums } = getMetadata(metadb);

		if (!handleList || handleList.Count === 0) {
			console.log("Audio Wizard => No tracks to analyze.");
			return { success: false };
		}

		return await new Promise((resolve) => {
			const onComplete = (success) => {
				if (!success) {
					console.log('Audio Wizard => Analysis failed or cancelled');
					resolve({ success: false, albums: new Map() });
					return;
				}

				console.log("Audio Wizard => Album metrics analysis complete!");

				const albumMetrics = new Map();
				const albumUniques = new Set();

				for (let i = 0; i < handleList.Count; i++) {
					const album = albums[i];
					if (!album || albumUniques.has(album)) continue;

					albumUniques.add(album);

					albumMetrics.set(album, {
						artist: artists[i],
						dr: AudioWizard.GetDynamicRangeAlbumFull(album),
						pd: AudioWizard.GetPureDynamicsAlbumFull(album)
					});
				}

				console.log(`Audio Wizard => Analyzed ${albumMetrics.size} unique album(s):`);

				for (const [album, data] of albumMetrics.entries()) {
					const drA = data.dr === -Infinity ? '-inf' : data.dr.toFixed(2);
					const pdA = data.pd === -Infinity ? '-inf' : data.pd.toFixed(2);

					console.log(`Audio Wizard => Album: ${data.artist} - ${album}`);
					console.log(`  DR-A: ${drA} | PD-A: ${pdA}`);
					console.log('');
				}

				resolve({ success: true, albums: albumMetrics });
			};

			AudioWizard.SetFullTrackAnalysisCallback(onComplete);
			AudioWizard.StartFullTrackAnalysis(metadata, chunkDuration);
		});
	}
	catch (e) {
		console.log(`Audio Wizard => Error in full-track album metrics analysis: ${e.message || e}`);
		return { success: false };
	}
}
```

**Notes**:
- Requires `plman` and `fb` objects.
- Uses `Promise` for asynchronous analysis with timeout.
- Check `FullTrackProcessing` property before starting.
- Use `GetDynamicRangeAlbumFull(album)` and `GetPureDynamicsAlbumFull(album)` with album name.
- Album metrics (DR-A, PD-A) are computed across all tracks in the album.

<br>
<br>

### Waveform Analysis

Generate waveform data:

```javascript
/**
 * Starts waveform analysis for single or multiple tracks.
 * @param {FbMetadbHandle|FbMetadbHandleList|null} metadb - The metadb handle(s).
 * @param {number} [resolution] - The optional resolution in points/sec from 1-1000.
 * @param {boolean} [downmixToMono] - The optional downmix of all channels are averaged to a single mono channel.
 * @returns {Promise<{success: boolean, tracks?: Array<{
 *    index: number, path: string, duration: number, channels: number,
 *    metricsPerChannel: number, metrics: string[], pointsPerSecond: number, waveformData: Array}>,
 *    waveformDataInfo?: Object
 * }>}
 */
async function startWaveformAnalysis(metadb, resolution = 1, downmixToMono = false) {
	if (!AudioWizard || AudioWizard.FullTrackProcessing) {
		return { success: false };
	}

	console.log("Audio Wizard => Starting waveform analysis...");

	try {
		const { metadata } = getMetadata(metadb);
		return await new Promise((resolve) => {
			const onComplete = (success) => {
				try {
					console.log(`Audio Wizard => Waveform callback fired, success: ${success}`);

					if (!success) {
						console.log('Audio Wizard => Waveform analysis failed');
						resolve({ success: false });
						return;
					}

					const tracks = [];
					const trackCount = AudioWizard.GetWaveformTrackCount();
					const waveformDataInfo = getWaveformDataInfo();
					console.log(`Audio Wizard => Processing ${trackCount} track(s)`);

					if (!waveformDataInfo) {
						console.log('Audio Wizard => Waveform analysis failed: could not retrieve data schema (GetWaveformDataInfo)');
						resolve({ success: false });
						return;
					}

					const { metricsPerChannel, metrics, pointsPerSecond } = waveformDataInfo;

					for (let i = 0; i < trackCount; i++) {
						const path = AudioWizard.GetWaveformTrackPath(i);
						const duration = AudioWizard.GetWaveformTrackDuration(i);
						const waveformData = AudioWizard.GetWaveformData(i);
						const channels = waveformData.length;
						const numPoints = channels > 0 ? waveformData[0].length / metricsPerChannel : 0;
						const durLog = duration.toFixed(2);
						const resLog = duration > 0 ? (numPoints / duration).toFixed(1) : '0.0';

						tracks.push({ index: i, path, duration, channels, metricsPerChannel, metrics, pointsPerSecond, waveformData });
						console.log(`Audio Wizard => Track ${i + 1}: ${channels}ch, ${numPoints} points over ${durLog}s, resolution: ~${resLog} pts/sec`);
						// console.log(`Audio Wizard => Track ${i + 1}: ${waveformData.map(v => Number(v.toFixed(3))).join(',')}`);
					}

					resolve({ success: true, tracks, waveformDataInfo });
				}
				catch (e) {
					AudioWizard.StopWaveformAnalysis();
					resolve({ success: false });
					console.log(`Audio Wizard => Error in waveform callback: ${e.message}`);
				}
			};

			AudioWizard.SetFullTrackWaveformCallback(onComplete);
			AudioWizard.StartWaveformAnalysis(metadata, resolution, downmixToMono);
		});
	}
	catch (e) {
		console.log(`Audio Wizard => Error in waveform analysis: ${e.message}`);
		AudioWizard.StopWaveformAnalysis();
		return { success: false };
	}
}
```

**Notes**:
- Uses `Promise` for asynchronous analysis with timeout.
- Check `FullTrackProcessing` property before starting.
- This method allows analyzing ANY tracks using the metadata array overload.
- Use `SetFullTrackWaveformCallback` for completion notification.
- Call `StopWaveformAnalysis()` to abort or clean up after completion.
- Resolution is in points per second (1-1000).
- When `downmixToMono = true`, all source channels are averaged to mono internally.
  `waveformData.length` will be `1`. This eliminates the need for manual per-script averaging and reduces storage by the channel count factor.

<br>
<br>

### Waveform Analysis (Persistence & Caching)

Demonstrates how to analyze multiple tracks and save the results to the local file system using compression.

```javascript
/**
 * Analyzes tracks and saves the waveform data to JSON files in a cache folder.
 * @param {FbMetadbHandle|FbMetadbHandleList|null} metadb - The metadb handle(s).
 * @param {string} cachePath - The folder where .awz.json files will be stored.
 * @param {number} [resolution] - The optional resolution in points/sec from 1-1000.
 * @param {boolean} [downmixToMono] - The optional downmix of all channels are averaged to a single mono channel.
 * @param {boolean} [prettify] - The optional prettified JSON output format - increases filesize.
 */
async function startWaveformAnalysisFileSaving(metadb, cachePath, resolution = 1, downmixToMono = false, prettify = false) {
	if (!AudioWizard || AudioWizard.FullTrackProcessing) return;

	const handleData = metadb || plman.GetPlaylistSelectedItems(plman.ActivePlaylist);
	const handleList = new FbMetadbHandleList(handleData);
	const handleArray = handleList.Convert();

	console.log(`Audio Wizard => Batch processing ${handleArray.length} tracks...`);

	const result = await startWaveformAnalysis(metadb, resolution, downmixToMono);
	if (!result.success) return;

	const fso = new ActiveXObject('Scripting.FileSystemObject');
	if (!fso.FolderExists(cachePath)) fso.CreateFolder(cachePath);
	const tfArtistTitle = fb.TitleFormat('%artist% - %title%');
	const regexIllegalChars = /[<>:"\/\\|?*]+/g;
	const regexFileExtension = /\.[^/.]+$/;

	for (const track of result.tracks) {
		const handle = handleArray[track.index];
		let fileName = tfArtistTitle.EvalWithMetadb(handle).trim();

		if (!fileName) {
			const baseName = track.path.split('\\').pop().replace(regexFileExtension, '');
			fileName = baseName || track.path;
		}

		fileName = fileName.replace(regexIllegalChars, '_').substring(0, 100);
		const fullPath = `${cachePath}\\${fileName}.awz.json`;
		const structuredData = [];
		const { metricsPerChannel } = track;

		for (let ch = 0; ch < track.channels; ch++) {
			const channelPoints = [];
			const chData = track.waveformData[ch];

			for (let pt = 0; pt < chData.length; pt += metricsPerChannel) {
				const point = new Array(metricsPerChannel);
				for (let m = 0; m < metricsPerChannel; m++) {
					point[m] = Math.round(chData[pt + m] * 1000) / 1000;
				}
				channelPoints.push(point);
			}
			structuredData.push(channelPoints);
		}

		const jsonObj = {
			componentVersion: result.waveformDataInfo.componentVersion,
			waveformDataVersion: result.waveformDataInfo.waveformDataVersion,
			channels: track.channels,
			duration: track.duration,
			metricsPerChannel,
			metrics: track.metrics,
			pointsPerSecond: result.waveformDataInfo.pointsPerSecond,
			data: structuredData
		};

		let jsonFile = '';

		if (prettify) {
			const regexMetricsField = /"metrics":\s*\[[^\]]*\]/g;
			const regexArraySpaceOpen = /\[ /g;
			const regexArraySpaceClose = / \]/g;
			const regexNumericArray = /\[([\s\d.,-]+)\]/g;

			// 1. Generate standard tabbed JSON string first
			jsonFile = JSON.stringify(jsonObj, null, '\t');

			// 2. Collapse the "metrics" string array into a single line
			jsonFile = jsonFile.replace(regexMetricsField, (match) =>
				match.replace(/\s+/g, ' ').replace(regexArraySpaceOpen, '[').replace(regexArraySpaceClose, ']'));

			// 3. Collapse the innermost number data arrays into single lines
			jsonFile = jsonFile.replace(regexNumericArray, (match, contents) =>
				`[${contents.trim().replace(/\s+/g, ' ')}]`);
		} else {
			jsonFile = JSON.stringify(jsonObj);
		}

		try {
			const file = fso.CreateTextFile(fullPath, true, true);
			file.Write(jsonFile);
			file.Close();
			console.log(`Audio Wizard => Saved: ${fullPath}`);
		} catch (e) {
			console.log(`Audio Wizard => Failed to save ${track.path}: ${e.message}`);
		}
	}
}
```

**Notes**:
- `GetWaveformData` returns an array of channel arrays, `waveformData.length` equals the channel count after any downmix.
- `waveformData[ch]` is a flat array where every metricsPerChannel values represent one time point.
   Call GetWaveformDataInfo() to get metricsPerChannel and the ordered metrics array
- For stereo (downmixToMono = false): `waveformData[0]` = left channel, `waveformData[1]` = right channel.
- For mono (downmixToMono = true): `waveformData[0]` = averaged mono signal; no further averaging needed in script code.
- The saved JSON reflects `channels: 1` when downmixToMono is true.
- For production use, consider LZString or LZUTF8 to reduce file size by up to 90%.

<br>
<br>

### Data Info Test Demo

A quick manual test that exercises `GetPhysicalFilePath`, `GetFullTrackMetricsDataInfo`, and both call shapes of `GetWaveformDataInfo` against a real selection, with pass/fail checks on the shapes described in this document.

```javascript
/**
 * A quick manual test to print Audio Wizard data structures.
 * Run with track(s) selected: startDataInfoTestDemo();
 * Check View > Console in foobar2000 for output.
 */
async function startDataInfoTestDemo() {
	if (!AudioWizard) {
		console.log('Audio Wizard => TEST ABORTED: AudioWizard is not available.');
		return;
	}

	const check = (label, condition) => console.log(`***[${condition ? 'PASS' : 'FAIL'}]*** ${label}`);
	const SEPARATOR = `\n${'-'.repeat(100)}\n`;

	// * TEST 1: GetPhysicalFilePath * //
	console.log('\n>>> TEST 1: GetPhysicalFilePath <<<');
	const handleData = plman.GetPlaylistSelectedItems(plman.ActivePlaylist);
	const handleList = new FbMetadbHandleList(handleData);
	const handle = handleList.Count > 0 ? handleList.Convert()[0] : null;

	if (handle) {
		const resolved = getPhysicalFilePath(handle);
		console.log(`handle.Path: ${handle.Path}`);
		console.log(`handle.RawPath: ${handle.RawPath}`);
		console.log(`Resolved: ${resolved}`);
		check('resolved path has no leftover scheme/pipe characters', !/unpack:\/\/|file:\/\/|\|/.test(resolved));
	} else {
		console.log('No track selected - select one in the playlist and re-run.');
	}

	console.log(SEPARATOR);

	// * TEST 2: getFullTrackMetricsDataInfo (schema only, no analysis needed) * //
	console.log('>>> TEST 2: getFullTrackMetricsDataInfo (schema only, no analysis needed) <<<');
	const fullInfo = getFullTrackMetricsDataInfo();
	console.log(JSON.stringify(fullInfo, null, 2));

	check('metricsPerTrack === 12', fullInfo && fullInfo.metricsPerTrack === 12);
	check('metrics.length === metricsPerTrack', fullInfo && fullInfo.metrics && fullInfo.metrics.length === fullInfo.metricsPerTrack);
	check("metrics uses API identifiers, e.g. 'PD' is present", !!(fullInfo && fullInfo.metrics && fullInfo.metrics.includes('PD')));

	console.log(SEPARATOR);

	// * TEST 3: getWaveformDataInfo() - component-level only, no analysis needed * //
	console.log('>>> TEST 3: getWaveformDataInfo() - component-level only, no analysis needed <<<');
	const waveInfoComponentOnly = getWaveformDataInfo();
	console.log(JSON.stringify(waveInfoComponentOnly, null, 2));

	check('metricsPerChannel === 5', waveInfoComponentOnly && waveInfoComponentOnly.metricsPerChannel === 5);
	check('metrics.length === metricsPerChannel', waveInfoComponentOnly && waveInfoComponentOnly.metrics && waveInfoComponentOnly.metrics.length === waveInfoComponentOnly.metricsPerChannel);
	check('no per-track fields at component level', waveInfoComponentOnly && !('channels' in waveInfoComponentOnly) && !('path' in waveInfoComponentOnly) && !('duration' in waveInfoComponentOnly));

	console.log(SEPARATOR);

	// * TEST 4: getWaveformDataInfo(trackIndex) - the optional-argument path * //
	console.log('>>> TEST 4: getWaveformDataInfo(trackIndex) - the optional-argument path <<<');
	const waveResult = await startWaveformAnalysis(null, 5);

	if (!waveResult.success) {
		console.log('Waveform analysis failed or nothing selected - select track(s) in the playlist and re-run.');
	}
	else {
		for (let i = 0; i < waveResult.tracks.length; i++) {
			const perTrackInfo = getWaveformDataInfo(i);
			console.log(`Track ${i}:`, JSON.stringify(perTrackInfo, null, 2));
			check(`track ${i} has channels/path/duration`, perTrackInfo && 'channels' in perTrackInfo && 'path' in perTrackInfo && 'duration' in perTrackInfo);
			check(`track ${i} decoded (channels > 0)`, perTrackInfo && perTrackInfo.channels > 0);
			check(`track ${i} channel count agrees between GetWaveformDataInfo and GetWaveformData`, perTrackInfo && perTrackInfo.channels === waveResult.tracks[i].channels);
		}

		console.log('=> error path: out-of-range track index <=');

		try {
			AudioWizard.GetWaveformDataInfo(waveResult.tracks.length + 5);
			check('out-of-range index throws instead of returning', false);
		} catch (e) {
			check(`out-of-range index throws (caught: ${e.message})`, true);
		}
	}

	console.log(SEPARATOR);

	// * TEST 5: schema consumed live inside startFullTrackMetricsBatch * //
	console.log('>>> TEST 5: schema consumed live inside startFullTrackMetricsBatch <<<');
	const batchResult = await startFullTrackMetricsBatch(null, 200);
	console.log('success:', batchResult.success);

	if (batchResult.success && fullInfo) {
		check('batch metrics length is a multiple of metricsPerTrack', batchResult.metrics.length % fullInfo.metricsPerTrack === 0);
		console.log('Check the per-track metric lines logged above used all 12 real metric names, not hardcoded ones.');
	}

	console.log(SEPARATOR);
	console.log('>>> ALL TESTS COMPLETE <<<\n');
}
```

**Notes**:
- Select one or more tracks in the playlist before running `startDataInfoTestDemo()`, then check `View` > `Console` for output.
- TEST 2 and TEST 3 don't require any analysis to have run - they only query the static schema.
- TEST 4 runs a real waveform analysis so it can exercise the per-track branch of `GetWaveformDataInfo(trackIndex)`.
- If you have an archived or CUE-sheet track handy, run TEST 1 against it too - a plain file doesn't exercise the archive-unwrapping path.

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
|:----------------------------------|:---------------------|:-----------|:----------------------------------------------------------------------------|
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
| FullTrackProcessing               | bool                 | Read-only  | Indicates if full-track analysis or waveform analysis is currently running. |
| SystemDebugLog                    | bool                 | Read/Write | Prints detailed debug logs in the foobar console.                           |

- **Peakmeter Monitoring**:
  - `PeakmeterOffset`: Adjusts gain for peakmeter measurements. Set as an integer (e.g., `AudioWizard.PeakmeterOffset = 5` for 5 dB); returns a float when read.
  - `PeakmeterAdjustedLeftRMS`, `PeakmeterAdjustedRightRMS`: Adjusted RMS levels (~-5 to +5 dBFS), processed with dynamic gain offset and fade-in effects, not raw RMS.
  - `PeakmeterAdjustedLeftSamplePeak`, `PeakmeterAdjustedRightSamplePeak`: Adjusted sample peaks (~-5 to +5 dBFS), processed with dynamic gain offset and smoothing.

- **Raw Audio Data**:
  - `RawAudioData`: Raw PCM samples; see [Raw Audio Data](#raw-audio-data) example.

- **Processing State**:
  - `FullTrackProcessing`: Use this to check if an analysis operation is in progress before starting a new one.

<br>
<br>

### Methods

| Name                            | Signature                                               | Description                                                           |
|:--------------------------------|:--------------------------------------------------------|:----------------------------------------------------------------------|
| GetPhysicalFilePath             | (virtualPath: string) -> string                         | Resolves a virtual/archive path to its physical on-disk file path.    |
|                                 |                                                         |                                                                       |
| StartRealTimeMonitoring         | (refreshRate: number, chunkDuration: number) -> void    | Starts real-time monitoring.                                          |
| StopRealTimeMonitoring          | () -> void                                              | Stops real-time monitoring.                                           |
| StartPeakmeterMonitoring        | (refreshRate: number, chunkDuration: number) -> void    | Starts peakmeter monitoring.                                          |
| StopPeakmeterMonitoring         | () -> void                                              | Stops peakmeter monitoring.                                           |
| StartRawAudioMonitoring         | (refreshRate: number, chunkDuration: number) -> void    | Starts raw audio data capture.                                        |
| StopRawAudioMonitoring          | () -> void                                              | Stops raw audio data capture.                                         |
| StartWaveformAnalysis           | (metadata: string[], resolution: number, [downmixToMono: boolean]) -> void | Starts asynchronous waveform analysis (1-1000 points/s). Pass `true` to downmix all channels to mono internally. |
| StopWaveformAnalysis            | () -> void                                              | Stops waveform analysis.                                              |
| GetWaveformData                 | (trackIndex: number) -> Array                           | Returns waveform data points for the specified track (0-based index). |
| GetWaveformDataInfo             | ([trackIndex: number]) -> string (JSON)                 | Returns the waveform data schema as JSON: `componentVersion`, `waveformDataVersion`, `metricsPerChannel`, `metrics`, `pointsPerSecond`. When `trackIndex` is provided, also includes that track's `channels`, `path`, `duration`. |
| GetWaveformTrackCount           | () -> number                                            | Returns the number of tracks loaded in waveform analysis.             |
| GetWaveformTrackDuration        | (trackIndex: number) -> number                          | Returns the duration in seconds for the specified waveform track.     |
| GetWaveformTrackPath            | (trackIndex: number) -> string                          | Returns the file path for the specified waveform track.               |
| SetFullTrackWaveformCallback    | (callback: (success: bool) => void) -> void             | Sets the callback for waveform analysis completion.                   |
| StartFullTrackAnalysis          | (metadata: string[], chunkDuration: number) -> void     | Starts asynchronous analysis.                                         |
| StopFullTrackAnalysis           | () -> void                                              | Stops full-track analysis.                                            |
| SetFullTrackAnalysisCallback    | (callback: (success: bool) => void) -> void             | Sets the callback for analysis completion.                            |
| GetFullTrackMetrics             | () -> Array                                             | Returns all metrics for all analyzed tracks.                          |
| GetFullTrackMetricsDataInfo     | () -> string (JSON)                                     | Returns the full-track metrics schema as JSON: `componentVersion`, `fullTrackMetricsDataVersion`, `metricsPerTrack`, `metrics`. |
| GetMomentaryLUFSFull            | ([index: number]) -> number                             | Returns Momentary LUFS for the specified track (default: 0).          |
| GetShortTermLUFSFull            | ([index: number]) -> number                             | Returns Short Term LUFS for the specified track (default: 0).         |
| GetIntegratedLUFSFull           | ([index: number]) -> number                             | Returns Integrated LUFS for the specified track (default: 0).         |
| GetRMSFull                      | ([index: number]) -> number                             | Returns RMS for the specified track (default: 0).                     |
| GetSamplePeakFull               | ([index: number]) -> number                             | Returns Sample Peak for the specified track (default: 0).             |
| GetTruePeakFull                 | ([index: number]) -> number                             | Returns True Peak for the specified track (default: 0).               |
| GetPSRFull                      | ([index: number]) -> number                             | Returns PSR for the specified track (default: 0).                     |
| GetPLRFull                      | ([index: number]) -> number                             | Returns PLR for the specified track (default: 0).                     |
| GetCrestFactorFull              | ([index: number]) -> number                             | Returns Crest Factor for the specified track (default: 0).            |
| GetLoudnessRangeFull            | ([index: number]) -> number                             | Returns Loudness Range for the specified track (default: 0).          |
| GetDynamicRangeFull             | ([index: number]) -> number                             | Returns Dynamic Range for the specified track (default: 0).           |
| GetPureDynamicsFull             | ([index: number]) -> number                             | Returns Pure Dynamics for the specified track (default: 0).           |
| GetDynamicRangeAlbumFull        | (albumName: string) -> number                           | Returns Dynamic Range album metric for the specified album.           |
| GetPureDynamicsAlbumFull        | (albumName: string) -> number                           | Returns Pure Dynamics album metric for the specified album.           |

- **File Path Resolution**:
  - `GetPhysicalFilePath(virtualPath)`: Pass `handle.RawPath`, not `handle.Path`, to disambiguate plain files from archive members.
  Returns the container file's physical path for zip/CUE/Unpack-wrapped tracks.

- **Real-Time Monitoring**:
  - `StartRealTimeMonitoring`: Set `refreshRate` (ms) and `chunkDuration` (ms) for update frequency and data granularity.

- **Peakmeter Monitoring**:
  - `StartPeakmeterMonitoring`: Adjust parameters for visualization responsiveness.

- **Raw Audio Monitoring**:
  - `StartRawAudioMonitoring`: Use cautiously due to high data volume.

- **Full-Track Analysis**:
  - `StartFullTrackAnalysis(metadata, chunkDuration)`: Analyzes specific tracks using metadata array.
  - Metadata format: Array of strings with format `"path\u001Fsubsong"` (Unicode Information Separator One, U+001F).
  - `StopFullTrackAnalysis`: Use to abort early.
  - `SetFullTrackAnalysisCallback`: Provide a JavaScript function that receives a boolean `success` parameter.
  - `GetFullTrackMetrics`: Returns a flat array of metrics for all analyzed tracks.
    Call `GetFullTrackMetricsDataInfo()` once per session to get `metricsPerTrack` and `metrics[]`
    - don't hardcode the stride or offsets, it can change between component versions (check `fullTrackMetricsDataVersion`).
    As of structure version 1, `metricsPerTrack` is 12 and `metrics` is
    `["M LUFS","S LUFS","I LUFS","RMS","SP","TP","PSR","PLR","CF","LRA","DR","PD"]`.
    Indexed as: `track_offset = track_index * metricsPerTrack`, then `GetFullTrackMetrics()[track_offset + metrics.indexOf(name)]`
    (i.e index into the data array returned by `GetFullTrackMetrics()`, not into `metrics[]` itself, which only holds the 12 name strings).
  - `GetFullTrackMetricsDataInfo()`: Returns the data schema as JSON (`componentVersion`, `fullTrackMetricsDataVersion`, `metricsPerTrack`,
    `metrics[]`). This is the source of truth for the `GetFullTrackMetrics` layout above.
  - `GetMomentaryLUFSFull`, `GetShortTermLUFSFull`, `GetIntegratedLUFSFull`, `GetRMSFull`, `GetSamplePeakFull`, `GetTruePeakFull`, `GetPSRFull`, `GetPLRFull`, `GetCrestFactorFull`, `GetLoudnessRangeFull`, `GetDynamicRangeFull`, `GetPureDynamicsFull`: Use track index (default: 0).

- **Full-Album Analysis**:
  - `GetDynamicRangeAlbumFull`: Use album name (string) to retrieve Dynamic Range album metric.
  - `GetPureDynamicsAlbumFull`: Use album name (string) to retrieve Pure Dynamics album metric.

- **Waveform Analysis**:
  - `StartWaveformAnalysis(metadata, resolution, downmixToMono)`: Analyzes specific tracks using metadata array (multi-track support).
  - Set `resolution` (points per second, 1-1000) for data granularity.
  - Set `downmixToMono` to `true` to average all channels into a single mono stream before analysis.
    When set, `GetWaveformData(i).length` will always be `1`, and no per-script averaging is needed.
  - `GetWaveformData(trackIndex)`: Returns an array of channel arrays (one flat array per channel).
    `waveformData.length` = number of channels (`1` when downmixToMono was true).
    `waveformData[ch]` is a flat array where every `metricsPerChannel` values represent one time point, in the order given by `metrics[]`.
    Call `GetWaveformDataInfo()` once per session to get `metricsPerChannel` and `metrics[]` - don't hardcode the layout,
    it can change between component versions (check `waveformDataVersion`).
    As of structure version 1, `metricsPerChannel` is 5 and `metrics` is `["rms", "rms_peak", "sample_peak", "min", "max"]`:
     - `rms`: RMS level (dB)
     - `rms_peak`: RMS Peak (dB, decaying)
     - `sample_peak`: Sample Peak (dB)
     - `min`: Min sample value (linear, -1.0 to 1.0)
     - `max`: Max sample value (linear, -1.0 to 1.0)
     For stereo (default): `waveformData[0]` = left, `waveformData[1]` = right.
     For mono output: `waveformData[0]` = averaged mono signal.
     Total time points = `waveformData[0].length / metricsPerChannel`.
  - `GetWaveformDataInfo([trackIndex])`: Returns the data schema as JSON
     (`componentVersion`, `waveformDataVersion`, `metricsPerChannel`, `metrics[]`, `pointsPerSecond`,
     plus `channels`/`path`/`duration` when `trackIndex` is given).
     This is the source of truth for the `GetWaveformData` layout above.
  - `GetWaveformTrackCount()`: Returns number of analyzed tracks, useful for looping.
  - `GetWaveformTrackDuration(trackIndex)` and `GetWaveformTrackPath(trackIndex)`: Retrieve track metadata for display or caching.
  - `SetFullTrackWaveformCallback`: Provide a JavaScript function that receives a boolean `success` parameter.

<br>
<br>

## Performance Considerations

- **Refresh Rate and Chunk Duration**:
  - Lower `refreshRate` (e.g., 10 ms) increases CPU usage but improves responsiveness. Use 17-50 ms for a balance.
- **Raw Audio Data**:
  - At 44.1 kHz, a 50 ms `chunkDuration` generates ~2205 samples per channel. Use smaller chunks (e.g., 10 ms) for lower memory usage.
- **Waveform Analysis**:
  - Higher `resolution` (e.g., more than 20 points per second) increases data points but may slow processing. Use 1-20 points per second for most applications.
- **Full-Track Analysis**:
  - Larger `chunkDuration` (e.g., 500 ms) reduces processing time but lowers granularity.
  - Check `FullTrackProcessing` property before starting new analysis to avoid overlapping operations.

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
|:----------------------------------------|:-----------------------------------------------------------------------------------------------------------|
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
| Full Metrics Data Structure             | Flat array of `metricsPerTrack` values per track, in `metrics[]` order.                                    |
| Waveform Data Structure                 | Per-channel flat array of `metricsPerChannel` values per time point, in `metrics[]` order.                 |
| Waveform Resolution                     | The density of data points per second in waveform analysis, ranging from 1 to 1000.                        |
| dBFS                                    | Decibels relative to Full Scale, a measure of amplitude relative to the maximum possible digital level.    |
| dBTP                                    | Decibels True Peak, accounting for inter-sample peaks in digital audio.                                    |

<br>
<br>

## Support

- [Audio Wizard on Github](https://github.com/The-Wizardium/Audio-Wizard)
