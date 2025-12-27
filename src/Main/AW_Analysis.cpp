/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Analysis Header File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW_Analysis.h"
#include "AW_Helpers.h"


///////////////////////////////
// * ANALYSIS INTERPOLATOR * //
///////////////////////////////
#pragma region Analysis Interpolator
AudioWizardAnalysisInterpolator::AudioWizardAnalysisInterpolator(WindowType window, unsigned int taps, unsigned int factor, unsigned int channels) {
	interpolation.factor = factor;
	interpolation.taps = taps;
	interpolation.channels = channels;
	interpolation.delay = (taps + factor - 1) / factor;
	interpolation.filters.resize(factor);
	interpolation.z.resize(channels, std::vector<double>(interpolation.delay, 0.0));
	interpolation.zi = 0;

	// Reserve space for filters
	for (auto& filter : interpolation.filters) {
		filter.index.reserve(interpolation.delay);
		filter.coeff.reserve(interpolation.delay);
	}

	// Generate filter coefficients
	SetInterpolatorWindow(window);

	// Normalize each phase's coefficients
	for (auto& filter : interpolation.filters) {
		double sum = std::accumulate(filter.coeff.begin(), filter.coeff.end(), 0.0);
		if (sum != 0.0) {
			for (double& c : filter.coeff) {
				c /= sum;
			}
		}
	}

	// Initialize delay buffers
	for (auto& channel_z : interpolation.z) {
		channel_z.resize(interpolation.delay, 0.0);
	}
}

size_t AudioWizardAnalysisInterpolator::ProcessInterpolation(size_t frames, const audioType* in, audioType* out) {
	for (size_t frame = 0; frame < frames; ++frame) {
		// Store input samples in delay buffers
		for (unsigned int chan = 0; chan < interpolation.channels; ++chan) {
			interpolation.z[chan][interpolation.zi] = *in++;
		}
		// Generate interpolated samples for each phase
		for (unsigned int f = 0; f < interpolation.factor; ++f) {
			for (unsigned int chan = 0; chan < interpolation.channels; ++chan) {
				auto acc = 0.0;
				const auto& filter = interpolation.filters[f];
				for (size_t t = 0; t < filter.index.size(); ++t) {
					int i = static_cast<int>(interpolation.zi) - static_cast<int>(filter.index[t]);
					if (i < 0) i += static_cast<int>(interpolation.delay);
					acc += interpolation.z[chan][i] * filter.coeff[t];
				}
				*out++ = static_cast<audioType>(acc);
			}
		}
		interpolation.zi = (interpolation.zi + 1) % interpolation.delay;
	}
	return frames * interpolation.factor;
}

double AudioWizardAnalysisInterpolator::CalculateTruePeakLinear(const ChunkData& chkData, AudioWizardAnalysisInterpolator* interp) {
	const size_t totalOriginalSamples = chkData.frames * chkData.channels;

	if (totalOriginalSamples == 0 || chkData.data == nullptr) {
		return 0.0;
	}

	const audioType* const data = chkData.data;
	audioType truePeak = 0.0;

	// Compute peak of original samples
	for (size_t i = 0; i < totalOriginalSamples; ++i) {
		truePeak = std::max(truePeak, std::abs(data[i]));
	}

	if (!interp || interp->interpolation.channels != chkData.channels || interp->interpolation.factor <= 0) {
		return truePeak;
	}

	// Process interpolation
	const size_t BLOCK_SIZE = 4096;
	std::vector<audioType> block;
	block.reserve(BLOCK_SIZE * interp->interpolation.factor * chkData.channels);

	for (size_t offset = 0; offset < chkData.frames; offset += BLOCK_SIZE) {
		const size_t block_frames = std::min(BLOCK_SIZE, chkData.frames - offset);
		block.resize(block_frames * interp->interpolation.factor * chkData.channels);
		interp->ProcessInterpolation(block_frames, data + offset * chkData.channels, block.data());
		// Find peak in interpolated samples
		for (audioType sample : block) {
			truePeak = std::max(truePeak, std::abs(sample));
		}
	}

	return truePeak;
}

void AudioWizardAnalysisInterpolator::SetInterpolatorWindow(WindowType window) {
	constexpr double PI = 3.14159265358979323846;
	constexpr double ALMOST_ZERO = 0.000001;

	// Generate window
	double beta = (window == WindowType::KAISER) ? 5.0 : 0.0; // Fixed beta for Kaiser as in original
	std::vector<double> windowVec = AWHAudioDSP::GenerateAudioWindow(window, interpolation.taps, beta);

	// Generate filter coefficients
	for (unsigned int j = 0; j < interpolation.taps; j++) {
		double m = static_cast<double>(j) - (interpolation.taps - 1.0) / 2.0;
		double c = 1.0;
		if (std::abs(m) > ALMOST_ZERO) {
			c = std::sin(m * PI / interpolation.factor) / (m * PI / interpolation.factor);
		}
		c *= windowVec[j]; // Apply window

		if (std::abs(c) > ALMOST_ZERO) {
			unsigned int f = j % interpolation.factor;
			interpolation.filters[f].index.push_back(j / interpolation.factor);
			interpolation.filters[f].coeff.push_back(c);
		}
	}
}
#pragma endregion


/////////////////////////
// * ANALYSIS FILTER * //
/////////////////////////
#pragma region Analysis Filter
inline double AudioWizardAnalysisFilter::ApplyFilter(const FilterCoeffs& coeffs, FilterState& state, double sample) {
	const double result = coeffs.b0 * sample
		+ coeffs.b1 * state.x1
		+ coeffs.b2 * state.x2
		- coeffs.a1 * state.y1
		- coeffs.a2 * state.y2;

	state.x2 = state.x1;
	state.x1 = sample;
	state.y2 = state.y1;
	state.y1 = result;

	return result;
}

void AudioWizardAnalysisFilter::DesignKWeightedPreFilter(FilterCoeffs& coeffs, double sampleRate) {
	auto it = kWeightedPreFilterCoeffs.find(sampleRate);

	if (it != kWeightedPreFilterCoeffs.end()) {
		coeffs = it->second;
	}
	else {
		// EBU R128 parameters for high-shelf filter
		const double f0 = 1681.974450955533; // Shelf center frequency
		const double G = 3.999843853973347;  // 4 dB gain
		const double Q = 0.7071752369554196; // Quality factor

		constexpr double PI = 3.14159265358979323846;
		const double K = std::tan(PI * f0 / sampleRate);
		const double Vh = std::pow(10.0, G / 20.0);
		const double Vb = std::pow(Vh, 0.4996667741545416);
		const double common = 1.0 + K / Q + K * K;

		coeffs.b0 = (Vh + Vb * K / Q + K * K) / common;
		coeffs.b1 = 2.0 * (K * K - Vh) / common;
		coeffs.b2 = (Vh - Vb * K / Q + K * K) / common;
		coeffs.a1 = 2.0 * (K * K - 1.0) / common;
		coeffs.a2 = (1.0 - K / Q + K * K) / common;
	}
}

void AudioWizardAnalysisFilter::DesignKWeightedRLBFilter(FilterCoeffs& coeffs, double sampleRate) {
	auto it = kWeightedRLBFilterCoeffs.find(sampleRate);

	if (it != kWeightedRLBFilterCoeffs.end()) {
		coeffs = it->second;
	}
	else {
		// EBU R128 parameters for high-pass filter
		const double f0 = 38.13547087602444; // Cutoff frequency
		const double Q = 0.5003270373238773; // Quality factor

		constexpr double PI = 3.14159265358979323846;
		const double K = std::tan(PI * f0 / sampleRate);
		const double common = 1.0 + K / Q + K * K;

		coeffs.b0 = 1.0;
		coeffs.b1 = -2.0;
		coeffs.b2 = 1.0;
		coeffs.a1 = 2.0 * (K * K - 1.0) / common;
		coeffs.a2 = (1.0 - K / Q + K * K) / common;
	}
}

double AudioWizardAnalysisFilter::GetChannelWeight(size_t index, size_t channelCount) {
	// Channel layouts directly using BS.2217-2 labels and weights
	static const std::unordered_map<size_t, std::pair<std::vector<std::string>, std::vector<double>>> LAYOUTS = {
		{ 1, {{ "Mono" },
			 { 1.00 }}},
		{ 2, {{ "L", "R" },
			 { 1.00, 1.00 }}},
		{ 5, {{ "L", "R", "C", "Ls", "Rs" },
			 { 1.00, 1.00, 1.00, 1.41, 1.41 }}},
		{ 6, {{ "L", "R", "C", "LFE", "Ls", "Rs" },
			 { 1.00, 1.00, 1.00, 0.00, 1.41, 1.41 }}},
		{ 8, {{ "L", "R", "C", "LFE", "Lss", "Rss", "Lrs", "Rrs" },
			 { 1.00, 1.00, 1.00, 0.00, 1.41, 1.41, 1.00, 1.00 }}},
		{ 10, {{ "L", "R", "C", "LFE", "Ls", "Rs", "Tfl", "Tfr", "Tbr", "Tbr" },
			  { 1.00, 1.00, 1.00, 0.00, 1.41, 1.41, 1.00, 1.00, 1.00, 1.00 }}},
		{ 12, {{ "L", "R", "C", "LFE", "Lss", "Rss", "Lrs", "Rrs", "Tfl", "Tfr", "Tbr", "Tbr" },
			  { 1.00, 1.00, 1.00, 0.00, 1.41, 1.41, 1.00, 1.00, 1.00, 1.00, 1.00, 1.00 }}},
		{ 24, {
			{ "FL", "FR", "FC", "LFE1", "BL", "BR",
			  "FLc", "FRc", "BC", "LFE2", "SiL", "SiR",
			  "TpFL", "TpFR", "TpFC", "TpC", "TpBL", "TpBR",
			  "TpSiL", "TpSiR", "TpBC", "BtFC", "BtFL", "BtFR"},
			{ 1.41, 1.41, 1.00, 0.00, 1.00, 1.00,
			  1.00, 1.00, 1.00, 0.00, 1.41, 1.41,
			  1.00, 1.00, 1.00, 1.00, 1.00, 1.00,
			  1.00, 1.00, 1.00, 1.00, 1.00, 1.00 }
		}}
	};

	auto it = LAYOUTS.find(channelCount);
	if (it == LAYOUTS.end() || index >= it->second.second.size()) {
		return 1.0;
	}

	return it->second.second[index];
}

void AudioWizardAnalysisFilter::ProcessKWeightedChunk(const ChunkData& chkData, FilterData& ftData, std::vector<double>& buffer) {
	if (!chkData.data || chkData.sampleRate <= 0.0 || chkData.frames * chkData.channels == 0) {
		return;
	}

	if (ftData.sampleRate != chkData.sampleRate || ftData.channelWeights.size() != chkData.channels) {
		ftData.sampleRate = chkData.sampleRate;
		ftData.channelWeights.resize(chkData.channels);
		ftData.preFilterCoeffs.resize(chkData.channels);
		ftData.preFilterStates.resize(chkData.channels);
		ftData.rlbFilterCoeffs.resize(chkData.channels);
		ftData.rlbFilterStates.resize(chkData.channels);

		for (size_t ch = 0; ch < chkData.channels; ++ch) {
			ftData.channelWeights[ch] = GetChannelWeight(ch, chkData.channels);
		}

		if (chkData.channels > 0) {
			DesignKWeightedPreFilter(ftData.preFilterCoeffs[0], ftData.sampleRate);
			DesignKWeightedRLBFilter(ftData.rlbFilterCoeffs[0], ftData.sampleRate);

			for (size_t ch = 1; ch < chkData.channels; ++ch) {
				ftData.preFilterCoeffs[ch] = ftData.preFilterCoeffs[0];
				ftData.rlbFilterCoeffs[ch] = ftData.rlbFilterCoeffs[0];
			}
		}
	}

	buffer.reserve(buffer.size() + chkData.frames);

	for (size_t i = 0; i < chkData.frames; ++i) {
		double totalPower = 0.0;
		const size_t baseIdx = i * chkData.channels;

		for (size_t ch = 0; ch < chkData.channels; ++ch) {
			const size_t idx = baseIdx + ch;
			const double sample = chkData.data[idx];

			const double afterPre = ApplyFilter(ftData.preFilterCoeffs[ch], ftData.preFilterStates[ch], sample);
			const double afterRLB = ApplyFilter(ftData.rlbFilterCoeffs[ch], ftData.rlbFilterStates[ch], afterPre);

			totalPower += ftData.channelWeights[ch] * (afterRLB * afterRLB);
		}

		buffer.push_back(totalPower);
	}
}

void AudioWizardAnalysisFilter::InitInterpolation(const ChunkData& chkData, FilterData& ftData) {
	unsigned int oversamplingFactor = (chkData.sampleRate < 96000.0) ? 4 : (chkData.sampleRate < 192000.0) ? 2 : 1;
	const unsigned int tapsPerPhase = 48; // 32, 40, 48, 56, 64
	const unsigned int totalTaps = oversamplingFactor * tapsPerPhase + 1; // +1 making it odd for filter symmetry

	if (oversamplingFactor > 1) {
		ftData.interp = std::make_unique<AudioWizardAnalysisInterpolator>(
			AudioWizardAnalysisInterpolator::WindowType::KAISER, totalTaps, oversamplingFactor, chkData.channels
		);
	}
	else {
		ftData.interp.reset();
	}
}
#pragma endregion


///////////////////////////////////////
// * ANALYSIS FULL-TRACK - METRICS * //
///////////////////////////////////////
#pragma region Analysis Full-Track - Metrics
double AudioWizardAnalysisFullTrack::GetMomentaryLUFSFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0) return -INFINITY;
	return ftData.momentaryLUFS;
}

double AudioWizardAnalysisFullTrack::GetShortTermLUFSFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0) return -INFINITY;
	return ftData.shortTermLUFS;
}

double AudioWizardAnalysisFullTrack::GetIntegratedLUFSFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0 || ftData.histogramOfBlockLoudness.empty()) return -INFINITY;

	// Convert vector index to LUFS: index = (lkfs * 10) + 700
	constexpr double MIN_LUFS = -70.0;
	constexpr double BIN_SIZE = 0.1;
	constexpr int OFFSET = 700; // -70.0 * 10 = -700 -> index 0

	// Step 1: Calculate relative threshold
	double sumWeighted = 0.0;
	int totalCount = 0;

	for (size_t bin = 0; bin < ftData.histogramOfBlockLoudness.size(); ++bin) {
		const int count = ftData.histogramOfBlockLoudness[bin];
		if (count == 0) continue;

		const double lkfs = (static_cast<int>(bin) - OFFSET) * BIN_SIZE;
		sumWeighted += std::pow(10.0, (lkfs + 0.691) / 10.0) * count;
		totalCount += count;
	}

	if (totalCount == 0) return MIN_LUFS;
	const double relativeThreshold = -0.691 + AWHAudio::PowerToDb(sumWeighted / totalCount) - 10.0;

	// Step 2: Sum blocks above relative threshold
	const auto thresholdBin = static_cast<int>((relativeThreshold * 10) + OFFSET);
	const size_t startBin = std::max(0, thresholdBin);

	sumWeighted = 0.0;
	totalCount = 0;

	for (size_t bin = startBin; bin < ftData.histogramOfBlockLoudness.size(); ++bin) {
		const int count = ftData.histogramOfBlockLoudness[bin];
		if (count == 0) continue;

		const double lkfs = (static_cast<int>(bin) - OFFSET) * BIN_SIZE;
		sumWeighted += std::pow(10.0, (lkfs + 0.691) / 10.0) * count;
		totalCount += count;
	}

	return totalCount > 0 ? (-0.691 + AWHAudio::PowerToDb(sumWeighted / totalCount)) : MIN_LUFS;
}

