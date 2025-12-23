/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Helpers Source File                        * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW_Analysis.h"
#include "AW_Helpers.h"


///////////////////////
// * AUDIO HELPERS * //
///////////////////////
#pragma region Audio Helpers
namespace AWHAudio {
	double GetFrequencyLimit(double sampleRate, bool ignoreFrequencyMax) {
		constexpr double MAX_FREQUENCY = 20000.0; // 20 kHz human hearing limit
		return !ignoreFrequencyMax ? std::min(MAX_FREQUENCY, sampleRate / 2.0) : sampleRate / 2.0;
	}

	double DbToLinear(double db) {
		return std::pow(10.0, db / 20.0);
	}

	double LinearToDb(double linear) {
		return 20.0 * std::log10(linear);
	}

	double PowerToDb(double power) {
		return 10.0 * std::log10(power);
	}

	double DbToPower(double db) {
		return std::pow(10.0, db / 10.0);
	}
}
#pragma endregion


//////////////////////////////
// * AUDIO BUFFER HELPERS * //
//////////////////////////////
#pragma region Audio Buffer Helpers
namespace AWHAudioBuffer {
	// At the moment not used here but in the header
}
#pragma endregion


////////////////////////////
// * AUDIO DATA HELPERS * //
////////////////////////////
#pragma region Audio Data Helpers
namespace AWHAudioData {
	// At the moment not used here but in the header
}
#pragma endregion


///////////////////////////
// * AUDIO DSP HELPERS * //
///////////////////////////
#pragma region Audio DSP Helpers
namespace AWHAudioDSP {
	void ApplyDecayToHeldValueDb(double& heldValue, double newValue, double decayBelowThresholdDb, double decayAboveThresholdDb, double threshold) {
		heldValue = std::max(newValue, heldValue + (newValue < threshold ? decayBelowThresholdDb : decayAboveThresholdDb));
	}

	double CalculateAveragePeakDb(const std::vector<double>& peakHistory, size_t historySize) {
		double sum = 0.0;
		size_t validCount = 0;

		for (size_t i = 0; i < historySize && i < peakHistory.size(); ++i) {
			if (peakHistory[i] != -INFINITY) {
				sum += peakHistory[i];
				++validCount;
			}
		}

		return (validCount > 0) ? sum / validCount : -INFINITY;
	}

	double CalculateDesiredGainDb(double currentPeakDb, double targetPeakDb, double maxGainCeilingDb, double noiseFloorDb) {
		if (currentPeakDb <= noiseFloorDb) return 0.0;
		return std::min(targetPeakDb - currentPeakDb, maxGainCeilingDb);
	}

	double CalculateExponentialAverage(const RingBufferSimple& history, double decay) {
		const double epsilon = 1e-12;
		if (history.size() == 0) return -INFINITY;

		double sum = 0.0;
		double weight = 0.0;
		double w = 1.0;

		for (auto it = history.end(); it != history.begin(); ) {
			--it;
			if (*it > -90.0) {
				sum += w * *it;
				weight += w;
			}
			w *= decay;
		}

		return weight > epsilon ? sum / weight : -INFINITY;
	}

	std::vector<double> ComputeBlockLoudness(const RingBufferSimple& kWeightedBuffer, size_t start, size_t blockSize) {
		std::vector<double> blockLoudness;
		if (blockSize == 0 || kWeightedBuffer.size() <= start) {
			return blockLoudness;
		}
		blockLoudness.reserve((kWeightedBuffer.size() - start) / blockSize + 1);

		for (size_t blockStart = start; blockStart < kWeightedBuffer.size(); blockStart += blockSize) {
			if (blockStart + blockSize > kWeightedBuffer.size()) break;
			double sumPower = 0.0;
			size_t validSamples = 0;

			for (size_t i = blockStart; i < blockStart + blockSize; ++i) {
				if (std::isfinite(kWeightedBuffer[i])) {
					sumPower += kWeightedBuffer[i];
					++validSamples;
				}
			}

			double meanPower = validSamples > 0 ? sumPower / validSamples : 1e-12;
			if (meanPower > 1e-12) {
				blockLoudness.push_back(-0.691 + AWHAudio::PowerToDb(meanPower));
			}
			else {
				blockLoudness.push_back(-INFINITY);
			}
		}

		return blockLoudness;
	}

	void ComputeBlockSamplesAndEnergy(
		const audioType* buffer, size_t startIdx, size_t blockSize,
		size_t channels, std::vector<double>& blockSamples, double& energy,
		const std::vector<double>* hannWindow) {
		energy = 0.0;
		if (blockSamples.size() < blockSize) {
			blockSamples.resize(blockSize, 0.0);
		}

		for (size_t j = 0; j < blockSize; ++j) {
			double sample = 0.0;
			for (size_t ch = 0; ch < channels; ++ch) {
				size_t idx = startIdx + j * channels + ch;
				sample += buffer[idx];
			}

			blockSamples[j] = sample / channels;

			if (hannWindow && j < hannWindow->size()) {
				blockSamples[j] *= (*hannWindow)[j];
			}

			energy += blockSamples[j] * blockSamples[j];
		}
	}

	void ComputeBlockSamplesAndEnergy(
		const std::vector<audioType>& buffer, size_t startIdx, size_t blockSize,
		size_t channels, std::vector<double>& blockSamples, double& energy,
		const std::vector<double>* hannWindow) {

		energy = 0.0;
		if (blockSamples.size() < blockSize) {
			blockSamples.resize(blockSize, 0.0);
		}

		for (size_t j = 0; j < blockSize; ++j) {
			double sample = 0.0;
			for (size_t ch = 0; ch < channels; ++ch) {
				size_t idx = startIdx + j * channels + ch;
				sample += buffer[idx];
			}

			blockSamples[j] = sample / channels;

			if (hannWindow && j < hannWindow->size()) {
				blockSamples[j] *= (*hannWindow)[j];
			}

			energy += blockSamples[j] * blockSamples[j];
		}
	}

	std::vector<double> ComputeRawSamples(const double* samples, size_t frameCount, size_t channelCount) {
		std::vector<double> rawSamples;
		if (!samples || frameCount == 0 || channelCount == 0) return rawSamples;
		rawSamples.reserve(frameCount);

		for (size_t i = 0; i < frameCount; ++i) {
			double sampleSum = 0.0;
			for (size_t ch = 0; ch < channelCount; ++ch) {
				sampleSum += samples[i * channelCount + ch];
			}
			rawSamples.push_back(sampleSum / channelCount); // Average channels
		}

		return rawSamples;
	}

	void DownmixToStereo(const audioType* multiChannelData, size_t frames, size_t channels, double* stereoData) {
		// Mono: duplicate to both stereo channels
		if (channels == 1) {
			for (size_t i = 0; i < frames; ++i) {
				stereoData[i * 2] = stereoData[i * 2 + 1] = multiChannelData[i];
			}
			return;
		}

		// Stereo: copy directly
		if (channels == 2) {
			std::copy(multiChannelData, multiChannelData + frames * 2, stereoData);
			return;
		}

		// Downmix coefficients for multi-channel layouts
		static const std::unordered_map<size_t, std::vector<double>> DOWNMIX_COEFFS = {
			// 5.0: L, R, C, Ls, Rs
			{ 5, { 1.0, 1.0, 0.707, 0.707, 0.707 }},
			// 5.1: L, R, C, LFE, Ls, Rs
			{ 6, { 1.0, 1.0, 0.707, 0.0, 0.707, 0.707 }},
			// 7.1: L, R, C, LFE, Lss, Rss, Lrs, Rrs
			{ 8, { 1.0, 1.0, 0.707, 0.0, 0.707, 0.707, 0.5, 0.5 }},
			// 7.1+2: L, R, C, LFE, Ls, Rs, Tfl, Tfr
			{ 10, { 1.0, 1.0, 0.707, 0.0, 0.707, 0.707, 0.5, 0.5, 0.0, 0.0 }},
			// 7.1+4: L, R, C, LFE, Lss, Rss, Lrs, Rrs, Tfl, Tfr, Tbr, Tbr
			{ 12, { 1.0, 1.0, 0.707, 0.0, 0.707, 0.707, 0.5, 0.5, 0.5, 0.5, 0.0, 0.0 }},
			// 22.2: FL, FR, FC, LFE1, BL, BR, FLc, FRc, BC, LFE2, SiL, SiR,
			//       TpFL, TpFR, TpFC, TpC, TpBL, TpBR, TpSiL, TpSiR, TpBC, BtFC, BtFL, BtFR
			{ 24, { 1.0, 1.0, 0.707, 0.0, 0.5, 0.5, 0.707, 0.707, 0.5, 0.0, 0.5, 0.5,
					0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 }}
		};

		// Precomputed channel contributions: true if channel contributes to left/right
		static const std::unordered_map<size_t, std::pair<std::vector<bool>, std::vector<bool>>> CHANNEL_MAP = {
			// 5.0: L, R, C, Ls, Rs
			{ 5, {{ true, false, true, true, false }, { false, true, true, false, true }} },
			// 5.1: L, R, C, LFE, Ls, Rs
			{ 6, {{ true, false, true, false, true, false }, {false, true, true, false, false, true }}},
			// 7.1: L, R, C, LFE, Lss, Rss, Lrs, Rrs
			{ 8, {{ true, false, true, false, true, false, true, false }, { false, true, true, false, false, true, false, true }}},
			// 7.1+2: L, R, C, LFE, Ls, Rs, Tfl, Tfr
			{ 10, {{ true, false, true, false, true, false, true, false, false, false }, { false, true, true, false, false, true, false, true, false, false }}},
			// 7.1+4: L, R, C, LFE, Lss, Rss, Lrs, Rrs, Tfl, Tfr, Tbr, Tbr
			{ 12, {{ true, false, true, false, true, false, true, false, true, false, false, false }, { false, true, true, false, false, true, false, true, false, true, false, false }}},
			// 22.2: FL, FR, FC, LFE1, BL, BR, FLc, FRc, BC, LFE2, SiL, SiR, TpFL, TpFR, TpFC, TpC, TpBL, TpBR, TpSiL, TpSiR, TpBC, BtFC, BtFL, BtFR
			{ 24, {{ true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, true, true, false },
				  { false, true, true, false, false, true, false, true, true, false, false, true, false, true, true, false, false, true, false, true, true, true, false, true }}}
		};

		// Get coefficients and channel map
		auto coeff_it = DOWNMIX_COEFFS.find(channels);
		const std::vector<double>& coeffs = coeff_it != DOWNMIX_COEFFS.end() ? coeff_it->second : std::vector<double>(channels, 1.0 / std::sqrt(channels));

		auto map_it = CHANNEL_MAP.find(channels);
		const auto& [left_map, right_map] = map_it != CHANNEL_MAP.end() ? map_it->second : std::pair{ std::vector<bool>(channels, true), std::vector<bool>(channels, true) };

		// Downmix
		for (size_t i = 0; i < frames; ++i) {
			double left = 0.0;
			double right = 0.0;
			size_t idx = i * channels;

			for (size_t ch = 0; ch < channels && ch < coeffs.size(); ++ch) {
				double sample = multiChannelData[idx + ch] * coeffs[ch];
				if (left_map[ch]) left += sample;
				if (right_map[ch]) right += sample;
			}

			stereoData[i * 2] = left;
			stereoData[i * 2 + 1] = right;
		}
	}

	void ExtractStereoChannels(const audioType* block, size_t stepSize, size_t numChannels,
		std::vector<double>& leftChannel, std::vector<double>& rightChannel) {
		if (numChannels == 2) { // Direct stereo channel extraction
			for (size_t j = 0; j < stepSize; ++j) {
				leftChannel[j] = block[j * numChannels];
				rightChannel[j] = block[j * numChannels + 1];
			}
		}
		else { // Downmix to stereo for non-stereo inputs
			std::vector<double> stereoBuffer(stepSize * 2);
			DownmixToStereo(block, stepSize, numChannels, stereoBuffer.data());

			for (size_t j = 0; j < stepSize; ++j) {
				leftChannel[j] = stereoBuffer[j * 2];
				rightChannel[j] = stereoBuffer[j * 2 + 1];
			}
		}
	}

	std::vector<double> GenerateAudioWindow(WindowType windowType, size_t taps, double beta) {
		std::vector<double> window(taps);
		constexpr double PI = 3.14159265358979323846;

		for (size_t j = 0; j < taps; ++j) {
			double t = static_cast<double>(j) / (taps - 1); // Normalized position: 0 to 1

			switch (windowType) {
				case WindowType::BLACKMAN: {
					window[j] = 0.42 - 0.5 * std::cos(2.0 * PI * t) + 0.08 * std::cos(4.0 * PI * t);
					break;
				}
				case WindowType::BLACKMAN_HARRIS: {
					window[j] = 0.35875 - 0.48829 * std::cos(2.0 * PI * t) +
						0.14128 * std::cos(4.0 * PI * t) - 0.01168 * std::cos(6.0 * PI * t);
					break;
				}
				case WindowType::HAMMING: {
					window[j] = 0.54 - 0.46 * std::cos(2.0 * PI * t);
					break;
				}
				case WindowType::HANN: {
					window[j] = 0.5 * (1.0 - std::cos(2.0 * PI * t));
					break;
				}
				case WindowType::KAISER: {
					double x = 2.0 * t - 1.0; // Normalized position: -1 to 1
					double bessel = AWHMath::CalculateBesselI0(beta * std::sqrt(1.0 - x * x));
					window[j] = bessel / AWHMath::CalculateBesselI0(beta);
					break;
				}
			}
		}

		return window;
	}

	std::vector<double> GenerateHannWindow(size_t windowSize) {
		auto& window = hannWindowCache[windowSize];
		if (!window.empty()) {
			return window;
		}

		window.resize(windowSize);
		double sum = 0.0;
		constexpr double PI = 3.14159265358979323846;

		for (size_t i = 0; i < windowSize; ++i) {
			window[i] = 0.5 * (1.0 - std::cos(2.0 * PI * i / (windowSize - 1)));
			sum += window[i] * window[i];
		}

		double scale = std::sqrt(windowSize / (2.0 * sum));

		for (auto& w : window) {
			w *= scale;
		}

		return window;
	}

	std::vector<double> NormalizeLoudness(const std::vector<double>& loudness, double blockDurationMs, double windowMs) {
		if (loudness.empty() || blockDurationMs <= 0.0) {
			return loudness;
		}

		const size_t blockCount = loudness.size();
		std::vector<double> normalizedLoudness(blockCount);
		const size_t normWindowBlocks = std::max<size_t>(1, static_cast<size_t>(std::round(windowMs / blockDurationMs)));

		for (size_t i = 0; i < blockCount; ++i) {
			if (loudness[i] == -INFINITY) {
				normalizedLoudness[i] = -INFINITY;
				continue;
			}

			// Compute moving average over a window centered at i
			double sum = 0.0;
			size_t count = 0;
			for (size_t j = std::max(0, static_cast<int>(i) - static_cast<int>(normWindowBlocks / 2));
				j < std::min(blockCount, i + normWindowBlocks / 2); ++j) {
				if (loudness[j] != -INFINITY) {
					sum += loudness[j];
					count++;
				}
			}
			double avg = count > 0 ? sum / count : 0.0;
			normalizedLoudness[i] = loudness[i] - avg;
		}

		return normalizedLoudness;
	}

	void ResampleToSampleRate(const ChunkData& inputChunk, ChunkData& outputChunk,
		double targetSampleRate, size_t taps, WindowType windowType, double beta) {
		double inputSampleRate = inputChunk.sampleRate;
		double ratio = targetSampleRate / inputSampleRate;
		size_t inputFrames = inputChunk.frames;
		auto outputFrames = static_cast<size_t>(std::ceil(inputFrames * ratio));
		const double PI = 3.14159265358979323846;

		// Configure output chunk
		outputChunk.frames = outputFrames;
		outputChunk.sampleRate = targetSampleRate;
		outputChunk.channels = inputChunk.channels;

		// Allocate output buffer
		std::vector<audioType> resampledData(outputFrames * inputChunk.channels, 0.0);

		// Anti-aliasing filter cutoff
		double cutoff = 0.98 * std::min(1.0, targetSampleRate / inputSampleRate);

		// Generate Kaiser window
		std::vector<double> window = GenerateAudioWindow(windowType, taps, beta);

		// Precompute sinc filter and resample
		std::vector<double> filter(taps);
		for (size_t k = 0; k < outputFrames; ++k) {
			double t = k / ratio;
			auto m0 = static_cast<int>(std::floor(t));
			double filterSum = 0.0;

			for (size_t i = 0; i < taps; ++i) {
				int m = m0 - static_cast<int>(taps / 2) + static_cast<int>(i);
				double sincArg = (t - m) * cutoff * PI;
				filter[i] = (sincArg == 0.0) ? cutoff : std::sin(sincArg) / sincArg;
				filter[i] *= window[i];
				filterSum += filter[i];
			}

			if (filterSum != 0.0) {
				for (double& c : filter) c /= filterSum;
			}

			for (size_t ch = 0; ch < inputChunk.channels; ++ch) {
				auto acc = 0.0;
				for (size_t i = 0; i < taps; ++i) {
					int m = m0 - static_cast<int>(taps / 2) + static_cast<int>(i);
					if (m >= 0 && m < static_cast<int>(inputFrames)) {
						size_t idx = static_cast<size_t>(m) * inputChunk.channels + ch;
						acc += filter[i] * inputChunk.data[idx];
					}
				}
				resampledData[k * inputChunk.channels + ch] = static_cast<audioType>(acc);
			}
		}

		// Transfer ownership to outputChunk
		outputChunk.setOwnedData(std::move(resampledData));
	}

	double SmoothValue(double current, double target, double attackCoeff, double releaseCoeff) {
		const double coeff = (target > current) ? attackCoeff : releaseCoeff;
		return current + coeff * (target - current);
	}
}
#pragma endregion


////////////////////////////////
// * AUDIO DYNAMICS HELPERS * //
////////////////////////////////
#pragma region Audio Dynamics Helpers
namespace AWHAudioDynamics {
	double ApplyPerceptualLoudnessAdaptation(double blockLufs, double stableDuration, double refLufs, double tau, double adaptStrength) {
		if (!std::isfinite(refLufs) || !std::isfinite(blockLufs)) {
			return blockLufs;
		}

		double deltaLUFS = blockLufs - refLufs;
		double adaptationFactor = 1.0 - std::exp(-stableDuration / tau);
		double adaptedDelta = deltaLUFS * (1.0 - adaptStrength * adaptationFactor);

		return refLufs + adaptedDelta;
	}

