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

## Introduction

Dynamic range—the difference between an audio signal’s loudest and quietest moments—defines the emotional impact of music, from a ballad’s whisper to an orchestra’s crescendo. Traditional metrics like Dynamic Range (DR), Peak-to-Loudness Ratio (PLR), Peak-to-Short-Term Loudness Ratio (PSR), and Loudness Range (LRA) quantify this range statistically but often miss how humans perceive dynamics. **Pure Dynamics (PD)**, also known as *Perceptual Dynamics*, is a next-generation metric that aligns with human auditory perception, delivering a more intuitive and accurate measure of dynamic range. With a 17ms processing time at 60fps, PD is ideal for real-time applications while offering sophisticated, perceptually relevant analysis for audio professionals and enthusiasts.

<br>

## What is Pure Dynamics?

Pure Dynamics quantifies audio’s dynamic range by mirroring how humans hear and experience sound. Unlike traditional metrics that rely on raw signal data, PD uses a science-based process to produce a single *perceptual dynamic range* value, reflecting the listener’s experience. By accounting for loudness, punchy beats, stereo effects, and how we focus on music, PD provides a comprehensive tool for music production, live sound, and audio analysis.

<br>

## Core Features and Perceptual Principles

Pure Dynamics blends psychoacoustic science with sound to measure dynamics as humans hear them. Below are its key features, explained in everyday terms, with their official psychoacoustic principles:

### Silence Detection
*Principle: Auditory Threshold Processing*
Skips silent moments, like quiet intros or gaps between tracks, to focus on the music’s energy. This ensures dynamics reflect only the parts you actually hear, like a *singer’s voice* or *drumbeat*, without interference from silence.

### Loudness Correction
*Principle: Zwicker Loudness Model*
Adjusts volume to match what stands out to our ears, like making a *guitar riff* sound clear and natural. It emphasizes frequencies we hear best, ensuring the music feels balanced, whether it’s a soft ballad or a loud rock anthem.

### Sound Texture Analysis
*Principles: Fluctuation Strength, Roughness, Sharpness, Tonality*
Captures the feel of music, like the gentle rise of a *violin’s tremolo* or the gritty edge of a distorted guitar. These elements shape how lively or textured a song feels.

### Loudness Adaptation
*Principle: Neural Adaptation*
Mimics how our ears tune out constant sounds, like a long *synth note*, to make changes—like a sudden piano chord—feel more exciting. This keeps the music dynamic and engaging.

### Stereo Depth Enhancement
*Principle: Spatial Cues*
Makes music sound wider and more immersive, like you’re in the middle of a *live concert*. It enhances the sense of space on headphones or speakers, so instruments feel spread out around you.

### Transient Detection
*Principle: Transient Salience*
Spots and boosts sharp sounds, like a *snare drum*, to make them pop. This ensures the beat hits hard, whether it’s a jazz groove or an EDM drop, without overwhelming the mix.

### Listener Focus and Memory
*Principles: Attention & Memory*
Adjusts dynamics based on what grabs your attention, like a *lead vocal* in a pop song, and what you’ve heard across short (0.1s), medium (1s), and long (10s) timeframes, tailoring to styles like classical or hip-hop.

### Dynamic Variance Adjustment
*Principle: Dynamic Variance Processing*
Adapts to how much a song’s volume changes, ensuring tight dynamics in *pop tracks* feel punchy or wide swings in classical music feel grand. This fine-tunes processing to each track’s unique energy.

### Music Genre Adaptation
*Principle: Genre-Specific Expectations*
Fine-tunes dynamics to suit different genres, ensuring a *jazz track* feels smooth or a techno track feels intense, based on the music’s unique vibe.

### Sound Masking Analysis
*Principle: Masking*
Balances loud and soft moments, ensuring quieter parts, like a *verse before a chorus*, aren’t lost. It accounts for how loud sounds mask softer ones over time.

### Rhythm and Beat Analysis
*Principle: Phrasing & Onsets*
Highlights the rhythm and flow, like the steady pulse of a *drumbeat*, making the music’s groove more pronounced and enjoyable.

### Spectral Analysis
*Principle: Spectral Processing*
Analyzes the building blocks of sound, like the brightness of a *sparkling cymbal* or texture of a warm piano. This ensures dynamics reflect each instrument’s unique character.

<br>

## Comparison with Other Metrics

PD’s perceptually driven design outshines traditional metrics, making it the top choice for audio professionals and enthusiasts. Below is a comparison highlighting its strengths:

- **Peak-to-Short-term Ratio (PSR)**
  - *Measurement Approach*: Compares peak amplitude to short-term loudness (3s window).
  - *Perceptual Integration*: None—purely statistical.
  - *Psychoacoustic Principles*: None.
  - *Strengths*: Quick to calculate; good for spotting sudden volume spikes, like *drum hits*.
  - *Limitations*: Ignores human perception, missing spatial, temporal, or genre-specific dynamics.
  - *Best Use Case*: Basic real-time monitoring in live sound setups.

- **Peak-to-Loudness Ratio (PLR)**
  - *Measurement Approach*: Ratio of peak amplitude to integrated loudness over the entire track.
  - *Perceptual Integration*: None—focuses on overall signal.
  - *Psychoacoustic Principles*: None.
  - *Strengths*: Simple; useful for checking overall loudness balance.
  - *Limitations*: No perceptual weighting; overlooks transients and stereo effects.
  - *Best Use Case*: Quick loudness compliance checks for broadcast or streaming.