double AudioWizardAnalysisFullTrack::GetRMSFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0) return -INFINITY;
	double meanSquare = ftData.originalSquaresSum / ftData.originalSampleCount;
	return AWHAudio::LinearToDb(sqrt(meanSquare));
}

double AudioWizardAnalysisFullTrack::GetSamplePeakFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0) return -INFINITY;
	return AWHAudio::LinearToDb(ftData.samplePeakMaxLinear);
}

double AudioWizardAnalysisFullTrack::GetTruePeakFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0) return -INFINITY;
	return AWHAudio::LinearToDb(ftData.truePeakMaxLinear);
}

double AudioWizardAnalysisFullTrack::GetPSRFull(const FullTrackData& ftData) {
	double shortTermLUFS = GetShortTermLUFSFull(ftData);

	if (ftData.originalSampleCount == 0 || shortTermLUFS == -INFINITY) return 0.0;

	double truePeakDb = AWHAudio::LinearToDb(ftData.truePeakMaxLinear);
	return truePeakDb - shortTermLUFS;
}

double AudioWizardAnalysisFullTrack::GetPLRFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0) return -INFINITY;
	double truePeakDb = AWHAudio::LinearToDb(ftData.truePeakMaxLinear);
	return truePeakDb - GetIntegratedLUFSFull(ftData);
}

double AudioWizardAnalysisFullTrack::GetCrestFactorFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0 || ftData.samplePeakMaxLinear == 0.0) return -INFINITY;

	double meanSquare = ftData.originalSquaresSum / ftData.originalSampleCount;
	double rms = std::sqrt(meanSquare);

	return AWHAudio::LinearToDb(ftData.samplePeakMaxLinear / rms);
}

double AudioWizardAnalysisFullTrack::GetLoudnessRangeFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0 || ftData.histogramOfBlockLoudnessLRA.empty()) return -INFINITY;

	constexpr double HISTOGRAM_OFFSET = -70.0; // Bin 0 = -70 LUFS
	constexpr double HISTOGRAM_STEP = 0.1;     // 0.1 LUFS per bin
	constexpr double LUFS_CONSTANT = 0.691;    // EBU R128 constant

	// Precompute LUFS and power values
	std::vector<std::pair<double, double>> binData(ftData.histogramOfBlockLoudnessLRA.size());
	for (size_t bin = 0; bin < binData.size(); ++bin) {
		double lufs = HISTOGRAM_OFFSET + bin * HISTOGRAM_STEP;
		binData[bin] = { lufs, std::pow(10.0, (lufs + LUFS_CONSTANT) / 10.0) };
	}

	// Step 1: Compute ungated loudness (blocks above -70 LUFS)
	double sumPower = 0.0;
	int totalCount = 0;
	for (size_t bin = 0; bin < ftData.histogramOfBlockLoudnessLRA.size(); ++bin) {
		int count = ftData.histogramOfBlockLoudnessLRA[bin];
		if (count == 0) continue;
		sumPower += binData[bin].second * static_cast<double>(count);
		totalCount += count;
	}

	if (totalCount == 0) return 0.0;

	double averagePower = sumPower / static_cast<double>(totalCount);
	double ungatedLUFS = -LUFS_CONSTANT + AWHAudio::PowerToDb(averagePower);
	double relativeThreshold = ungatedLUFS - 20.0;

	// Step 2: Build gated bins (only above threshold)
	auto startBin = static_cast<size_t>(std::max(0.0, std::ceil((relativeThreshold - HISTOGRAM_OFFSET) / HISTOGRAM_STEP)));
	std::vector<std::pair<double, int>> gatedBins;
	gatedBins.reserve(ftData.histogramOfBlockLoudnessLRA.size() - startBin);
	int gatedCount = 0;

	for (size_t bin = startBin; bin < ftData.histogramOfBlockLoudnessLRA.size(); ++bin) {
		int count = ftData.histogramOfBlockLoudnessLRA[bin];
		if (count > 0) {
			gatedBins.emplace_back(binData[bin].first, count);
			gatedCount += count;
		}
	}

	if (gatedCount == 0) return 0.0;

	// Step 3: Compute percentiles
	auto getPercentile = [&](double percentile) {
		auto targetIdx = static_cast<int>(std::round(percentile * (static_cast<double>(gatedCount) - 1.0)));
		for (const auto& [lufs, count] : gatedBins) {
			targetIdx -= count;
			if (targetIdx < 0) return lufs;
		}
		return gatedBins.back().first;
	};

	double lufsLow = getPercentile(0.10);
	double lufsHigh = getPercentile(0.95);
	double lra = lufsHigh - lufsLow;

	return std::max(0.0, lra);
}

double AudioWizardAnalysisFullTrack::GetDynamicRangeFull(const FullTrackData& ftData) {
	if (ftData.originalRMSLinearLeft.empty() || ftData.originalPeakLinearLeft.empty() ||
		ftData.originalRMSLinearRight.empty() || ftData.originalPeakLinearRight.empty() ||
		ftData.originalSampleCount == 0 || ftData.originalSampleCount < static_cast<size_t>(ftData.sampleRate * 3.0)) {
		return 0.0;
	}

	constexpr double MIN_RMS_LINEAR = 1e-5;  // Minimum RMS to avoid log(0)
	constexpr double TOP_RMS_FRACTION = 0.2; // Top 20% RMS blocks per DR14 spec
	std::vector<size_t> drIndices;
	drIndices.reserve(std::max(ftData.originalRMSLinearLeft.size(), ftData.originalRMSLinearRight.size()));

	// Computes DR14 for one channel: selects top 20% RMS blocks and second-largest peak, per DRMeter manual.
	auto calculateDR = [&drIndices](const std::vector<double>& rmsLinear, const std::vector<double>& peakLinear) {
		const size_t numBlocks = rmsLinear.size();
		if (numBlocks < 2) return 0.0;

		if (drIndices.size() < numBlocks) {
			drIndices.resize(numBlocks);
			std::iota(drIndices.begin(), drIndices.begin() + numBlocks, 0);
		}

		// Peak detection
		double max1 = 0.0;
		double max2 = 0.0;
		for (double peak : peakLinear) {
			if (peak > max1) {
				max2 = max1;
				max1 = peak;
			}
			else if (peak > max2) {
				max2 = peak;
			}
		}
		double secondLargestPeakLinear = std::max(max2, MIN_RMS_LINEAR);

		// Top-20% RMS selection
		const auto top20 = static_cast<size_t>(std::ceil(TOP_RMS_FRACTION * numBlocks));
		if (top20 == 0) return 0.0;

		// Partition for top20 RMS blocks
		std::nth_element(drIndices.begin(), drIndices.begin() + top20, drIndices.begin() + numBlocks, [&](size_t a, size_t b) {
			return rmsLinear[a] > rmsLinear[b];
		});

		// Calculate RMS of top blocks
		double sumSquares = 0.0;
		for (size_t i = 0; i < top20; ++i) {
			sumSquares += rmsLinear[drIndices[i]] * rmsLinear[drIndices[i]];
		}

		// From DRMeter manual (DRMeter_UM.pdf, p. 19, RMS Meters):
		// "The RMS value is corrected by + 3dB so that sine waves have the same peak and RMS value,
		// as is the case with most other RMS meters."
		// Standard RMS = peak / sqrt(2) ≈ peak / 1.4142135623730951, so corrected RMS = sqrt(2) * standardRMS.
		// In dB: 20 * log10(sqrt(2)) ≈ 3.0103.
		constexpr double DR14_RMS_CORRECTION = 3.0103;
		double overallRMSLinearSquared = std::max(sumSquares / top20, MIN_RMS_LINEAR * MIN_RMS_LINEAR);
		double overallRMSdB = 0.5 * AWHAudio::LinearToDb(overallRMSLinearSquared) + DR14_RMS_CORRECTION;

		return AWHAudio::LinearToDb(secondLargestPeakLinear) - overallRMSdB;
	};

	double drLeft = calculateDR(ftData.originalRMSLinearLeft, ftData.originalPeakLinearLeft);
	double drRight = calculateDR(ftData.originalRMSLinearRight, ftData.originalPeakLinearRight);

	// No rounding to have more precise vs the official algorithm std::round((drLeft + drRight)
	return std::max(0.0, (drLeft + drRight) / 2.0);
}

double AudioWizardAnalysisFullTrack::GetPureDynamicsFull(const FullTrackData& ftData) {
	if (ftData.originalSampleCount == 0 || ftData.pureDynamicsBlockSums.size() == 0 ||
		ftData.stepSize == 0 || ftData.sampleRate <= 0.0) {
		return -INFINITY;
	}

	// Pipeline order:
	// Correction > Adaptation > Binaural > Transient Detection > Cognitive > Transient Application > Spread mirrors auditory processing:
	// Peripheral > Spatial > Temporal > Cognitive > Integrative

	// Step 0: Initialize dynamics struct and loudness
	FullTrackDataDynamics dynamics;
	ProcessDynamicsInitialization(ftData, dynamics);

	// Step 1: Correct loudness for perceptual factors (psychoacoustic: Zwicker model)
	// Produces: dynamics.correctedLoudness
	ProcessDynamicsLoudnessCorrection(ftData, dynamics);

	// Step 2: Compute preliminary transient score
	// Sets: dynamics.transientScore
	ProcessDynamicsPreliminaryTransient(dynamics);

	// Step 3: Adapt loudness for temporal habituation (psychoacoustic: neural adaptation)
	// Updates: dynamics.adaptedLoudness
	ProcessDynamicsLoudnessAdaptation(dynamics);

	// Step 4: Adjust for binaural perception (psychoacoustic: spatial cues)
	// Modifies: dynamics.adaptedLoudness
	ProcessDynamicsBinauralAdjustment(dynamics);

	// Step 5: Detect transients for cognitive loudness (psychoacoustic: onset salience)
	// Sets: dynamics.transientBoosts
	ProcessDynamicsTransientBoostsDetection(dynamics);

	// Step 6: Apply cognitive loudness adjustments (psychoacoustic: attention, genre weighting)
	// Adjusts: dynamics.adaptedLoudness
	ProcessDynamicsCognitiveLoudness(ftData, dynamics);

	// Step 7: Apply transient boosts to enhance loudness (psychoacoustic: transient amplification)
	// Finalizes: dynamics.adaptedLoudness
	ProcessDynamicsTransientBoostsAdjustment(dynamics);

	// Step 8: Compute dynamic spread (psychoacoustic: integrative dynamic range)
	// Sets: dynamics.pureDynamics (final value)
	ProcessDynamicsSpread(dynamics);

	return dynamics.pureDynamics;
}

std::map<std::wstring, double> AudioWizardAnalysisFullTrack::GetAlbumMetricFull(
	const std::vector<FullTrackResults>& results, const std::function<double(const FullTrackResults&)>& metricAccessor) {
	std::map<std::wstring, double> albumMetrics;

	if (results.empty()) return albumMetrics;

	// Group tracks by album
	std::map<std::wstring, std::vector<const FullTrackResults*>> albumGroups;
	for (const auto& result : results) {
		albumGroups[result.album].push_back(&result);
	}

	// Compute average metric for each album
	for (const auto& [album, tracks] : albumGroups) {
		double sumMetric = 0.0;
		size_t count = tracks.size();
		if (count == 0) continue;

		for (const auto* result : tracks) {
			sumMetric += metricAccessor(*result);
		}
		albumMetrics[album] = sumMetric / count;
	}

	return albumMetrics;
}
#pragma endregion