	double ApplyPerceptualLoudnessCorrection(bool isRealTime, double fastlAdjustments, double blockLufs, double integratedLUFS, double highFreqPower, double variance, double meanPower) {
		struct LoudnessConfig {
			double slopeLow;
			double slopeHigh;
			double hfBoost;
			double varScale;
			double maxDelta;
			double minDelta;
			double trackContextWeight;
			double nonlinearExp;    // For full-track exponent
			double nonlinearFactor; // For real-time exponential term
			double nonlinearScale;  // Scaling factor for nonlinear term
		};

		static constexpr double L_REF = -23.0; // Adjusted for music (EBU R128)
		static constexpr double TRANSITION = -12.0;
		static constexpr double EPSILON = 1e-12;
		static constexpr double VARIANCE_NORM_REF = 200.0; // Music-tuned

		static const std::array<LoudnessConfig, 2> configs = {{
			{ 0.06, 0.07, 0.2, 0.04, 12.0, -12.0, 0.3, 0.4, 0.0, 0.5 }, // Full-track offline
			{ 0.04, 0.08, 0.1, 0.05, 10.0, -10.0, 0.2, 0.0, 0.25, 1.2 }  // Real-time monitoring
		}};

		const LoudnessConfig& config = configs[isRealTime];

		double deltaLUFS = blockLufs - L_REF;
		double trackContext = config.trackContextWeight * (integratedLUFS - L_REF);
		double varFactor = std::min(1.0, variance / VARIANCE_NORM_REF);
		double slope = (blockLufs < TRANSITION) ? config.slopeLow : config.slopeHigh;
		slope += config.varScale * varFactor;

		const double sign = deltaLUFS >= 0 ? 1.0 : -1.0;
		double linearDelta = slope * (deltaLUFS + trackContext);
		double nonlinearDelta = isRealTime
			? config.nonlinearFactor * (std::exp(std::abs(deltaLUFS) / 25.0) - 1.0) * sign * config.nonlinearScale
			: std::pow(std::abs(deltaLUFS), config.nonlinearExp + 0.15 * varFactor) * sign * config.nonlinearScale;

		double hfAdjust = config.hfBoost * (highFreqPower / (meanPower + EPSILON));

		if (isRealTime) { // Lightweight pseudo-sharpness for real-time
			double pseudoSharpness = 1.0 + 0.6 * (highFreqPower / (meanPower + EPSILON)); // Simplified proxy
			hfAdjust += 0.3 * (pseudoSharpness - 1.0); // Mimic 0.25 dB/acum scaling
		}

		hfAdjust = std::min(hfAdjust, 1.2);
		double delta = linearDelta + nonlinearDelta + hfAdjust + fastlAdjustments;
		delta = std::max(config.minDelta, std::min(config.maxDelta, delta));

		return blockLufs + delta;
	}

	// Apply transient boost to a loudness value with adjustable weighting
	double ApplyTransientBoost(double baseLufs, double transientBoost, double weight) {
		double adjustedLufs = baseLufs + AWHAudio::PowerToDb(transientBoost); // Convert boost to dB
		return weight * baseLufs + (1.0 - weight) * adjustedLufs;
	}

	std::vector<double> ComputeBaseBlockLoudness(const RingBufferSimple& blockSums, size_t blockSize, double integratedLUFS, double silenceThreshold, std::vector<double>* validLoudness) {
		std::vector<double> baseLoudness(blockSums.size(), -90.0);

		if (validLoudness) {
			validLoudness->clear();
			validLoudness->reserve(blockSums.size());
		}
		if (blockSums.size() == 0 || blockSize == 0) {
			return baseLoudness;
		}

		size_t idx = 0;
		const double invBlockSize = 1.0 / blockSize;

		for (auto it = blockSums.begin(); it != blockSums.end(); ++it, ++idx) {
			double power = *it;
			if (!std::isfinite(power) || power < 0.0) {
				continue;
			}

			const double meanPower = power * invBlockSize;
			if (meanPower < 1e-12) {
				continue;
			}

			double baseLufs = -0.691 + AWHAudio::PowerToDb(meanPower);
			if (std::isfinite(baseLufs) && baseLufs > silenceThreshold) {
				baseLoudness[idx] = baseLufs;
				if (validLoudness) {
					validLoudness->push_back(baseLufs);
				}
			}
		}

		if (validLoudness && validLoudness->empty()) {
			double fallbackLufs = (integratedLUFS != -INFINITY) ? integratedLUFS : -70.0;
			std::fill(baseLoudness.begin(), baseLoudness.end(), fallbackLufs);
		}

		return baseLoudness;
	}

	double ComputeBinauralPerception(bool isRealTime, bool ildOnly, size_t blockSize, double sampleRate,
		const double* leftChannel, const double* rightChannel) {
		constexpr double itdWeight = 0.3;  // ITD weight: 0.3 - Blauert (1997)
		constexpr double ildWeight = 0.4;  // ILD weight: 0.4 - Blauert (1997)
		constexpr double maxItdMs = 0.68;  // Max ITD: 0.68 ms - Blauert (1997)
		constexpr double maxIld = 15.0;    // Max ILD: 15 dB - Blauert (1997)
		constexpr double epsilon = 1e-12;

		// Compute RMS for ILD
		double leftRms = 0.0;
		double rightRms = 0.0;
		for (size_t i = 0; i < blockSize; ++i) {
			leftRms += leftChannel[i] * leftChannel[i];
			rightRms += rightChannel[i] * rightChannel[i];
		}
		leftRms = std::sqrt(leftRms / (blockSize + epsilon));
		rightRms = std::sqrt(rightRms / (blockSize + epsilon));

		// Calculate ILD
		double ild = 0.0;
		if (leftRms > epsilon && rightRms > epsilon) {
			ild = AWHAudio::LinearToDb(leftRms / (rightRms + epsilon));
			ild = std::abs(ild);
			ild = std::min(ild, maxIld);
		}

		// Calculate ITD
		double itd = 0.0;
		if (!ildOnly && !isRealTime) {  // Offline mode for full-track analysis
			const double blockDurationMs = (blockSize / sampleRate) * 1000.0;  // Block duration in ms
			const double maxShiftMs = std::min(maxItdMs, blockDurationMs / 2.0);  // Limit shift to half block duration
			const auto maxShiftSamples = static_cast<int>(maxShiftMs * sampleRate / 1000.0);
			double maxCorr = 0.0;
			int bestShift = 0;

			for (int shift = -maxShiftSamples; shift <= maxShiftSamples; ++shift) {
				double corr = 0.0;
				for (size_t i = 0; i < blockSize - std::abs(shift); ++i) {
					int shifted = static_cast<int>(i) + shift;
					if (shifted >= 0 && static_cast<size_t>(shifted) < blockSize) {
						corr += leftChannel[i] * rightChannel[shifted];
					}
				}
				corr = std::abs(corr) / (blockSize + epsilon);
				if (corr > maxCorr) {
					maxCorr = corr;
					bestShift = shift;
				}
			}
			itd = (std::abs(bestShift) / sampleRate) * 1000.0;  // Convert to ms
			itd = std::min(itd, maxItdMs) / maxItdMs;  // Normalize to 0–1
		}

		// Combine factors (sample rate independent due to normalization)
		double binauralFactor = ildWeight * (ild / maxIld);
		if (!ildOnly) binauralFactor += itdWeight * itd;

		return std::min(1.0, binauralFactor);
	}

	double ComputeCognitiveLoudness(bool isRealTime,
		const RingBufferSimple& loudnessHistory100ms, const RingBufferSimple& loudnessHistory1s, const RingBufferSimple& loudnessHistory10s,
		double currentLufs, double variance, const std::vector<double>& transientBoosts, const std::vector<double>& bandPower, double blockDurationMs, double sampleRate,
		double spectralCentroid, double spectralFlatness, double spectralFlux, double genreFactor) {

		constexpr double attentionWeight = 0.4;
		constexpr double memoryWeight = 0.3;
		constexpr double maxContrast = 20.0;

		// Dynamic weights for music
		double spectralWeight = 0.45 * std::clamp(spectralCentroid / 12000.0, 0.0, 1.0) * genreFactor;
		double rhythmWeight = 0.35 * std::clamp(spectralFlux / 0.15, 0.0, 1.0) * (1.0 - genreFactor);
		double transientWeight = 0.3 * ComputeTransientDensity(transientBoosts, blockDurationMs) / 10.0 * genreFactor;
		double totalWeight = spectralWeight + rhythmWeight + transientWeight + 0.1;
		spectralWeight /= totalWeight;
		rhythmWeight /= totalWeight;
		transientWeight /= totalWeight;

		double stabilityWeight = 0.1 / totalWeight;
		double varNorm = std::clamp(variance / 35.0, 0.2, 1.5);
		double decay100ms = 0.85 - 0.05 * varNorm;
		double decay1s = 0.9 - 0.03 * (1.0 - spectralFlatness);
		double decay10s = 0.95 + 0.02 * varNorm;

		double avg100ms = AWHAudioDSP::CalculateExponentialAverage(loudnessHistory100ms, decay100ms);
		double avg1s = AWHAudioDSP::CalculateExponentialAverage(loudnessHistory1s, decay1s);
		double avg10s = AWHAudioDSP::CalculateExponentialAverage(loudnessHistory10s, decay10s);

		double recentLufs = currentLufs;
		if (avg100ms != -INFINITY && avg1s != -INFINITY && avg10s != -INFINITY) {
			double wShort = 0.5 - 0.15 * varNorm;
			recentLufs = wShort * avg100ms + (0.6 - wShort) * avg1s + (0.3 - wShort / 2) * avg10s;
		}

		double delta = currentLufs - recentLufs;
		double attentionFactor = std::min(std::abs(delta) / maxContrast, 1.0) * attentionWeight;
		double contrast = std::min(std::abs(delta) * (1.0 + 0.15 * std::log1p(variance)) / maxContrast, 1.0);

		double transientScore = std::clamp(ComputeTransientDensity(transientBoosts, blockDurationMs) / 7.0, 0.0, 1.0);
		auto rhythmWindowBlocks = static_cast<size_t>(2000.0 / blockDurationMs);
		double rhythmScore = std::clamp(ComputeOnsetRate(transientBoosts, blockDurationMs, rhythmWindowBlocks) / 4.0, 0.0, 1.0);

		double spectralScore = 0.1;
		if (!bandPower.empty() && bandPower[0] >= 1e-12) {
			double centroidNorm = std::clamp(spectralCentroid / 12000.0, 0.0, 1.0);
			double flatnessNorm = 1.0 - std::clamp(spectralFlatness, 0.0, 1.0);
			double fluxNorm = std::clamp(spectralFlux / 0.15, 0.0, 1.0);
			spectralScore = 0.5 * centroidNorm + 0.3 * flatnessNorm + 0.2 * fluxNorm;
			spectralScore = std::clamp(spectralScore, 0.0, 1.0);
		}

		double stabilityScore = 1.0 - std::clamp(std::log1p(variance) / std::log1p(35.0), 0.0, 1.0);
		double entropy = AWHMath::CalculateEntropy({ transientScore, rhythmScore, spectralScore, stabilityScore });

		double genreWeight = spectralWeight * spectralScore + rhythmWeight * rhythmScore + transientWeight * transientScore + stabilityWeight * stabilityScore;
		genreWeight *= (0.9 + 0.1 * entropy);
		genreWeight = std::clamp(genreWeight, 0.0, 1.0);

		double memoryFactor = memoryWeight * contrast * (1.0 - 0.3 * genreWeight);
		return std::clamp(attentionFactor + memoryFactor + 0.5 * genreWeight, 0.0, 1.0);
	}

	void ComputeFastlPrinciples(
		bool isRealTime, std::vector<double>& fastlAdjustments, const std::vector<std::vector<double>>& bandPowers,
		const std::vector<double>& blockLoudness, size_t stepSize, double sampleRate, double variance, double varianceScale,
		const RingBufferSimple* history) {
		const size_t blockCount = blockLoudness.size();
		fastlAdjustments.assign(blockCount, 0.0);

		double f_mod_shared = 4.0; // Default modulation frequency (Hz) for real-time fallback
		const double frameTimeMs = (static_cast<double>(stepSize) / sampleRate) * 1000.0;

		// Real-time optimization: Compute f_mod once per chunk using history
		if (isRealTime && history && history->size() >= AWHAudioFFT::BARK_BAND_NUMBER) {
			size_t maxHistoryBlocks = 10;
			size_t historyBlocks = std::min(maxHistoryBlocks, history->size() / AWHAudioFFT::BARK_BAND_NUMBER);
			std::vector<std::vector<double>> historyVec;
			historyVec.reserve(historyBlocks);
			for (size_t i = 0; i < historyBlocks; ++i) {
				std::vector<double> blockPowers(AWHAudioFFT::BARK_BAND_NUMBER);
				for (size_t j = 0; j < AWHAudioFFT::BARK_BAND_NUMBER; ++j) {
					blockPowers[j] = (*history)[i * AWHAudioFFT::BARK_BAND_NUMBER + j];
				}
				historyVec.push_back(blockPowers);
			}
			f_mod_shared = AWHAudioFFT::EstimateModulationFrequency(historyVec, historyBlocks - 1, frameTimeMs, 0, 6);
		}

		for (size_t i = 0; i < blockCount; ++i) {
			if (blockLoudness[i] <= -100.0 || i >= bandPowers.size() ||
				bandPowers[i].size() != AWHAudioFFT::BARK_BAND_NUMBER) {
				continue;
			}

			// Compute spectral characteristics for genre adaptation
			double highFreqPower = 0.0;
			double lowFreqPower = 0.0;
			double totalPower = 0.0;
			double maxPower = 0.0;
			for (size_t b = 0; b < AWHAudioFFT::BARK_BAND_NUMBER; ++b) {
				const double pwr = bandPowers[i][b];
				if (pwr > AWHAudioFFT::EPSILON) {
					totalPower += pwr;
					maxPower = std::max(maxPower, pwr);
					if (b >= 18) highFreqPower += pwr; // >4 kHz
					if (b < 3) lowFreqPower += pwr;   // <100 Hz
				}
			}
			double hfRatio = (totalPower > AWHAudioFFT::EPSILON) ? highFreqPower / totalPower : 0.0;
			double lfRatio = (totalPower > AWHAudioFFT::EPSILON) ? lowFreqPower / totalPower : 0.0;
			double genreFactor = (variance > 100.0) ? 0.7 : (hfRatio > 0.4) ? 0.3 : (lfRatio > 0.5) ? 0.5 : 0.4;

			// Compute spectral flatness for adaptive weighting
			double flatness = AWHAudioFFT::ComputeSpectralFlatness(bandPowers[i], AWHAudioFFT::BARK_BAND_NUMBER);

			// Modulation analysis
			double f_mod = isRealTime ? f_mod_shared : AWHAudioFFT::EstimateModulationFrequency(bandPowers, i, frameTimeMs, 0, 6);
			double mod_depth = 0.94 * std::min(1.0 + 0.05 * hfRatio, 1.05); // Capped at +5%

			// Psychoacoustic metrics with real-time optimization
			double fluctuation = ComputeFluctuationStrength(isRealTime, bandPowers[i], f_mod, mod_depth);
			std::vector<double> f_mod_vec(AWHAudioFFT::BARK_BAND_NUMBER, f_mod);
			double roughness = ComputeRoughness(isRealTime, bandPowers[i], f_mod_vec, mod_depth);
			double sharpness = ComputeSharpness(isRealTime, bandPowers[i]);
			double tonality = (totalPower > AWHAudioFFT::EPSILON) ? maxPower / totalPower : 0.1;

			// Adaptive weights based on spectral flatness
			double roughnessWeight = 0.4 + 0.2 * (1.0 - flatness); // 0.4–0.6, less for flatter spectra
			double tonalityWeight = 1.0 + 0.5 * flatness;         // 1.0–1.5, more for flatter spectra

			// Sensory pleasantness model (Zwicker & Fastl, Eq. 9.2)
			double R_term = std::exp(roughnessWeight * roughness);
			double S_term = std::exp(1.08 * (sharpness - 1.0));
			double T_term = tonalityWeight - std::exp(-2.43 * tonality);
			double N_term = std::exp(0.023 * std::pow(blockLoudness[i] - 14.0, 2));

			// Combine fluctuation with roughness
			double f_mod_weight = 1.0 - std::abs(f_mod - 4.0) / 4.0;
			R_term *= (1.0 + f_mod_weight * fluctuation * 0.35);

			// Final adjustment in dB
			double P_ratio = R_term * S_term * N_term / T_term;
			fastlAdjustments[i] = -10.0 * std::log10(P_ratio + AWHAudioFFT::EPSILON) * (1.0 - 0.3 * varianceScale * genreFactor);
		}
	}

	void ComputePerceptualLoudnessCorrection(
		bool isRealTime, const std::vector<double>& fastlAdjustments, const std::vector<double>& blockLoudness, const std::vector<double>& highFreqPowers, double integratedLUFS, double variance,
		std::vector<double>& correctedLoudness, std::vector<double>& validLoudness, const RingBufferSimple* offlineBlockSums, size_t offlineStepSize) {
		const size_t blockCount = blockLoudness.size();

		if (blockCount == 0 || blockCount != highFreqPowers.size() ||
			(!isRealTime && (!offlineBlockSums || blockCount != offlineBlockSums->size() || offlineStepSize == 0))) {
			correctedLoudness.assign(blockCount, -INFINITY);
			validLoudness.clear();
			return;
		}

		correctedLoudness.resize(blockCount, -INFINITY);
		if (!isRealTime) {
			validLoudness.clear();
			validLoudness.reserve(blockCount);
		}

		if (isRealTime) { // Real-time mode: Compute meanPower from blockLoudness, no offlineBlockSums needed
			for (size_t idx = 0; idx < blockCount; ++idx) {
				if (blockLoudness[idx] == -INFINITY) {
					continue;
				}

				double meanPower = std::pow(10.0, (blockLoudness[idx] + 0.691) / 10.0);
				if (meanPower < 1e-12) {
					correctedLoudness[idx] = -70.0;
					continue;
				}

				correctedLoudness[idx] = ApplyPerceptualLoudnessCorrection(
					isRealTime, {}, blockLoudness[idx], integratedLUFS, highFreqPowers[idx], variance, meanPower
				);
			}
		}
		else { // Offline mode: Use offlineBlockSums iterator
			size_t idx = 0;
			for (auto it = offlineBlockSums->begin(); it != offlineBlockSums->end(); ++it, ++idx) {
				if (blockLoudness[idx] == -INFINITY) {
					continue;
				}

				double meanPower = *it / offlineStepSize;
				if (meanPower < 1e-12) {
					correctedLoudness[idx] = -70.0;
					continue;
				}

				correctedLoudness[idx] = ApplyPerceptualLoudnessCorrection(
					isRealTime, fastlAdjustments[idx], blockLoudness[idx], integratedLUFS, highFreqPowers[idx], variance, meanPower
				);

				validLoudness.push_back(correctedLoudness[idx]);
			}
		}
	}

