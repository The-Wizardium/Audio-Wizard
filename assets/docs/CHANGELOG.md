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

*Deep within the blazing **Rubynar Sanctum**, arcane scholars of **The Wizardium**
labor to decipher the sealed passages of the **Audio Wizard**, a radiant chapter in the eternal grimoire.
Though not all pages are yet restored, each revelation refines the **Ruby Spell**,
weaving elegance and stability into foobar2000’s ethereal form.*

<br>

<h3 align="center"><em><span title="The Wisdom Of The Divine Flame">⸺ Sapientia Flamma Divina ⸺</span></em></h3>
<div align="center"><a href="https://github.com/The-Wizardium">A Sacred Chapter Of The Wizardium</a></div>

<br>
<h2></h2>
<br>
<br>

## Version 0.2.0 - 23-12-2025
This release marks a major evolution with **Wizardious Dynamic Peaks**™ — our trademark arcane visualization — and a ground-up rewrite of real-time audio acquisition for buttery-smooth, flicker-free rendering, alongside powerful new multi-track batch processing capabilities and an expanded scripting API.

#### Added
- Full **multi-track batch processing** support in both full-track metrics and waveform analysis via explicit metadata array overload (falls back to selected items if omitted).
- New API methods for querying multi-track waveform results: `GetWaveformTrackCount`, `GetWaveformTrackDuration`, and `GetWaveformTrackPath`.
- Example demonstrating **persistent waveform caching** to disk (`.aw.json` files) with safe filename handling and structured data export.
- `FullTrackProcessing` read-only property to check if analysis is currently running.
- `SystemDebugLog` property to toggle detailed debug logging at runtime.
- Success/failure boolean parameter to all analysis completion callbacks.
- Independent resolution control for waveform analysis using **points per second**.
- **Wizardious Dynamic Peaks**™ in real-time monitoring:
  - Activated in `Meters + Peaks` and `Values + Meters + Peaks` metric modes
  - Trademarked arcane visualization with glowing ethereal bullet, flowing magical trail, radiant impact burst, vertical center gradient, and seamless dark/light theme adaptation
  - Physics-based decay and natural fall-off for lifelike transient response
  - Double-buffered rendering architecture providing perfectly smooth, flicker-free animation for all meters at maximum refresh rates

#### Changed
- **Breaking**: `StartWaveformAnalysis` now accepts metadata array and points-per-second resolution.
- **Breaking**: `StartFullTrackAnalysis` now requires explicit metadata array.
- **Breaking**: `GetWaveformData` now requires a `trackIndex` parameter (multi-track support); the legacy `WaveformData` property has been removed.
- **Breaking**: All analysis callbacks now receive a boolean `success` parameter.
- Waveform resolution now uses **points per second** (1–1000) instead of milliseconds for clearer control.
- Relaxed minimum refresh rate to 10 ms and expanded chunk duration range (10–1000 ms).
- Reduced console output — detailed logs now only appear when `SystemDebugLog` is enabled.
- `GetFullTrackMetrics` now uses the last analyzed track list for consistency.

#### Fixed
- Unstable or flickering peak meters during critical signals.
- Inaccurate loudness integration windows caused by timing inconsistencies.
- Missed short transients and single-sample spikes in real-time detection.
- Track metric mismatches when playlist changes during analysis.
- Missing failure callbacks when no tracks provided or analysis errors occur.
- Waveform analysis now correctly uses independent chunk settings.
- Improved thread safety and early abort handling.
- Optimized buffered audio processing for variable chunk sizes.

#### Improved
- **Real-time audio acquisition and rendering** — Complete architectural rewrite:
  - **Timing system**: Absolute-time synchronized chunk fetching with proper delta-timing
  - **Transient capture**: Latch-based peak metrics ensuring no single-sample spikes are missed
  - **Reliability**: Automatic discontinuity detection and resynchronization
  - **Robustness**: Catch-up mechanism for missed audio during seeks or pauses
  - **Responsiveness**: Immediate metric freeze on playback pause
  - **Recovery**: Robust handling of stream restarts and temporary errors
  - **Performance**: Precision sleep and efficient UI update throttling with message-based rendering
- Audio decoding performance through dynamic buffering and reduced reallocations.
- Error handling and internal logging throughout the API.
- Waveform completion detection in multi-track scenarios.
- Overall documentation with updated API reference, new examples, and clearer notes on breaking changes, resolution, and callbacks.

<br>
<br>

## Version 0.1.0 - 02-09-2025
- Initial release of Audio Wizard.