///////////////////////////////////////////////////
// * ANALYSIS FULL-TRACK - DYNAMICS PROCESSING * //
///////////////////////////////////////////////////
#pragma region Analysis Full-Track - Dynamics Processing
void AudioWizardAnalysisFullTrack::ProcessDynamicsInitialization(const FullTrackData& ftData, FullTrackDataDynamics& dynamics) {
	// Configuration and Metadata
	dynamics.blockCount = ftData.pureDynamicsBlockSums.size();
	dynamics.blockDurationMs = (static_cast<double>(ftData.stepSize) / ftData.sampleRate) * 1000.0;

	// Dynamics Metrics
	dynamics.genreFactor = 0.5;
	dynamics.spectralCentroidMean = 0.0;
	dynamics.spectralFlatnessMean = 0.0;
	dynamics.spectralFluxMean = 0.0;
	dynamics.transientScore = 0.0;
	dynamics.transientDensity = 0.0;
	dynamics.pureDynamics = 0.0;

	// Initialize spectral vectors for genre factor computation
	dynamics.spectralCentroid.resize(dynamics.blockCount, 0.0);
	dynamics.spectralFlatness.resize(dynamics.blockCount, 1.0);
	dynamics.spectralFlux.resize(dynamics.blockCount, 0.0);

	// Copy spectral factors from ftData
	dynamics.spectralCentroid.assign(ftData.spectralCentroid.begin(), ftData.spectralCentroid.begin() + dynamics.blockCount);
	dynamics.spectralFlatness.assign(ftData.spectralFlatness.begin(), ftData.spectralFlatness.begin() + dynamics.blockCount);
	dynamics.spectralFlux.assign(ftData.spectralFlux.begin(), ftData.spectralFlux.begin() + dynamics.blockCount);

	// Compute genre-adaptive spectral factors
	AWHAudioFFT::ComputeSpectralGenreFactors(
		dynamics.spectralCentroid, dynamics.spectralFlatness, dynamics.spectralFlux,
		dynamics.spectralCentroidMean, dynamics.spectralFlatnessMean, dynamics.spectralFluxMean, dynamics.genreFactor
	);

	// Processing Vectors
	dynamics.validLoudness.clear();
	dynamics.validLoudness.reserve(dynamics.blockCount);
	dynamics.correctedLoudness.resize(dynamics.blockCount, -INFINITY);
	dynamics.adaptedLoudness.resize(dynamics.blockCount, -INFINITY);
	dynamics.transientBoosts.resize(dynamics.blockCount, 1.0);
	dynamics.phrasingScore.resize(dynamics.blockCount, 0.0);

	// Loudness Metrics
	dynamics.integratedLUFS = GetIntegratedLUFSFull(ftData);
	const double offset = -20.0 + 5.0 * (1.0 - dynamics.genreFactor); // -20.0 to -17.0
	const double silenceThreshold = std::max(dynamics.integratedLUFS + offset, -70.0);
	dynamics.blockLoudness = AWHAudioDynamics::ComputeBaseBlockLoudness(
		ftData.pureDynamicsBlockSums, ftData.stepSize, dynamics.integratedLUFS, silenceThreshold, &dynamics.validLoudness
	);
	dynamics.variance = AWHMath::CalculateVarianceOnline(dynamics.blockLoudness);
	double varianceDenom = 30.0 + 40.0 * dynamics.genreFactor + 0.2 * dynamics.variance; // 30–70 dB², genre-adjusted
	dynamics.varianceScale = std::tanh(std::max(1.0, dynamics.variance / varianceDenom));

	// Psychoacoustic Factors
	dynamics.criticalBandFactor.resize(dynamics.blockCount, 1.0);
	dynamics.harmonicComplexityFactor.resize(dynamics.blockCount, 0.0);
	dynamics.maskingFactor.resize(dynamics.blockCount, 1.0);
	dynamics.frequencyPowers.resize(dynamics.blockCount, 0.0);
	dynamics.binauralFactor.resize(dynamics.blockCount, 1.0);

	// Copy remaining psychoacoustic factors from ftData
	dynamics.criticalBandFactor.assign(ftData.criticalBandFactor.begin(), ftData.criticalBandFactor.begin() + dynamics.blockCount);
	dynamics.harmonicComplexityFactor.assign(ftData.harmonicComplexityFactor.begin(), ftData.harmonicComplexityFactor.begin() + dynamics.blockCount);
	dynamics.maskingFactor.assign(ftData.maskingFactor.begin(), ftData.maskingFactor.begin() + dynamics.blockCount);
	dynamics.frequencyPowers.assign(ftData.frequencyPowers.begin(), ftData.frequencyPowers.begin() + dynamics.blockCount);
	dynamics.binauralFactor.assign(ftData.binauralFactor.begin(), ftData.binauralFactor.begin() + dynamics.blockCount);
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsLoudnessCorrection(const FullTrackData& ftData, FullTrackDataDynamics& dynamics) {
	if (ftData.bandPowers.size() < dynamics.blockCount || dynamics.blockLoudness.size() != dynamics.blockCount) {
		dynamics.correctedLoudness.assign(dynamics.blockCount, -INFINITY);
		dynamics.validLoudness.clear();
		return;
	}

	dynamics.validLoudness.clear();
	dynamics.validLoudness.reserve(dynamics.blockCount);

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.blockLoudness[i] <= -90.0 || dynamics.blockLoudness[i] == -INFINITY) {
			dynamics.correctedLoudness[i] = -80.0; // Music-specific floor
			continue;
		}
		dynamics.correctedLoudness[i] = dynamics.blockLoudness[i];
		dynamics.validLoudness.push_back(dynamics.correctedLoudness[i]);
	}

	std::vector<double> fastlAdjustments;
	AWHAudioDynamics::ComputeFastlPrinciples(false, fastlAdjustments,
		ftData.bandPowers, dynamics.blockLoudness, ftData.stepSize, ftData.sampleRate,
		dynamics.variance, dynamics.varianceScale, {}
	);

	AWHAudioDynamics::ComputePerceptualLoudnessCorrection(false, fastlAdjustments,
		dynamics.blockLoudness, dynamics.frequencyPowers, dynamics.integratedLUFS, dynamics.variance,
		dynamics.correctedLoudness, dynamics.validLoudness, &ftData.pureDynamicsBlockSums, ftData.stepSize
	);

	size_t validIdx = 0;
	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.correctedLoudness[i] == -INFINITY || dynamics.correctedLoudness[i] == -100.0) continue;

		if (ftData.bandPowers[i].size() == AWHAudioFFT::BARK_BAND_NUMBER) {
			double totalSpecificLoudness = 0.0;
			for (size_t b = 0; b < AWHAudioFFT::BARK_BAND_NUMBER; ++b) {
				double intensity = ftData.bandPowers[i][b];
				double thresh = AWHAudioFFT::BARK_QUIET_THRESHOLD_INTENSITIES[b] * (1.0 + ftData.maskingFactor[i]);
				double ratio = intensity / (thresh + AWHAudioFFT::EPSILON);
				double N_prime = AWHAudioFFT::SPECIFIC_LOUDNESS_CONST * (std::pow(ratio, 0.23) - 1.0);
				if (N_prime > 0.0) totalSpecificLoudness += N_prime;
			}

			double loudnessLevel = 40.0 + 10.0 * std::log2(totalSpecificLoudness + AWHAudioFFT::EPSILON);
			double adjustment = loudnessLevel - dynamics.correctedLoudness[i];

			// Dynamic clamping based on perceptual loudness variance (music-tuned)
			double loudnessVariance = AWHMath::CalculateVarianceOnline(dynamics.validLoudness);
			double clampLimit = 12.0 + 8.0 * std::tanh(loudnessVariance / 25.0); // 12–20 dB
			adjustment = std::clamp(adjustment, -clampLimit, clampLimit);

			double genreWeight = 0.7 + 0.3 * dynamics.genreFactor; // 0.7–1.0, refined for music
			double varAdjust = 0.5 * std::tanh(dynamics.variance / 40.0); // Smoother music adaptation
			dynamics.correctedLoudness[i] += adjustment * genreWeight * (1.0 - varAdjust);
		}

		if (validIdx < dynamics.validLoudness.size()) {
			dynamics.validLoudness[validIdx] = dynamics.correctedLoudness[i];
			validIdx++;
		}
	}
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsPreliminaryTransient(FullTrackDataDynamics& dynamics) {
	double transientScore = 0.0;
	double transientCount = 0.0;
	std::vector<double> validLoudness;
	validLoudness.reserve(dynamics.blockCount);

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.correctedLoudness[i] != -INFINITY) validLoudness.push_back(dynamics.correctedLoudness[i]);
	}

	// Adaptive window size based on musical tempo (50–120 ms)
	double tempoFactor = std::clamp(dynamics.spectralFluxMean / 0.15, 0.5, 1.5);
	auto windowSize = static_cast<size_t>(std::round((50.0 + 70.0 * dynamics.genreFactor) / dynamics.blockDurationMs * tempoFactor));

	for (size_t i = windowSize; i < dynamics.blockCount; ++i) {
		double localSum = 0.0;
		double localSumSq = 0.0;
		size_t localCount = 0;
		for (size_t j = i - windowSize; j <= i; ++j) {
			if (dynamics.correctedLoudness[j] != -INFINITY) {
				localSum += dynamics.correctedLoudness[j];
				localSumSq += dynamics.correctedLoudness[j] * dynamics.correctedLoudness[j];
				localCount++;
			}
		}
		if (localCount < 2) continue;

		double localMean = localSum / localCount;
		double localVariance = (localSumSq / localCount) - (localMean * localMean);
		double localVarScale = std::clamp(localVariance / (dynamics.variance + 1e-12), 0.0, 2.0);

		// Dynamic threshold based on spectral features
		double fluxAdjust = std::clamp(dynamics.spectralFlux[i] / 0.15, 0.0, 1.5);
		double flatnessAdjust = 1.0 - std::clamp(dynamics.spectralFlatness[i], 0.0, 1.0);
		double threshold = 0.5 * (1.0 + fluxAdjust + 0.5 * flatnessAdjust); // 0.5–2.0 dB
		double rateThreshold = 0.01 * (1.0 + 0.5 * localVarScale + 0.4 * fluxAdjust);

		if (i > 0 && dynamics.correctedLoudness[i] != -INFINITY && dynamics.correctedLoudness[i - 1] != -INFINITY) {
			double loudnessChange = dynamics.correctedLoudness[i] - dynamics.correctedLoudness[i - 1];
			double rate = loudnessChange / (dynamics.blockDurationMs / 1000.0);
			if (loudnessChange > threshold && rate > rateThreshold) {
				transientCount += 1.0;
			}
		}
	}

	transientScore = (transientCount * 1000.0) / (dynamics.blockCount * dynamics.blockDurationMs + 1e-12) / 10.0;
	dynamics.transientScore = std::clamp(transientScore, 0.0, 1.0);
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsLoudnessAdaptation(FullTrackDataDynamics& dynamics) {
	double stableDuration = 0.0;
	double prevLufs = -INFINITY;

	// Adaptive tau based on musical context (80–200 ms)
	double baseTau = 80.0 + 120.0 * (1.0 - dynamics.spectralFlatnessMean) * (1.0 - dynamics.genreFactor);
	double tau = std::clamp(baseTau, 80.0, 200.0);

	double frequencyFactor = std::log1p(std::accumulate(dynamics.frequencyPowers.begin(), dynamics.frequencyPowers.end(), 0.0) / (dynamics.blockCount + 1e-12) / 0.001);
	double entropy = AWHMath::CalculateEntropy({ dynamics.transientScore, dynamics.spectralFlatnessMean, dynamics.spectralFluxMean });
	double adaptStrength = std::clamp(0.7 * (1.0 - entropy) + 0.3 * frequencyFactor, 0.0, 1.0) * dynamics.varianceScale;

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.correctedLoudness[i] == -INFINITY) {
			stableDuration = 0.0;
			prevLufs = -INFINITY;
			dynamics.adaptedLoudness[i] = -INFINITY;
			continue;
		}

		stableDuration = (prevLufs != -INFINITY && std::abs(dynamics.correctedLoudness[i] - prevLufs) < 1.5)
			? stableDuration + dynamics.blockDurationMs : 0.0;

		if (prevLufs == -INFINITY) {
			dynamics.adaptedLoudness[i] = dynamics.correctedLoudness[i];
		}
		else {
			dynamics.adaptedLoudness[i] = AWHAudioDynamics::ApplyPerceptualLoudnessAdaptation(
				dynamics.correctedLoudness[i], stableDuration, prevLufs, tau, adaptStrength
			);
		}

		prevLufs = dynamics.correctedLoudness[i];
	}
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsBinauralAdjustment(FullTrackDataDynamics& dynamics) {
	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.adaptedLoudness[i] == -INFINITY) {
			continue;
		}

		double binauralFactor = (i < dynamics.binauralFactor.size()) ? dynamics.binauralFactor[i] : 1.0;
		double stereoWidth = std::min(binauralFactor / 0.5, 1.0);
		double binauralAdjust = 3.0 + 3.0 * stereoWidth * (0.8 + 0.4 * dynamics.genreFactor);

		dynamics.adaptedLoudness[i] += binauralAdjust;
	}
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsCognitiveLoudness(const FullTrackData& ftData, FullTrackDataDynamics& dynamics) {
	const size_t window100ms = std::max<size_t>(1, static_cast<size_t>(100.0 / dynamics.blockDurationMs));
	const size_t window1s = std::max<size_t>(1, static_cast<size_t>(1000.0 / dynamics.blockDurationMs));
	const size_t window10s = std::max<size_t>(1, static_cast<size_t>(10000.0 / dynamics.blockDurationMs));

	// Dynamic weights tuned for musical cognition
	double spectralWeight = 0.45 * std::clamp(dynamics.spectralCentroidMean / 12000.0, 0.0, 1.0) * dynamics.genreFactor;
	double rhythmWeight = 0.35 * std::clamp(dynamics.spectralFluxMean / 0.15, 0.0, 1.0) * (1.0 - dynamics.genreFactor);
	double transientWeight = 0.3 * dynamics.transientScore * dynamics.genreFactor;
	double harmonicWeight = 0.2 * std::accumulate(dynamics.harmonicComplexityFactor.begin(), dynamics.harmonicComplexityFactor.end(), 0.0) / dynamics.blockCount;
	double totalWeight = spectralWeight + rhythmWeight + transientWeight + harmonicWeight + 0.1;
	spectralWeight /= totalWeight;
	rhythmWeight /= totalWeight;
	transientWeight /= totalWeight;
	harmonicWeight /= totalWeight;

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.adaptedLoudness[i] == -INFINITY || i >= ftData.bandPowers.size()) continue;

		ftData.loudnessHistory100ms.pushBack(dynamics.adaptedLoudness[i]);
		ftData.loudnessHistory1s.pushBack(dynamics.adaptedLoudness[i]);
		ftData.loudnessHistory10s.pushBack(dynamics.adaptedLoudness[i]);
		if (ftData.loudnessHistory100ms.size() > window100ms) ftData.loudnessHistory100ms.trim(window100ms);
		if (ftData.loudnessHistory1s.size() > window1s) ftData.loudnessHistory1s.trim(window1s);
		if (ftData.loudnessHistory10s.size() > window10s) ftData.loudnessHistory10s.trim(window10s);

		const std::vector<double>& bandPower = ftData.bandPowers[i];
		if (bandPower.size() != AWHAudioFFT::BARK_BAND_NUMBER) continue;

		double totalPower = std::accumulate(bandPower.begin(), bandPower.end(), 0.0);
		if (totalPower < 1e-12) continue;

		double cogFactor = AWHAudioDynamics::ComputeCognitiveLoudness(
			false, ftData.loudnessHistory100ms, ftData.loudnessHistory1s, ftData.loudnessHistory10s,
			dynamics.adaptedLoudness[i], dynamics.variance, dynamics.transientBoosts, bandPower,
			dynamics.blockDurationMs, ftData.sampleRate,
			dynamics.spectralCentroid[i], dynamics.spectralFlatness[i], dynamics.spectralFlux[i], dynamics.genreFactor
		);

		double harmonicBoost = 0.5 * dynamics.harmonicComplexityFactor[i];
		double phrasingBoost = 0.4 * dynamics.phrasingScore[i];
		double spatialBoost = 0.3 * dynamics.binauralFactor[i];

		// Dynamic cap based on musical context (2–5 dB)
		double cogCap = 2.0 + 3.0 * (dynamics.transientDensity + dynamics.genreFactor) / 2.0;
		double cogBoost = std::min(cogCap, cogFactor + harmonicWeight * harmonicBoost + rhythmWeight * phrasingBoost + spatialBoost);

		dynamics.adaptedLoudness[i] += cogBoost;
	}
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsTransientBoostsDetection(FullTrackDataDynamics& dynamics) {
	std::vector<double> loudnessNormalized = AWHAudioDSP::NormalizeLoudness(dynamics.adaptedLoudness, dynamics.blockDurationMs, 500.0);

	dynamics.transientBoosts = AWHAudioDynamics::DetectTransients(loudnessNormalized,
		dynamics.blockDurationMs, dynamics.harmonicComplexityFactor, dynamics.maskingFactor,
		dynamics.spectralFlux, dynamics.spectralCentroid, dynamics.spectralFlatness,
		dynamics.genreFactor, dynamics.varianceScale
	);

	dynamics.transientDensity = std::count_if(dynamics.transientBoosts.begin(), dynamics.transientBoosts.end(),
		[](double b) { return b > 1.1; }) / (dynamics.blockCount + 1e-12);
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsTransientBoostsAdjustment(FullTrackDataDynamics& dynamics) {
	dynamics.validLoudness.clear();
	dynamics.validLoudness.reserve(dynamics.blockCount);

	double boostCap = 3.0 + 2.5 * (1.0 - std::clamp(dynamics.transientDensity, 0.0, 1.0)) * dynamics.genreFactor; // 3–5.5 dB
	double weight = 0.65 + 0.25 * dynamics.transientScore * (1.0 - dynamics.transientDensity); // 0.65–0.9
	weight = std::clamp(weight, 0.65, 0.9);

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.adaptedLoudness[i] == -INFINITY) continue;

		double boostDB = boostCap * std::log1p(dynamics.transientBoosts[i] - 1.0);
		double adjusted = dynamics.adaptedLoudness[i] + (1.0 - weight) * boostDB;
		dynamics.adaptedLoudness[i] = weight * dynamics.adaptedLoudness[i] + (1.0 - weight) * adjusted;
		dynamics.validLoudness.push_back(dynamics.adaptedLoudness[i]);
		dynamics.phrasingScore[i] = AWHAudioDynamics::ComputePhrasingScore(
			dynamics.transientBoosts, dynamics.blockDurationMs, i
		);
	}
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsSpread(FullTrackDataDynamics& dynamics) {
	// 1. Normalize loudness with 3000ms time constant
	const std::vector<double> loudnessNormalized = AWHAudioDSP::NormalizeLoudness(
		dynamics.adaptedLoudness, dynamics.blockDurationMs, 3000.0
	);

	// 2. Filter loudness values above perceptual threshold (-80 dB)
	std::vector<double> validLoudness;
	validLoudness.reserve(dynamics.blockCount);
	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (loudnessNormalized[i] > -80.0) {
			validLoudness.push_back(loudnessNormalized[i]);
		}
	}
	if (validLoudness.empty()) {
		dynamics.pureDynamics = 0.0;
		return;
	}

	// 3. Compute statistical measures for dynamics distribution
	const double mean = std::accumulate(validLoudness.begin(), validLoudness.end(), 0.0) / validLoudness.size();
	const double kurtosis = AWHMath::CalculateKurtosis(validLoudness, mean, dynamics.integratedLUFS);
	const double kurtosisFactor = std::clamp(kurtosis / 6.0, 0.5, 2.0);

	// 4. Calculate focus and weighting parameters
	const double iqrFocusFactor = std::clamp(0.5 - 0.2 * (kurtosisFactor - 1.0), 0.4, 0.8);
	const double rangeFocusFactor = std::clamp(0.5 + 0.2 * (kurtosisFactor - 1.0), 0.4, 0.8);
	const double longTermWeight = 0.7 + 0.2 * (1.0 - dynamics.varianceScale) * kurtosisFactor;

	// 5. Compute psychoacoustic parameters for temporal masking
	const double dynamicThreshold = std::max(dynamics.integratedLUFS - 22.0 + 4.0 * dynamics.genreFactor, -80.0);
	const double preMaskingMs = 20.0 + 30.0 * dynamics.varianceScale; // 20–50 ms
	const double postMaskingMs = 100.0 + 140.0 * dynamics.varianceScale; // 100–240 ms
	const std::vector<double> temporalWeights = AWHAudioDynamics::ComputeTemporalWeights(loudnessNormalized, dynamics.blockDurationMs, preMaskingMs, postMaskingMs, dynamics.variance);

	// 6. Define adaptive window sizes based on rhythm and genre
	const double rhythmFactor = 1.0 + 0.4 * (dynamics.spectralFluxMean / 0.15) * dynamics.genreFactor;
	const size_t shortWindowBlocks = std::max<size_t>(4, static_cast<size_t>(std::round((150.0 + 650.0 * dynamics.genreFactor) / dynamics.blockDurationMs * rhythmFactor)));
	const size_t longWindowBlocks = std::max<size_t>(10, static_cast<size_t>(std::round((1500.0 + 3000.0 * dynamics.genreFactor) / dynamics.blockDurationMs * rhythmFactor)));
	const size_t minShortBlocks = shortWindowBlocks / 2;
	const size_t minLongBlocks = longWindowBlocks / 2;
	const size_t minTotalBlocks = minLongBlocks * 2;
	const size_t windowStep = std::max<size_t>(1, static_cast<size_t>(std::round(longWindowBlocks / 4.0)));

	// 7. Compute dynamic spreads over short and long windows
	std::vector<double> shortSpreads;
	std::vector<double> longSpreads;
	std::vector<double> weights;
	shortSpreads.reserve((dynamics.blockCount + windowStep - 1) / windowStep);
	longSpreads.reserve((dynamics.blockCount + windowStep - 1) / windowStep);
	weights.reserve((dynamics.blockCount + windowStep - 1) / windowStep);
	size_t totalValidBlocks = 0;

	for (size_t start = 0; start + longWindowBlocks <= dynamics.blockCount; start += windowStep) {
		// Short-term window
		std::vector<double> shortLoudness;
		shortLoudness.reserve(shortWindowBlocks);
		for (size_t j = start; j < start + shortWindowBlocks && j < dynamics.blockCount; ++j) {
			if (dynamics.adaptedLoudness[j] > dynamicThreshold) {
				shortLoudness.push_back(loudnessNormalized[j] * temporalWeights[j]);
			}
		}
		if (shortLoudness.size() >= minShortBlocks) {
			shortSpreads.push_back(AWHAudioDynamics::ComputeDynamicSpread(shortLoudness, -INFINITY, iqrFocusFactor, 1.0));
		}

		// Long-term window
		std::vector<double> longLoudness;
		longLoudness.reserve(longWindowBlocks);
		for (size_t j = start; j < start + longWindowBlocks && j < dynamics.blockCount; ++j) {
			if (dynamics.adaptedLoudness[j] > dynamicThreshold) {
				longLoudness.push_back(loudnessNormalized[j] * temporalWeights[j]);
				++totalValidBlocks;
			}
		}
		if (longLoudness.size() >= minLongBlocks) {
			longSpreads.push_back(AWHAudioDynamics::ComputeDynamicSpread(longLoudness, -INFINITY, iqrFocusFactor, 1.0));
			const double longAvg = std::accumulate(longLoudness.begin(), longLoudness.end(), 0.0) / longLoudness.size();
			const double loudnessFactor = std::pow(10.0, (longAvg + 80.0) / 80.0);
			weights.push_back(0.7 + 0.2 * loudnessFactor);
		}
	}

	// 8. Validate results
	if (dynamics.blockCount < minLongBlocks || totalValidBlocks < minTotalBlocks ||
		shortSpreads.empty() || longSpreads.empty()) {
		dynamics.pureDynamics = 0.0;
		return;
	}

	// 9. Aggregate and adjust dynamics
	const double longWeightedAvg = AWHMath::CalculateWeightedAverage(longSpreads, weights);
	const double longMedian = AWHMath::CalculateMedian(longSpreads);
	const double shortMedian = AWHMath::CalculateMedian(shortSpreads);
	const double longTermContribution = longTermWeight * (rangeFocusFactor * longMedian + (1.0 - rangeFocusFactor) * longWeightedAvg);
	const double shortTermContribution = (1.0 - longTermWeight) * shortMedian;
	const double baseSpread = longTermContribution + shortTermContribution;
	const double kurtosisAdjustment = 1.0 + 0.1 * (kurtosisFactor - 1.0);
	const double genreAdjustment = 0.9 + 0.2 * dynamics.genreFactor;

	// 10. Set final dynamics
	dynamics.pureDynamics = baseSpread * kurtosisAdjustment * genreAdjustment;
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsFactors(FullTrackData& ftData) {
	if (ftData.dynamicsBlockBuffer.available() <= ftData.stepSize * ftData.channels) {
		return;
	}

	// Constants
	constexpr double MIN_RMS = 1e-6;
	constexpr double MIN_ENERGY = 1e-12;
	constexpr double MIN_BAND_POWER = 1e-12;

	// Calculate blocks
	const auto samplesPerBlock = ftData.stepSize * ftData.channels;
	const size_t numBlocks = std::min(
		(ftData.dynamicsBlockBuffer.available() + samplesPerBlock - 1) / samplesPerBlock, ftData.pureDynamicsBlockSums.size()
	);

	// Calculate indexes
	const size_t indexStart = ftData.bandPowers.size();
	const size_t indexEnd = indexStart + numBlocks;

	// Resize vectors
	ftData.bandPowers.resize(indexEnd);
	ftData.binauralFactor.resize(indexEnd);
	ftData.criticalBandFactor.resize(indexEnd);
	ftData.harmonicComplexityFactor.resize(indexEnd, 0.0);
	ftData.maskingFactor.resize(indexEnd);
	ftData.frequencyPowers.resize(indexEnd);
	ftData.spectralCentroid.resize(indexEnd, 0.0);
	ftData.spectralFlatness.resize(indexEnd, 1.0);
	ftData.spectralFlux.resize(indexEnd, 0.0);

	// Temporary buffers
	std::vector<audioType> block(samplesPerBlock);
	std::vector<double> blockSamples(ftData.fftSize, 0.0);
	std::vector<double> leftChannel(ftData.stepSize);
	std::vector<double> rightChannel(ftData.stepSize);
	std::vector<double> powerSpectrum(ftData.fftSize / 2 + 1);
	std::vector<double> barkBandPower(AWHAudioFFT::BARK_BAND_NUMBER, 0.0);
	std::vector<std::complex<double>> fftOutput(ftData.fftSize);

	// Process each block
	for (size_t i = 0; i < numBlocks; ++i) {
		const size_t index = indexStart + i;
		size_t readCount;
		size_t samplesToRead = std::min(samplesPerBlock, ftData.dynamicsBlockBuffer.available());

		if (!ftData.dynamicsBlockBuffer.read(block.data(), samplesToRead, &readCount) || readCount == 0) {
			ftData.bandPowers[index] = std::vector<double>(AWHAudioFFT::BARK_BAND_NUMBER, MIN_BAND_POWER);
			ftData.binauralFactor[index] = 1.0;
			ftData.criticalBandFactor[index] = 1.0;
			ftData.harmonicComplexityFactor[index] = 0.0;
			ftData.maskingFactor[index] = 1.0;
			ftData.frequencyPowers[index] = MIN_BAND_POWER;
			ftData.spectralCentroid[index] = 0.0;
			ftData.spectralFlatness[index] = 1.0;
			ftData.spectralFlux[index] = 0.0;
			continue;
		}

		// Extract stereo channels for spatial perception
		AWHAudioDSP::ExtractStereoChannels(block.data(), ftData.stepSize, ftData.channels, leftChannel, rightChannel);
		ftData.binauralFactor[index] = AWHAudioDynamics::ComputeSpatialScore(leftChannel, rightChannel, ftData.stepSize, ftData.sampleRate);
		// ftData.binauralFactor[index] = 1.0;

		// Energy computation
		double energy;
		AWHAudioDSP::ComputeBlockSamplesAndEnergy(block, 0, ftData.stepSize, ftData.channels, blockSamples, energy, &ftData.hannWindow);
		double rms = ftData.stepSize > 0 ? std::sqrt(energy / ftData.stepSize) : 0.0;

		if (rms < MIN_RMS || energy <= MIN_ENERGY) {
			ftData.bandPowers[index] = std::vector<double>(AWHAudioFFT::BARK_BAND_NUMBER, MIN_BAND_POWER);
			ftData.criticalBandFactor[index] = 1.0;
			ftData.harmonicComplexityFactor[index] = 0.0;
			ftData.maskingFactor[index] = 1.0;
			ftData.frequencyPowers[index] = MIN_BAND_POWER;
			ftData.spectralCentroid[index] = 0.0;
			ftData.spectralFlatness[index] = 1.0;
			ftData.spectralFlux[index] = 0.0;
			continue;
		}

		// FFT and power spectrum
		AWHAudioFFT::ComputeFFTGeneral(blockSamples, fftOutput);
		AWHAudioFFT::ComputePowerSpectrum(fftOutput.data(), ftData.fftSize, ftData.stepSize, powerSpectrum);
		AWHAudioFFT::MapPowerSpectrumToBarkBands(powerSpectrum, ftData.fftSize, ftData.sampleRate, barkBandPower);
		ftData.bandPowers[index] = barkBandPower;

		// Psychoacoustic factors
		ftData.criticalBandFactor[index] = AWHAudioFFT::ComputeCriticalBandsFromPowerSpectrum(powerSpectrum, ftData.fftSize, ftData.sampleRate, barkBandPower);
		ftData.harmonicComplexityFactor[index] = AWHAudioFFT::ComputeHarmonicComplexity(barkBandPower);
		ftData.maskingFactor[index] = AWHAudioFFT::ComputeFrequencyMaskingFromPowerSpectrum(powerSpectrum, ftData.fftSize, ftData.sampleRate, barkBandPower);
		ftData.frequencyPowers[index] = AWHAudioFFT::ComputePerceptualFrequencyPower(barkBandPower, ftData.barkWeights);

		// Spectral features
		ftData.spectralCentroid[index] = AWHAudioFFT::ComputeSpectralCentroid(barkBandPower, ftData.sampleRate);
		ftData.spectralFlatness[index] = AWHAudioFFT::ComputeSpectralFlatness(barkBandPower, AWHAudioFFT::BARK_BAND_NUMBER);
		ftData.spectralFlux[index] = AWHAudioFFT::ComputeSpectralFlux(barkBandPower, ftData.bandPowersPrevious, AWHAudioFFT::BARK_BAND_NUMBER);
		ftData.bandPowersPrevious = barkBandPower; // Update for next iteration

		// ftData.spectralCentroid[index] = 4000.0;
		// ftData.spectralFlatness[index] = 1.0;
		// ftData.spectralFlux[index] = 0.05;
		// ftData.bandPowersPrevious = barkBandPower;

		// Debug logging
		//if (index == 600) {
		//	FB2K_console_formatter() << "Block " << i << " FFT Output (first 10 bins):\n";
		//	for (size_t j = 0; j < std::min<size_t>(10, fftOutput.size()); ++j) {
		//		FB2K_console_formatter() << "Bin " << j << ": " << fftOutput[j].real() << " + " << fftOutput[j].imag() << "i\n";
		//	}

		//	FB2K_console_formatter() << "Block " << i << " Power Spectrum (first 10 bins):\n";
		//	for (size_t j = 0; j < std::min<size_t>(10, powerSpectrum.size()); ++j) {
		//		double freq = j * ftData.sampleRate / ftData.fftSize;
		//		FB2K_console_formatter() << "Freq " << freq << " Hz: " << powerSpectrum[j] << " (dB: " << AWHAudio::PowerToDb(powerSpectrum[j] + AWHAudioFFT::EPSILON) << ")\n";
		//	}

		//	FB2K_console_formatter() << "Block " << i << " Bark Band Powers:\n";
		//	for (size_t b = 0; b < AWHAudioFFT::BARK_BAND_NUMBER; ++b) {
		//		FB2K_console_formatter() << "Band " << b << ": " << barkBandPower[b] << " (dB: " << AWHAudio::PowerToDb(barkBandPower[b] + AWHAudioFFT::EPSILON) << ")\n";
		//	}

		//	FB2K_console_formatter() << "Block " << i << " Spectral Features:\n";
		//	FB2K_console_formatter() << "  Frequency Power: " << ftData.frequencyPowers[index] << " (dB: " << AWHAudio::PowerToDb(ftData.frequencyPowers[index] + AWHAudioFFT::EPSILON) << ")\n";
		//	FB2K_console_formatter() << "  Binaural Factor: " << ftData.binauralFactor[index] << "\n";
		//	FB2K_console_formatter() << "  Critical Band Factor: " << ftData.criticalBandFactor[index] << "\n";
		//	FB2K_console_formatter() << "  Harmonic Complexity Factor: " << ftData.harmonicComplexityFactor[index] << "\n";
		//	FB2K_console_formatter() << "  Masking Factor: " << ftData.maskingFactor[index] << "\n";
		//	FB2K_console_formatter() << "  Spectral Centroid: " << ftData.spectralCentroid[index] << " Hz\n";
		//	FB2K_console_formatter() << "  Spectral Flatness: " << ftData.spectralFlatness[index] << "\n";
		//	FB2K_console_formatter() << "  Spectral Flux: " << ftData.spectralFlux[index] << "\n";
		//}
	}
}