	double ComputeDynamicSpread(const std::vector<double>& loudness, double threshold, double alpha, double scaleFactor) {
		std::vector<double> filteredLoudness;
		filteredLoudness.reserve(loudness.size());

		for (double lufs : loudness) {
			if (lufs > threshold) filteredLoudness.push_back(lufs);
		}
		if (filteredLoudness.empty()) return 0.0;

		std::sort(filteredLoudness.begin(), filteredLoudness.end());
		size_t q1 = filteredLoudness.size() / 4;
		size_t q3 = 3 * filteredLoudness.size() / 4;
		double iqr = filteredLoudness[q3] - filteredLoudness[q1];
		double maxMin = filteredLoudness.back() - filteredLoudness.front();
		double avg = std::accumulate(filteredLoudness.begin(), filteredLoudness.end(), 0.0) / filteredLoudness.size();
		double scale = scaleFactor * (1.0 + 0.5 / (1.0 + std::exp(-(avg + 30.0) / 15.0)));

		return (alpha * iqr + (1.0 - alpha) * maxMin) * scale;
	}

	double ComputeFluctuationStrength(bool isRealTime, const std::vector<double>& bandPowers, double f_mod, double mod_depth) {
		if (bandPowers.size() < AWHAudioFFT::BARK_BAND_NUMBER || f_mod <= 0.0) return 0.0;

		// Total loudness in sones - Zwicker & Fastl - pp. 168-182
		double totalLoudness = 0.0;
		for (size_t b = 0; b < AWHAudioFFT::BARK_BAND_NUMBER; ++b) {
			if (bandPowers[b] > AWHAudioFFT::EPSILON) {
				double ratio = bandPowers[b] / AWHAudioFFT::BARK_QUIET_THRESHOLD_INTENSITIES[b];
				double N_prime = AWHAudioFFT::SPECIFIC_LOUDNESS_CONST * (std::pow(ratio, 0.23) - 1.0);
				if (N_prime > 0.0) totalLoudness += N_prime;
			}
		}
		if (totalLoudness < AWHAudioFFT::EPSILON) return 0.0;

		// Loudness level in phons - Zwicker & Fastl - Eq 8.17, p. 182
		double L_N = 40.0 + 10.0 * std::log2(totalLoudness);

		// Modulation depth to ΔL - Zwicker & Fastl -Fig 10.3, p. 249
		double m_eff = std::clamp(mod_depth, 0.0, 0.94);
		double depth_factor = (m_eff < 0.2) ? 0.0 : (1.25 * m_eff - 0.25); // Refined linear fit

		// Formula - Zwicker & Fastl - Eq 10.3, p. 256
		double f_mod_term = (f_mod / 4.0) + (4.0 / f_mod);
		constexpr double VACIL_SCALE = 0.008; // Zwicker & Fastl - Eq 10.3, p. 256

		return std::max(0.0, VACIL_SCALE * depth_factor * L_N / f_mod_term);
	}

	double ComputeOnsetRate(const std::vector<double>& transientBoosts, double blockDurationMs, size_t windowBlocks) {
		if (transientBoosts.size() < windowBlocks) return 0.0;

		constexpr double DB_THRESHOLD = 0.5; // JND AM detection - Zwicker & Fastl - p. 251
		double onsetCount = 0.0;
		double windowDuration = windowBlocks * blockDurationMs / 1000.0;

		for (size_t i = 1; i < transientBoosts.size(); ++i) {
			double ratio = (transientBoosts[i] + AWHAudioFFT::EPSILON) / (transientBoosts[i - 1] + AWHAudioFFT::EPSILON);
			if (AWHAudio::LinearToDb(ratio) > DB_THRESHOLD) {
				onsetCount += 1.0;
			}
		}

		return onsetCount / std::max(0.001, windowDuration);
	}

	double ComputeRoughness(bool isRealTime, const std::vector<double>& bandPowers,
		const std::vector<double>& f_mod_per_band, double mod_depth) {
		if (bandPowers.size() < AWHAudioFFT::BARK_BAND_NUMBER ||
			f_mod_per_band.size() < AWHAudioFFT::BARK_BAND_NUMBER) {
			return 0.0;
		}

		// Mean modulation frequency (15-300 Hz) - Zwicker & Fastl - pp. 258-260
		double f_mod_sum = 0.0;
		int valid_bands = 0;

		for (size_t b = 0; b < AWHAudioFFT::BARK_BAND_NUMBER; ++b) {
			if (bandPowers[b] > AWHAudioFFT::EPSILON && f_mod_per_band[b] >= 15.0 && f_mod_per_band[b] <= 300.0) {
				f_mod_sum += f_mod_per_band[b];
				valid_bands++;
			}
		}
		if (valid_bands == 0) return 0.0;
		double f_mod_avg = f_mod_sum / valid_bands;

		// Bandpass check - Zwicker & Fastl - p. 260
		if (f_mod_avg < 15.0 || f_mod_avg > 300.0) return 0.0;

		// Modulation depth to dB difference - Zwicker & Fastl - Eq 11.3, p. 263
		double m_eff = std::clamp(mod_depth, 0.0, 0.94);
		double dB_diff = AWHAudio::LinearToDb((1.0 + m_eff) / (1.0 - m_eff + AWHAudioFFT::EPSILON));

		// Active bands - Zwicker & Fastl - pp. 261-262
		int active_bands = 0;
		for (double power : bandPowers) {
			if (power > AWHAudioFFT::EPSILON) active_bands++;
		}

		// Formula - Zwicker & Fastl - Eq 11.3, p. 263
		constexpr double ASPER_SCALE = 0.3; // Zwicker & Fastl - Eq 11.3, p. 263
		return std::max(0.0, ASPER_SCALE * (f_mod_avg / 1000.0) * dB_diff * active_bands);
	}

	double ComputeSharpness(bool isRealTime, const std::vector<double>& bandPowers) {
		if (bandPowers.size() < AWHAudioFFT::BARK_BAND_NUMBER) return 1.0;

		double totalWeighted = 0.0;
		double totalLoudness = 0.0;

		for (size_t b = 0; b < AWHAudioFFT::BARK_BAND_NUMBER; ++b) {
			if (bandPowers[b] > AWHAudioFFT::EPSILON) {
				double ratio = bandPowers[b] / AWHAudioFFT::BARK_QUIET_THRESHOLD_INTENSITIES[b];
				double N_prime = AWHAudioFFT::SPECIFIC_LOUDNESS_CONST * (std::pow(ratio, 0.23) - 1.0);
				if (N_prime < 0.0) continue;

				// Weighting g(z) to 4.0 at 25 Bark - Zwicker & Fastl - Fig 9.2, p. 242
				auto z = static_cast<double>(b);
				double g_z = (z <= 16.0) ? 1.0 : 1.0 + 0.333 * (z - 16.0); // Adjusted for 25 bands

				totalWeighted += N_prime * g_z * z;
				totalLoudness += N_prime;
			}
		}

		constexpr double ACUM_SCALE = 0.11; // Zwicker & Fastl - Eq 9.1, p. 242
		return (totalLoudness < AWHAudioFFT::EPSILON) ? 1.0 : std::max(0.0, ACUM_SCALE * totalWeighted / totalLoudness);
	}

	double ComputePhrasingScore(const std::vector<double>& transientBoosts, double blockDurationMs, size_t currentBlock) {
		if (blockDurationMs <= 0.0 || transientBoosts.empty()) return 0.0;

		const auto windowBlocks = static_cast<size_t>(4000.0 / blockDurationMs); // 4s window
		if (transientBoosts.size() < windowBlocks || currentBlock < windowBlocks / 2) return 0.0;

		// Define analysis window
		const size_t start = currentBlock > windowBlocks / 2 ? currentBlock - windowBlocks / 2 : 0;
		const size_t end = std::min(currentBlock + windowBlocks / 2, transientBoosts.size());
		const size_t windowSize = end - start;

		// Compute mean and variance (R0) in one pass
		double mean = 0.0;
		double R0 = 0.0;
		for (size_t i = start; i < end; ++i) {
			mean += transientBoosts[i];
		}
		mean /= windowSize;
		for (size_t i = start; i < end; ++i) {
			const double c = transientBoosts[i] - mean;
			R0 += c * c;
		}
		R0 /= windowSize;
		if (R0 < 1e-12) return 0.0; // Flat signal

		// Lag range for 20-300 BPM (0.2s to 3s)
		const size_t minLag = std::max(static_cast<size_t>(200.0 / blockDurationMs), size_t(1));
		const size_t maxLag = std::min(static_cast<size_t>(3000.0 / blockDurationMs), windowSize - 1);
		if (minLag > maxLag) return 0.0;

		// Thread-local storage for centered values
		static thread_local std::vector<double> centeredStorage(4096);
		if (centeredStorage.size() < windowSize) {
			centeredStorage.resize(windowSize);
		}

		// Store centered values
		for (size_t i = 0; i < windowSize; ++i) {
			centeredStorage[i] = transientBoosts[start + i] - mean;
		}

		// Compute max normalized autocorrelation
		double maxAutocorr = 0.0;
		const double invR0 = 1.0 / R0;
		for (size_t lag = minLag; lag <= maxLag; ++lag) {
			double sum = 0.0;
			const size_t count = windowSize - lag;
			for (size_t i = lag; i < windowSize; ++i) {
				sum += centeredStorage[i] * centeredStorage[i - lag];
			}
			const double autocorrVal = (sum / count) * invR0;
			maxAutocorr = std::max(maxAutocorr, autocorrVal);
		}

		return std::clamp(maxAutocorr, 0.0, 1.0);
	}

	double ComputeSpatialScore(const std::vector<double>& leftChannel,
		const std::vector<double>& rightChannel, size_t blockSize, double sampleRate) {
		// Enhanced binaural perception with IACC, sample rate independent
		double baseBinaural = ComputeBinauralPerception(false, false,
			blockSize, sampleRate, leftChannel.data(), rightChannel.data()
		);

		// Interaural Cross-Correlation (IACC) normalized by block energy
		double leftEnergy = 0.0;
		double rightEnergy = 0.0;
		for (size_t i = 0; i < blockSize; ++i) {
			leftEnergy += leftChannel[i] * leftChannel[i];
			rightEnergy += rightChannel[i] * rightChannel[i];
		}
		double normFactor = std::sqrt(leftEnergy * rightEnergy) + 1e-12;

		const double maxShiftMs = 0.68; // 0.68ms max ITD
		const auto maxShift = static_cast<int>(maxShiftMs * sampleRate / 1000.0);
		double maxCorr = 0.0;
		for (int shift = -maxShift; shift <= maxShift; ++shift) {
			double corr = 0.0;
			for (size_t i = 0; i < blockSize - std::abs(shift); ++i) {
				int shifted = static_cast<int>(i) + shift;
				if (shifted >= 0 && static_cast<size_t>(shifted) < blockSize) {
					corr += leftChannel[i] * rightChannel[shifted];
				}
			}
			maxCorr = std::max(maxCorr, std::abs(corr));
		}
		double iacc = maxCorr / normFactor; // Normalized IACC

		// Combine binaural factor and IACC
		double spatialScore = 0.6 * baseBinaural + 0.4 * iacc;
		return std::clamp(spatialScore, 0.0, 1.0);
	}

	std::vector<double> ComputeTemporalWeights(const std::vector<double>& loudness, double blockDurationMs, double preMaskingMs, double postMaskingMs, double variance) {
		if (blockDurationMs <= 0.0 || preMaskingMs <= 0.0 || postMaskingMs <= 0.0) {
			return std::vector<double>(loudness.size(), 1.0);
		}

		// Adjust masking durations for high-variance tracks
		double adjPreMaskingMs = variance > 100.0 ? 50.0 : preMaskingMs;
		double adjPostMaskingMs = variance > 100.0 ? 250.0 : postMaskingMs;

		std::vector<double> weights(loudness.size(), 1.0);
		const double peakThreshold = AWHMath::CalculatePercentile(loudness, 0.9);
		if (peakThreshold == -INFINITY) {
			return weights;
		}

		// Precompute inverses for exponential decay
		const double invPreMaskingMs = 1.0 / adjPreMaskingMs;
		const double invPostMaskingMs = 1.0 / adjPostMaskingMs;

		// Precompute maximum index offsets for masking
		const auto preMaskingSteps = static_cast<size_t>(std::ceil(adjPreMaskingMs / blockDurationMs));
		const auto postMaskingSteps = static_cast<size_t>(std::ceil(adjPostMaskingMs / blockDurationMs));

		for (size_t i = 0; i < loudness.size(); ++i) {
			if (loudness[i] > peakThreshold) {
				// Pre-masking (backward)
				size_t startJ = (i > preMaskingSteps) ? i - preMaskingSteps : 0;
				for (size_t j = startJ; j < i; ++j) {
					double timeDiffMs = (i - j) * blockDurationMs;
					double backwardMasking = std::exp(-timeDiffMs * invPreMaskingMs);
					weights[j] = std::max(0.8, weights[j] * (0.9 + 0.1 * backwardMasking));
				}

				// Post-masking (forward)
				size_t endJ = std::min(loudness.size(), i + postMaskingSteps + 1);
				for (size_t j = i + 1; j < endJ; ++j) {
					double timeDiffMs = (j - i) * blockDurationMs;
					double forwardMasking = std::exp(-timeDiffMs * invPostMaskingMs);
					weights[j] = std::max(0.6, weights[j] * (0.7 + 0.3 * forwardMasking));
				}
			}
		}

		return weights;
	}

	double ComputeTransientDensity(const std::vector<double>& transientBoosts, double blockDurationMs) {
		double epsilon = 1e-12;
		double transientCount = 0.0;

		for (double boost : transientBoosts) {
			if (boost > 1.05) transientCount += 1.0;
		}

		return (transientCount * 1000.0) / (transientBoosts.size() * blockDurationMs + epsilon);
	}

	std::vector<double> DetectTransients(
		const std::vector<double>& inputLoudness, double blockDurationMs,
		const std::vector<double>& harmonicComplexityFactor,
		const std::vector<double>& maskingFactor,
		const std::vector<double>& spectralFlux,
		const std::vector<double>& spectralCentroid,
		const std::vector<double>& spectralFlatness,
		double genreFactor, double varianceScale) {

		const size_t blockCount = inputLoudness.size();
		std::vector<double> transientBoosts(blockCount, 1.0);

		if (blockCount < 2 || blockDurationMs <= 0.0 ||
			blockCount != spectralFlux.size() ||
			blockCount != spectralCentroid.size() ||
			blockCount != spectralFlatness.size() ||
			blockCount != harmonicComplexityFactor.size() ||
			blockCount != maskingFactor.size()) {
			return transientBoosts;
		}

		// Constants
		const double epsilon = 1e-12;
		const double maxBoost = 2.5; // Reasonable cap for all genres

		// Adaptive parameters (tuned for music)
		const double tau = 100.0 + 50.0 * (1.0 - genreFactor); // 100–150 ms
		const double alpha = 1.0 - std::exp(-blockDurationMs / tau);
		const double alphaDensity = 1.0 - std::exp(-blockDurationMs / 1500.0); // 1.5s for music
		const double k = 1.8 - 0.5 * varianceScale * (1.0 - genreFactor); // 1.3–1.8
		const double beta = 0.6 + 0.4 * genreFactor; // 0.6–1.0
		const double gamma = 1.2 + 0.6 * genreFactor; // 1.2–1.8
		const double maskingTime = 40.0 + 30.0 * (1.0 - genreFactor); // 40–70 ms
		const double releaseDb = 2.5 + 2.0 * genreFactor; // 2.5–4.5 dB
		const auto maskingBlocks = static_cast<size_t>(std::round(maskingTime / blockDurationMs));

		// State variables
		double emaMean = 0.0;
		double emaVar = 0.0;
		double transientDensityEma = 0.0;
		size_t lastTransientBlock = 0;
		double lastTransientLoudness = -INFINITY;

		for (size_t i = 1; i < blockCount; ++i) {
			if (!std::isfinite(inputLoudness[i]) || !std::isfinite(inputLoudness[i - 1])) {
				continue;
			}

			// Compute transient score with precomputed factors
			double loudnessDelta = inputLoudness[i] - inputLoudness[i - 1];
			double normDelta = loudnessDelta / (8.0 + epsilon);
			double normFlux = spectralFlux[i] / (0.15 + epsilon);
			double normCentroid = spectralCentroid[i] / (12000.0 + epsilon);
			double normFlatness = 1.0 - spectralFlatness[i];
			double normComplexity = harmonicComplexityFactor[i];
			double normMasking = 1.0 - maskingFactor[i];

			// Weighted transient score
			double transientScore =
				0.4 * normFlux + 0.3 * normDelta + 0.15 * normCentroid +
				0.1 * normFlatness - 0.1 * normComplexity + 0.05 * normMasking;

			// Initialize EMA on first block
			if (i == 1) {
				emaMean = transientScore;
				emaVar = 0.0;
				continue;
			}

			// Adaptive threshold
			double threshold = emaMean + k * std::sqrt(emaVar + epsilon);

			// Temporal masking
			bool masked = (i - lastTransientBlock < maskingBlocks) && (inputLoudness[i] < lastTransientLoudness + releaseDb);

			// Detect and boost
			if (!masked && transientScore > threshold) {
				double excess = (transientScore - threshold) / (threshold + epsilon);
				double boost = 1.0 + beta * excess;
				boost /= (1.0 + gamma * transientDensityEma); // Density adjustment
				transientBoosts[i] = std::min(maxBoost, std::max(1.0, boost));

				lastTransientBlock = i;
				lastTransientLoudness = inputLoudness[i];
			}

			// Update EMAs
			double prevEmaMean = emaMean;
			emaMean = alpha * transientScore + (1.0 - alpha) * emaMean;
			emaVar = alpha * std::pow(transientScore - prevEmaMean, 2) + (1.0 - alpha) * emaVar;
			transientDensityEma = alphaDensity * (transientBoosts[i] > 1.0 ? 1.0 : 0.0) + (1.0 - alphaDensity) * transientDensityEma;
		}

		return transientBoosts;
	}
}
#pragma endregion


///////////////////////////
// * AUDIO FFT HELPERS * //
///////////////////////////
#pragma region Audio FFT Helpers
namespace AWHAudioFFT {
	const std::array<double, BARK_BAND_NUMBER + 1> BARK_BAND_FREQUENCY_EDGES = [] {
		std::array<double, BARK_BAND_NUMBER + 1> edges{};

		for (int i = 0; i <= BARK_BAND_NUMBER; ++i) {
			edges[i] = MapBarkToFrequency(static_cast<double>(i));
		}

		return edges;
	}();

	void PrecomputeBitReversal(size_t N) {
		auto& indices = bitReversalCache[N];
		indices.resize(N);

		for (size_t i = 0; i < N; ++i) {
			size_t j = 0;
			size_t m = i;
			for (size_t len = N >> 1; len >= 1; len >>= 1) {
				j = (j << 1) | (m & 1);
				m >>= 1;
			}
			indices[i] = j;
		}
	}

