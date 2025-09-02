<p align="center">
  <br>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../images/PD-Logo-Dark.svg">
    <img alt="Pure Dynamics Logo" src="../images/PD-Logo-Light.svg" width="350">
  </picture>
  <br>
  <br>
</p>

<h1 align="center">Pure Dynamics</h1>
<p align="center">A Novel And Unique Dynamic Range Metric</p>

<br>
<br>

## Pure Dynamics Algorithm

The PD algorithm, implemented in `GetPureDynamicsFull`, processes audio through a multi-stage pipeline
that mirrors auditory perception: peripheral processing, spatial adjustments, temporal analysis, cognitive weighting,
and integrative spread calculation. Below are the preparation, pipeline stages, and performance details,
with helper functions integrated for clarity.

<br>

### Preparation and Spectral Analysis

The pipeline begins with `ProcessDynamicsFactors`, computing spectral features via Fast Fourier Transform (FFT)
on the Traunmüller 25 Bark scale (20 Hz to ~23.8 kHz, 3 Hz bin width).
This aligns with human hearing using `AWHAudioFFT::BARK_BAND_FREQUENCY_EDGES`. Key features include:

- **Spectral Centroid** (`AWHAudioFFT::ComputeSpectralCentroid`): Indicates tonal brightness.
- **Spectral Flatness** (`AWHAudioFFT::ComputeSpectralFlatness`): Balances tonal and noisy components.
- **Spectral Flux** (`AWHAudioFFT::ComputeSpectralFlux`): Tracks dynamic transitions.
- **Critical Band Factor** (`AWHAudioFFT::ComputeCriticalBandsFromPowerSpectrum`): Groups frequencies perceptually.
- **Harmonic Complexity Factor** (`AWHAudioFFT::ComputeHarmonicComplexity`): Assesses harmonic structure.
- **Frequency Masking Factor** (`AWHAudioFFT::ComputeFrequencyMaskingFromPowerSpectrum`): Handles louder frequencies masking quieter ones.
- **Frequency Powers** (`AWHAudioFFT::ComputePerceptualFrequencyPower`): Computes weighted power across Bark bands.
- **Binaural Factor** (`AWHAudioDynamics::ComputeSpatialScore`): Evaluates spatial perception.

These features, cached via `AWHAudioFFT::barkWeightCache`, initialize `FullTrackDataDynamics`
with block sizes, genre factors, and loudness metrics (`ProcessDynamicsInitialization`).

<br>

### Pipeline Stages

#### Stage 0: Initialization
Initializes block sizes, genre factors, silence thresholds, and loudness metrics using `ProcessDynamicsInitialization`,
producing `dynamics.blockLoudness`, `dynamics.genreFactor`, and spectral vectors.
*Helpers: AWHAudioDynamics::ComputeBaseBlockLoudness, AWHAudioFFT::ComputeSpectralGenreFactors*

<br>

#### Stage 1: Loudness Correction
Adjusts loudness using Zwicker and Fastl models with genre-specific clamping (12-20 dB) via `ProcessDynamicsLoudnessCorrection`,
producing `dynamics.correctedLoudness`.
*Helpers: AWHAudioDynamics::ComputeFastlPrinciples, AWHAudioDynamics::ApplyPerceptualLoudnessCorrection, AWHAudioDynamics::ComputePerceptualLoudnessCorrection*

<br>

#### Stage 2: Preliminary Transient Scoring
Estimates transient activity based on loudness changes and spectral flux using `ProcessDynamicsPreliminaryTransient`,
setting `dynamics.transientScore`.
*Helpers: None*

<br>

#### Stage 3: Temporal Adaptation
Simulates neural habituation (80-200ms window) to enhance dynamic contrast via `ProcessDynamicsLoudnessAdaptation`,
updating `dynamics.adaptedLoudness`.
*Helpers: AWHAudioDynamics::ApplyPerceptualLoudnessAdaptation*

<br>

#### Stage 4: Binaural Adjustment
Enhances loudness with spatial cues (ILD, ITD, IACC), adding 2-4.5 dB using `ProcessDynamicsBinauralAdjustment`,
modifying `dynamics.adaptedLoudness`.
*Helpers: AWHAudioDynamics::ComputeSpatialScore, AWHAudioDynamics::ComputeBinauralPerception*

<br>

#### Stage 5: Transient Detection
Identifies transients using spectral flux and loudness deltas via `ProcessDynamicsTransientBoostsDetection`,
setting `dynamics.transientBoosts`.
*Helpers: AWHAudioDynamics::DetectTransients, AWHAudioDynamics::ComputeTransientDensity*

<br>

#### Stage 6: Cognitive Adjustment
Applies attention, memory, and genre-based loudness tweaks (up to 3 dB) using `ProcessDynamicsCognitiveLoudness`,
adjusting `dynamics.adaptedLoudness`.
*Helpers: AWHAudioDynamics::ComputeCognitiveLoudness, AWHAudioDynamics::ComputeOnsetRate*

<br>

#### Stage 7: Transient Boosting
Amplifies transients with genre-adjusted caps (3-5.5 dB) via `ProcessDynamicsTransientBoostsAdjustment`,
finalizing `dynamics.adaptedLoudness`.
*Helpers: AWHAudioDynamics::ApplyTransientBoost, AWHAudioDynamics::ComputePhrasingScore*

<br>

#### Stage 8: Dynamic Spread
Computes the final PD value using kurtosis and temporal masking via `ProcessDynamicsSpread`,
setting `dynamics.pureDynamics`.
*Helpers: AWHAudioDynamics::ComputeDynamicSpread, AWHAudioDynamics::ComputeTemporalWeights*

<br>

### Performance

Pure Dynamics processes audio instantly, achieving a 17ms latency at 60fps, making it seamless for live performances,
studio mixing, or checking a song’s energy on the go. Despite its complex and computationally intensive pipeline,
PD remains highly efficient through advanced optimization techniques.
It uses a **Ring Buffer** to chunk audio into manageable blocks, minimizing memory usage while maintaining smooth processing.
Additionally, PD leverages **multi-track parallel processing** to analyze multiple audio streams simultaneously,
ensuring high performance even with demanding real-time applications.
These optimizations make PD both powerful and resource-efficient, suitable for professional and consumer use alike.