void AudioWizardAnalysisFullTrack::ProcessDynamicsChunkData(const ChunkData& chkData, FullTrackData& ftData) {
	if (ftData.stepSize == 0 || ftData.channels == 0 || ftData.sampleRate <= 0.0) {
		return;
	}

	ftData.dynamicsBlockBuffer.write(chkData.data, chkData.frames * ftData.channels);
	ProcessDynamicsFactors(ftData);
}
#pragma endregion


//////////////////////////////////////////////////
// * ANALYSIS FULL-TRACK - GENERAL PROCESSING * //
//////////////////////////////////////////////////
#pragma region Analysis Full-Track - General Processing
void AudioWizardAnalysisFullTrack::ProcessKWeightedSum(FullTrackData& ftData, const std::vector<double>& chunkBuffer) {
	if (chunkBuffer.empty()) {
		ftData.pureDynamicsBlockSums.pushBack(0.0);
		ftData.shortTermBlockSums.pushBack(0.0);
		ftData.integratedBlockSums.pushBack(0.0);
		return;
	}

	for (double framePower : chunkBuffer) {
		ftData.kWeightedSumSquares += framePower;
		ftData.kWeightedFrames++;

		// Accumulate into current 100ms block
		ftData.currentBlockSum += framePower;
		ftData.currentBlockFrames++;

		// When block is complete, add to RingBufferSimple
		if (ftData.currentBlockFrames == ftData.stepSize) {
			ftData.pureDynamicsBlockSums.pushBack(ftData.currentBlockSum);
			ftData.shortTermBlockSums.pushBack(ftData.currentBlockSum);
			ftData.integratedBlockSums.pushBack(ftData.currentBlockSum);
			ftData.currentBlockSum = 0.0;
			ftData.currentBlockFrames = 0;
		}
	}
}