	void PrecomputeTwiddlesGeneral(size_t N) {
		static const double PI = 3.14159265358979323846;
		auto& twiddles = twiddleCache[N];
		size_t totalTwiddles = 0;

		// Factorize N
		std::vector<size_t> factors;
		size_t tempN = N;
		for (size_t r = 2; tempN > 1; ) {
			if (tempN % r == 0) {
				factors.push_back(r);
				tempN /= r;
			}
			else {
				r++;
				if (r > tempN) break;
			}
		}
		if (factors.empty()) factors.push_back(N);

		// Calculate total twiddles
		size_t n1 = 1;
		for (size_t r : factors) {
			totalTwiddles += (r / 2) * n1;
			n1 *= r;
		}

		twiddles.resize(totalTwiddles);
		size_t idx = 0;
		n1 = 1;

		for (size_t r : factors) {
			size_t n2 = n1 * r;
			size_t m2 = r / 2;
			for (size_t j = 0; j < m2; ++j) {
				for (size_t i = 0; i < n1; ++i) {
					twiddles[idx + j] = std::polar(1.0, -2.0 * PI *
					(static_cast<double>(i) + static_cast<double>(n1) * static_cast<double>(j)) / n2);
				}
			}
			idx += m2;
			n1 = n2;
		}
	}

	void PrecomputeTwiddlesPower2(size_t N) {
		auto& twiddles = twiddleCache[N];
		static const double PI = 3.14159265358979323846;
		size_t totalTwiddles = 0;

		for (size_t m = 4; m <= N; m <<= 2) {
			totalTwiddles += 3 * (m / 4); // 3 twiddles per radix-4 butterfly
		}
		if (N & (N - 1)) { // Mixed-radix for non-power-of-4
			for (size_t m = 2; m <= N; m <<= 1) {
				totalTwiddles += m / 2;
			}
		}

		twiddles.resize(totalTwiddles);
		size_t idx = 0;

		for (size_t m = 4; m <= N; m <<= 2) {
			size_t m4 = m / 4;
			for (size_t j = 0; j < m4; ++j) {
				twiddles[idx + j] = std::polar(1.0, -2.0 * PI * j / m);           // W_m^j
				twiddles[idx + j + m4] = std::polar(1.0, -2.0 * PI * 2 * j / m); // W_m^{2j}
				twiddles[idx + j + 2 * m4] = std::polar(1.0, -2.0 * PI * 3 * j / m); // W_m^{3j}
			}
			idx += 3 * m4;
		}
	}

	size_t CalculateFFTSize(bool usePower2, double sampleRate, double& targetBinWidth, size_t stepSize, size_t maxFftSize) {
		if (sampleRate <= 0 || targetBinWidth <= 0) return 0;

		double idealFftSize = sampleRate / targetBinWidth;
		size_t fftSize;

		if (usePower2) {
			fftSize = static_cast<size_t>(std::ceil(idealFftSize)); // Round up to the next power of two
			fftSize = std::max(fftSize, stepSize); // Ensure at least stepSize
			fftSize = 1ULL << static_cast<size_t>(std::ceil(std::log2(static_cast<double>(fftSize)))); // Next power of two
			fftSize = std::min(fftSize, maxFftSize); // Cap at maxFftSize
			targetBinWidth = sampleRate / fftSize; // Adjust targetBinWidth based on the chosen FFT size
		}
		else {
			fftSize = static_cast<size_t>(std::round(idealFftSize));
			fftSize = std::max(fftSize, stepSize); // Ensure at least stepSize for zero-padding
			fftSize = std::min(fftSize, maxFftSize); // Cap for practicality
		}

		return fftSize;
	}

	void ComputeFFTBluestein(const std::vector<double>& input, std::vector<std::complex<double>>& output, const std::vector<double>& window) {
		static const double PI = 3.14159265358979323846;
		const size_t N = input.size();

		if (N <= 1) {
			output.resize(N);
			if (N == 1) output[0] = std::complex<double>(input[0], 0.0);
			return;
		}

		// Find M = smallest power of two >= 2N - 1
		size_t M = 1;
		while (M < 2 * N - 1) M <<= 1;

		// Compute b[n] = exp(-pi*i*n^2 / N) for n=0 to N-1
		std::vector<std::complex<double>> b(N);
		for (size_t n = 0; n < N; ++n) {
			double arg = -PI * n * n / N;
			b[n] = std::polar(1.0, arg);
		}

		// Compute a[n] = input[n] * window[n] * b[n], zero-padded to M
		std::vector<std::complex<double>> a(M, 0.0);
		bool useWindow = !window.empty() && window.size() == N;
		for (size_t n = 0; n < N; ++n) {
			double win = useWindow ? window[n] : 1.0;
			a[n] = std::complex<double>(input[n] * win, 0.0) * b[n];
		}

		// Compute c[n] = exp(pi*i*n^2 / N) for n=0 to M-1
		std::vector<std::complex<double>> c(M);
		for (size_t n = 0; n < M; ++n) {
			double arg = PI * n * n / N;
			c[n] = std::polar(1.0, arg);
		}

		// Compute FFTs
		std::vector<std::complex<double>> FFT_a(M);
		ComputeComplexFFTPower2(a, FFT_a);
		std::vector<std::complex<double>> FFT_c(M);
		ComputeComplexFFTPower2(c, FFT_c);

		// Pointwise multiplication
		std::vector<std::complex<double>> p(M);
		for (size_t k = 0; k < M; ++k) {
			p[k] = FFT_a[k] * FFT_c[k];
		}

		// Compute IFFT: Since ComputeComplexFFTPower2 is unscaled, IFFT(p) = (1/M) * conj(FFT(conj(p)))
		std::vector<std::complex<double>> conj_p(M);
		for (size_t k = 0; k < M; ++k) {
			conj_p[k] = std::conj(p[k]);
		}

		std::vector<std::complex<double>> temp(M);
		ComputeComplexFFTPower2(conj_p, temp);
		std::vector<std::complex<double>> y(M);
		for (size_t n = 0; n < M; ++n) {
			y[n] = std::conj(temp[n]) / static_cast<double>(M);
		}

		// Compute output
		output.resize(N);
		for (size_t k = 0; k < N; ++k) {
			output[k] = b[k] * y[k];
		}
	}

	// General Cooley-Tukey FFT for arbitrary sizes
	void ComputeFFTGeneral(const std::vector<double>& input, std::vector<std::complex<double>>& output, const std::vector<double>& window) {
		const size_t N = input.size();
		if (N <= 1) return;
		output.resize(N);

		// Input preparation with optional windowing
		bool useWindow = !window.empty() && window.size() == N;
		for (size_t i = 0; i < N; ++i) {
			output[i] = std::complex<double>(input[i] * (useWindow ? window[i] : 1.0), 0.0);
		}

		// Precompute twiddle factors if not cached
		auto& twiddles = twiddleCache[N];
		if (twiddles.empty()) PrecomputeTwiddlesGeneral(N);

		// Factorize N for Cooley-Tukey stages
		std::vector<size_t> factors;
		size_t tempN = N;
		for (size_t r = 2; tempN > 1; ) {
			if (tempN % r == 0) {
				factors.push_back(r);
				tempN /= r;
			}
			else {
				r++;
				if (r > tempN) break; // Handle prime N
			}
		}
		if (factors.empty()) factors.push_back(N); // N is prime

		// If N is prime use faster Bluestein
		if (factors.size() == 1 && factors[0] == N) {
			ComputeFFTBluestein(input, output, window);
			return;
		}

		// In-place Cooley-Tukey stages
		size_t n1 = 1;
		size_t twiddleIdx = 0;
		for (size_t r : factors) {
			size_t n2 = n1 * r;
			size_t m2 = r / 2;
			for (size_t k = 0; k < N; k += n2) {
				for (size_t j = 0; j < m2; ++j) {
					std::complex<double> w = twiddles[twiddleIdx + j];
					for (size_t i = 0; i < n1; ++i) {
						size_t idx1 = k + i + n1 * j;
						size_t idx2 = k + i + n1 * (j + m2);
						std::complex<double> u = output[idx1];
						std::complex<double> t = w * output[idx2];
						output[idx1] = u + t;
						output[idx2] = u - t;
					}
				}
			}
			n1 = n2;
			twiddleIdx += m2;
		}
	}

	void ComputeFFTPower2(const std::vector<double>& input, std::vector<std::complex<double>>& output, const std::vector<double>& window) {
		const size_t N = input.size();
		if (N <= 1 || (N & (N - 1)) != 0) return;
		output.resize(N);

		// Input preparation with optional windowing
		bool useWindow = !window.empty() && window.size() == N;
		for (size_t i = 0; i < N; ++i) {
			output[i] = std::complex<double>(input[i] * (useWindow ? window[i] : 1.0), 0.0);
		}

		// Bit-reversal permutation
		auto& indices = bitReversalCache[N];
		if (indices.empty()) PrecomputeBitReversal(N);
		for (size_t i = 0; i < N; ++i) {
			if (indices[i] > i) std::swap(output[i], output[indices[i]]);
		}

		// Radix-4 FFT
		auto& twiddles = twiddleCache[N];
		if (twiddles.empty()) PrecomputeTwiddlesPower2(N);
		size_t twiddleIdx = 0;
		for (size_t m = 4; m <= N; m <<= 2) {
			size_t m4 = m >> 2;
			size_t m2 = m >> 1;
			for (size_t k = 0; k < N; k += m) {
				for (size_t j = 0; j < m4; ++j) {
					// Radix-4 butterfly
					std::complex<double> w1 = (m == 4 && j == 0) ? 1.0 : twiddles[twiddleIdx + j];
					std::complex<double> w2 = (m == 4 && j == 0) ? 1.0 : twiddles[twiddleIdx + j + m4];
					std::complex<double> w3 = (m == 4 && j == 0) ? 1.0 : twiddles[twiddleIdx + j + 2 * m4];
					std::complex<double> t0 = output[k + j];
					std::complex<double> t1 = output[k + j + m4] * w1;
					std::complex<double> t2 = output[k + j + m2] * w2;
					std::complex<double> t3 = output[k + j + 3 * m4] * w3;
					std::complex<double> u0 = t0 + t2;
					std::complex<double> u1 = t1 + t3;
					std::complex<double> u2 = t0 - t2;
					std::complex<double> u3 = t1 - t3;
					output[k + j] = u0 + u1;
					output[k + j + m4] = u0 - u1;
					output[k + j + m2] = u2 + std::complex<double>(0, 1) * u3;
					output[k + j + 3 * m4] = u2 - std::complex<double>(0, 1) * u3;
				}
			}
			twiddleIdx += 3 * m4;
		}

		// Radix-2 for remaining stages if N not a power of 4 (rare for audio)
		if (twiddleIdx < twiddles.size()) {
			for (size_t m = 2; m <= N; m <<= 1) {
				size_t m2 = m / 2;
				for (size_t k = 0; k < N; k += m) {
					for (size_t j = 0; j < m2; ++j) {
						std::complex<double> w = twiddles[twiddleIdx + j];
						std::complex<double> t = w * output[k + j + m2];
						std::complex<double> u = output[k + j];
						output[k + j] = u + t;
						output[k + j + m2] = u - t;
					}
				}
				twiddleIdx += m2;
			}
		}
	}

	// Computes the FFT of a complex-valued input for power-of-two sizes, producing unscaled FFT coefficients
	void ComputeComplexFFTPower2(const std::vector<std::complex<double>>& input, std::vector<std::complex<double>>& output) {
		const size_t N = input.size();
		if (N <= 1 || (N & (N - 1)) != 0) return;
		output = input; // Copy input to output for in-place processing

		// Bit-reversal permutation
		auto& indices = bitReversalCache[N];
		if (indices.empty()) PrecomputeBitReversal(N);
		for (size_t i = 0; i < N; ++i) {
			if (indices[i] > i) std::swap(output[i], output[indices[i]]);
		}

		// Radix-4 FFT for primary stages
		auto& twiddles = twiddleCache[N];
		if (twiddles.empty()) PrecomputeTwiddlesPower2(N);
		size_t twiddleIdx = 0;
		for (size_t m = 4; m <= N; m <<= 2) {
			size_t m4 = m >> 2;
			size_t m2 = m >> 1;
			for (size_t k = 0; k < N; k += m) {
				for (size_t j = 0; j < m4; ++j) {
					std::complex<double> w1 = twiddles[twiddleIdx + j];
					std::complex<double> w2 = twiddles[twiddleIdx + j + m4];
					std::complex<double> w3 = twiddles[twiddleIdx + j + 2 * m4];
					std::complex<double> t0 = output[k + j];
					std::complex<double> t1 = output[k + j + m4] * w1;
					std::complex<double> t2 = output[k + j + m2] * w2;
					std::complex<double> t3 = output[k + j + 3 * m4] * w3;
					std::complex<double> u0 = t0 + t2;
					std::complex<double> u1 = t1 + t3;
					std::complex<double> u2 = t0 - t2;
					std::complex<double> u3 = t1 - t3;
					output[k + j] = u0 + u1;
					output[k + j + m4] = u0 - u1;
					output[k + j + m2] = u2 + std::complex<double>(0, 1) * u3;
					output[k + j + 3 * m4] = u2 - std::complex<double>(0, 1) * u3;
				}
			}
			twiddleIdx += 3 * m4;
		}

		// Radix-2 for remaining stages if needed
		if (twiddleIdx < twiddles.size()) {
			for (size_t m = 2; m <= N; m <<= 1) {
				size_t m2 = m / 2;
				for (size_t k = 0; k < N; k += m) {
					for (size_t j = 0; j < m2; ++j) {
						std::complex<double> w = twiddles[twiddleIdx + j];
						std::complex<double> t = w * output[k + j + m2];
						std::complex<double> u = output[k + j];
						output[k + j] = u + t;
						output[k + j + m2] = u - t;
					}
				}
				twiddleIdx += m2;
			}
		}
	}

	// Optimized Real FFT for real-time monitoring inputs, producing unscaled FFT coefficients
	void ComputeRFFT(const std::vector<double>& input, std::vector<std::complex<double>>& output) {
		static const double PI = 3.14159265358979323846;
		const size_t N = input.size();
		if (N <= 1 || (N & (N - 1)) != 0 || output.size() < N / 2 + 1) return;

		// Prepare N/2 complex input from real pairs
		std::vector<std::complex<double>> complexInput(N / 2);
		for (size_t i = 0; i < N / 2; ++i) {
			complexInput[i] = std::complex<double>(input[2 * i], input[2 * i + 1]);
		}

		// Compute N/2-point FFT of complex input
		std::vector<std::complex<double>> tempOutput(N / 2);
		ComputeComplexFFTPower2(complexInput, tempOutput);

		// Unpack to single-sided spectrum (DC to Nyquist)
		output.resize(N / 2 + 1);
		output[0] = std::complex<double>(tempOutput[0].real() + tempOutput[0].imag(), 0.0); // DC component
		output[N / 2] = std::complex<double>(tempOutput[0].real() - tempOutput[0].imag(), 0.0); // Nyquist component

		for (size_t k = 1; k < N / 2; ++k) {
			std::complex<double> w = std::polar(1.0, -2.0 * PI * k / N);
			std::complex<double> h1 = tempOutput[k];
			std::complex<double> h2 = std::conj(tempOutput[N / 2 - k]);
			std::complex<double> Ek = 0.5 * (h1 + h2); // Even part
			std::complex<double> Ok = std::complex<double>(0, -0.5) * (h1 - h2); // Odd part
			output[k] = Ek + w * Ok; // Unscaled FFT coefficient
		}
	}

	// Compute Bark band powers from time-domain samples
	void ComputeBandPowers(const std::vector<double>& samples, size_t fftSize, size_t stepSize, double sampleRate, std::vector<double>& bandPower, std::vector<std::complex<double>>& fftOutput) {
		if (samples.empty() || fftSize == 0 || bandPower.size() != BARK_BAND_NUMBER || sampleRate <= 0.0) {
			bandPower.assign(BARK_BAND_NUMBER, 0.0);
			return;
		}

		ComputeFFTPower2(samples, fftOutput);

		// Compute power spectrum
		std::vector<double> powerSpectrum;
		ComputePowerSpectrum(fftOutput.data(), fftSize, stepSize, powerSpectrum);

		// Map to Bark bands
		MapPowerSpectrumToBarkBands(powerSpectrum, fftSize, sampleRate, bandPower);
	}

	void ComputeBandPowersFromPowerSpectrum(const std::vector<double>& powerSpectrum, size_t fftSize,
		double sampleRate, const std::vector<double>& barkBandPower, std::vector<double>& bandPower) {

		if (powerSpectrum.empty() || fftSize == 0 || sampleRate <= 0.0 || barkBandPower.size() != BARK_BAND_NUMBER) {
			bandPower.assign(BARK_BAND_NUMBER, 0.0);
			return;
		}

		bandPower = barkBandPower;
	}

	// Computes center frequencies for each Bark band, used for weighting and masking
	std::vector<double> ComputeBarkCenterFrequencies(double sampleRate) {
		std::vector<double> centerFreqs(BARK_BAND_NUMBER);
		const double freqLimit = AWHAudio::GetFrequencyLimit(sampleRate);

		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			double lower = BARK_BAND_FREQUENCY_EDGES[b];
			double upper = (b == BARK_BAND_NUMBER - 1) ? freqLimit : BARK_BAND_FREQUENCY_EDGES[b + 1];
			centerFreqs[b] = 0.5 * (lower + upper); // Center frequency for each bark band
		}