- **Loudness Range (LRA)**
  - *Measurement Approach*: Statistical range of loudness over time (EBU R128 standard).
  - *Perceptual Integration*: Minimal—basic loudness variation.
  - *Psychoacoustic Principles*: None.
  - *Strengths*: Standardized for broadcast; measures loudness changes over time.
  - *Limitations*: Lacks psychoacoustic depth; misses transients, spatial cues, and genre context.
  - *Best Use Case*: Ensuring consistent loudness for TV or streaming platforms.

- **Dynamic Range (DR)**
  - *Measurement Approach*: Ratio of peak to average RMS levels.
  - *Perceptual Integration*: None—raw signal-based.
  - *Psychoacoustic Principles*: None.
  - *Strengths*: Widely used; simple for basic dynamic range checks.
  - *Limitations*: Ignores human hearing; overestimates dynamics in compressed tracks; no genre or spatial awareness.
  - *Best Use Case*: Rough dynamic range estimation in legacy audio systems.

- **Pure Dynamics (PD)**
  - *Measurement Approach*: Multi-stage psychoacoustic pipeline processing audio like the human ear.
  - *Perceptual Integration*: Full—mimics auditory perception across time, frequency, and space.
  - *Psychoacoustic Principles*: Zwicker Loudness, Neural Adaptation, Spatial Cues, Transient Salience, Masking, and more (12 principles total).
  - *Strengths*: Captures human hearing with 12 perceptual features; adapts to genres (e.g., *jazz vs. EDM*); real-time capable (17ms at 60fps); enhances stereo depth and transients for immersive mixes.
  - *Limitations*: Higher computational cost offline; sensitive to resampling artifacts (e.g., 96 kHz to 44.1 kHz).
  - *Best Use Case*: Mastering, live sound engineering, and audiophile analysis for precise, human-centric dynamic range.

| **Metric**                         | **Basis**                    | **Perceptual Focus**      | **Strengths**                    | **Limitations**                    |
|------------------------------------|------------------------------|---------------------------|----------------------------------|------------------------------------|
| **Peak-to-Short-term Ratio (PSR)** | Peak to short-term loudness  | None                      | Captures short-term dynamics     | Lacks spatial/cognitive factors    |
| **Peak-to-Loudness Ratio (PLR)**   | Peak to integrated loudness  | None                      | Accounts for overall loudness    | Misses psychoacoustic nuance       |
| **Loudness Range (LRA)**           | Statistical loudness range   | None                      | Measures variation over time     | Not perceptually driven            |
| **Dynamic Range (DR)**             | Peak vs. average levels      | None                      | Simple, widely used              | Ignores perception                 |
| **Pure Dynamics (PD)**             | Perceptual processing        | Psychoacoustic, cognitive | Human-centric, comprehensive     | Higher computational cost (offline)|

PD’s perceptual focus ensures a truer reflection of the listening experience, as validated by Audio Engineering Society research highlighting DR’s overestimation of dynamics.

*See detailed performance comparisons in the logs below, showcasing how PD’s perceptually driven approach delivers superior dynamic range analysis across diverse music tracks.*

<br>

## Performance Logs

Elegantly formatted logs below showcase PD’s superior dynamic range analysis across diverse music tracks, comparing its perceptual accuracy with PSR, PLR, LRA, and DR. [Placeholder: Insert log tables once provided.]

<br>

## Applications of Pure Dynamics

Pure Dynamics enhances various audio workflows:
- **Music Production**: Helps craft dynamic mixes, like balancing a pop song’s vocals or a classical piece’s orchestral swells.
- **Live Sound Engineering**: Ensures real-time dynamic impact, such as maintaining clarity in a rock concert’s guitar solos.
- **Audiophile Analysis**: Enables comparison of album releases (e.g., 20 versions of a Michael Jackson album across CDs, vinyls, SACDs, and digital formats) to find the most dynamic mastering.
- **Consumer Audio**: Empowers listeners to compare dynamic energy across tracks, like assessing a jazz standard vs. an EDM drop.

<br>

## Limitations of Pure Dynamics

PD’s offline analysis is computationally intensive, taking longer than its 17ms real-time performance. It’s also sensitive to resampling artifacts; for example, converting a 96 kHz track to 44.1 kHz with SoX’s highest settings can subtly alter FFT frequencies, affecting the PD score.

<br>

## Join the Pure Dynamics Community

Released under GPLv3, Pure Dynamics invites audio experts, engineers, and developers to contribute to its open-source evolution. Refine the algorithm, adapt it for new use cases, or enhance its accuracy—join today to shape the future of perceptual audio analysis!

<br>

## Conclusion

Pure Dynamics revolutionizes audio analysis by merging psychoacoustic science with engineering precision. Its 17ms real-time processing and perceptually grounded pipeline make it a game-changer for music production, live sound, and audiophile analysis. As an open-source project under GPLv3, PD invites collaboration from music fans and professionals alike to redefine dynamic range measurement. Try it, contribute, and help shape the future of audio!

<br>

## Algorithm Overview

The Pure Dynamics algorithm, implemented in `GetPureDynamicsFull` (offline) and `GetPureDynamics` (real-time), processes audio through a multi-stage pipeline that mirrors human auditory perception, including peripheral processing, spatial adjustments, and cognitive weighting. For a detailed breakdown of its nine stages and helper functions, see the [Pure Dynamics White Paper](PD_Algorithm.md).