void AudioWizardAnalysisFullTrack::ProcessShortTermLUFS(FullTrackData& ftData) {
	while (ftData.shortTermBlockSums.size() >= 30) { // Process all available 3-second windows (30 blocks)
		double sum = 0.0;
		for (size_t i = 0; i < 30; ++i) { // Sum the oldest 30 blocks (each 100ms)
			sum += ftData.shortTermBlockSums[i];
		}
		double mean = sum / static_cast<double>(30 * ftData.stepSize); // Convert to mean power
		double lufs = -0.691 + AWHAudio::PowerToDb(mean);
		ftData.shortTermLUFS = std::max(ftData.shortTermLUFS, lufs);

		if (lufs > -70.0) {
			int binKey = static_cast<int>(std::round(lufs * 10.0)) + 700;
			binKey = std::clamp(binKey, 0, static_cast<int>(ftData.histogramOfBlockLoudnessLRA.size() - 1));
			ftData.histogramOfBlockLoudnessLRA[binKey]++;
		}

		ftData.shortTermBlockSums.trim(ftData.shortTermBlockSums.size() - 1); // Slide window by removing the oldest block
	}
}

void AudioWizardAnalysisFullTrack::ProcessIntegratedLUFS(FullTrackData& ftData) {
	while (ftData.integratedBlockSums.size() >= 4) { // Process all available 400ms windows (4 blocks)
		double sum = 0.0;
		for (size_t i = 0; i < 4; ++i) { // Sum the oldest 4 blocks (each 100ms)
			sum += ftData.integratedBlockSums[i];
		}
		double mean = sum / static_cast<double>(4 * ftData.stepSize); // Convert to mean power
		double lkfs = -0.691 + AWHAudio::PowerToDb(mean);
		ftData.momentaryLUFS = std::max(ftData.momentaryLUFS, lkfs);

		if (lkfs > -70.0) {
			int binKey = static_cast<int>(std::round(lkfs * 10.0)) + 700;
			binKey = std::clamp(binKey, 0, static_cast<int>(ftData.histogramOfBlockLoudness.size() - 1));
			ftData.histogramOfBlockLoudness[binKey]++;
		}

		ftData.integratedBlockSums.trim(ftData.integratedBlockSums.size() - 1); // Slide window by removing the oldest block
	}
}

void AudioWizardAnalysisFullTrack::ProcessOriginalBlocks(FullTrackData& ftData) {
	const auto samplesPerBlock = static_cast<size_t>(3.0 * ftData.sampleRate) * ftData.channels;
	// Reserve space based on expected full blocks
	size_t expectedBlocks = ftData.originalBlockBuffer.available() / samplesPerBlock;
	ftData.originalRMSLinearLeft.reserve(ftData.originalRMSLinearLeft.size() + expectedBlocks);
	ftData.originalRMSLinearRight.reserve(ftData.originalRMSLinearRight.size() + expectedBlocks);
	ftData.originalPeakLinearLeft.reserve(ftData.originalPeakLinearLeft.size() + expectedBlocks);
	ftData.originalPeakLinearRight.reserve(ftData.originalPeakLinearRight.size() + expectedBlocks);

	while (ftData.originalBlockBuffer.available() >= samplesPerBlock) {
		std::vector<audioType> block(samplesPerBlock);
		size_t readCount;
		// Read exactly samplesPerBlock, no partial reads
		if (!ftData.originalBlockBuffer.read(block.data(), samplesPerBlock, &readCount) || readCount != samplesPerBlock) {
			break;
		}
		// No zero-padding needed since we only process full blocks
		double sumSquaresLeft = 0.0;
		double sumSquaresRight = 0.0;
		double currentPeakLeft = 0.0;
		double currentPeakRight = 0.0;
		const size_t numChannels = ftData.channels;
		const size_t samplesPerChannel = samplesPerBlock / numChannels;

		for (size_t i = 0; i < samplesPerBlock; i += numChannels) {
			double left = block[i];
			double right = numChannels > 1 ? block[i + 1] : left;
			sumSquaresLeft += left * left;
			sumSquaresRight += right * right;
			currentPeakLeft = std::max(currentPeakLeft, std::abs(left));
			currentPeakRight = std::max(currentPeakRight, std::abs(right));
		}

		ftData.originalRMSLinearLeft.push_back(std::sqrt(sumSquaresLeft / samplesPerChannel));
		ftData.originalRMSLinearRight.push_back(std::sqrt(sumSquaresRight / samplesPerChannel));
		ftData.originalPeakLinearLeft.push_back(currentPeakLeft);
		ftData.originalPeakLinearRight.push_back(currentPeakRight);
	}
}

void AudioWizardAnalysisFullTrack::ProcessOriginalSamples(const ChunkData& chkData, FullTrackData& ftData) {
	size_t count = chkData.frames * chkData.channels;
	ftData.originalBlockBuffer.write(chkData.data, count);
	double sumSquares = 0.0;

	for (size_t i = 0; i < count; ++i) {
		double sample = chkData.data[i];
		sumSquares += sample * sample;
	}

	ftData.originalSquaresSum += sumSquares;
	ftData.originalSampleCount += count;
}

void AudioWizardAnalysisFullTrack::ProcessSamplePeakMax(const ChunkData& chkData, FullTrackData& ftData) {
	audioType chunkSamplePeak = 0.0;

	for (size_t i = 0; i < chkData.frames * chkData.channels; ++i) {
		chunkSamplePeak = std::max(chunkSamplePeak, std::abs(chkData.data[i]));
	}

	ftData.samplePeakMaxLinear = std::max(ftData.samplePeakMaxLinear, chunkSamplePeak);
}

void AudioWizardAnalysisFullTrack::ProcessTruePeakMax(const ChunkData& chkData, FullTrackData& ftData) {
	auto chunkTruePeakLinear = AudioWizardAnalysisInterpolator::CalculateTruePeakLinear(
		chkData, ftData.filterData.interp.get()
	);

	if (chunkTruePeakLinear > ftData.truePeakMaxLinear) {
		ftData.truePeakMaxLinear = static_cast<audioType>(chunkTruePeakLinear);
	}
}
#pragma endregion


///////////////////////////////////////////////
// * ANALYSIS FULL-TRACK - MAIN PROCESSING * //
///////////////////////////////////////////////
#pragma region Analysis Full-Track - Main Processing
void AudioWizardAnalysisFullTrack::InitFullTrackState(const ChunkData& chkData, FullTrackData& ftData) {
	if (ftData.sampleRate != 0.0) return;

	ftData.bitDepth = AWHMeta::GetBitDepth(ftData.handle);
	ftData.channels = chkData.channels;
	ftData.sampleRate = chkData.sampleRate;
	ftData.blockSize = static_cast<size_t>(0.4 * ftData.sampleRate); // 400ms block size
	ftData.stepSize = static_cast<size_t>(0.1 * ftData.sampleRate);  // 100ms step size
	ftData.shortTermWindow = static_cast<size_t>(3.0 * ftData.sampleRate); // 3s window for LUFS

	// Initialize FFT size, Hann window and Bark weights
	double targetBinWidth = 3.0;
	ftData.fftSize = AWHAudioFFT::CalculateFFTSize(false, ftData.sampleRate, targetBinWidth, ftData.fftSize);
	ftData.barkWeights = AWHAudioFFT::ComputeBarkWeights(ftData.sampleRate);
	ftData.hannWindow = AWHAudioDSP::GenerateHannWindow(ftData.stepSize);

	FB2K_console_formatter() << "Bit Depth: " << ftData.bitDepth << ", Sample Rate: " << ftData.sampleRate
		<< " Hz, fftSize: " << ftData.fftSize << ", Bin Width: " << (ftData.sampleRate / ftData.fftSize) << " Hz";

	// Initialize histograms
	constexpr int MAX_BIN = 800;
	ftData.histogramOfBlockLoudness.resize(MAX_BIN + 1, 0);
	ftData.histogramOfBlockLoudnessLRA.resize(MAX_BIN + 1, 0);

	// Initialize interpolation for true peak
	AudioWizardAnalysisFilter::InitInterpolation(chkData, ftData.filterData);
}

void AudioWizardAnalysisFullTrack::TestSyntheticInput(const FullTrackData& ftData) {
	std::vector<double> bandPower(AWHAudioFFT::BARK_BAND_NUMBER, 0.0);
	std::vector<std::complex<double>> fftOutput(ftData.fftSize);

	// Test frequencies: fixed + Bark band centers
	std::vector<double> testFreqs = { 100.0, 1000.0, 5000.0, 15000.0, 20000.0 };
	std::vector<double> testAmps = { 1.0, 1.0, 1.0, 1.0, 1.0 };
	std::vector<size_t> barkBandIndices = { 5, 10, 15, 20, 24 }; // Key bands to test
	const double freqLimit = AWHAudio::GetFrequencyLimit(ftData.sampleRate);

	for (size_t b : barkBandIndices) {
		double centerFreq = AWHAudioFFT::ComputeBarkToFrequencies(b, ftData.sampleRate);
		testFreqs.push_back(centerFreq);
		testAmps.push_back(1.0);
	}

	double binWidth = ftData.sampleRate / ftData.fftSize;
	size_t k_max = ftData.fftSize / 2 + 1;
	std::vector<double> powerSpectrum(k_max, 0.0);
	double inputEnergy = 0.0;

	for (size_t i = 0; i < testFreqs.size(); ++i) {
		double freq = testFreqs[i];
		if (freq > freqLimit) continue;
		auto k = static_cast<size_t>(std::round(freq / binWidth));
		if (k < k_max) {
			powerSpectrum[k] = testAmps[i] * testAmps[i];
			inputEnergy += powerSpectrum[k];
		}
	}

	FB2K_console_formatter() << "DebugSynthetic: Sample Rate: " << ftData.sampleRate
		<< " Hz, fftSize: " << ftData.fftSize << ", Bin Width: " << binWidth
		<< ", Input Energy: " << inputEnergy << "\n";

	for (size_t i = 0; i < testFreqs.size(); ++i) {
		auto k = static_cast<size_t>(std::round(testFreqs[i] / binWidth));
		if (k < k_max && testFreqs[i] <= freqLimit) {
			pfc::string_formatter label;
			if (i < 5) {
				label << "Fixed";
			}
			else {
				label << "Band " << static_cast<int>(barkBandIndices[i - 5]);
			}
			FB2K_console_formatter() << "Freq " << testFreqs[i] << " Hz, Bin " << k
				<< ", Power: " << powerSpectrum[k] << " (" << label << ")\n";
		}
	}

	AWHAudioFFT::MapPowerSpectrumToBarkBands(powerSpectrum, ftData.fftSize, ftData.sampleRate, bandPower);

	for (size_t b = 0; b < AWHAudioFFT::BARK_BAND_NUMBER; ++b) {
		FB2K_console_formatter() << "DebugSynthetic: Bark Band " << b << ", Power: " << bandPower[b] << "\n";
	}

	std::array<double, AWHAudioFFT::BARK_BAND_NUMBER> barkWeights = AWHAudioFFT::ComputeBarkWeights(ftData.sampleRate);
	double frequencyPower = AWHAudioFFT::ComputePerceptualFrequencyPower(bandPower, barkWeights);
	double spectralCentroid = AWHAudioFFT::ComputeSpectralCentroid(bandPower, ftData.sampleRate);
	double spectralFlatness = AWHAudioFFT::ComputeSpectralFlatness(bandPower, AWHAudioFFT::BARK_BAND_NUMBER);

	FB2K_console_formatter() << "DebugSynthetic: Frequency Power: " << frequencyPower
		<< " (dB: " << AWHAudio::PowerToDb(frequencyPower + AWHAudioFFT::EPSILON)
		<< "), Spectral Centroid: " << spectralCentroid
		<< " Hz, Spectral Flatness: " << spectralFlatness << "\n";
}

void AudioWizardAnalysisFullTrack::ResetFullTrackData(FullTrackData& ftData) {
	ftData.sampleRate = 0.0;
	ftData.shortTermBlockSums.clear();
	ftData.integratedBlockSums.clear();
	ftData.loudnessHistory100ms.clear();
	ftData.loudnessHistory1s.clear();
	ftData.loudnessHistory10s.clear();
	ftData.histogramOfBlockLoudness.clear();
	ftData.histogramOfBlockLoudnessLRA.clear();
	ftData.originalBlockBuffer.clear();
	ftData.originalRMSLinearLeft.clear();
	ftData.originalRMSLinearRight.clear();
	ftData.originalPeakLinearLeft.clear();
	ftData.originalPeakLinearRight.clear();
	ftData.dynamicsBlockBuffer.clear();
	ftData.pureDynamicsBlockSums.clear();
	ftData.frequencyPowers.clear();
	ftData.bandPowers.clear();
	ftData.bandPowersPrevious.clear();
	ftData.binauralFactor.clear();
	ftData.criticalBandFactor.clear();
	ftData.harmonicComplexityFactor.clear();
	ftData.maskingFactor.clear();
}

void AudioWizardAnalysisFullTrack::ProcessFullTrackChunk(const ChunkData& chkData, FullTrackData& ftData) {
	InitFullTrackState(chkData, ftData);

	if (ftData.sampleRate != chkData.sampleRate) return;

	const size_t chunkSeconds = BufferSettings::BUFFER_CAPACITY_CHUNK_SEC_MID;
	const auto samplesPerChunk = chunkSeconds * static_cast<size_t>(ftData.sampleRate) * ftData.channels;
	size_t remainingSamples = chkData.frames * ftData.channels;

	for (size_t offset = 0; offset < remainingSamples; offset += samplesPerChunk) {
		size_t samplesToProcess = std::min(samplesPerChunk, remainingSamples - offset);
		size_t framesToProcess = samplesToProcess / ftData.channels;

		ChunkData subChunk = chkData;
		subChunk.frames = framesToProcess;
		subChunk.data = chkData.data + offset;

		// Process K-Weighting
		std::vector<double> chunkBuffer;
		AudioWizardAnalysisFilter::ProcessKWeightedChunk(subChunk, ftData.filterData, chunkBuffer);
		ProcessKWeightedSum(ftData, chunkBuffer);

		// Process Loudness
		ProcessShortTermLUFS(ftData);
		ProcessIntegratedLUFS(ftData);

		// Process Original Audio
		ProcessOriginalSamples(subChunk, ftData);
		ProcessOriginalBlocks(ftData);
		ProcessSamplePeakMax(subChunk, ftData);
		ProcessTruePeakMax(subChunk, ftData);

		// Process Dynamics
		ProcessDynamicsChunkData(subChunk, ftData);
	}
}

void AudioWizardAnalysisFullTrack::ProcessFullTrackResults(metadb_handle_ptr track, const FullTrackData& ftData, FullTrackResults& ftResult) {
	ftResult.handle = track;

	// Get Metadata
	ftResult.artist = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetMetadataField(track, "artist")).get_ptr();
	ftResult.album = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetMetadataField(track, "album")).get_ptr();
	ftResult.title = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetMetadataField(track, "title")).get_ptr();
	ftResult.duration = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetDuration(track)).get_ptr();
	ftResult.year = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetMetadataField(track, "date")).get_ptr();
	ftResult.genre = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetMetadataField(track, "genre")).get_ptr();
	ftResult.format = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetFileFormat(track));
	ftResult.channels = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetTechnicalInfoField(track, "channels")).get_ptr();
	ftResult.bitDepth = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetTechnicalInfoField(track, "bitspersample") + " bit").get_ptr();
	ftResult.bitrate = pfc::stringcvt::string_wide_from_utf8(AWHMeta::GetTechnicalInfoField(track, "bitrate") + " kbps").get_ptr();
	ftResult.sampleRate = AWHString::FormatSampleRate(ftData.sampleRate);

	// Get metrics
	double integratedLUFS = GetIntegratedLUFSFull(ftData);
	double truePeakDb = AWHAudio::LinearToDb(ftData.truePeakMaxLinear);
	ftResult.momentaryLUFS = AWHMath::RoundTo(ftData.momentaryLUFS, 1);
	ftResult.shortTermLUFS = AWHMath::RoundTo(ftData.shortTermLUFS, 1);
	ftResult.integratedLUFS = AWHMath::RoundTo(integratedLUFS, 1);
	ftResult.RMS = AWHMath::RoundTo(GetRMSFull(ftData), 1);
	ftResult.samplePeak = AWHMath::RoundTo(GetSamplePeakFull(ftData), 1);
	ftResult.truePeak = AWHMath::RoundTo(truePeakDb, 1);
	ftResult.PSR = AWHMath::RoundTo(GetPSRFull(ftData), 1);
	ftResult.PLR = AWHMath::RoundTo(truePeakDb - integratedLUFS, 1);
	ftResult.crestFactor = AWHMath::RoundTo(GetCrestFactorFull(ftData), 1);
	ftResult.loudnessRange = AWHMath::RoundTo(GetLoudnessRangeFull(ftData), 1);
	ftResult.dynamicRange = AWHMath::RoundTo(GetDynamicRangeFull(ftData), 1);
	ftResult.pureDynamics = AWHMath::RoundTo(GetPureDynamicsFull(ftData), 1);
}
#pragma endregion