		return centerFreqs;
	}

	// Convert Bark band index to frequency (Hz) using Traunmüller's (1990) formula
	double ComputeBarkToFrequencies(size_t barkBand, double sampleRate) {
		const double freqLimit = AWHAudio::GetFrequencyLimit(sampleRate);

		if (barkBand < BARK_BAND_NUMBER + 1) {
			double edge = BARK_BAND_FREQUENCY_EDGES[barkBand];
			return (edge > freqLimit) ? freqLimit : edge;
		}

		return MapBarkToFrequency(static_cast<double>(barkBand));
	}

	std::array<double, BARK_BAND_NUMBER> ComputeBarkWeights(double sampleRate) {
		std::array<double, BARK_BAND_NUMBER> weights{};
		const double freqLimit = AWHAudio::GetFrequencyLimit(sampleRate);

		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			double upperEdge = (b == BARK_BAND_NUMBER - 1) ? freqLimit : BARK_BAND_FREQUENCY_EDGES[b + 1];
			double centerFreq = 0.5 * (BARK_BAND_FREQUENCY_EDGES[b] + upperEdge);
			// Gaussian weighting peaking at 3 kHz (ISO 226:2003)
			weights[b] = 0.02 + 0.06 * std::exp(-std::pow(std::log10(centerFreq / 3000.0) / 0.5, 2));
			// Moore & Glasberg (2007) high-frequency correction
			if (centerFreq > 5000.0) {
				weights[b] *= 1.0 + 0.2 * std::log10(centerFreq / 5000.0);
			}
			// Adjusted low-frequency compensation (less aggressive than Fletcher-Munson)
			weights[b] *= 1.0 / (1.0 + std::pow(centerFreq / 1000.0, -0.5));
		}

		// Normalize weights
		double sum = std::accumulate(weights.begin(), weights.end(), 0.0);
		for (auto& w : weights) w /= (sum + EPSILON);

		return weights;
	}

	// Compute critical band factor based on Bark band power distribution
	double ComputeCriticalBands(const std::complex<double>* fftData, size_t fftSize, size_t stepSize, double sampleRate) {
		if (fftData == nullptr || fftSize == 0 || sampleRate <= 0.0) return 1.0;

		// Compute power spectrum
		std::vector<double> powerSpectrum;
		ComputePowerSpectrum(fftData, fftSize, stepSize, powerSpectrum);

		// Map to Bark bands
		std::vector<double> barkBandPower(BARK_BAND_NUMBER, 0.0);
		MapPowerSpectrumToBarkBands(powerSpectrum, fftSize, sampleRate, barkBandPower);

		// Compute critical band factor
		double totalDynamicContribution = 0.0;
		double bandWeight = 1.0 / BARK_BAND_NUMBER;

		for (size_t i = 0; i < BARK_BAND_NUMBER; ++i) {
			// Convert band power to dB, relative to reference intensity (E_0)
			barkBandPower[i] = AWHAudio::PowerToDb((barkBandPower[i] / E_0) + EPSILON);
			// Compute dynamic range relative to -20 dB threshold
			double dynamicRange = std::abs(barkBandPower[i] - (-20.0));
			// Normalize contribution and accumulate (capped at 1.0 per band)
			totalDynamicContribution += bandWeight * std::min(dynamicRange / 15.0, 1.0);
		}

		return std::min(1.0, totalDynamicContribution);
	}

	double ComputeCriticalBandsFromPowerSpectrum(const std::vector<double>& powerSpectrum, size_t fftSize,
		double sampleRate, const std::vector<double>& barkBandPower) {
		if (powerSpectrum.empty() || fftSize == 0 || sampleRate <= 0.0 || barkBandPower.size() != BARK_BAND_NUMBER) {
			return 1.0;
		}

		// Find maximum band power to set a relative threshold
		double maxPower = *std::max_element(barkBandPower.begin(), barkBandPower.end());
		if (maxPower < EPSILON) return 1.0; // No significant energy

		// Threshold: -60 dB below max power, psychoacoustically relevant (Zwicker & Fastl)
		double threshold = maxPower * std::pow(10.0, -6.0); // -60 dB
		size_t activeBands = 0;

		// Count bands with significant energy
		for (const auto& power : barkBandPower) {
			if (power > threshold) {
				activeBands++;
			}
		}

		// Normalize to a factor between 0 and 1 based on active bands
		double factor = static_cast<double>(activeBands) / BARK_BAND_NUMBER;
		return std::clamp(factor, 0.0, 1.0);
	}

	// Compute frequency masking ratio (unmasked energy / total energy)
	double ComputeFrequencyMasking(const std::complex<double>* fftData, size_t fftSize, size_t stepSize, double sampleRate) {
		if (fftData == nullptr || fftSize == 0 || sampleRate <= 0.0) return 0.0;

		// Compute power spectrum
		std::vector<double> powerSpectrum;
		ComputePowerSpectrum(fftData, fftSize, stepSize, powerSpectrum);

		// Map to Bark bands
		std::vector<double> bandPower(BARK_BAND_NUMBER, 0.0);
		MapPowerSpectrumToBarkBands(powerSpectrum, fftSize, sampleRate, bandPower);

		// Precompute center frequencies
		std::vector<double> centerFreqs = ComputeBarkCenterFrequencies(sampleRate);
		std::vector<double> excitation(BARK_BAND_NUMBER, 0.0);
		std::vector<double> maskedThresholdDb(BARK_BAND_NUMBER, -100.0); // dB re: E_0, initialized low to ensure masking dominance

		// Initialize with quiet thresholds
		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			maskedThresholdDb[b] = AWHAudio::PowerToDb(BARK_QUIET_THRESHOLD_INTENSITIES[b] / E_0);
			excitation[b] = bandPower[b] / (E_0 + EPSILON);
		}

		// Apply Zwicker’s spreading function with level-dependent upward masking (ISO 532-1:2017, Annex B)
		for (size_t m = 0; m < BARK_BAND_NUMBER; ++m) {
			if (bandPower[m] < 1e-12) continue; // Skip negligible maskers
			double maskerLevelDb = AWHAudio::PowerToDb(excitation[m] + EPSILON);
			if (maskerLevelDb < 0) continue; // Ignore sub-threshold maskers

			for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
				double dz = static_cast<double>(b) - m; // Bark difference
				double maskContrib;
				if (dz > 0) { // Downward spread (masker at lower frequency)
					maskContrib = maskerLevelDb - 27.0 * dz; // Fixed slope (Zwicker & Fastl, p. 172)
				}
				else if (dz < 0) { // Upward spread (masker at higher frequency)
					double upwardSlope = 24.0 + (230.0 / centerFreqs[m]) - (0.2 * maskerLevelDb); // Level-dependent
					maskContrib = maskerLevelDb - upwardSlope * (-dz); // dz negative, make positive
				}
				else { // Same band
					maskContrib = maskerLevelDb;
				}
				maskedThresholdDb[b] = std::max(maskedThresholdDb[b], maskContrib);
			}
		}

		// Calculate unmasked energy ratio
		double totalEnergy = 0.0;
		double unmaskedEnergy = 0.0;
		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			double intensity = excitation[b] * E_0;
			totalEnergy += intensity;
			double thresh = std::pow(10.0, maskedThresholdDb[b] / 10.0) * E_0;
			if (intensity > thresh) {
				unmaskedEnergy += intensity - thresh; // Only count energy above threshold
			}
		}

		return (totalEnergy > EPSILON) ? unmaskedEnergy / totalEnergy : 1.0;
	}

	double ComputeFrequencyMaskingFromPowerSpectrum(const std::vector<double>& powerSpectrum, size_t fftSize,
		double sampleRate, const std::vector<double>& barkBandPower) {
		if (powerSpectrum.empty() || fftSize == 0 || sampleRate <= 0.0 ||
			barkBandPower.size() != BARK_BAND_NUMBER) {
			return 0.0;
		}

		// Retrieve or compute center frequencies
		auto& centerFreqs = centerFreqsCache[sampleRate];
		if (centerFreqs.empty()) {
			centerFreqs = ComputeBarkCenterFrequencies(sampleRate);
		}

		std::vector<double> excitation(BARK_BAND_NUMBER, 0.0);
		std::vector<double> maskedThresholdDb(BARK_BAND_NUMBER, -100.0);

		// Initialize thresholds and excitation
		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			maskedThresholdDb[b] = AWHAudio::PowerToDb(BARK_QUIET_THRESHOLD_INTENSITIES[b] / E_0);
			excitation[b] = barkBandPower[b] / (E_0 + EPSILON);
		}

		// Apply Zwicker’s spreading function for masking, excluding self-masking
		for (size_t m = 0; m < BARK_BAND_NUMBER; ++m) {
			if (barkBandPower[m] < 1e-12) continue; // Skip negligible maskers
			double maskerLevelDb = AWHAudio::PowerToDb(excitation[m] + EPSILON);
			if (maskerLevelDb < 0) continue;

			for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
				if (b == m) continue; // Skip self-masking
				double dz = static_cast<double>(b) - m;
				double maskContrib;
				if (dz > 0) {
					maskContrib = maskerLevelDb - 27.0 * dz; // Downward spread
				}
				else {
					double upwardSlope = 24.0 + (230.0 / centerFreqs[m]) - (0.2 * maskerLevelDb);
					maskContrib = maskerLevelDb - upwardSlope * (-dz); // Upward spread
				}
				maskedThresholdDb[b] = std::max(maskedThresholdDb[b], maskContrib);
			}
		}

		// Calculate unmasked energy ratio
		double totalEnergy = 0.0;
		double unmaskedEnergy = 0.0;
		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			double intensity = excitation[b] * E_0;
			totalEnergy += intensity;
			double thresh = std::pow(10.0, maskedThresholdDb[b] / 10.0) * E_0;
			if (intensity > thresh) {
				unmaskedEnergy += intensity - thresh;
			}
		}

		return (totalEnergy > EPSILON) ? unmaskedEnergy / totalEnergy : 1.0;
	}

	// Computes perceptually weighted frequency power from all 25 bark band powers.
	double ComputePerceptualFrequencyPower(
		const std::vector<double>& bandPower, const std::array<double, BARK_BAND_NUMBER>& barkWeights) {
		if (bandPower.size() != BARK_BAND_NUMBER) return 1e-12;

		double power = 0.0;
		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			power += barkWeights[b] * bandPower[b];
		}

		return std::max(1e-12, power);
	}

	// Batch version for processing multiple blocks efficiently.
	std::vector<double> ComputePerceptualFrequencyPowers(
		const std::vector<std::vector<double>>& allBandPowers, const std::array<double, BARK_BAND_NUMBER>& barkWeights) {
		std::vector<double> freqPowers;
		freqPowers.reserve(allBandPowers.size());

		for (const auto& bandPower : allBandPowers) {
			freqPowers.push_back(ComputePerceptualFrequencyPower(bandPower, barkWeights));
		}

		return freqPowers;
	}

	// Compute power spectrum from FFT output
	void ComputePowerSpectrum(const std::complex<double>* fftData, size_t fftSize, size_t stepSize, std::vector<double>& powerSpectrum) {
		if (fftData == nullptr || fftSize == 0) {
			powerSpectrum.clear();
			return;
		}

		// Calculate window energy for Hann window normalization (sum of squares = stepSize / 2.0)
		double windowEnergy = stepSize / 2.0; // Matches Hann window scaling where sum(w[n]^2) = stepSize / 2.0

		// Resize power spectrum to hold single-sided spectrum (DC to Nyquist, inclusive)
		powerSpectrum.resize(fftSize / 2 + 1);

		// Compute power spectrum for each frequency bin
		for (size_t k = 0; k <= fftSize / 2; ++k) {
			double magSquared = std::norm(fftData[k]); // Calculate squared magnitude of FFT output (|X[k]|^2)

			// Compute single-sided power spectrum, normalizing by window energy
			// - For DC (k=0) and Nyquist (k=fftSize/2), use |X[k]|^2 / windowEnergy
			// - For other bins, use 2 * |X[k]|^2 / windowEnergy to account for negative frequencies
			// - This normalization (Version 2) corrects for Hann window attenuation to estimate
			//   the original signal's power, suitable for psychoacoustic analysis (e.g., Bark scale,
			//   specific loudness in ComputeFluctuationStrength, ComputeSharpness).
			powerSpectrum[k] = (k == 0 || k == fftSize / 2) ? magSquared / windowEnergy : 2.0 * magSquared / windowEnergy;

			// Alternative version: use when the power spectrum should reflect the windowed signal's
			// energy (w[n] * s[n]) without compensating for window attenuation, e.g., for direct
			// energy comparisons with ComputeBlockSamplesAndEnergy or when window effects are handled downstream.
			// powerSpectrum[k] = (k == 0 || k == fftSize / 2) ? magSquared / fftSize : 2.0 * magSquared / fftSize;
		}
	}

	double ComputeHarmonicComplexity(const std::vector<double>& bandPower) {
		if (bandPower.size() != AWHAudioFFT::BARK_BAND_NUMBER) return 0.0;

		// Spectral peak analysis for harmonic richness
		double totalPower = 0.0;
		double harmonicPower = 0.0;
		std::vector<double> peaks;
		const double peakThreshold = 0.1; // dB threshold above mean for peak detection
		const double minPeaks = 5.0;      // Minimum expected peaks for full complexity scaling

		// Compute total power across all Bark bands
		for (double power : bandPower) {
			totalPower += power;
		}
		double meanPower = totalPower / bandPower.size();
		double threshold = meanPower * std::pow(10.0, peakThreshold / 10.0);

		// Detect peaks: local maxima exceeding the threshold
		for (size_t i = 1; i < bandPower.size() - 1; ++i) {
			if (bandPower[i] > threshold && bandPower[i] > bandPower[i - 1] && bandPower[i] > bandPower[i + 1]) {
				peaks.push_back(bandPower[i]);
				harmonicPower += bandPower[i];
			}
		}

		// Compute complexity: harmonic power ratio scaled by peak count relative to minPeaks
		double peakFactor = std::min(1.0, static_cast<double>(peaks.size()) / minPeaks);
		double complexity = (totalPower > 1e-12) ? (harmonicPower / totalPower) * peakFactor : 0.0;
		return std::clamp(complexity, 0.0, 1.0);
	}

	double ComputeSpectralCentroid(const std::vector<double>& bandPowers, double sampleRate) {
		if (bandPowers.size() != BARK_BAND_NUMBER) return 0.0;

		std::vector<double> centerFreqs = ComputeBarkCenterFrequencies(sampleRate);
		double weightedSum = 0.0;
		double totalPower = 0.0;

		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			weightedSum += centerFreqs[b] * bandPowers[b];
			totalPower += bandPowers[b];
		}

		return totalPower < EPSILON ? 0.0 : weightedSum / totalPower;
	}

	// Computes spectral flatness for a block, measuring spectral complexity
	// Inputs: bandPowers (bark band powers for a block), numBands (number of bark bands)
	// Returns: Spectral flatness value (0–1), where lower values indicate more complex spectra
	double ComputeSpectralFlatness(const std::vector<double>& bandPowers, size_t numBands) {
		if (bandPowers.size() != numBands || numBands == 0) {
			return 1.0; // Default to flat spectrum for invalid input
		}

		double totalPower = 0.0;
		double geoMean = 0.0;

		for (size_t b = 0; b < numBands; ++b) {
			double power = bandPowers[b] + EPSILON;
			totalPower += power;
			geoMean += std::log(power);
		}

		double flatness = (totalPower > EPSILON) ? std::exp(geoMean / numBands) / (totalPower / numBands) : 1.0;

		return std::clamp(flatness, 0.0, 1.0); // Psychoacoustically relevant range (Jesteadt, 1980)
	}

	// Computes spectral flux for a block, measuring frequency content change between consecutive blocks
	// Inputs: bandPowers (current and previous bark band powers), numBands (number of bark bands)
	// Returns: Spectral flux value, normalized for psychoacoustic relevance
	double ComputeSpectralFlux(const std::vector<double>& currentBandPower, const std::vector<double>& prevBandPower, size_t bandCount) {
		if (currentBandPower.size() != bandCount || prevBandPower.size() != bandCount) {
			return 0.0; // Invalid input
		}

		// Compute sum of absolute differences between Bark band powers
		double flux = 0.0;
		for (size_t i = 0; i < bandCount; ++i) {
			flux += std::abs(currentBandPower[i] - prevBandPower[i]);
		}

		// Normalize by total power of current block, with scaling to reduce sensitivity
		double totalPower = std::accumulate(currentBandPower.begin(), currentBandPower.end(), 0.0) + EPSILON;
		double normalizedFlux = flux / totalPower;

		return std::clamp(normalizedFlux, 0.0, 1.0);
	}

	void ComputeSpectralGenreFactors(
		const std::vector<double>& spectralCentroid, const std::vector<double>& spectralFlatness, const std::vector<double>& spectralFlux,
		double& spectralCentroidMean, double& spectralFlatnessMean, double& spectralFluxMean, double& genreFactor) {

		spectralCentroidMean = 0.0;
		spectralFlatnessMean = 0.0;
		spectralFluxMean = 0.0;
		genreFactor = 0.5;

		// Determine block count and set thresholds
		const size_t blockCount = std::min({ spectralCentroid.size(), spectralFlatness.size(), spectralFlux.size() });
		constexpr double MIN_THRESHOLD = 1e-10; // Small positive value for numerical stability
		const size_t MIN_SAMPLES = std::max<size_t>(5, blockCount / 10); // Adaptive minimum for reliable stats

		// Single-pass computation of means and variances using Welford's algorithm
		double centroidSum = 0.0;
		double flatnessSum = 0.0;
		double fluxSum = 0.0;
		double centroidMeanAcc = 0.0;
		double flatnessMeanAcc = 0.0;
		double fluxMeanAcc = 0.0;
		double centroidM2 = 0.0;
		double flatnessM2 = 0.0;
		double fluxM2 = 0.0;
		size_t validCount = 0;

		for (size_t i = 0; i < blockCount; ++i) {
			if (spectralCentroid[i] > MIN_THRESHOLD && spectralFlatness[i] > MIN_THRESHOLD) {
				++validCount;

				// Update centroid statistics
				const double centroidVal = spectralCentroid[i];
				const double centroidDelta = centroidVal - centroidMeanAcc;
				centroidMeanAcc += centroidDelta / validCount;
				centroidM2 += centroidDelta * (centroidVal - centroidMeanAcc);
				centroidSum += centroidVal;

				// Update flatness statistics
				const double flatnessVal = spectralFlatness[i];
				const double flatnessDelta = flatnessVal - flatnessMeanAcc;
				flatnessMeanAcc += flatnessDelta / validCount;
				flatnessM2 += flatnessDelta * (flatnessVal - flatnessMeanAcc);
				flatnessSum += flatnessVal;

				// Update flux statistics
				const double fluxVal = spectralFlux[i];
				const double fluxDelta = fluxVal - fluxMeanAcc;
				fluxMeanAcc += fluxDelta / validCount;
				fluxM2 += fluxDelta * (fluxVal - fluxMeanAcc);
				fluxSum += fluxVal;
			}
		}

		if (validCount < MIN_SAMPLES) return;

		// Finalize means
		spectralCentroidMean = centroidSum / validCount;
		spectralFlatnessMean = flatnessSum / validCount;
		spectralFluxMean = fluxSum / validCount;

		// Compute standard deviations (sample variance with N-1)
		const double fluxStdDev = (validCount > 1) ? std::sqrt(fluxM2 / (validCount - 1)) : 0.0;
		const double centroidStdDev = (validCount > 1) ? std::sqrt(centroidM2 / (validCount - 1)) : 0.0;
		const double flatnessStdDev = (validCount > 1) ? std::sqrt(flatnessM2 / (validCount - 1)) : 0.0;

		// Adaptive normalization for music perception
		const double flatnessAdjusted = 1.0 - std::clamp(spectralFlatnessMean, 0.0, 1.0); // Inverse flatness (tonality)
		const double centroidScale = 8000.0 + 4000.0 * flatnessAdjusted; // Range 8000–12000 Hz
		const double centroidNorm = std::clamp(spectralCentroidMean / centroidScale, 0.0, 1.0);

		const double fluxScale = 0.12 + 0.04 * (1.0 - centroidNorm); // Range 0.12–0.16, centroid-dependent
		const double fluxNorm = std::clamp(spectralFluxMean / fluxScale, 0.0, 1.0);

		// Normalize variability terms
		const double fluxVarNorm = std::clamp(fluxStdDev / 0.12, 0.0, 1.0); // Flux variability
		const double centroidVarNorm = std::clamp(centroidStdDev / 2000.0, 0.0, 1.0); // Centroid variability
		const double flatnessVarNorm = std::clamp(flatnessStdDev / 0.2, 0.0, 1.0); // Flatness variability

		// Compute perceptual factors with feature interactions
		const double brightness = std::pow(centroidNorm, 1.10); // High-frequency emphasis
		const double contrast = flatnessAdjusted * (0.60 + 0.40 * brightness); // Tonal vs. noisy balance
		const double energyShift = fluxNorm * (0.50 + 0.35 * brightness + 0.15 * flatnessAdjusted); // Dynamic energy
		const double variability = (0.6 * fluxVarNorm + 0.25 * centroidVarNorm + 0.15 * flatnessVarNorm) * (0.75 + 0.25 * brightness); // Combined variability

		// Compute genre factor with optimized weights (sum to 1.0)
		genreFactor = 0.35 * brightness + 0.33 * contrast + 0.22 * energyShift + 0.10 * variability;
		genreFactor = std::clamp(genreFactor, 0.20, 1.0);
	}

	double EstimateModulationFrequency(const std::vector<std::vector<double>>& bandPowersHistory, size_t currentBlock,
		double blockDurationMs, size_t startBand, size_t bandCount) {
		constexpr size_t HISTORY_SIZE = 20; // ~0.5s at 25ms/block

		if (currentBlock < HISTORY_SIZE - 1 || currentBlock >= bandPowersHistory.size()) {
			return (startBand < 6) ? 4.0 : 70.0; // Fallback
		}

		std::vector<double> powerSum(HISTORY_SIZE, 0.0);
		size_t startIdx = currentBlock - HISTORY_SIZE + 1;
		for (size_t t = 0; t < HISTORY_SIZE; ++t) {
			size_t blockIdx = startIdx + t;
			for (size_t b = startBand; b < startBand + bandCount && b < bandPowersHistory[blockIdx].size(); ++b) {
				powerSum[t] += bandPowersHistory[blockIdx][b];
			}
		}

		// Compute mean
		double mean = 0.0;
		for (double p : powerSum) mean += p;
		mean /= HISTORY_SIZE;

		// Autocorrelation for lags 1 to HISTORY_SIZE/2
		size_t maxLag = HISTORY_SIZE / 2;
		double maxCorr = -1.0;
		size_t bestLag = 1;
		for (size_t lag = 1; lag <= maxLag; ++lag) {
			double corr = 0.0;
			for (size_t t = 0; t < HISTORY_SIZE - lag; ++t) {
				corr += (powerSum[t] - mean) * (powerSum[t + lag] - mean);
			}
			if (corr > maxCorr) {
				maxCorr = corr;
				bestLag = lag;
			}
		}

		if (bestLag == 0) return (startBand < 6) ? 4.0 : 70.0;
		double periodMs = bestLag * blockDurationMs;
		double f_mod = 1000.0 / periodMs;

		return std::clamp(f_mod, (startBand < 6) ? 1.0 : 20.0, (startBand < 6) ? 20.0 : 300.0);
	}

	// Helper function to convert Bark scale to frequency (Hz) using Traunmüller's formula
	double MapBarkToFrequency(double z) {
		double z_prime = z;

		if (z < 2.0) z_prime = z + 0.15 * (2.0 - z);
		else if (z > 20.1) z_prime = z + 0.22 * (z - 20.1);

		return 1960.0 * (z_prime + 0.53) / (26.28 - z_prime);
	}

	// Helper function to convert frequency (Hz) to Bark scale using Traunmüller's formula
	double MapFrequencyToBark(double freq) {
		double z = 26.81 * freq / (1960.0 + freq) - 0.53;

		if (z < 2.0) z += 0.15 * (2.0 - z);
		else if (z > 20.1) z += 0.22 * (z - 20.1);

		return z;
	}

	// Maps power spectrum to Bark bands using Gaussian weighting.
	// - Updated to use the adjusted BARK_BAND_FREQUENCY_EDGES (20 Hz–20 kHz).
	// - Distributes FFT bin power across bands based on Bark-scale proximity.
	void MapPowerSpectrumToBarkBands(const std::vector<double>& powerSpectrum, size_t fftSize,
		double sampleRate, std::vector<double>& bandPower) {
		if (powerSpectrum.empty() || bandPower.size() != BARK_BAND_NUMBER || fftSize <= 2 || sampleRate <= 0.0) {
			std::fill(bandPower.begin(), bandPower.end(), 0.0);
			return;
		}

		// FFT parameters
		const size_t numBins = powerSpectrum.size(); // Single-sided spectrum (fftSize / 2 + 1)
		const double binWidth = sampleRate / fftSize;
		const double freqLimit = AWHAudio::GetFrequencyLimit(sampleRate);

		// Psychoacoustic parameters for Gaussian weighting
		constexpr double GAUSSIAN_SPREAD = 0.6; // Spread in Bark scale (~1 Bark FWHM)
		constexpr double WEIGHT_THRESHOLD = 0.01; // Minimum weight for efficiency

		// Retrieve or compute precomputed weights from cache
		auto key = std::make_pair(fftSize, sampleRate);
		auto it = barkWeightCache.find(key);
		if (it == barkWeightCache.end()) {
			// Compute weights for each bin and band
			std::vector<std::vector<std::pair<size_t, double>>> weights(numBins);
			std::vector<double> bandCenters(BARK_BAND_NUMBER);
			for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
				double freq = ComputeBarkToFrequencies(b, sampleRate);
				bandCenters[b] = MapFrequencyToBark(freq); // Center in Bark scale
			}
			for (size_t k = 0; k < numBins; ++k) {
				double freq = k * binWidth;
				if (freq > freqLimit) break;
				double z = MapFrequencyToBark(freq);
				double totalWeight = 0.0;
				std::vector<std::pair<size_t, double>> bandWeights;
				for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
					double deltaBark = std::abs(z - bandCenters[b]);
					double weight = std::exp(-0.5 * std::pow(deltaBark / GAUSSIAN_SPREAD, 2.0));
					if (weight > WEIGHT_THRESHOLD) {
						bandWeights.emplace_back(b, weight);
						totalWeight += weight;
					}
				}
				if (totalWeight > 0.0) {
					for (auto& [b, w] : bandWeights) {
						w /= totalWeight; // Normalize weights
					}
					weights[k] = std::move(bandWeights);
				}
			}
			it = barkWeightCache.try_emplace(key, weights).first;
		}
		const auto& weights = it->second;

		// Map power spectrum to Bark bands using cached weights
		std::fill(bandPower.begin(), bandPower.end(), 0.0);
		for (size_t k = 0; k < numBins; ++k) {
			if (k * binWidth > freqLimit) break;
			for (const auto& [b, normalizedWeight] : weights[k]) {
				bandPower[b] += powerSpectrum[k] * normalizedWeight;
			}
		}

		// Ensure non-zero minimum power
		for (size_t b = 0; b < BARK_BAND_NUMBER; ++b) {
			bandPower[b] = std::max(bandPower[b], EPSILON);
		}
	}
}
#pragma endregion