//////////////////////////////////////
// * ANALYSIS REAL-TIME - METRICS * //
//////////////////////////////////////
#pragma region Analysis Real-Time - Metrics
double AudioWizardAnalysisRealTime::GetMomentaryLUFS(const ChunkData& chkData, const RealTimeData& rtData) {
	auto maxSamples = static_cast<size_t>(0.4 * chkData.sampleRate);
	return ProcessLUFS(rtData.kWeightedBuffer, maxSamples);
}

double AudioWizardAnalysisRealTime::GetShortTermLUFS(const ChunkData& chkData, const RealTimeData& rtData) {
	auto maxSamples = static_cast<size_t>(3.0 * chkData.sampleRate);
	return ProcessLUFS(rtData.kWeightedBuffer, maxSamples);
}

double AudioWizardAnalysisRealTime::GetRMS(const ChunkData& chkData) {
	double sumSquares = 0.0;

	for (size_t i = 0; i < chkData.frames; ++i) {
		for (size_t ch = 0; ch < chkData.channels; ++ch) {
			double sample = chkData.data[i * chkData.channels + ch];
			sumSquares += sample * sample;
		}
	}

	return AWHAudio::LinearToDb(std::sqrt(sumSquares / static_cast<double>(chkData.frames * chkData.channels)));
}

double AudioWizardAnalysisRealTime::GetTruePeak(const ChunkData& chkData, RealTimeData& rtData) {
	double truePeakLinear = AudioWizardAnalysisInterpolator::CalculateTruePeakLinear(
		chkData, rtData.filterData.interp.get()
	);

	return AWHAudio::LinearToDb(truePeakLinear);
}

double AudioWizardAnalysisRealTime::GetPSR(const ChunkData& chkData, RealTimeData& rtData) {
	double truePeak = GetTruePeak(chkData, rtData);
	double shortTermLUFS = GetShortTermLUFS(chkData, rtData);
	return truePeak - shortTermLUFS;
}

double AudioWizardAnalysisRealTime::GetPLR(double truePeak, double integratedLUFS) {
	return truePeak - integratedLUFS;
}

double AudioWizardAnalysisRealTime::GetCrestFactor(const ChunkData& chkData) {
	double peak = ProcessFramePeak(chkData);
	double sumSquares = 0.0;

	for (size_t i = 0; i < chkData.frames; ++i) {
		for (size_t ch = 0; ch < chkData.channels; ++ch) {
			double sample = chkData.data[i * chkData.channels + ch];
			sumSquares += sample * sample;
		}
	}

	double RMS = std::sqrt(sumSquares / static_cast<double>(chkData.frames * chkData.channels));
	return (RMS == 0.0) ? 0.0 : AWHAudio::LinearToDb(peak / RMS);
}

double AudioWizardAnalysisRealTime::GetDynamicRange(const ChunkData& chkData, RealTimeData& rtData) {
	const size_t frames = chkData.frames;
	if (chkData.frames == 0) return 0.0;

	constexpr double MIN_RMS_LINEAR = 1e-5;
	constexpr double TOP_RMS_FRACTION = 0.2;

	// Calculate RMS and peak for the current 100ms chunk
	auto calculateChunkMetrics = [&chkData, frames](size_t channelIdx, RingBufferSimple& rmsBuffer, RingBufferSimple& peakBuffer) {
		double sumSquares = 0.0;
		double chunkPeak = 0.0;
		const size_t stride = chkData.channels;
		const audioType* samplePtr = chkData.data + channelIdx;

		for (size_t i = 0; i < frames; ++i) {
			const double sample = *samplePtr;
			sumSquares += sample * sample;
			chunkPeak = std::max(chunkPeak, std::abs(sample));
			samplePtr += stride;
		}

		const double chunkRMS = std::sqrt(sumSquares / frames);
		rmsBuffer.pushBack(chunkRMS);
		peakBuffer.pushBack(chunkPeak);
	};

	// Aggregate RMS over 3000ms (30 chunks) for stability
	auto aggregateRMS = [](const RingBufferSimple& rmsBuffer, size_t chunksPerBlock) {
		std::vector<double> aggregatedRMS;
		size_t numBlocks = rmsBuffer.size() / chunksPerBlock;
		if (numBlocks == 0) return aggregatedRMS;

		for (size_t i = 0; i < numBlocks; ++i) {
			double sumSquares = 0.0;
			for (size_t j = 0; j < chunksPerBlock; ++j) {
				size_t idx = i * chunksPerBlock + j;
				double rms = rmsBuffer[idx];
				sumSquares += rms * rms;
			}
			aggregatedRMS.push_back(std::sqrt(sumSquares / chunksPerBlock));
		}

		return aggregatedRMS;
	};

	// Compute DR over the aggregated RMS and peaks
	auto calculateDR = [&chkData](const std::vector<double>& rmsAggregated, const RingBufferSimple& peakBuffer) {
		const size_t numBlocks = rmsAggregated.size();
		if (numBlocks < 2) return 0.0;

		// Second-largest peak over the window
		double max1 = 0.0;
		double max2 = 0.0;
		for (size_t i = 0; i < peakBuffer.size(); ++i) {
			const double p = peakBuffer[i];
			if (p > max1) {
				max2 = max1;
				max1 = p;
			}
			else if (p > max2) max2 = p;
		}
		const double secondLargestPeakLinear = std::max(max2, MIN_RMS_LINEAR);

		// Top 20% RMS
		const auto top20 = static_cast<size_t>(std::ceil(TOP_RMS_FRACTION * numBlocks));
		if (top20 == 0) return 0.0;

		std::vector<double> rmsSorted = rmsAggregated;
		std::partial_sort(rmsSorted.begin(), rmsSorted.begin() + top20, rmsSorted.end(), std::greater<double>());

		double sumSquares = 0.0;
		for (size_t i = 0; i < top20; ++i) {
			sumSquares += rmsSorted[i] * rmsSorted[i];
		}

		// From DRMeter manual (DRMeter_UM.pdf, p. 19, RMS Meters):
		// "The RMS value is corrected by + 3dB so that sine waves have the same peak and RMS value,
		// as is the case with most other RMS meters."
		// Standard RMS = peak / sqrt(2) ≈ peak / 1.4142135623730951, so corrected RMS = sqrt(2) * standardRMS.
		// In dB: 20 * log10(sqrt(2)) ≈ 3.0103.
		constexpr double DR14_RMS_CORRECTION = 3.0103;
		double overallRMSLinear = std::max(std::sqrt(sumSquares / top20), MIN_RMS_LINEAR);
		double overallRMSdB = AWHAudio::LinearToDb(overallRMSLinear) + DR14_RMS_CORRECTION;

		return AWHAudio::LinearToDb(secondLargestPeakLinear) - overallRMSdB;
	};

	// Smoothing with faster response
	auto smoothDR = [](double drNew, double const& drPrevious, double smoothingFactor = 0.2) {
		return smoothingFactor * drPrevious + (1.0 - smoothingFactor) * drNew;
	};

	constexpr size_t CHUNKS_PER_BLOCK = 30; // 3000ms (30 * 100ms)
	double dr;

	if (chkData.channels >= 2) {
		calculateChunkMetrics(0, rtData.drRMSBufferLeft, rtData.drPeakBufferLeft);
		calculateChunkMetrics(1, rtData.drRMSBufferRight, rtData.drPeakBufferRight);
		auto rmsLeftAggregated = aggregateRMS(rtData.drRMSBufferLeft, CHUNKS_PER_BLOCK);
		auto rmsRightAggregated = aggregateRMS(rtData.drRMSBufferRight, CHUNKS_PER_BLOCK);
		double drLeft = calculateDR(rmsLeftAggregated, rtData.drPeakBufferLeft);
		double drRight = calculateDR(rmsRightAggregated, rtData.drPeakBufferRight);
		dr = (drLeft + drRight) / 2.0;
	}
	else {
		calculateChunkMetrics(0, rtData.drRMSBuffer, rtData.drPeakBuffer);
		auto rmsAggregated = aggregateRMS(rtData.drRMSBuffer, CHUNKS_PER_BLOCK);
		dr = calculateDR(rmsAggregated, rtData.drPeakBuffer);
	}

	return smoothDR(std::max(0.0, dr), rtData.drPrevious);
}

double AudioWizardAnalysisRealTime::GetPureDynamics(const ChunkData& chkData, RealTimeData& rtData) {
	if (!chkData.data || chkData.frames == 0 || rtData.blockSize == 0) {
		return rtData.pureDynamicsEMA;
	}

	// Pipeline order:
	// Correction > Adaptation > Binaural > Transient Detection > Cognitive > Transient Application > Spread mirrors auditory processing:
	// Peripheral > Spatial > Temporal > Cognitive > Integrative

	// Step 0: Initialize dynamics struct and loudness
	RealTimeDataDynamics dynamics{ chkData, rtData };
	ProcessDynamicsInitialization(dynamics);

	// Step 1: Correct loudness for perceptual factors (psychoacoustic: Zwicker model)
	// Produces: dynamics.correctedLoudness
	ProcessDynamicsLoudnessCorrection(dynamics);

	// Step 2: Adapt loudness for temporal habituation (psychoacoustic: neural adaptation)
	// Updates: dynamics.adaptedLoudness
	ProcessDynamicsLoudnessAdaptation(dynamics);

	// Step 3: Adjust for binaural perception (psychoacoustic: spatial cues)
	// Modifies: dynamics.adaptedLoudness
	ProcessDynamicsBinauralAdjustment(dynamics);

	// Step 4: Detect transients for cognitive loudness (psychoacoustic: onset salience)
	// Sets: dynamics.transientBoosts
	ProcessDynamicsTransientBoostsDetection(dynamics);

	// Step 5: Apply cognitive loudness adjustments (psychoacoustic: attention, genre weighting)
	// Adjusts: dynamics.adaptedLoudness
	ProcessDynamicsCognitiveLoudness(dynamics);

	// Step 6: Apply transient boosts to enhance loudness (psychoacoustic: transient amplification)
	// Finalizes: dynamics.adaptedLoudness
	ProcessDynamicsTransientBoostsAdjustment(dynamics);

	// Step 7: Compute dynamic spread (psychoacoustic: integrative dynamic range)
	// Updates: dynamics.rtData.pureDynamicsEMA (final value)
	ProcessDynamicsSpread(dynamics);

	return dynamics.rtData.pureDynamicsEMA;
}

double AudioWizardAnalysisRealTime::GetPhaseCorrelation(const ChunkData& chkData) {
	if (chkData.channels < 2) return 1.0;

	double sumLR = 0.0;
	double sumL = 0.0;
	double sumR = 0.0;

	for (size_t i = 0; i < chkData.frames; i += chkData.channels) {
		double L = chkData.data[i];
		double R = chkData.data[i + 1];
		sumLR += L * R;
		sumL += L * L;
		sumR += R * R;
	}

	if (sumL == 0.0 || sumR == 0.0) return 0.0;

	return sumLR / (std::sqrt(sumL) * std::sqrt(sumR));
}

double AudioWizardAnalysisRealTime::GetStereoWidth(const ChunkData& chkData) {
	if (chkData.channels < 2) return 1.0;

	double sumMid = 0.0;
	double sumSide = 0.0;

	for (size_t i = 0; i < chkData.frames; i += chkData.channels) {
		double L = chkData.data[i];
		double R = chkData.data[i + 1];
		double mid = (L + R) / 2.0;
		double side = (L - R) / 2.0;
		sumMid += mid * mid;
		sumSide += side * side;
	}

	if (sumMid == 0.0) return 1.0;

	return std::sqrt(sumSide / sumMid) * 2.0 * 50.0;
}
#pragma endregion


//////////////////////////////////////////////////
// * ANALYSIS REAL-TIME - DYNAMICS PROCESSING * //
//////////////////////////////////////////////////
#pragma region Analysis Real-Time - Dynamics Processing
void AudioWizardAnalysisRealTime::ProcessDynamicsInitialization(RealTimeDataDynamics& dynamics) {
	// Initialize core parameters from real-time data
	dynamics.blockDurationMs = dynamics.rtData.blockDurationMs;
	dynamics.integratedLUFS = dynamics.rtData.integratedLUFS;

	// Compute block loudness from the last 1.5s of K-weighted buffer
	const size_t blockSize = dynamics.rtData.blockSize;
	const auto maxSamples = static_cast<size_t>(1.5 * dynamics.chkData.sampleRate);
	const size_t start = dynamics.rtData.kWeightedBuffer.size() > maxSamples ? dynamics.rtData.kWeightedBuffer.size() - maxSamples : 0;
	dynamics.blockLoudness = AWHAudioDSP::ComputeBlockLoudness(dynamics.rtData.kWeightedBuffer, start, blockSize);
	dynamics.blockCount = dynamics.blockLoudness.size();
	if (dynamics.blockCount == 0) return;

	// Reserve capacity for vectors to minimize reallocations
	dynamics.correctedLoudness.reserve(dynamics.blockCount);
	dynamics.adaptedLoudness.reserve(dynamics.blockCount);
	dynamics.transientBoosts.reserve(dynamics.blockCount);

	// Retrieve precomputed spectral features from rtData
	dynamics.bandPowers = dynamics.rtData.bandPowers;
	dynamics.frequencyPowers = dynamics.rtData.frequencyPowers;
	dynamics.harmonicComplexityFactor = dynamics.rtData.harmonicComplexityFactor;
	dynamics.maskingFactor = dynamics.rtData.maskingFactor;
	dynamics.spectralCentroid = dynamics.rtData.spectralCentroid;
	dynamics.spectralFlatness = dynamics.rtData.spectralFlatness;
	dynamics.spectralFlux = dynamics.rtData.spectralFlux;

	// Ensure vectors match block count, filling with defaults if FFT was skipped
	if (dynamics.bandPowers.size() != dynamics.blockCount) {
		dynamics.bandPowers.resize(dynamics.blockCount, std::vector<double>(AWHAudioFFT::BARK_BAND_NUMBER, AWHAudioFFT::EPSILON));
		dynamics.frequencyPowers.resize(dynamics.blockCount, AWHAudioFFT::EPSILON);
		dynamics.harmonicComplexityFactor.resize(dynamics.blockCount, 0.0);
		dynamics.maskingFactor.resize(dynamics.blockCount, 1.0);
		dynamics.spectralCentroid.resize(dynamics.blockCount, 0.0);
		dynamics.spectralFlatness.resize(dynamics.blockCount, 1.0);
		dynamics.spectralFlux.resize(dynamics.blockCount, 0.0);
	}

	// Compute variance and scale for perceptual adjustments
	dynamics.variance = AWHMath::CalculateVarianceOnline(dynamics.blockLoudness);
	double varianceDenom = 30.0 + 40.0 * dynamics.rtData.genreFactor + 0.2 * dynamics.variance;
	dynamics.varianceScale = std::tanh(std::max(1.0, dynamics.variance / varianceDenom));

	// Initialize loudness and transient processing states
	dynamics.correctedLoudness.assign(dynamics.blockCount, -70.0);
	dynamics.adaptedLoudness.assign(dynamics.blockCount, std::numeric_limits<double>::lowest());
	dynamics.transientBoosts.assign(dynamics.blockCount, 1.0);
	dynamics.stableDuration = 0.0;
	dynamics.prevLufs = -INFINITY;

	// Update FFT caching parameters
	dynamics.lastBlockCount = dynamics.blockCount;
	dynamics.lastSampleRate = dynamics.chkData.sampleRate;
	dynamics.frameCounter++;
}