/////////////////////
// * COM HELPERS * //
/////////////////////
#pragma region COM Helpers
namespace AWHCOM {
	HRESULT LogError(HRESULT errorCode, const std::wstring& source, const std::wstring& description, bool setErrorInfo) {
		pfc::string8 logMessage;

		logMessage << pfc::stringcvt::string_utf8_from_wide(source.c_str())
			<< ": " << pfc::stringcvt::string_utf8_from_wide(description.c_str())
			<< ", HRESULT: 0x" << pfc::format_hex(errorCode);

		FB2K_console_formatter() << logMessage;

		if (setErrorInfo && FAILED(errorCode)) {
			CComPtr<ICreateErrorInfo> createErrorInfo;

			if (SUCCEEDED(CreateErrorInfo(&createErrorInfo))) {
				createErrorInfo->SetSource(CComBSTR(source.c_str()));
				createErrorInfo->SetDescription(CComBSTR(description.c_str()));

				CComPtr<IErrorInfo> errorInfo;
				if (SUCCEEDED(createErrorInfo->QueryInterface(IID_IErrorInfo, reinterpret_cast<void**>(&errorInfo)))) {
					SetErrorInfo(0, errorInfo);
				}
			}
		}

		return errorCode;
	}

	void CreateCallback(VARIANT& targetCallback, const VARIANT* newCallback, const char* callbackName) {
		if (!newCallback) {
			FB2K_console_formatter() << "Audio Wizard => CreateCallback: Set" << callbackName << "Callback failed, callback is null";
			return;
		}

		if (targetCallback.vt == VT_DISPATCH && targetCallback.pdispVal != nullptr) {
			targetCallback.pdispVal->Release();
		}

		HRESULT hr = VariantCopy(&targetCallback, newCallback);
		if (FAILED(hr)) {
			FB2K_console_formatter() << "Audio Wizard => CreateCallback: VariantCopy failed for " << callbackName << " callback, HRESULT=" << hr;
			return;
		}

		if (targetCallback.vt == VT_DISPATCH && targetCallback.pdispVal != nullptr) {
			targetCallback.pdispVal->AddRef();
		}
		else {
			FB2K_console_formatter() << "Audio Wizard => CreateCallback: " << callbackName << " callback is invalid or empty";
		}
	}

	void FireCallback(const VARIANT& callback, bool success, const std::function<void()>& postAction) {
		if (callback.vt == VT_DISPATCH && callback.pdispVal != nullptr) {
			fb2k::inMainThread([callback, success, postAction] {
				VARIANT arg;
				arg.vt = VT_BOOL;
				arg.boolVal = success ? VARIANT_TRUE : VARIANT_FALSE;

				DISPPARAMS params;
				params.rgvarg = &arg;
				params.rgdispidNamedArgs = nullptr;
				params.cArgs = 1;
				params.cNamedArgs = 0;

				HRESULT hr = callback.pdispVal->Invoke(DISPID_VALUE, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &params, nullptr, nullptr, nullptr);

				if (FAILED(hr)) {
					FB2K_console_formatter() << "Audio Wizard => FireCallback: Callback invocation failed, HRESULT: " << hr;
				}

				if (postAction) postAction();
			});
		}
		else if (postAction) {
			fb2k::inMainThread(postAction);
		}
	}

	metadb_handle_list GetMetadbHandlesFromStringArray(const VARIANT& metadata) {
		metadb_handle_list tracks;

		if (metadata.vt != (VT_ARRAY | VT_BSTR) && metadata.vt != (VT_ARRAY | VT_VARIANT)) {
			return tracks;
		}

		SAFEARRAY* tracksArray = metadata.parray;
		if (!tracksArray) {
			return tracks;
		}

		long lbound;
		long ubound;
		if (FAILED(SafeArrayGetLBound(tracksArray, 1, &lbound)) ||
			FAILED(SafeArrayGetUBound(tracksArray, 1, &ubound))) {
			return tracks;
		}

		long trackCount = ubound - lbound + 1;
		if (trackCount <= 0) {
			return tracks;
		}

		bool isVariant = (metadata.vt == (VT_ARRAY | VT_VARIANT));

		auto addTrackFromBstr = [&](BSTR bstr) {
			if (!bstr) return;

			pfc::string8 trackInfo(pfc::stringcvt::string_utf8_from_utf16(
				reinterpret_cast<const char16_t*>(bstr), SysStringLen(bstr)));

			t_size separatorPos = pfc::string_find_first(trackInfo, '\x1F', 0);
			if (separatorPos == pfc::infinite_size) return;

			pfc::string8 path = trackInfo.subString(0, separatorPos);
			pfc::string8 subsongStr = trackInfo.subString(separatorPos + 1);
			auto subsong = pfc::atoui_ex(subsongStr, pfc::infinite_size);

			metadb_handle_ptr handle;
			static_api_ptr_t<metadb> metadbApi;
			metadbApi->handle_create(handle, make_playable_location(path, subsong));
			if (handle.is_valid()) {
				tracks.add_item(handle);
			}
		};

		for (long i = 0; i < trackCount; ++i) {
			long idx = lbound + i;

			if (!isVariant) {
				BSTR bstr = nullptr;
				if (SUCCEEDED(SafeArrayGetElement(tracksArray, &idx, &bstr)) && bstr) {
					addTrackFromBstr(bstr);
					SysFreeString(bstr);
				}
			}
			else {
				VARIANT varTrack;
				VariantInit(&varTrack);
				if (SUCCEEDED(SafeArrayGetElement(tracksArray, &idx, &varTrack))) {
					if (varTrack.vt == VT_BSTR && varTrack.bstrVal) {
						addTrackFromBstr(varTrack.bstrVal);
					}
				}
				VariantClear(&varTrack);
			}
		}

		return tracks;
	}

	HRESULT GetOptionalLong(const VARIANT* variant, LONG& output) {
		output = 0;

		if (variant != nullptr) {
			if (variant->vt == VT_I4) {
				output = variant->lVal;
			}
			else if (variant->vt != VT_ERROR || variant->scode != DISP_E_PARAMNOTFOUND) {
				FB2K_console_formatter() << "Audio Wizard => GetOptionalLong: Invalid variant type or error code";
				return E_INVALIDARG;
			}
		}

		return S_OK;
	}

	SafeArrayAccess::SafeArrayAccess(SAFEARRAY* inputPsa) : psa(inputPsa) {
		if (!psa) {
			hr = E_INVALIDARG;
			FB2K_console_formatter() << "Audio Wizard => SafeArrayAccess: Invalid SAFEARRAY";
			return;
		}

		hr = SafeArrayAccessData(psa, reinterpret_cast<void**>(&data));

		if (FAILED(hr)) {
			FB2K_console_formatter() << "Audio Wizard => SafeArrayAccessData failed with HRESULT: " << hr;
			data = nullptr;
		}
	}

	SafeArrayAccess::~SafeArrayAccess() {
		if (data && psa) SafeArrayUnaccessData(psa);
	}

	SAFEARRAY* CreateSafeArrayFromData(size_t size, float defaultValue, const char* context) {
		if (static_cast<ULONG>(size) > std::numeric_limits<ULONG>::max()) {
			FB2K_console_formatter() << "Audio Wizard => " << context << ": Invalid size (" << size << ")";
			return nullptr;
		}

		SAFEARRAY* psa = SafeArrayCreateVector(VT_R4, 0, static_cast<ULONG>(size));
		if (!psa) {
			FB2K_console_formatter() << "Audio Wizard => " << context << ": Failed to create SAFEARRAY";
			return nullptr;
		}

		SafeArrayAccess access(psa);
		if (FAILED(access.getHr())) {
			SafeArrayDestroy(psa);
			FB2K_console_formatter() << "Audio Wizard => " << context << ": SafeArrayAccessData failed: " << access.getHr();
			return nullptr;
		}

		auto* pData = access.getData();
		std::fill(pData, pData + size, defaultValue);

		return psa;
	}
}
#pragma endregion


////////////////////////////
// * CONVERSION HELPERS * //
////////////////////////////
#pragma region Conversion Helpers
namespace AWHConvert {
	bool BOOLFromVARIANT(const VARIANT& var, bool defaultValue) {
		return (var.vt == VT_BOOL) ? (var.boolVal == VARIANT_TRUE) : defaultValue;
	}

	double DOUBLEFromVARIANT(const VARIANT& var, double defaultValue) {
		return (var.vt == VT_R8) ? var.dblVal : defaultValue;
	}

	long INTFromVARIANT(const VARIANT& var, long defaultValue) {
		return (var.vt == VT_I4) ? var.intVal : defaultValue;
	}

	short SHORTFromVARIANT(const VARIANT& var, short defaultValue) {
		return (var.vt == VT_I2) ? var.iVal : defaultValue;
	}

	std::wstring STRINGFromVARIANT(const VARIANT& var, const std::wstring& defaultValue) {
		return (var.vt == VT_BSTR) ? std::wstring(var.bstrVal) : defaultValue;
	}

	ULONGLONG FileTimeToUll(const FILETIME& ft) {
		return (static_cast<ULONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
	}

	double MsToSec(int ms) {
		return static_cast<double>(ms) / 1000.0;
	}

	int SecToMs(double sec) {
		return static_cast<int>(sec * 1000.0 + 0.5);
	}

	CSize DialogUnitsToPixel(HWND hWnd, int dluX, int dluY) {
		CRect rect(0, 0, dluX, dluY);
		::MapDialogRect(hWnd, &rect);
		return CSize(rect.Width(), rect.Height());
	}

	int PercentToPixels(double percent, int dimension) {
		return static_cast<int>(percent * dimension);
	}

	double PixelsToPercent(int pixels, int dimension) {
		return (dimension > 0) ? (static_cast<double>(pixels) / dimension) : 0.0;
	}
}
#pragma endregion


///////////////////////////
// * DARK MODE HELPERS * //
///////////////////////////
#pragma region Dark Mode Helpers
namespace AWHDarkMode {
	namespace {
		std::shared_ptr<fb2k::CCoreDarkModeHooks>& GetHooks() {
			static std::shared_ptr<fb2k::CCoreDarkModeHooks> hooks;
			hooks = std::make_shared<fb2k::CCoreDarkModeHooks>();
			return hooks;
		}
		bool isDark = false;
	}

	void AddControls(HWND wnd) {
		auto hooks = GetHooks();
		if (hooks) hooks->AddControls(wnd);
	}

	void AddControlsWithExclude(HWND parent, HWND exclude) {
		auto hooks = GetHooks();
		if (!hooks) return;

		auto hWndChild = GetWindow(parent, GW_CHILD);
		while (hWndChild) {
			if (hWndChild != exclude) {
				AddCtrlAuto(hWndChild);
			}
			hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT);
		}
	}

	void AddCtrlAuto(HWND wnd) {
		auto hooks = GetHooks();
		if (hooks) hooks->AddCtrlAuto(wnd);
	}

	void AddDialog(HWND wnd) {
		auto hooks = GetHooks();
		if (hooks) hooks->AddDialog(wnd);
	}

	void AddDialogWithControls(HWND wnd) {
		auto hooks = GetHooks();
		if (hooks) hooks->AddDialogWithControls(wnd);
	}

	void Cleanup() {
		GetHooks().reset();
	}

	bool IsDark() {
		auto hooks = GetHooks();
		if (!hooks) return isDark;

		isDark = hooks->operator bool();
		return isDark;
	}

	void SetDark(bool dark) {
		auto hooks = GetHooks();

		if (hooks) {
			hooks->SetDark(dark);
			isDark = dark;
		}
	}
}
#pragma endregion


////////////////////////
// * DIALOG HELPERS * //
////////////////////////
#pragma region Dialog Helpers
namespace AWHDialog {
	bool CreateCustomFont(CFont& font, int height, int weight, const TCHAR* faceName) {
		LOGFONT lf = { 0 };
		lf.lfHeight = height;
		lf.lfWeight = weight;
		_tcscpy_s(lf.lfFaceName, LF_FACESIZE, faceName);

		if (!font.CreateFontIndirect(&lf)) { // Fallback to system font
			_tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("MS Shell Dlg"));

			HDC hdc = GetDC(nullptr);
			lf.lfHeight = -MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			ReleaseDC(nullptr, hdc);

			return font.CreateFontIndirect(&lf);
		}

		return true;
	}

	bool GetCheckBoxState(HWND hWnd, int id) {
		return IsDlgButtonChecked(hWnd, id) == BST_CHECKED;
	}

	void SetCheckBox(HWND hWnd, int id, bool checked) {
		CheckDlgButton(hWnd, id, checked ? BST_CHECKED : BST_UNCHECKED);
	}

	void SetControlEnableState(HWND hWnd, const std::vector<int>& controlIDs, bool enabled) {
		for (int id : controlIDs) {
			::EnableWindow(GetDlgItem(hWnd, id), enabled);
		}
	}

	int GetDropDownIndex(HWND hWnd, int id) {
		LRESULT selectedIndex = SendDlgItemMessage(hWnd, id, CB_GETCURSEL, 0, 0);
		return selectedIndex == CB_ERR ? -1 : static_cast<int>(selectedIndex);
	}

	void SetDropDownMenu(HWND hWnd, int id, const std::vector<std::wstring>& items, int selectedItem) {
		SendDlgItemMessage(hWnd, id, CB_RESETCONTENT, 0, 0);

		for (const auto& item : items) {
			SendDlgItemMessage(hWnd, id, CB_ADDSTRING, 0, (LPARAM)item.c_str());
		}

		SendDlgItemMessage(hWnd, id, CB_SETCURSEL, (WPARAM)selectedItem, 0);
	}

	int GetInputFieldNumber(HWND hWnd, int id, BOOL* pSuccess, BOOL signedValue) {
		return GetDlgItemInt(hWnd, id, pSuccess, signedValue);
	}

	void SetInputFieldNumber(HWND hWnd, int id, int value, BOOL signedValue) {
		SetDlgItemInt(hWnd, id, static_cast<UINT>(value), signedValue);
	}

	pfc::string8 GetInputFieldText(HWND hWnd, int id) {
		int length = ::GetWindowTextLength(GetDlgItem(hWnd, id));
		if (length <= 0) return pfc::string8();

		std::wstring tempStr(static_cast<size_t>(length) + 1, L'\0');
		GetDlgItemTextW(hWnd, id, &tempStr[0], length + 1);
		tempStr.resize(length);

		return pfc::stringcvt::string_utf8_from_wide(tempStr.c_str()).get_ptr();
	}

	void SetInputFieldText(HWND hWnd, int id, const std::wstring& text) {
		SetDlgItemText(hWnd, id, text.c_str());
	}

	void SetSpinControlRange(HWND hWnd, int id, int minVal, int maxVal) {
		SendDlgItemMessage(hWnd, id, UDM_SETRANGE32, static_cast<WPARAM>(minVal), static_cast<LPARAM>(maxVal));
	}

	LRESULT CALLBACK SpinControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
		switch (message) { // Only need to subclass the spin controls because of flicker issues, do double buffering here
			case WM_PAINT: {
				AWHGraphics::WindowDC winDC(hWnd);
				const RECT& rc = winDC.GetPaintRect();
				int width = rc.right - rc.left;
				int height = rc.bottom - rc.top;

				AWHGraphics::MemoryDC memDC(winDC.GetDC(), width, height);

				if (!memDC.GetDC()) {
					DefSubclassProc(hWnd, WM_ERASEBKGND, (WPARAM)winDC.GetDC(), 0);
					DefSubclassProc(hWnd, WM_PRINTCLIENT, (WPARAM)winDC.GetDC(), PRF_CLIENT);
					return 0;
				}

				DefSubclassProc(hWnd, WM_ERASEBKGND, (WPARAM)memDC.GetDC(), 0);
				DefSubclassProc(hWnd, WM_PRINTCLIENT, (WPARAM)memDC.GetDC(), PRF_CLIENT);
				BitBlt(winDC.GetDC(), 0, 0, width, height, memDC.GetDC(), 0, 0, SRCCOPY);

				return 0;
			}

			case WM_ERASEBKGND: {
				return 1;
			}

			case WM_NCDESTROY: {
				RemoveWindowSubclass(hWnd, SpinControlProc, uIdSubclass);
				break;
			}

			default: {
				return DefSubclassProc(hWnd, message, wParam, lParam);
			}
		}

		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	void SpinControlSubclass(HWND hWnd) {
		SetWindowSubclass(hWnd, SpinControlProc, 0, 0);
	}
}
#pragma endregion


//////////////////////////
// * GRAPHICS HELPERS * //
//////////////////////////
#pragma region Graphics Helpers
namespace AWHGraphics {
	MemoryDC::MemoryDC(HDC hdcRef, int width, int height) :
		m_hdcMem(CreateCompatibleDC(hdcRef)),
		m_bitmap(CreateTheBitmap(width, height, 1, GetDeviceCaps(hdcRef, BITSPIXEL), nullptr)),
		m_bmpSel(m_hdcMem, m_bitmap.get()) {
		if (!m_hdcMem) {
			FB2K_console_formatter() << "Audio Wizard => Failed to create memory DC";
		}
		if (!m_bitmap.get()) {
			DeleteDC(m_hdcMem);
			FB2K_console_formatter() << "Audio Wizard => Failed to create compatible bitmap";
		}
	}

	MemoryDC::~MemoryDC() {
		if (m_hdcMem) {
			DeleteDC(m_hdcMem);
		}
	}

	WindowDC::WindowDC(HWND hwnd) : m_hwnd(hwnd), m_hdc(nullptr) {
		m_hdc = BeginPaint(hwnd, &m_ps);
		if (!m_hdc) {
			FB2K_console_formatter() << "Audio Wizard => Failed to get window DC";
		}
	}

	WindowDC::~WindowDC() {
		if (m_hdc) {
			EndPaint(m_hwnd, &m_ps);
		}
	}

	Bitmap CreateTheBitmap(int width, int height, UINT planes, UINT bitsPerPixel, const void* data) {
		if (width <= 0 || height <= 0) {
			FB2K_console_formatter() << "Audio Wizard => invalid bitmap dimensions";
			return Bitmap(nullptr);
		}

		HBITMAP bitmap = ::CreateBitmap(width, height, planes, bitsPerPixel, data);
		if (!bitmap) FB2K_console_formatter() << "Audio Wizard => failed to create bitmap";
		return Bitmap(bitmap);
	}

	Brush CreateTheSolidBrush(COLORREF color) {
		HBRUSH brush = ::CreateSolidBrush(color);
		if (!brush) FB2K_console_formatter() << "Audio Wizard => failed to create solid brush";
		return Brush(brush);
	}

	Brush CreateThePatternBrush(HBITMAP bitmap) {
		HBRUSH brush = ::CreatePatternBrush(bitmap);
		if (!brush) FB2K_console_formatter() << "Audio Wizard => failed to create pattern brush";
		return Brush(brush);
	}

	Font CreateTheFont(int height, int width, const wchar_t* face, int weight, DWORD italic, DWORD underline, DWORD strikeout, DWORD charset) {
		HFONT font = ::CreateFontW(height, width, 0, 0, weight, italic, underline, strikeout, charset,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, face
		);
		if (!font) FB2K_console_formatter() << "Audio Wizard => failed to create font";
		return Font(font);
	}

	Palette CreateThePalette(const LOGPALETTE* palette) {
		HPALETTE pal = ::CreatePalette(palette);
		if (!pal) FB2K_console_formatter() << "Audio Wizard => failed to create palette";
		return Palette(pal);
	}

	Pen CreateThePen(int style, int width, COLORREF color) {
		HPEN pen = ::CreatePen(style, width, color);
		if (!pen) FB2K_console_formatter() << "Audio Wizard => failed to create pen";
		return Pen(pen);
	}

	Region CreateTheRectRegion(const RECT& rect) {
		HRGN region = ::CreateRectRgnIndirect(&rect);
		if (!region) FB2K_console_formatter() << "Audio Wizard => failed to create region";
		return Region(region);
	}

	void DrawTheHorizontalLine(HDC hdc, int xStart, int xEnd, int y, COLORREF color) {
		if (!hdc) return;
		Pen hPen = CreateThePen(PS_SOLID, 1, color);
		GDISelector penSel(hdc, hPen.get());
		MoveToEx(hdc, xStart, y, nullptr);
		LineTo(hdc, xEnd, y);
	}

	void DrawTheRect(HDC hdc, const RECT& rect, COLORREF fillColor, COLORREF borderColor, int borderWidth) {
		if (!hdc) {
			FB2K_console_formatter() << "Audio Wizard => Invalid HDC";
			return;
		}

		Brush fillBrush = CreateTheSolidBrush(fillColor);
		if (!fillBrush.get()) {
			FB2K_console_formatter() << "Audio Wizard => Failed to create fill brush";
			return;
		}

		if (borderWidth > 0) {
			Pen borderPen = CreateThePen(PS_SOLID, borderWidth, borderColor);
			if (!borderPen.get()) {
				FB2K_console_formatter() << "Audio Wizard => Failed to create border pen";
				return;
			}

			GDISelector brushSel(hdc, fillBrush.get());
			GDISelector penSel(hdc, borderPen.get());
			Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
		}
		else {
			FillRect(hdc, &rect, fillBrush.get());
		}
	}

	void DrawTheText(HDC hdc, const RECT& rect, const CStringW& text, COLORREF color, HFONT font, UINT format) {
		if (!hdc) {
			FB2K_console_formatter() << "Audio Wizard => AWHGraphics::drawText: Invalid HDC";
			return;
		}

		GDISelector fontSel(hdc, font);
		SetTextColor(hdc, color);
		SetBkMode(hdc, TRANSPARENT);
		DrawTextW(hdc, text, text.GetLength(), const_cast<LPRECT>(&rect), format);
	}
}
#pragma endregion


//////////////////////
// * MATH HELPERS * //
//////////////////////
#pragma region Math Helpers
namespace AWHMath {
	// Coefficient calculation for exponential decay
	double CalculateCoefficient(double deltaTime, double timeConstant) {
		return std::exp(-deltaTime / timeConstant);
	}

	double CalculateEntropy(const std::vector<double>& features) {
		if (features.empty()) return 0.0;

		const double epsilon = 1e-12;
		double total = 0.0;
		for (double f : features) total += f + epsilon;

		double entropy = 0.0;
		for (double f : features) {
			double w = (f + epsilon) / total;
			entropy -= w * std::log(w + epsilon);
		}

		size_t n = features.size();
		double max_entropy = (n > 1) ? std::log(static_cast<double>(n)) : 1.0;
		return std::clamp(entropy / max_entropy, 0.0, 1.0);
	}

	// Approximate IQR using a histogram-based approach
	double CalculateIQR(const std::vector<double>& data) {
		if (data.empty()) return 0.0;

		// Find min and max for histogram range
		double minVal = *std::min_element(data.begin(), data.end());
		double maxVal = *std::max_element(data.begin(), data.end());
		if (minVal >= maxVal) return 0.0;

		const size_t NUM_BINS = 100;
		std::vector<size_t> histogram(NUM_BINS, 0);
		double binWidth = (maxVal - minVal) / NUM_BINS;

		// Populate histogram
		for (double val : data) {
			size_t bin = std::min<size_t>(NUM_BINS - 1, static_cast<size_t>((val - minVal) / binWidth));
			histogram[bin]++;
		}

		// Find Q1 (25th percentile) and Q3 (75th percentile)
		size_t totalCount = data.size();
		size_t q1Count = totalCount / 4;
		size_t q3Count = 3 * totalCount / 4;
		size_t currentCount = 0;
		double q1 = 0.0;
		double q3 = 0.0;

		for (size_t i = 0; i < NUM_BINS; ++i) {
			currentCount += histogram[i];
			double binValue = minVal + (i + 0.5) * binWidth;
			if (q1 == 0.0 && currentCount >= q1Count) {
				q1 = binValue;
			}
			if (q3 == 0.0 && currentCount >= q3Count) {
				q3 = binValue;
				break;
			}
		}

		return q3 - q1;
	}

	double CalculateKurtosis(const std::vector<double>& data, double mean, double defaultValue, bool biasCorrected) {
		const size_t n = data.size();

		if (n < 2 || (biasCorrected && n < 4) || !std::isfinite(mean) || !std::isfinite(defaultValue)) {
			return defaultValue;
		}

		constexpr double EPSILON = 1e-10;
		double variance = 0.0;
		double fourthMoment = 0.0;

		for (const double x : data) {
			if (!std::isfinite(x)) return defaultValue;
			const double diff = x - mean;
			const double diff2 = diff * diff;
			variance += diff2;
			fourthMoment += diff2 * diff2;
		}

		// Normalize moments by n
		variance /= n;
		fourthMoment /= n;

		if (variance < EPSILON) {
			return 3.0;
		}

		// Calculate raw kurtosis
		const double varianceSquared = variance * variance;
		double kurtosis = fourthMoment / varianceSquared;

		if (biasCorrected) {
			const auto n_d = static_cast<double>(n);
			const double num = n_d * (n_d + 1.0) * kurtosis - 3.0 * (n_d - 1.0) * (n_d - 1.0);
			const double denom = (n_d - 1.0) * (n_d - 2.0) * (n_d - 3.0);
			kurtosis = num / denom + 3.0; // Convert to non-excess kurtosis
		}

		return kurtosis;
	}

	double CalculateMean(const std::vector<double>& vec, double invalidValue) {
		double sum = 0.0;
		size_t count = 0;

		for (const auto& val : vec) {
			if (val != invalidValue && std::isfinite(val)) {
				sum += val;
				count++;
			}
		}

		return count > 0 ? sum / count : invalidValue;
	}

	// Compute median of a sorted vector
	double CalculateMedian(std::vector<double> data) {
		if (data.empty()) return -INFINITY;

		std::sort(data.begin(), data.end());
		size_t mid = data.size() / 2;

		return data.size() % 2 == 0 ? (data[mid - 1] + data[mid]) / 2.0 : data[mid];
	}

	double CalculatePercentile(const std::vector<double>& data, double percentile) {
		if (data.empty()) return -INFINITY;

		std::vector<double> sortedData = data;
		std::sort(sortedData.begin(), sortedData.end());
		size_t idx = std::max<size_t>(1, static_cast<size_t>(percentile * sortedData.size())) - 1;

		return sortedData[idx];
	}

	// Smoothing factor for attack/release (1 - exp(-deltaTime / timeConstant))
	double CalculateSmoothingFactor(double deltaTime, double timeConstant) {
		return 1.0 - CalculateCoefficient(deltaTime, timeConstant);
	}

	// Calculate variance of block loudness for dynamic alpha adjustment
	double CalculateVariance(const std::vector<double>& data) {
		if (data.empty()) return 0.0;

		double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
		double sumSquaredDiff = 0.0;

		for (double x : data) {
			sumSquaredDiff += (x - mean) * (x - mean);
		}

		return sumSquaredDiff / data.size();
	}

	// Calculate variance incrementally using Welford's algorithm
	double CalculateVarianceOnline(const std::vector<double>& data) {
		size_t count = 0;
		double mean = 0.0;
		double m2 = 0.0;

		for (double x : data) {
			if (!std::isfinite(x)) continue;
			++count;
			double delta = x - mean;
			mean += delta / count;
			m2 += delta * (x - mean);
		}

		return count > 1 ? m2 / (count - 1) : 0.0;
	}

	double CalculateWeightedAverage(const std::vector<double>& values, const std::vector<double>& weights) {
		if (values.empty() || values.size() != weights.size()) {
			return 0.0;
		}

		double weightedSum = 0.0;
		double weightSum = 0.0;
		for (size_t i = 0; i < values.size(); ++i) {
			weightedSum += values[i] * weights[i];
			weightSum += weights[i];
		}

		return weightSum > 0.0 ? weightedSum / weightSum : 0.0;
	}

	// Simple approximation of modified Bessel function I0 for Kaiser window
	double CalculateBesselI0(double x) {
		x = std::abs(x);
		static std::array<double, 10001> cache;
		static bool initialized = false;
		static const double PI = 3.14159265358979323846;

		if (!initialized) {
			for (int i = 0; i <= 10000; ++i) {
				double x_val = i * 0.001;
				cache[i] = std::cyl_bessel_i(0, x_val);
			}
			initialized = true;
		}

		if (x <= 10.0) {
			double i_float = x / 0.001;
			auto i = static_cast<int>(i_float);
			if (i < 10000) {
				double frac = i_float - i;
				return cache[i] + frac * (cache[static_cast<size_t>(i) + 1] - cache[i]);
			}
			else {
				return cache[10000];
			}
		}
		else {
			double inv_x = 1.0 / x;
			double sum = 1.0 + (1.0 / 8) * inv_x + (9.0 / 128) * inv_x * inv_x;
			return std::exp(x) / std::sqrt(2 * PI * x) * sum;
		}
	}

	// Cosine fade interpolation (ease-in/ease-out)
	double CalculateCosineFadeProgress(double elapsedTime, double fadeDuration) {
		double progress = std::clamp(elapsedTime / fadeDuration, 0.0, 1.0);
		static const double PI = 3.14159265358979323846;
		return (1.0 - std::cos(progress * PI)) / 2.0;
	}

	double CalculateEaseOutCubic(double t) {
		t -= 1.0f;
		return t * t * t + 1.0f;
	}

	double CalculateEaseOutExpo(double t) {
		return t == 1.0 ? 1.0 : 1.0 - std::pow(2.0, -10.0 * t);
	}

	double CalculateEaseOutQuad(double t) {
		return 1.0f - (1.0f - t) * (1.0f - t);
	}

	double RoundTo(double value, int decimalPlaces) {
		double factor = std::pow(10.0, decimalPlaces);
		return std::round(value * factor) / factor;
	}

	bool ValueOverThreshold(double newValue, double oldValue, double threshold) {
		return std::abs(newValue - oldValue) >= threshold;
	}
}
#pragma endregion


//////////////////////
// * META HELPERS * //
//////////////////////
#pragma region Meta Helpers
namespace AWHMeta {
	struct String8Hash {
		size_t operator()(const pfc::string8& str) const {
			return std::hash<std::string_view>{}(str.get_ptr());
		}
	};

	bool GetInfoContainer(const metadb_handle_ptr& track, metadb_info_container::ptr& infoContainer) {
		if (!track.is_valid()) return false;

		try {
			infoContainer = track->get_info_ref();
			return infoContainer.is_valid();
		}
		catch (...) {
			return false;
		}
	}

	unsigned GetBitDepth(const metadb_handle_ptr& track) {
		pfc::string8 bitDepthStr = GetTechnicalInfoField(track, "bitspersample");

		try {
			return std::stoul(bitDepthStr.c_str());
		}
		catch (...) {
			return 16; // Fallback to 16 if conversion fails
		}
	}

	pfc::string8 GetDuration(const metadb_handle_ptr& track) {
		metadb_info_container::ptr infoContainer;

		if (!GetInfoContainer(track, infoContainer)) return pfc::string8("Unknown Duration");
		double length = infoContainer->info().get_length();

		if (length > 0) {
			auto minutes = static_cast<int>(length / 60);
			auto seconds = static_cast<int>(length) % 60;

			pfc::string8 result;
			result << pfc::format_int(minutes) << ":";
			if (seconds < 10) result << "0"; // Add leading zero if needed
			result << pfc::format_int(seconds);

			return result;
		}

		return pfc::string8("Unknown Duration");
	}

	pfc::string8 GetFileFormat(const metadb_handle_ptr& track) {
		if (!track.is_valid()) return pfc::string8("Unknown Format");

		const char* ext = pfc::io::path::getFileExtension(track->get_path());

		return (ext && *ext) ? pfc::string8(ext).toLower() : pfc::string8("Unknown Format");
	}