void AudioWizardAnalysisRealTime::ProcessDynamicsLoudnessCorrection(RealTimeDataDynamics& dynamics) {
	// Compute Fastl adjustments for perceptual correction
	std::vector<double> fastlAdjustments(dynamics.blockCount, 0.0);
	AWHAudioDynamics::ComputeFastlPrinciples(true, fastlAdjustments,
		dynamics.bandPowers, dynamics.blockLoudness, dynamics.rtData.blockSize, dynamics.chkData.sampleRate,
		dynamics.variance, dynamics.varianceScale, &dynamics.rtData.bandPowersHistory
	);

	// Apply perceptual loudness correction with Fastl adjustments
	std::vector<double> dummyValidLoudness;
	AWHAudioDynamics::ComputePerceptualLoudnessCorrection(true, fastlAdjustments,
		dynamics.blockLoudness, dynamics.frequencyPowers, dynamics.integratedLUFS, dynamics.variance,
		dynamics.correctedLoudness,	dummyValidLoudness, nullptr, 0
	);
}

void AudioWizardAnalysisRealTime::ProcessDynamicsLoudnessAdaptation(RealTimeDataDynamics& dynamics) {
	dynamics.adaptedLoudness.clear();
	dynamics.adaptedLoudness.reserve(dynamics.blockCount);

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.correctedLoudness[i] == -INFINITY) {
			dynamics.stableDuration = 0.0;
			dynamics.prevLufs = -INFINITY;
			dynamics.adaptedLoudness.push_back(-INFINITY);
			continue;
		}

		if (dynamics.prevLufs != -INFINITY && std::abs(dynamics.correctedLoudness[i] - dynamics.prevLufs) < 1.0) {
			dynamics.stableDuration += dynamics.blockDurationMs;
		}
		else {
			dynamics.stableDuration = 0.0;
			dynamics.prevLufs = dynamics.correctedLoudness[i];
		}

		dynamics.adaptedLoudness.push_back(AWHAudioDynamics::ApplyPerceptualLoudnessAdaptation(
			dynamics.correctedLoudness[i], dynamics.stableDuration, -20.0, 15.0, 0.3)
		);
	}
}

void AudioWizardAnalysisRealTime::ProcessDynamicsBinauralAdjustment(RealTimeDataDynamics& dynamics) {
	if (dynamics.chkData.channels < 1 || !dynamics.chkData.data || dynamics.chkData.frames == 0) return;

	const size_t blockSize = dynamics.rtData.blockSize;
	if (dynamics.stereoBuffer.size() < blockSize * 2) dynamics.stereoBuffer.resize(blockSize * 2);
	if (dynamics.leftChannel.size() < blockSize) dynamics.leftChannel.resize(blockSize);
	if (dynamics.rightChannel.size() < blockSize) dynamics.rightChannel.resize(blockSize);

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.adaptedLoudness[i] == -INFINITY) continue;
		size_t startSample = i * blockSize * dynamics.chkData.channels;
		if (startSample + blockSize * dynamics.chkData.channels > dynamics.chkData.frames * dynamics.chkData.channels) continue;

		AWHAudioDSP::DownmixToStereo(dynamics.chkData.data + startSample, blockSize, dynamics.chkData.channels, dynamics.stereoBuffer.data());

		for (size_t j = 0; j < blockSize; ++j) {
			size_t idx = j * 2;
			dynamics.leftChannel[j] = dynamics.stereoBuffer[idx];
			dynamics.rightChannel[j] = dynamics.stereoBuffer[idx + 1];
		}

		double binauralFactor = AWHAudioDynamics::ComputeBinauralPerception(true, true,
			blockSize, dynamics.chkData.sampleRate, dynamics.leftChannel.data(), dynamics.rightChannel.data()
		);
		dynamics.adaptedLoudness[i] += (2.0 + 2.5 * binauralFactor); // Range 2.0–4.5 dB per ISO 532-1:2017
	}
}

void AudioWizardAnalysisRealTime::ProcessDynamicsCognitiveLoudness(RealTimeDataDynamics& dynamics) {
	const size_t window100ms = std::max<size_t>(1, static_cast<size_t>(100.0 / dynamics.blockDurationMs));
	const size_t window1s = std::max<size_t>(1, static_cast<size_t>(1000.0 / dynamics.blockDurationMs));
	const size_t window10s = std::max<size_t>(1, static_cast<size_t>(10000.0 / dynamics.blockDurationMs));

	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.adaptedLoudness[i] == -INFINITY) continue;

		dynamics.rtData.loudnessHistory100ms.pushBack(dynamics.adaptedLoudness[i]);
		dynamics.rtData.loudnessHistory1s.pushBack(dynamics.adaptedLoudness[i]);
		dynamics.rtData.loudnessHistory10s.pushBack(dynamics.adaptedLoudness[i]);
		if (dynamics.rtData.loudnessHistory100ms.size() > window100ms) dynamics.rtData.loudnessHistory100ms.trim(window100ms);
		if (dynamics.rtData.loudnessHistory1s.size() > window1s) dynamics.rtData.loudnessHistory1s.trim(window1s);
		if (dynamics.rtData.loudnessHistory10s.size() > window10s) dynamics.rtData.loudnessHistory10s.trim(window10s);

		// Use 25-band bark powers for cognitive loudness
		std::vector<double> bandPower = (i < dynamics.bandPowers.size() && !dynamics.bandPowers[i].empty()) ?
			dynamics.bandPowers[i] : std::vector<double>(AWHAudioFFT::BARK_BAND_NUMBER, AWHAudioFFT::EPSILON);

		// Use computed spectral features instead of defaults
		double cogFactor = AWHAudioDynamics::ComputeCognitiveLoudness(true,
			dynamics.rtData.loudnessHistory100ms, dynamics.rtData.loudnessHistory1s, dynamics.rtData.loudnessHistory10s,
			dynamics.adaptedLoudness[i], dynamics.variance, dynamics.transientBoosts, bandPower, dynamics.blockDurationMs, dynamics.chkData.sampleRate,
			dynamics.spectralCentroid[i], dynamics.spectralFlatness[i], dynamics.spectralFlux[i], dynamics.rtData.genreFactor
		);
		dynamics.adaptedLoudness[i] += 3.0 * cogFactor; // Apply cognitive loudness adjustment (max 3 dB boost)
	}
}

void AudioWizardAnalysisRealTime::ProcessDynamicsTransientBoostsDetection(RealTimeDataDynamics& dynamics) {
	std::vector<double> loudnessNormalized = AWHAudioDSP::NormalizeLoudness(
		dynamics.adaptedLoudness, dynamics.blockDurationMs, 3000.0
	);

	dynamics.transientBoosts = AWHAudioDynamics::DetectTransients(
		loudnessNormalized, dynamics.blockDurationMs, dynamics.harmonicComplexityFactor,
		dynamics.maskingFactor,	dynamics.spectralFlux, dynamics.spectralCentroid,
		dynamics.spectralFlatness, dynamics.rtData.genreFactor,	dynamics.varianceScale
	);
}

void AudioWizardAnalysisRealTime::ProcessDynamicsTransientBoostsAdjustment(RealTimeDataDynamics& dynamics) {
	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (dynamics.adaptedLoudness[i] == -INFINITY) continue;

		dynamics.adaptedLoudness[i] = AWHAudioDynamics::ApplyTransientBoost(
			dynamics.adaptedLoudness[i], dynamics.transientBoosts[i], 0.7
		);
	}
}

void AudioWizardAnalysisRealTime::ProcessDynamicsSpread(RealTimeDataDynamics& dynamics) {
	// 1. Normalize loudness with 3000ms time constant
	const std::vector<double> loudnessNormalized = AWHAudioDSP::NormalizeLoudness(
		dynamics.adaptedLoudness, dynamics.blockDurationMs, 3000.0
	);

	// 2. Filter loudness values above perceptual threshold (-80 dB)
	std::vector<double> validLoudness;
	validLoudness.reserve(dynamics.blockCount);
	for (size_t i = 0; i < dynamics.blockCount; ++i) {
		if (loudnessNormalized[i] > -80.0) {
			validLoudness.push_back(loudnessNormalized[i]);
		}
	}
	if (validLoudness.empty()) return;

	// 3. Compute statistical measures for dynamics distribution
	const double mean = std::accumulate(validLoudness.begin(), validLoudness.end(), 0.0) / validLoudness.size();
	const double kurtosis = AWHMath::CalculateKurtosis(validLoudness, mean, dynamics.integratedLUFS);
	const double kurtosisFactor = std::clamp(kurtosis / 6.0, 0.5, 2.0);

	// 4. Calculate focus parameters
	const double iqrFocusFactor = std::clamp(0.5 - 0.2 * (kurtosisFactor - 1.0), 0.4, 0.8);
	const double rangeFocusFactor = std::clamp(0.5 + 0.2 * (kurtosisFactor - 1.0), 0.4, 0.8);

	// 5. Compute psychoacoustic parameters for temporal masking
	const double dynamicThreshold = std::max(dynamics.integratedLUFS - 22.0 + 4.0 * dynamics.rtData.genreFactor, -80.0);
	const double preMaskingMs = 20.0 + 30.0 * dynamics.varianceScale; // 20–50 ms
	const double postMaskingMs = 100.0 + 140.0 * dynamics.varianceScale; // 100–240 ms
	const std::vector<double> temporalWeights = AWHAudioDynamics::ComputeTemporalWeights(
		loudnessNormalized, dynamics.blockDurationMs, preMaskingMs, postMaskingMs, dynamics.variance
	);

	// 6. Define window parameters for real-time processing
	const size_t shortWindowBlocks = 10 + static_cast<size_t>(10 * (1.0 - dynamics.varianceScale));
	const size_t minBlocks = shortWindowBlocks / 2;
	const double updateRateMs = 50.0 + 50.0 * (1.0 - dynamics.varianceScale);
	const auto windowStep = static_cast<size_t>(updateRateMs * dynamics.chkData.sampleRate / 1000.0);
	const size_t start = dynamics.rtData.kWeightedBuffer.size() > static_cast<size_t>(1.5 * dynamics.chkData.sampleRate)
		? dynamics.rtData.kWeightedBuffer.size() - static_cast<size_t>(1.5 * dynamics.chkData.sampleRate) : 0;

	// 7. Compute dynamic spreads over short windows
	std::vector<double> shortSpreads;
	std::vector<double> weights;
	shortSpreads.reserve(4); // Approximate max windows in 1.5s
	weights.reserve(4);

	for (size_t windowStart = start; windowStart + shortWindowBlocks <= dynamics.rtData.kWeightedBuffer.size(); windowStart += windowStep) {
		std::vector<double> windowLoudness;
		windowLoudness.reserve(shortWindowBlocks);
		size_t numBlocks = 0;
		for (size_t blockIdx = windowStart; blockIdx + dynamics.rtData.blockSize <= dynamics.rtData.kWeightedBuffer.size() && numBlocks < shortWindowBlocks; blockIdx += dynamics.rtData.blockSize) {
			const size_t loudnessIdx = (blockIdx - start) / dynamics.rtData.blockSize;
			if (loudnessIdx >= dynamics.adaptedLoudness.size() || loudnessIdx >= temporalWeights.size()) continue;
			if (dynamics.adaptedLoudness[loudnessIdx] > dynamicThreshold) {
				windowLoudness.push_back(loudnessNormalized[loudnessIdx] * temporalWeights[loudnessIdx]);
			}
			++numBlocks;
		}
		if (numBlocks < minBlocks || windowLoudness.empty()) continue;

		// Compute spread and weight
		const double pureDynamics = AWHAudioDynamics::ComputeDynamicSpread(windowLoudness, -INFINITY, iqrFocusFactor, 1.5);
		shortSpreads.push_back(pureDynamics);

		std::sort(windowLoudness.begin(), windowLoudness.end());
		const double iqr = windowLoudness.size() > 3 ? windowLoudness[3 * windowLoudness.size() / 4] - windowLoudness[windowLoudness.size() / 4] : 0.0;
		const double avgLoudness = std::accumulate(windowLoudness.begin(), windowLoudness.end(), 0.0) / (windowLoudness.size() + 1e-12);
		const double dynamicWeight = std::min(1.5, iqr / 10.0);
		const double loudnessFactor = std::pow(10.0, (avgLoudness + 80.0) / 80.0);
		weights.push_back(0.5 + 0.5 * dynamicWeight + 0.5 * loudnessFactor);
	}

	if (shortSpreads.empty()) return;

	// 8. Aggregate spreads
	const double weightedAverage = AWHMath::CalculateWeightedAverage(shortSpreads, weights);
	const double median = AWHMath::CalculateMedian(shortSpreads);
	const double baseSpread = rangeFocusFactor * median + (1.0 - rangeFocusFactor) * weightedAverage;

	// 9. Apply adjustments
	const double kurtosisAdjustment = 1.0 + 0.1 * (kurtosisFactor - 1.0);
	const double genreAdjustment = 0.9 + 0.2 * dynamics.rtData.genreFactor;
	const double finalSpread = baseSpread * kurtosisAdjustment * genreAdjustment;

	// 10. Smooth output with EMA
	const double emaSmoothingFactor = 0.5 + 0.2 * dynamics.varianceScale;
	dynamics.rtData.gatingLoudnessEMA = emaSmoothingFactor * mean + (1.0 - emaSmoothingFactor) * dynamics.rtData.gatingLoudnessEMA;
	dynamics.rtData.pureDynamicsEMA = emaSmoothingFactor * finalSpread + (1.0 - emaSmoothingFactor) * dynamics.rtData.pureDynamicsEMA;
}