	pfc::string8 GetMetadataField(const metadb_handle_ptr& track, const char* const field) {
		if (!track.is_valid()) return pfc::string8("Unknown");

		pfc::string8 result;
		metadb_info_container::ptr infoContainer;

		if (GetInfoContainer(track, infoContainer)) {
			const file_info& info = infoContainer->info();
			t_size index = info.meta_find(field);
			if (index != pfc_infinite && info.meta_enum_value_count(index) > 0) {
				result = info.meta_enum_value(index, 0);
			}
		}
		if (result.is_empty()) {
			static const std::unordered_map<pfc::string8, std::function<pfc::string8(const metadb_handle_ptr&)>, String8Hash> fallbacks = {
				{"artist", [](const metadb_handle_ptr&)   { return pfc::string8("Unknown Artist"); }},
				{"album",  [](const metadb_handle_ptr&)   { return pfc::string8("Unknown Album"); }},
				{"title",  [](const metadb_handle_ptr& t) { return pfc::string_filename_ext(t->get_location().get_path()); }},
				{"date",   [](const metadb_handle_ptr&)   { return pfc::string8("Unknown Year"); }},
				{"genre",  [](const metadb_handle_ptr&)   { return pfc::string8("Unknown Genre"); }}
			};
			auto it = fallbacks.find(field);
			return it != fallbacks.end() ? it->second(track) : pfc::string8("Unknown");
		}

		return result;
	}

	pfc::string8 GetTechnicalInfoField(const metadb_handle_ptr& track, const char* const field) {
		if (!track.is_valid()) return pfc::string8("Unknown");

		pfc::string8 result;
		metadb_info_container::ptr infoContainer;

		if (GetInfoContainer(track, infoContainer)) {
			const file_info& info = infoContainer->info();
			t_size index = info.info_find(field);
			if (index != pfc_infinite) {
				result = info.info_enum_value(index);
			}
		}
		if (result.is_empty()) {
			static const std::unordered_map<pfc::string8, pfc::string8, String8Hash> fallbacks = {
				{"channels",   "Unknown Channels"},
				{"bitrate",    "Unknown Bitrate"},
				{"samplerate", "Unknown Sample Rate"}
			};
			auto it = fallbacks.find(field);
			return it != fallbacks.end() ? it->second : pfc::string8("Unknown");
		}

		return result;
	}
}
#pragma endregion


/////////////////////////////
// * PERFORMANCE HELPERS * //
/////////////////////////////
#pragma region Performance Helpers
namespace AWHPerf {
	template<typename Query, typename Counter>
	void CleanupPerfData(Query& query, Counter& counter) {
		if (query) {
			PdhCloseQuery(query);
			query = nullptr;
			counter = nullptr;
		}
	}

	HANDLE GetFoobarProcessHandle(bool& pidChanged) {
		pidChanged = false;

		// Try cached PID first
		HANDLE hProcess = nullptr;
		if (AWCPU::foobarCachedPid != 0) {
			hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, AWCPU::foobarCachedPid);
			if (!hProcess) {
				AWCPU::foobarCachedPid = 0;
				pidChanged = true;
			}
		}

		// Search for foobar2000.exe if no valid process handle
		if (!hProcess) {
			HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hSnapshot == INVALID_HANDLE_VALUE) {
				return nullptr;
			}

			PROCESSENTRY32W pe = { sizeof(pe) };
			if (Process32FirstW(hSnapshot, &pe)) {
				do {
					if (_wcsicmp(pe.szExeFile, L"foobar2000.exe") == 0) {
						hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pe.th32ProcessID);
						if (hProcess) {
							AWCPU::foobarCachedPid = pe.th32ProcessID;
							pidChanged = true;
							break;
						}
					}
				} while (Process32NextW(hSnapshot, &pe));
			}
			CloseHandle(hSnapshot);
		}

		return hProcess;
	}

	double GetCpuFoobarUsage(int refreshRate) {
		std::scoped_lock lock(AWCPU::foobarMutex);
		const ULONGLONG now = GetTickCount64();

		// Return cached value if within refresh rate
		if (now - AWCPU::foobarLastSampleTime < static_cast<ULONGLONG>(refreshRate)) {
			return AWCPU::foobarCachedCpu;
		}

		bool pidChanged = false;
		double cpuUsage = 0.0;
		FILETIME creationTime;
		FILETIME exitTime;
		FILETIME kernelTime;
		FILETIME userTime;

		HANDLE hProcess = GetFoobarProcessHandle(pidChanged);
		if (!hProcess) {
			AWCPU::foobarLastSampleTime = now;
			AWCPU::foobarCachedCpu = 0.0;
			return 0.0;
		}

		if (!GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
			AWCPU::foobarLastSampleTime = now;
			AWCPU::foobarCachedCpu = 0.0;
			return 0.0;
		}

		const ULONGLONG currentTotal = AWHConvert::FileTimeToUll(kernelTime) + AWHConvert::FileTimeToUll(userTime);

		if (!pidChanged && AWCPU::foobarLastSampleTime != 0) {
			const ULONGLONG timeDiff = currentTotal - AWCPU::foobarLastTotalTime;
			const ULONGLONG timeInterval = (now - AWCPU::foobarLastSampleTime) * 10000;

			if (timeInterval > 0) {
				const double cpuUsageRaw = (static_cast<double>(timeDiff) * 100.0) / (static_cast<double>(timeInterval) * AWCPU::numProcessors);
				cpuUsage = std::clamp(cpuUsageRaw, 0.0, 100.0);
			}
		}

		AWCPU::foobarLastTotalTime = currentTotal;
		AWCPU::foobarLastSampleTime = now;
		AWCPU::foobarCachedCpu = cpuUsage;

		return cpuUsage;
	}

	double GetCpuSystemUsage(int refreshRate) {
		std::scoped_lock lock(AWCPU::systemMutex);
		ULONGLONG now = GetTickCount64();

		if (now - AWCPU::systemLastSampleTime < refreshRate) {
			return AWCPU::systemCachedCpu;
		}

		if (!AWCPU::systemQuery) {
			if (PdhOpenQuery(nullptr, 0, &AWCPU::systemQuery) != ERROR_SUCCESS) {
				AWCPU::systemLastSampleTime = 0;
				return AWCPU::systemCachedCpu = 0.0;
			}

			if (PdhAddCounter(AWCPU::systemQuery, L"\\Processor Information(_Total)\\% Processor Utility", 0, &AWCPU::systemCounter) != ERROR_SUCCESS) {
				CleanupPerfData(AWCPU::systemQuery, AWCPU::systemCounter);
				AWCPU::systemLastSampleTime = 0;
				return AWCPU::systemCachedCpu = 0.0;
			}

			if (PdhCollectQueryData(AWCPU::systemQuery) != ERROR_SUCCESS) {
				CleanupPerfData(AWCPU::systemQuery, AWCPU::systemCounter);
				AWCPU::systemLastSampleTime = 0;
				return AWCPU::systemCachedCpu = 0.0;
			}

			AWCPU::systemLastSampleTime = now;
			return AWCPU::systemCachedCpu = 0.0;
		}

		if (PdhCollectQueryData(AWCPU::systemQuery) != ERROR_SUCCESS) {
			CleanupPerfData(AWCPU::systemQuery, AWCPU::systemCounter);
			AWCPU::systemCachedCpu = 0.0;
			AWCPU::systemLastSampleTime = 0;
			return AWCPU::systemCachedCpu;
		}

		PDH_FMT_COUNTERVALUE value;
		if (PdhGetFormattedCounterValue(AWCPU::systemCounter, PDH_FMT_DOUBLE, nullptr, &value) != ERROR_SUCCESS) {
			CleanupPerfData(AWCPU::systemQuery, AWCPU::systemCounter);
			AWCPU::systemCachedCpu = 0.0;
			AWCPU::systemLastSampleTime = 0;
			return AWCPU::systemCachedCpu;
		}

		AWCPU::systemCachedCpu = std::clamp(value.doubleValue, 0.0, 100.0);
		AWCPU::systemLastSampleTime = now;
		return AWCPU::systemCachedCpu;
	}

	std::pair<double, double> GetMemoryFoobarUsage(int refreshRate) {
		std::scoped_lock lock(AWRAM::foobarMutex);
		ULONGLONG now = GetTickCount64();

		if (now - AWRAM::foobarLastSampleTime < refreshRate) {
			return AWRAM::foobarCachedMemory;
		}

		if (!AWRAM::foobarQuery) {
			if (PdhOpenQuery(nullptr, 0, &AWRAM::foobarQuery) != ERROR_SUCCESS) {
				AWRAM::foobarLastSampleTime = 0;
				return { 0.0, 0.0 };
			}

			if (PdhAddCounter(AWRAM::foobarQuery, L"\\Process(foobar2000*)\\Working Set", 0, &AWRAM::foobarWSCounter) != ERROR_SUCCESS ||
				PdhAddCounter(AWRAM::foobarQuery, L"\\Process(foobar2000*)\\Private Bytes", 0, &AWRAM::foobarPBCounter) != ERROR_SUCCESS) {
				CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarWSCounter);
				CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarPBCounter);

				AWRAM::foobarQuery = nullptr;
				AWRAM::foobarLastSampleTime = 0;

				return { 0.0, 0.0 };
			}

			if (PdhCollectQueryData(AWRAM::foobarQuery) != ERROR_SUCCESS) {
				CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarWSCounter);
				CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarPBCounter);

				AWRAM::foobarQuery = nullptr;
				AWRAM::foobarLastSampleTime = 0;

				return { 0.0, 0.0 };
			}

			AWRAM::foobarLastSampleTime = now;
			return { 0.0, 0.0 };
		}

		if (PdhCollectQueryData(AWRAM::foobarQuery) != ERROR_SUCCESS) {
			CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarWSCounter);
			CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarPBCounter);

			AWRAM::foobarQuery = nullptr;
			AWRAM::foobarCachedMemory = { 0.0, 0.0 };
			AWRAM::foobarLastSampleTime = 0;

			return { 0.0, 0.0 };
		}

		PDH_FMT_COUNTERVALUE wsValue;
		PDH_FMT_COUNTERVALUE pbValue;
		if (PdhGetFormattedCounterValue(AWRAM::foobarWSCounter, PDH_FMT_DOUBLE, nullptr, &wsValue) != ERROR_SUCCESS ||
			PdhGetFormattedCounterValue(AWRAM::foobarPBCounter, PDH_FMT_DOUBLE, nullptr, &pbValue) != ERROR_SUCCESS) {

			CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarWSCounter);
			CleanupPerfData(AWRAM::foobarQuery, AWRAM::foobarPBCounter);

			AWRAM::foobarQuery = nullptr;
			AWRAM::foobarCachedMemory = { 0.0, 0.0 };
			AWRAM::foobarLastSampleTime = 0;

			return { 0.0, 0.0 };
		}

		AWRAM::foobarCachedMemory = {
			wsValue.doubleValue / (1024.0 * 1024.0),
			pbValue.doubleValue / (1024.0 * 1024.0)
		};

		AWRAM::foobarLastSampleTime = now;
		return AWRAM::foobarCachedMemory;
	}

	double GetMemorySystemUsage(int refreshRate) {
		std::scoped_lock lock(AWRAM::systemMutex);
		ULONGLONG now = GetTickCount64();

		if (now - AWRAM::systemLastSampleTime < refreshRate) {
			return AWRAM::systemCachedMemory;
		}

		MEMORYSTATUSEX memInfo{ sizeof(MEMORYSTATUSEX) };
		if (GlobalMemoryStatusEx(&memInfo)) {
			AWRAM::systemCachedMemory = std::clamp(
				(static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) * 100.0) /
				static_cast<double>(memInfo.ullTotalPhys), 0.0, 100.0
			);
			AWRAM::systemLastSampleTime = now;
		}
		else {
			AWRAM::systemCachedMemory = 0.0;
			AWRAM::systemLastSampleTime = 0;
		}

		return AWRAM::systemCachedMemory;
	}
}
#pragma endregion


////////////////////////
// * STRING HELPERS * //
////////////////////////
#pragma region String Helpers
namespace AWHString {
	bool EqualsIgnoreCase(const std::string& input, const char* compareValue) {
		return pfc::string8(input.c_str()).toLower() == compareValue;
	}

	std::string FormatDate(std::chrono::system_clock::time_point time, const char* format) {
		std::time_t time_t = std::chrono::system_clock::to_time_t(time);
		std::tm tm;
		localtime_s(&tm, &time_t);

		std::ostringstream oss;
		oss << std::put_time(&tm, format);

		return oss.str();
	}

	std::wstring FormatSampleRate(const double& sampleRate) {
		if (std::isnan(sampleRate) || sampleRate < 0) {
			return L"Invalid Sample Rate";
		}

		double sr_kHz = sampleRate / 1000.0;
		std::wstringstream ss;
		ss << std::fixed << std::setprecision(1) << sr_kHz << L" kHz";

		return ss.str();
	}

	CStringA FormatTimestampMs(double ms) {
		if (ms < 0 || ms > 2'147'483'647'000.0) {
			return CStringA("00:00:00.000");
		}

		auto total_ms = static_cast<int64_t>(ms);
		int64_t hours = total_ms / 3'600'000;
		total_ms -= hours * 3'600'000;
		int64_t minutes = total_ms / 60'000;
		total_ms -= minutes * 60'000;
		int64_t seconds = total_ms / 1'000;
		total_ms -= seconds * 1'000;
		int64_t milliseconds = total_ms;

		CStringA result;
		result.Format("%02lld:%02lld:%02lld.%03lld", hours, minutes, seconds, milliseconds);
		return result;
	}

	double GetElapsedTime(const std::chrono::steady_clock::time_point& startTime) {
		auto elapsed = std::chrono::steady_clock::now() - startTime;
		return std::chrono::duration<double>(elapsed).count();
	}

	std::wstring GetProcessingTime(const std::chrono::steady_clock::time_point& startTime, int precision) {
		double processingTime = GetElapsedTime(startTime);

		pfc::string8 timeStrUtf8 = pfc::format_time_ex(processingTime, precision);
		pfc::stringcvt::string_wide_from_utf8 timeWide(timeStrUtf8);

		return std::wstring(timeWide.get_ptr());
	}

	std::wstring GetProcessingSpeed(double totalDuration, const std::chrono::steady_clock::time_point& startTime, int precision) {
		double processingTime = GetElapsedTime(startTime);
		double speed = processingTime > 0 ? totalDuration / processingTime : 0.0;

		std::wstring speedStr(32, L'\0');
		int written = swprintf_s(speedStr.data(), speedStr.size(), L"%.*fx", precision, speed);
		written > 0 ? speedStr.resize(written) : speedStr.clear();

		return speedStr;
	}

	CStringW GetWindowTextCStringW(HWND hWnd) {
		CStringW text;
		const int textLength = GetWindowTextLengthW(hWnd);
		if (textLength <= 0) return text;

		GetWindowTextW(hWnd, text.GetBuffer(textLength + 1), textLength + 1);
		text.ReleaseBuffer();

		return text;
	}

	std::vector<CStringW> SplitString(const CStringW& input, const wchar_t* delimiter) {
		std::vector<CStringW> result;
		int pos = 0;
		CStringW token;

		while ((token = input.Tokenize(delimiter, pos)) != L"") {
			token.Trim();
			if (!token.IsEmpty()) result.push_back(token);
		}

		return result;
	}

	pfc::string8 ToFixed(int precision, double value) {
		std::ostringstream oss;
		oss << std::fixed << std::setprecision(precision) << value;
		return pfc::string8(oss.str().c_str());
	}

	pfc::string8 ToFixed(int precision, int value) {
		return ToFixed(precision, static_cast<double>(value));
	}

	CStringW ToFixedW(int precision, double value) {
		std::wostringstream woss;
		woss << std::fixed << std::setprecision(precision) << value;
		return CStringW(woss.str().c_str());
	}

	pfc::string8 ToLowerCase(const std::string& input) {
		return pfc::string8(input.c_str()).toLower();
	}

	pfc::string8 ToNarrow(const std::wstring& input) {
		pfc::stringcvt::string_utf8_from_wide_t<> converter(input.c_str(), input.length());
		return pfc::string8(converter.get_ptr());
	}

	std::wstring ToWide(const pfc::string8& input) {
		return std::wstring(pfc::stringcvt::string_wide_from_utf8(input.get_ptr()).get_ptr());
	}
};
#pragma endregion


//////////////////////
// * TEXT HELPERS * //
//////////////////////
#pragma region Text Helpers
namespace AWHText {
	HFONT CreateScaledFont(HFONT baseFont, HDC hdc, int ptDelta) {
		LOGFONT lf;
		GetObject(baseFont, sizeof(LOGFONT), &lf);

		int dpi = GetDeviceCaps(hdc, LOGPIXELSY);
		lf.lfHeight -= MulDiv(ptDelta, dpi, 72);

		return CreateFontIndirect(&lf);
	}

	CStringA FormatAlignedTextLine(const std::vector<CStringA>& values, const std::vector<int>& widths,
		const std::vector<bool>& isNumeric, const CStringA& separator) {
		CStringA line;

		for (size_t i = 0; i < values.size(); ++i) {
			int padding = widths[i] - values[i].GetLength();
			if (isNumeric[i]) {
				line.Append(CStringA(' ', std::max(0, padding)) + values[i]);
			}
			else {
				line.Append(values[i] + CStringA(' ', std::max(0, padding)));
			}
			if (i < values.size() - 1) {
				line.Append(separator);
			}
		}

		return line + "\r\n";
	}

	int GetFontHeight(HDC hdc, HFONT hFont) {
		if (!hFont) return 0;

		AWHGraphics::GDISelector fontSel(hdc, hFont);
		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);

		return tm.tmHeight;
	}

	void MeasureText(HDC hdc, const CStringW& text, CSize& size) {
		GetTextExtentPoint32W(hdc, text, text.GetLength(), &size);
	}

	int MeasureTextWidth(HDC hdc, const CStringW& text) {
		RECT rc = { 0, 0, 0, 0 };
		DrawTextW(hdc, text, text.GetLength(), &rc, DT_LEFT | DT_SINGLELINE | DT_CALCRECT | DT_NOPREFIX);
		return rc.right;
	}


	CStringA WriteFancyHeader(const CStringA& titlePrefix, const CStringA& date) {
		CStringA content("\xEF\xBB\xBF"); // UTF-8 BOM

		// Combine title and optional date
		CStringA fullTitle = titlePrefix + date;
		CStringA fullTitleUpper;
		for (const char* p = fullTitle; *p; ++p) {
			fullTitleUpper.AppendChar(static_cast<char>(toupper(*p)));
		}

		// Create decorative line
		CStringA line('/', fullTitle.GetLength() + 10);

		// Build header
		content.AppendFormat(
			"%s\r\n"
			"// * %s * //\r\n"
			"%s\r\n\r\n",
			line.GetString(),
			fullTitleUpper.GetString(),
			line.GetString()
		);

		return content;
	}
};
#pragma endregion