void AudioWizardAnalysisRealTime::ProcessDynamicsFactors(const ChunkData& chkData, RealTimeData& rtData) {
	constexpr double MIN_ENERGY = 1e-12;
	constexpr double SPECTRAL_FLUX_THRESHOLD = 0.05;

	// Preallocate rtData members
	size_t blockCount = chkData.frames / rtData.blockSize;
	rtData.bandPowers = std::vector<std::vector<double>>(blockCount, std::vector<double>(AWHAudioFFT::BARK_BAND_NUMBER, AWHAudioFFT::EPSILON));
	rtData.frequencyPowers = std::vector<double>(blockCount, AWHAudioFFT::EPSILON);
	rtData.harmonicComplexityFactor = std::vector<double>(blockCount, 0.0);
	rtData.maskingFactor = std::vector<double>(blockCount, 1.0);
	rtData.spectralCentroid = std::vector<double>(blockCount, 0.0);
	rtData.spectralFlatness = std::vector<double>(blockCount, 1.0);
	rtData.spectralFlux = std::vector<double>(blockCount, 0.0);

	// Preallocate buffers for FFT and spectral processing
	std::vector<double> blockSamples(rtData.fftSize, 0.0);
	std::vector<std::complex<double>> fftOutput(rtData.fftSize);
	std::vector<double> powerSpectrum(rtData.fftSize / 2 + 1);

	// Determine if FFT is needed
	bool computeFFT = (rtData.blocksTotal % 3 == 0);
	double lastFlux = rtData.spectralFluxSum / (rtData.blocksTotal > 0 ? rtData.blocksTotal : 1);
	if (lastFlux > SPECTRAL_FLUX_THRESHOLD) computeFFT = true;

	if (computeFFT) {
		for (size_t i = 0; i < blockCount; ++i) {
			size_t startSample = i * rtData.blockSize * chkData.channels;
			if (startSample + rtData.blockSize * chkData.channels > chkData.frames * chkData.channels) break;

			// Process block
			double energy;
			const audioType* blockPtr = chkData.data + startSample;
			AWHAudioDSP::ComputeBlockSamplesAndEnergy(blockPtr, 0, rtData.blockSize, chkData.channels, blockSamples, energy, &rtData.hannWindow);

			if (energy <= MIN_ENERGY) continue;

			// Compute FFT and power spectrum
			AWHAudioFFT::ComputeFFTPower2(blockSamples, fftOutput);
			AWHAudioFFT::ComputePowerSpectrum(fftOutput.data(), rtData.fftSize, rtData.blockSize, powerSpectrum);

			// Map to bark bands and compute spectral features
			AWHAudioFFT::MapPowerSpectrumToBarkBands(powerSpectrum, rtData.fftSize, chkData.sampleRate, rtData.bandPowers[i]);
			double freqPower = AWHAudioFFT::ComputePerceptualFrequencyPower(rtData.bandPowers[i], rtData.barkWeights);
			rtData.frequencyPowers[i] = (freqPower != -INFINITY ? freqPower : AWHAudioFFT::EPSILON);

			rtData.harmonicComplexityFactor[i] = AWHAudioFFT::ComputeHarmonicComplexity(rtData.bandPowers[i]);
			rtData.maskingFactor[i] = AWHAudioFFT::ComputeFrequencyMaskingFromPowerSpectrum(powerSpectrum, rtData.fftSize, chkData.sampleRate, rtData.bandPowers[i]);

			rtData.spectralCentroid[i] = AWHAudioFFT::ComputeSpectralCentroid(rtData.bandPowers[i], chkData.sampleRate);
			rtData.spectralFlatness[i] = AWHAudioFFT::ComputeSpectralFlatness(rtData.bandPowers[i], AWHAudioFFT::BARK_BAND_NUMBER);
			if (i > 0) {
				rtData.spectralFlux[i] = AWHAudioFFT::ComputeSpectralFlux(rtData.bandPowers[i], rtData.bandPowers[i - 1], AWHAudioFFT::BARK_BAND_NUMBER);
			}
			else if (!rtData.bandPowersPrevious.empty()) {
				rtData.spectralFlux[i] = AWHAudioFFT::ComputeSpectralFlux(rtData.bandPowers[i], rtData.bandPowersPrevious, AWHAudioFFT::BARK_BAND_NUMBER);
			}

			// Update running sums
			rtData.spectralCentroidSum += rtData.spectralCentroid[i];
			rtData.spectralFlatnessSum += rtData.spectralFlatness[i];
			rtData.spectralFluxSum += rtData.spectralFlux[i];
			rtData.blocksTotal++;
		}

		// Update band powers history
		if (!rtData.bandPowers.empty()) {
			rtData.bandPowersPrevious = rtData.bandPowers.back();
			for (const auto& bp : rtData.bandPowers) {
				for (const auto& power : bp) {
					rtData.bandPowersHistory.pushBack(power);
				}
			}
		}
	}

	// Update genre factor every ~100 blocks
	if (rtData.blocksTotal >= 100) {
		double centroidMean = rtData.spectralCentroidSum / rtData.blocksTotal;
		double flatnessMean = rtData.spectralFlatnessSum / rtData.blocksTotal;
		double fluxMean = rtData.spectralFluxSum / rtData.blocksTotal;

		AWHAudioFFT::ComputeSpectralGenreFactors(
			{ centroidMean }, { flatnessMean }, { fluxMean },
			centroidMean, flatnessMean, fluxMean, rtData.genreFactor
		);

		rtData.spectralCentroidSum = 0.0;
		rtData.spectralFlatnessSum = 0.0;
		rtData.spectralFluxSum = 0.0;
		rtData.blocksTotal = 0;
	}
}
#pragma endregion


/////////////////////////////////////////////////
// * ANALYSIS REAL-TIME - GENERAL PROCESSING * //
/////////////////////////////////////////////////
#pragma region Analysis Real-Time - General Processing
double AudioWizardAnalysisRealTime::ProcessLUFS(const RingBufferSimple& buffer, size_t maxSamples) {
	if (buffer.size() == 0) return -INFINITY;

	// Use only the most recent maxSamples
	size_t start = buffer.size() > maxSamples ? buffer.size() - maxSamples : 0;
	double sum = 0.0;

	for (size_t i = start; i < buffer.size(); ++i) {
		sum += buffer[i];
	}

	double mean = sum / std::min(buffer.size(), maxSamples);

	return -0.691 + AWHAudio::PowerToDb(mean);
}

void AudioWizardAnalysisRealTime::ProcessIntegratedLUFS(const std::vector<double>& tempBuffer, RealTimeData& rtData) {
	constexpr double ABSOLUTE_GATE = -70.0; // EBU R128 absolute gate
	constexpr double RELATIVE_GATE = -10.0; // EBU R128 relative gate

	for (double framePower : tempBuffer) {
		rtData.currentBlockSum += framePower;
		rtData.currentBlockFrames++;

		if (rtData.currentBlockFrames == rtData.blockSize) {
			double block_power = rtData.currentBlockSum;
			rtData.integratedLUFSBuffer.pushBack(block_power);
			rtData.currentBlockSum = 0.0;
			rtData.currentBlockFrames = 0;
		}
	}

	// Compute integrated LUFS with two-stage gating
	if (rtData.integratedLUFSBuffer.size() > 0) {
		// Stage 1: Absolute gate at -70 LUFS
		double sum_power = 0.0;
		size_t count = 0;
		for (size_t i = 0; i < rtData.integratedLUFSBuffer.size(); ++i) {
			double block_power = rtData.integratedLUFSBuffer[i];
			double block_loudness = -0.691 + AWHAudio::PowerToDb(block_power / rtData.blockSize);
			if (block_loudness > ABSOLUTE_GATE) {
				sum_power += block_power;
				count++;
			}
		}

		// Stage 2: Relative gate at -10 LUFS
		rtData.gatedPowerSum = 0.0;
		rtData.gatedBlockCount = 0;
		if (count > 0) {
			double mean_power = sum_power / static_cast<double>(count * rtData.blockSize);
			double integrated_lufs = -0.691 + AWHAudio::PowerToDb(mean_power);
			double relative_threshold = integrated_lufs + RELATIVE_GATE;

			for (size_t i = 0; i < rtData.integratedLUFSBuffer.size(); ++i) {
				double block_power = rtData.integratedLUFSBuffer[i];
				double block_loudness = -0.691 + AWHAudio::PowerToDb(block_power / rtData.blockSize);
				if (block_loudness > relative_threshold) {
					rtData.gatedPowerSum += block_power;
					rtData.gatedBlockCount++;
				}
			}
		}

		// Update PLR
		if (rtData.gatedBlockCount > 0) {
			double mean_power = rtData.gatedPowerSum / static_cast<double>(rtData.gatedBlockCount * rtData.blockSize);
			rtData.integratedLUFS = -0.691 + AWHAudio::PowerToDb(mean_power);
			rtData.PLR = AWHMath::RoundTo(GetPLR(rtData.truePeak, rtData.integratedLUFS), 1);
		}
		else {
			rtData.integratedLUFS = -INFINITY;
			rtData.PLR = -INFINITY;
		}
	}
}

std::pair<double, double> AudioWizardAnalysisRealTime::ProcessFrameRMS(const ChunkData& chkData) {
	double sumSquaresLeft = 0.0;
	double sumSquaresRight = 0.0;
	const bool isStereo = chkData.channels >= 2;

	for (size_t i = 0; i < chkData.frames; ++i) {
		const size_t idx = i * chkData.channels;
		const double left = chkData.data[idx];
		const double right = isStereo ? chkData.data[idx + 1] : left;
		sumSquaresLeft += left * left;
		sumSquaresRight += right * right;
	}

	const auto samples = static_cast<double>(chkData.frames);
	const double rmsLinearLeft = std::sqrt(sumSquaresLeft / samples);
	const double rmsLinearRight = std::sqrt(sumSquaresRight / samples);

	return {
		rmsLinearLeft > 0.0 ? AWHAudio::LinearToDb(rmsLinearLeft) : -INFINITY,
		rmsLinearRight > 0.0 ? AWHAudio::LinearToDb(rmsLinearRight) : -INFINITY
	};
}

double AudioWizardAnalysisRealTime::ProcessFramePeak(const ChunkData& chkData) {
	double framePeak = 0.0;

	for (size_t i = 0; i < chkData.frames; ++i) {
		for (size_t ch = 0; ch < chkData.channels; ++ch) {
			double sample = chkData.data[i * chkData.channels + ch];
			framePeak = std::max(framePeak, std::abs(sample));
		}
	}

	return framePeak;
}

std::pair<double, double> AudioWizardAnalysisRealTime::ProcessFramePeaks(const ChunkData& chkData) {
	double framePeakLeft = 0.0;
	double framePeakRight = 0.0;
	const bool isStereo = chkData.channels >= 2;

	for (size_t i = 0; i < chkData.frames; ++i) {
		const size_t idx = i * chkData.channels;
		const double left = chkData.data[idx];
		const double right = isStereo ? chkData.data[idx + 1] : left;
		framePeakLeft = std::max(framePeakLeft, std::abs(left));
		framePeakRight = std::max(framePeakRight, std::abs(right));
	}

	return {
		framePeakLeft > 0.0 ? AWHAudio::LinearToDb(framePeakLeft) : -INFINITY,
		framePeakRight > 0.0 ? AWHAudio::LinearToDb(framePeakRight) : -INFINITY
	};
}
#pragma endregion


//////////////////////////////////////////////
// * ANALYSIS REAL-TIME - MAIN PROCESSING * //
//////////////////////////////////////////////
#pragma region Analysis Real-Time - Main Processing
void AudioWizardAnalysisRealTime::InitRealTimeState(const ChunkData& chkData, RealTimeData& rtData) {
	if (rtData.filterData.sampleRate != 0.0) return; // Prevent re-initialization

	rtData.filterData.sampleRate = chkData.sampleRate;
	rtData.blockSize = static_cast<size_t>(0.1 * chkData.sampleRate); // 100ms blocks for loudness
	rtData.blockDurationMs = (static_cast<double>(rtData.blockSize) / chkData.sampleRate) * 1000.0;

	// Loudness buffers
	auto blocks100ms = static_cast<size_t>(100.0 / rtData.blockDurationMs); // ~100ms
	auto blocks1s = static_cast<size_t>(1000.0 / rtData.blockDurationMs);   // ~1s
	auto blocks10s = static_cast<size_t>(10000.0 / rtData.blockDurationMs); // ~10s
	rtData.loudnessHistory100ms.reset(std::min(blocks100ms, BufferSettings::BUFFER_CAPACITY_HISTORY_MID));
	rtData.loudnessHistory1s.reset(std::min(blocks1s, BufferSettings::BUFFER_CAPACITY_HISTORY_MID));
	rtData.loudnessHistory10s.reset(std::min(blocks10s, BufferSettings::BUFFER_CAPACITY_HISTORY_MID));
	rtData.kWeightedBuffer.reset(static_cast<size_t>(3.0 * chkData.sampleRate)); // 3s for K-weighting
	rtData.shortTermLUFSBuffer.reset(static_cast<size_t>(30.0 * chkData.sampleRate / rtData.blockSize)); // 30s
	rtData.integratedLUFSBuffer.reset(static_cast<size_t>(30.0 * chkData.sampleRate / rtData.blockSize)); // 30s

	// DR buffers
	auto chunksPer3Seconds = static_cast<size_t>(3.0 * chkData.sampleRate / chkData.frames); // 3s window (30 chunks at 100ms)
	rtData.drRMSBuffer.reset(chunksPer3Seconds);
	rtData.drPeakBuffer.reset(chunksPer3Seconds);
	rtData.drRMSBufferLeft.reset(chunksPer3Seconds);
	rtData.drRMSBufferRight.reset(chunksPer3Seconds);
	rtData.drPeakBufferLeft.reset(chunksPer3Seconds);
	rtData.drPeakBufferRight.reset(chunksPer3Seconds);
	rtData.drPrevious = 0.0;

	// Dynamics
	double targetBinWidth = chkData.sampleRate / rtData.blockSize; // Frequency resolution based on blockSize
	rtData.fftSize = AWHAudioFFT::CalculateFFTSize(true, chkData.sampleRate, targetBinWidth, rtData.blockSize);
	rtData.hannWindow = AWHAudioDSP::GenerateHannWindow(rtData.blockSize);
	rtData.barkWeights = AWHAudioFFT::ComputeBarkWeights(chkData.sampleRate);
	rtData.bandPowersHistory.reset(10 * AWHAudioFFT::BARK_BAND_NUMBER);

	AudioWizardAnalysisFilter::InitInterpolation(chkData, rtData.filterData);
}

void AudioWizardAnalysisRealTime::ProcessRealtimeChunk(const ChunkData& chkData, RealTimeData& rtData) {
	InitRealTimeState(chkData, rtData);

	// Process K-weighted chunk
	std::vector<double> tempBuffer;
	AudioWizardAnalysisFilter::ProcessKWeightedChunk(chkData, rtData.filterData, tempBuffer);
	rtData.kWeightedBuffer.append(tempBuffer);

	// Compute and store Short-Term LUFS
	rtData.shortTermLUFS = AWHMath::RoundTo(GetShortTermLUFS(chkData, rtData), 1);
	rtData.shortTermLUFSBuffer.pushBack(rtData.shortTermLUFS);

	// Process integrated LUFS
	ProcessIntegratedLUFS(tempBuffer, rtData);

	// Process Dynamics
	ProcessDynamicsFactors(chkData, rtData);

	// Compute other metrics
	auto [leftFrameRMS, rightFrameRMS] = ProcessFrameRMS(chkData);
	auto [leftFramePeak, rightFramePeak] = ProcessFramePeaks(chkData);
	rtData.leftRMS = AWHMath::RoundTo(leftFrameRMS, 1);
	rtData.rightRMS = AWHMath::RoundTo(rightFrameRMS, 1);
	rtData.leftSamplePeak = AWHMath::RoundTo(leftFramePeak, 1);
	rtData.rightSamplePeak = AWHMath::RoundTo(rightFramePeak, 1);

	rtData.momentaryLUFS = AWHMath::RoundTo(GetMomentaryLUFS(chkData, rtData), 1);
	rtData.truePeak = AWHMath::RoundTo(GetTruePeak(chkData, rtData), 1);
	rtData.RMS = AWHMath::RoundTo(GetRMS(chkData), 1);
	rtData.PSR = AWHMath::RoundTo(GetPSR(chkData, rtData), 1);
	rtData.crestFactor = AWHMath::RoundTo(GetCrestFactor(chkData), 1);
	rtData.dynamicRange = AWHMath::RoundTo(GetDynamicRange(chkData, rtData), 1);
	rtData.pureDynamics = AWHMath::RoundTo(GetPureDynamics(chkData, rtData), 1);
	rtData.phaseCorrelation = AWHMath::RoundTo(GetPhaseCorrelation(chkData), 1);
	rtData.stereoWidth = AWHMath::RoundTo(GetStereoWidth(chkData), 1);
}
#pragma endregion
