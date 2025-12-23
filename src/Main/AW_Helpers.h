/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Helpers Header File                        * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "AW_Settings.h"


///////////////////////
// * AUDIO HELPERS * //
///////////////////////
#pragma region Audio Helpers
namespace AWHAudio {
	double GetFrequencyLimit(double sampleRate, bool ignoreFrequencyMax = false);
	double DbToLinear(double db);
	double LinearToDb(double linear);
	double PowerToDb(double power);
	double DbToPower(double db);
}
#pragma endregion


//////////////////////////////
// * AUDIO BUFFER HELPERS * //
//////////////////////////////
#pragma region Audio Buffer Helpers
namespace AWHAudioBuffer {
	// * BUFFER * //
	struct BufferSettings {
		// Low Tier => 6 GB RAM
		static const size_t BUFFER_CAPACITY_AUDIO_LOW = 2000000;    // ~16 MB (RingBuffer, audio samples)
		static const size_t BUFFER_CAPACITY_SUMS_LOW = 100000;      // ~800 KB (RingBufferSimple, block sums)
		static const size_t BUFFER_CAPACITY_HISTORY_LOW = 10000;    // ~80 KB (RingBufferSimple, loudness history)
		static const size_t BUFFER_CAPACITY_CHUNK_SEC_LOW = 3;      // 3 seconds buffer chunks
		// Mid Tier => 16 GB RAM
		static const size_t BUFFER_CAPACITY_AUDIO_MID = 5000000;    // ~40 MB (RingBuffer, audio samples)
		static const size_t BUFFER_CAPACITY_SUMS_MID = 250000;      // ~2 MB (RingBufferSimple, block sums)
		static const size_t BUFFER_CAPACITY_HISTORY_MID = 20000;    // ~160 KB (RingBufferSimple, loudness history)
		static const size_t BUFFER_CAPACITY_CHUNK_SEC_MID = 6;      // 6 seconds buffer chunks
		// High Tier => 32 GB RAM
		static const size_t BUFFER_CAPACITY_AUDIO_HIGH = 10000000;  // ~80 MB (RingBuffer, audio samples)
		static const size_t BUFFER_CAPACITY_SUMS_HIGH = 500000;     // ~4 MB (RingBufferSimple, block sums)
		static const size_t BUFFER_CAPACITY_HISTORY_HIGH = 40000;   // ~320 KB (RingBufferSimple, loudness history)
		static const size_t BUFFER_CAPACITY_CHUNK_SEC_HIGH = 12;    // 12 seconds buffer chunks
	};

	template<typename T>
	class DoubleBuffer {
	public:
		explicit DoubleBuffer(size_t size) : buffers{ std::vector<T>(size), std::vector<T>(size) }, size(size) {
			if (size == 0) {
				FB2K_console_formatter() << "Audio Wizard => DoubleBuffer: Invalid size";
				throw std::invalid_argument("DoubleBuffer size must be non-zero");
			}
		}

		size_t capacity() const {
			return size;
		}

		bool write(const T* data, size_t count) {
			if (!data || count > size) {
				FB2K_console_formatter() << "Audio Wizard => DoubleBuffer::write: Invalid data or count exceeds size (" << count << " > " << size << ")";
				return false;
			}
			std::lock_guard<std::mutex> lock(mutex);
			size_t back = 1 - active.load(std::memory_order_relaxed);
			std::fill(buffers[back].begin(), buffers[back].end(), T{});
			memcpy(buffers[back].data(), data, count * sizeof(T));
			validCount = count;
			active.store(back, std::memory_order_release);
			return true;
		}

		bool read(T* data, size_t count, size_t* readCount) {
			if (!data || !readCount || count > size) {
				FB2K_console_formatter() << "Audio Wizard => DoubleBuffer::read: Invalid data, readCount, or count exceeds size (" << count << " > " << size << ")";
				return false;
			}
			std::lock_guard<std::mutex> lock(mutex);
			size_t front = active.load(std::memory_order_acquire);
			size_t valid = validCount;
			count = std::min(count, valid);
			if (count == 0) {
				*readCount = 0;
				FB2K_console_formatter() << "Audio Wizard => DoubleBuffer::read: No valid samples (validCount = 0)";
				return false;
			}
			memcpy(data, buffers[front].data(), count * sizeof(T));
			*readCount = count;
			FB2K_console_formatter() << "Audio Wizard => DoubleBuffer::read: Read " << count << " samples";
			return true;
		}

	private:
		std::array<std::vector<T>, 2> buffers;
		const size_t size;
		std::atomic<size_t> active{ 0 };
		size_t validCount{ 0 };
		std::mutex mutex;
	};

	template<typename T>
	class RingBuffer {
	public:
		explicit RingBuffer(size_t capacity) :
			buffer(static_cast<T*>(operator new[](capacity * sizeof(T), std::align_val_t{ 64 }))),
			capacity(capacity) {
			if (capacity == 0) {
				FB2K_console_formatter() << "Audio Wizard => RingBuffer: Invalid capacity";
				throw std::invalid_argument("RingBuffer capacity must be non-zero");
			}
		}

		~RingBuffer() {
			operator delete[](buffer, std::align_val_t{ 64 });
		}

		bool write(const T* data, size_t count) {
			size_t currentWrite = writePos.load(std::memory_order_relaxed);
			size_t currentRead = readPos.load(std::memory_order_acquire);
			size_t avail = (currentRead > currentWrite)
				? (currentRead - currentWrite - 1)
				: (capacity - currentWrite + currentRead - 1);

			if (count > avail) {
				overflowCount.fetch_add(1, std::memory_order_relaxed);
				return false;
			}

			size_t firstPart = std::min(count, capacity - currentWrite);
			memcpy(&buffer[currentWrite], data, firstPart * sizeof(T));
			if (count > firstPart) {
				memcpy(&buffer[0], data + firstPart, (count - firstPart) * sizeof(T));
			}

			size_t newWrite = (currentWrite + count) % capacity;
			size_t expected = currentWrite;
			while (!writePos.compare_exchange_weak(expected, newWrite, std::memory_order_release)) {
				expected = writePos.load(std::memory_order_relaxed);
			}
			return true;
		}

		bool read(T* dest, size_t count, size_t* readCount) const {
			size_t currentWrite = writePos.load(std::memory_order_acquire);
			size_t currentRead = readPos.load(std::memory_order_relaxed);
			size_t avail = (currentWrite >= currentRead)
				? (currentWrite - currentRead)
				: (capacity - currentRead + currentWrite);

			count = std::min(count, avail);
			*readCount = count;
			if (count == 0) return false;

			size_t firstPart = std::min(count, capacity - currentRead);
			memcpy(dest, &buffer[currentRead], firstPart * sizeof(T));
			if (count > firstPart) {
				memcpy(dest + firstPart, &buffer[0], (count - firstPart) * sizeof(T));
			}

			size_t newRead = (currentRead + count) % capacity;
			size_t expected = currentRead;
			while (!readPos.compare_exchange_weak(expected, newRead, std::memory_order_release)) {
				expected = readPos.load(std::memory_order_relaxed);
			}
			return true;
		}

		size_t available() const {
			size_t currentWrite = writePos.load(std::memory_order_acquire);
			size_t currentRead = readPos.load(std::memory_order_relaxed);
			return (currentWrite >= currentRead)
				? (currentWrite - currentRead)
				: (capacity - currentRead + currentWrite);
		}

		void advance(size_t count) {
			size_t currentRead = readPos.load(std::memory_order_relaxed);
			size_t newRead = (currentRead + count) % capacity;
			size_t expected = currentRead;
			while (!readPos.compare_exchange_weak(expected, newRead, std::memory_order_release)) {
				expected = readPos.load(std::memory_order_relaxed);
			}
		}

		void clear() {
			size_t expectedRead = readPos.load(std::memory_order_relaxed);
			while (!readPos.compare_exchange_weak(expectedRead, 0, std::memory_order_release)) {
				expectedRead = readPos.load(std::memory_order_relaxed);
			}
			size_t expectedWrite = writePos.load(std::memory_order_relaxed);
			while (!writePos.compare_exchange_weak(expectedWrite, 0, std::memory_order_release)) {
				expectedWrite = writePos.load(std::memory_order_relaxed);
			}
			overflowCount.store(0, std::memory_order_release);
		}

		size_t getOverflowCount() const {
			return overflowCount.load(std::memory_order_relaxed);
		}

	private:
		T* buffer;
		const size_t capacity;
		mutable std::atomic<size_t> readPos{ 0 };
		std::atomic<size_t> writePos{ 0 };
		std::atomic<size_t> overflowCount{ 0 };
	};

	class RingBufferSimple {
	public:
		explicit RingBufferSimple(size_t capacity) :
			buffer(std::make_unique<double[]>(capacity)), capacity(capacity) {
			if (capacity == 0) {
				FB2K_console_formatter() << "Audio Wizard => RingBufferSimple: Invalid capacity";
				throw std::invalid_argument("RingBufferSimple capacity must be non-zero");
			}
		}

		class Iterator {
		public:
			using iterator_category = std::bidirectional_iterator_tag;
			using value_type = double;
			using difference_type = std::ptrdiff_t;
			using pointer = double*;
			using reference = double&;

			Iterator(const RingBufferSimple* buffer, size_t index) : index(index), buffer(buffer) {}

			double operator*() const { return buffer->operator[](index); }
			Iterator& operator++() { ++index; return *this; }
			Iterator operator++(int) { Iterator tmp = *this; ++index; return tmp; }
			Iterator& operator--() { --index; return *this; }
			Iterator operator--(int) { Iterator tmp = *this; --index; return tmp; }
			bool operator==(const Iterator& other) const { return index == other.index; }
			bool operator!=(const Iterator& other) const { return index != other.index; }
			Iterator& operator+=(difference_type n) { index += n; return *this; }
			Iterator operator+(difference_type n) const { return Iterator(buffer, index + n); }
			difference_type operator-(const Iterator& other) const { return index - other.index; }

		private:
			size_t index;
			const RingBufferSimple* buffer;
		};

		Iterator begin() const { return Iterator(this, 0); }
		Iterator end() const { return Iterator(this, count); }

		void reset(size_t newCapacity) {
			if (newCapacity == 0) {
				FB2K_console_formatter() << "Audio Wizard => RingBufferSimple: Invalid capacity";
				throw std::invalid_argument("RingBufferSimple capacity must be non-zero");
			}
			buffer = std::make_unique<double[]>(newCapacity);
			capacity = newCapacity;
			start = 0;
			count = 0;
		}

		void pushBack(double value) {
			size_t index = (start + count) % capacity;
			buffer[index] = value;
			if (count < capacity) {
				++count;
			}
			else {
				start = (start + 1) % capacity;
			}
		}

		void append(const std::vector<double>& data) {
			for (double value : data) {
				pushBack(value);
			}
		}

		size_t size() const { return count; }

		double operator[](size_t i) const {
			return buffer[(start + i) % capacity];
		}

		void clear() {
			start = 0;
			count = 0;
		}

		void trim(size_t maxSize) {
			if (maxSize >= count) return;
			if (maxSize == 0) {
				clear();
				return;
			}
			size_t excess = count - maxSize;
			start = (start + excess) % capacity;
			count = maxSize;
		}

	private:
		std::unique_ptr<double[]> buffer;
		size_t capacity;
		size_t start = 0;
		size_t count = 0;
	};
}
#pragma endregion


////////////////////////////
// * AUDIO DATA HELPERS * //
////////////////////////////
#pragma region Audio Data Helpers
namespace AWHAudioData {
	inline service_ptr_t<visualisation_stream_v3> visStream;

	struct ChunkData {
		const audioType* data;
		std::vector<audioType> ownedData;
		unsigned channels;
		t_size frames;
		double sampleRate;

		ChunkData() : data(nullptr), channels(0), frames(0), sampleRate(0.0) {}

		explicit ChunkData(const audio_chunk_impl& chunk) :
			data(chunk.get_data()),
			channels(std::max(chunk.get_channels(), 1u)),
			frames(chunk.get_sample_count()),
			sampleRate(chunk.get_sample_rate()) {
		}

		void setOwnedData(std::vector<audioType>&& dataVec) {
			ownedData = std::move(dataVec);
			data = ownedData.data();
		}
	};

	struct ChunkMetadata {
		std::atomic<bool> isValid = false;
		std::atomic<double> timestamp = 0.0;

		ChunkMetadata() = default;
		ChunkMetadata(const ChunkMetadata& other) :
			isValid(other.isValid.load(std::memory_order_acquire)),
			timestamp(other.timestamp.load(std::memory_order_acquire)) {}

		ChunkMetadata& operator=(const ChunkMetadata& other) {
			if (this != &other) {
				isValid.store(other.isValid.load(std::memory_order_acquire), std::memory_order_release);
				timestamp.store(other.timestamp.load(std::memory_order_acquire), std::memory_order_release);
			}
			return *this;
		}
	};

	struct Chunk {
		std::shared_ptr<audio_chunk_impl> chunk;
		ChunkMetadata metadata;
		Chunk() : chunk(std::make_shared<audio_chunk_impl>()) {}
		Chunk(const Chunk& other) = default;
		Chunk& operator=(const Chunk& other) = default;
		Chunk(Chunk&&) = default;
		Chunk& operator=(Chunk&&) = default;
		~Chunk() = default;
	};
}
#pragma endregion


///////////////////////////
// * AUDIO DSP HELPERS * //
///////////////////////////
#pragma region Audio DSP Helpers
namespace AWHAudioDSP {
	using ChunkData = AWHAudioData::ChunkData;
	using RingBufferSimple = AWHAudioBuffer::RingBufferSimple;

	enum class WindowType { BLACKMAN, BLACKMAN_HARRIS, HAMMING, HANN, KAISER };
	inline thread_local std::unordered_map<size_t, std::vector<double>> hannWindowCache;

	void ApplyDecayToHeldValueDb(double& heldValue, double newValue, double decayBelowThresholdDb, double decayAboveThresholdDb, double threshold);
	double CalculateAveragePeakDb(const std::vector<double>& peakHistory, size_t historySize);
	double CalculateDesiredGainDb(double currentPeakDb, double targetPeakDb, double maxGainCeilingDb, double noiseFloorDb);
	double CalculateExponentialAverage(const RingBufferSimple& history, double decay);

	std::vector<double> ComputeBlockLoudness(const RingBufferSimple& kWeightedBuffer, size_t start, size_t blockSize);
	void ComputeBlockSamplesAndEnergy(const audioType* buffer, size_t startIdx, size_t blockSize, size_t channels, std::vector<double>& blockSamples, double& energy, const std::vector<double>* hannWindow);
	void ComputeBlockSamplesAndEnergy(const std::vector<audioType>& buffer, size_t startIdx, size_t blockSize, size_t channels, std::vector<double>& blockSamples, double& energy, const std::vector<double>* hannWindow = nullptr);
	std::vector<double> ComputeRawSamples(const double* samples, size_t frameCount, size_t channelCount);

	void DownmixToStereo(const audioType* multiChannelData, size_t frames, size_t channels, double* stereoData);
	void ExtractStereoChannels(const audioType* block, size_t stepSize, size_t numChannels, std::vector<double>& leftChannel, std::vector<double>& rightChannel);
	std::vector<double> GenerateAudioWindow(WindowType windowType, size_t taps, double beta = 5.0);
	std::vector<double> GenerateHannWindow(size_t windowSize);
	std::vector<double> NormalizeLoudness(const std::vector<double>& loudness, double blockDurationMs, double windowMs);
	void ResampleToSampleRate(const ChunkData& inputChunk, ChunkData& outputChunk, double targetSampleRate, size_t taps, WindowType windowType = WindowType::KAISER, double beta = 5.0);
	double SmoothValue(double current, double target, double attackCoeff, double releaseCoeff);
};
#pragma endregion


////////////////////////////////
// * AUDIO DYNAMICS HELPERS * //
////////////////////////////////
#pragma region Audio Dynamics Helpers
namespace AWHAudioDynamics {
	using RingBufferSimple = AWHAudioBuffer::RingBufferSimple;

	struct ModulationHistory {
		std::vector<std::unique_ptr<RingBufferSimple>> bandPowerHistory;

		ModulationHistory(size_t numBands, size_t historySize) {
			bandPowerHistory.reserve(numBands);
			for (size_t i = 0; i < numBands; ++i) {
				bandPowerHistory.push_back(std::make_unique<RingBufferSimple>(historySize));
			}
		}

		void update(const std::vector<double>& bandPowers) {
			for (size_t b = 0; b < bandPowers.size() && b < bandPowerHistory.size(); ++b) {
				bandPowerHistory[b]->pushBack(bandPowers[b]);
			}
		}
	};

	double ApplyPerceptualLoudnessAdaptation(double blockLufs, double stableDuration, double refLufs, double tau, double adaptStrength);
	double ApplyPerceptualLoudnessCorrection(bool isRealTime, double fastlAdjustments, double blockLufs, double integratedLUFS, double highFreqPower, double variance, double meanPower);
	double ApplyTransientBoost(double baseLufs, double transientBoost, double weight);

	std::vector<double> ComputeBaseBlockLoudness(const RingBufferSimple& blockSums, size_t blockSize, double integratedLUFS, double silenceThreshold, std::vector<double>* validLoudness);

	double ComputeBinauralPerception(bool isRealTime, bool ildOnly, size_t blockSize, double sampleRate, const double* leftChannel, const double* rightChannel);
	double ComputeCognitiveLoudness(bool isRealTime, const RingBufferSimple& loudnessHistory100ms, const RingBufferSimple& loudnessHistory1s, const RingBufferSimple& loudnessHistory10s,
		double currentLufs, double variance, const std::vector<double>& transientBoosts, const std::vector<double>& bandPower, double blockDurationMs, double sampleRate,
		double spectralCentroid, double spectralFlatness, double spectralFlux, double genreFactor
	);
	void ComputeFastlPrinciples(
		bool isRealTime, std::vector<double>& fastlAdjustments, const std::vector<std::vector<double>>& bandPowers,
		const std::vector<double>& blockLoudness, size_t stepSize, double sampleRate, double variance, double varianceScale, const RingBufferSimple* history
	);
	void ComputePerceptualLoudnessCorrection(
		bool isRealTime, const std::vector<double>& fastlAdjustments, const std::vector<double>& blockLoudness,
		const std::vector<double>& highFreqPowers, double integratedLUFS, double variance, std::vector<double>& correctedLoudness, std::vector<double>& validLoudness,
		const RingBufferSimple* offlineBlockSums, size_t offlineStepSize
	);
	double ComputeDynamicSpread(const std::vector<double>& loudness, double threshold, double alpha, double scaleFactor);
	double ComputeFluctuationStrength(bool isRealTime, const std::vector<double>& bandPowers, double f_mod, double mod_depth);
	double ComputeOnsetRate(const std::vector<double>& transientBoosts, double blockDurationMs, size_t windowBlocks);
	double ComputeRoughness(bool isRealTime, const std::vector<double>& bandPowers, const std::vector<double>& f_mod_per_band, double mod_depth);
	double ComputeSharpness(bool isRealTime, const std::vector<double>& bandPowers);

	double ComputePhrasingScore(const std::vector<double>& transientBoosts, double blockDurationMs, size_t currentBlock);
	double ComputeSpatialScore(const std::vector<double>& leftChannel, const std::vector<double>& rightChannel, size_t blockSize, double sampleRate);

	std::vector<double> ComputeTemporalWeights(const std::vector<double>& loudness, double blockDurationMs, double preMaskingMs, double postMaskingMs, double variance);
	double ComputeTransientDensity(const std::vector<double>& transientBoosts, double blockDurationMs);

	std::vector<double> DetectTransients(const std::vector<double>& inputLoudness, double blockDurationMs,
		const std::vector<double>& harmonicComplexityFactor, const std::vector<double>& maskingFactor,
		const std::vector<double>& spectralFlux, const std::vector<double>& spectralCentroid, const std::vector<double>& spectralFlatness,
		double genreFactor, double varianceScale
	);
};
#pragma endregion


///////////////////////////
// * AUDIO FFT HELPERS * //
///////////////////////////
#pragma region Audio FFT Helpers
namespace AWHAudioFFT {
	struct PairHash {
		template <typename T1, typename T2>
		std::size_t operator()(const std::pair<T1, T2>& p) const {
			auto h1 = std::hash<T1>{}(p.first);
			auto h2 = std::hash<T2>{}(p.second);
			return h1 ^ (h2 << 1);
		}
	};

	// Cache for bark band weights, bit-reversal indices, twiddle factors, Hann window and center frequencies globally for reuse across blocks
	inline thread_local std::unordered_map<std::pair<size_t, double>, std::vector<std::vector<std::pair<size_t, double>>>, PairHash> barkWeightCache;
	inline thread_local std::unordered_map<double, std::vector<double>> centerFreqsCache;
	inline thread_local std::unordered_map<size_t, std::vector<size_t>> bitReversalCache;
	inline thread_local std::unordered_map<size_t, std::vector<std::complex<double>>> twiddleCache;
	inline thread_local std::unordered_map<size_t, std::vector<std::complex<double>>> twiddleCacheReal;

	// Constants for psychoacoustic calculations
	constexpr int BARK_BAND_NUMBER = 25; // Number of Traunmüller Bark bands for loudness calculation (20 Hz to 20 kHz)
	constexpr double E_0 = 1e-12; // Reference intensity: 0 dB SPL (ISO 389-7:2019, 10^-12 W/m²)
	constexpr double EPSILON = 1e-12; // Small value to prevent division by zero
	constexpr double SPECIFIC_LOUDNESS_CONST = 0.0635; // Specific loudness rate constant (ISO 532-1:2017)

	// Tonotopic frequency edges (Hz) for 25 Bark bands, covering ~20.1 Hz to ~23.8 kHz.
	// Implements Traunmüller's (1990) corrected Bark scale with adjustments for low (<200 Hz)
	// and high (>12.5 kHz) frequencies to match experimental tonotopic data.
	extern const std::array<double, BARK_BAND_NUMBER + 1> BARK_BAND_FREQUENCY_EDGES;

	// Threshold intensities (W/m²) in quiet for each of the 25 Traunmüller bark bands (20 Hz to 20 kHz).
	// Generated using GNU Octave 10.2.0 by interpolating ISO 389-7:2019 free-field hearing thresholds (dB SPL, Table 1, Tf column)
	// to the center frequencies of Traunmüller bark bands, then converting to intensity using I = 10^(L/10) * 10^-12 W/m².
	// Traunmüller's (1990) bark scale is used with low-frequency (<2 Bark) and high-frequency (>20.1 Bark) corrections.
	static const std::array<double, BARK_BAND_NUMBER> BARK_QUIET_THRESHOLD_INTENSITIES = {
		1.210146e-09, 7.638664e-11, 1.660719e-11, 6.374928e-12, 3.724069e-12,
		2.584254e-12, 1.826482e-12, 1.325391e-12, 1.008215e-12, 8.256848e-13,
		7.381689e-13, 7.221948e-13, 7.618455e-13, 8.242796e-13, 8.736017e-13,
		9.060610e-13, 9.334855e-13, 1.005680e-12, 1.262954e-12, 2.012184e-12,
		4.596745e-12, 1.133413e-11, 1.701570e-10, 3.386632e-08, 1.000000e-07
	};

	void PrecomputeBitReversal(size_t N);
	void PrecomputeTwiddlesGeneral(size_t N);
	void PrecomputeTwiddlesPower2(size_t N);
	size_t CalculateFFTSize(bool usePower2, double sampleRate, double& targetBinWidth, size_t stepSize, size_t maxFftSize = 262144);
	void ComputeFFTBluestein(const std::vector<double>& input, std::vector<std::complex<double>>& output, const std::vector<double>& window);
	void ComputeFFTGeneral(const std::vector<double>& input, std::vector<std::complex<double>>& output, const std::vector<double>& window = {});
	void ComputeFFTPower2(const std::vector<double>& input, std::vector<std::complex<double>>& output, const std::vector<double>& window = {});
	void ComputeComplexFFTPower2(const std::vector<std::complex<double>>& input, std::vector<std::complex<double>>& output);
	void ComputeRFFT(const std::vector<double>& input, std::vector<std::complex<double>>& output);

	void ComputeBandPowers(const std::vector<double>& samples, size_t fftSize, size_t stepSize, double sampleRate, std::vector<double>& bandPower, std::vector<std::complex<double>>& fftOutput);
	void ComputeBandPowersFromPowerSpectrum(const std::vector<double>& powerSpectrum, size_t fftSize, double sampleRate, const std::vector<double>& barkBandPower, std::vector<double>& bandPower);
	std::vector<double> ComputeBarkCenterFrequencies(double sampleRate);
	double ComputeBarkToFrequencies(size_t barkBand, double sampleRate);
	std::array<double, BARK_BAND_NUMBER> ComputeBarkWeights(double sampleRate);

	double ComputeCriticalBands(const std::complex<double>* fftData, size_t fftSize, size_t stepSize, double sampleRate);
	double ComputeCriticalBandsFromPowerSpectrum(const std::vector<double>& powerSpectrum, size_t fftSize, double sampleRate, const std::vector<double>& barkBandPower);
	double ComputeFrequencyMasking(const std::complex<double>* fftData, size_t fftSize, size_t stepSize, double sampleRate);
	double ComputeFrequencyMaskingFromPowerSpectrum(const std::vector<double>& powerSpectrum, size_t fftSize, double sampleRate, const std::vector<double>& barkBandPower);
	double ComputePerceptualFrequencyPower(const std::vector<double>& bandPower, const std::array<double, BARK_BAND_NUMBER>& barkWeights);
	std::vector<double> ComputePerceptualFrequencyPowers(const std::vector<std::vector<double>>& allBandPowers, const std::array<double, BARK_BAND_NUMBER>& barkWeights);
	void ComputePowerSpectrum(const std::complex<double>* fftData, size_t fftSize, size_t stepSize, std::vector<double>& powerSpectrum);

	double ComputeHarmonicComplexity(const std::vector<double>& bandPower);
	double ComputeSpectralCentroid(const std::vector<double>& bandPowers, double sampleRate);
	double ComputeSpectralFlatness(const std::vector<double>& bandPowers, size_t numBands);
	double ComputeSpectralFlux(const std::vector<double>& currentBandPowers, const std::vector<double>& prevBandPowers, size_t numBands);
	void ComputeSpectralGenreFactors(const std::vector<double>& spectralCentroid, const std::vector<double>& spectralFlatness, const std::vector<double>& spectralFlux,
		double& spectralCentroidMean, double& spectralFlatnessMean, double& spectralFluxMean, double& genreFactor
	);

	double EstimateModulationFrequency(const std::vector<std::vector<double>>& bandPowersHistory, size_t currentBlock, double blockDurationMs, size_t startBand, size_t bandCount);
	double MapBarkToFrequency(double z);
	double MapFrequencyToBark(double freq);
	void MapPowerSpectrumToBarkBands(const std::vector<double>& powerSpectrum, size_t fftSize, double sampleRate, std::vector<double>& bandPower);
}
#pragma endregion


/////////////////////
// * COM HELPERS * //
/////////////////////
#pragma region COM Helpers
namespace AWHCOM {
	HRESULT LogError(HRESULT errorCode, const std::wstring& source, const std::wstring& description, bool setErrorInfo = false);

	void CreateCallback(VARIANT& targetCallback, const VARIANT* newCallback, const char* callbackName);
	void FireCallback(const VARIANT& callback, bool success, const std::function<void()>& postAction = nullptr);

	metadb_handle_list GetMetadbHandlesFromStringArray(const VARIANT& metadata);
	HRESULT GetOptionalLong(const VARIANT* variant, LONG& output);

	class SafeArrayAccess {
	public:
		explicit SafeArrayAccess(SAFEARRAY* psa);
		~SafeArrayAccess();
		float* getData() const { return data; }
		HRESULT getHr() const { return hr; }

	private:
		SAFEARRAY* psa = nullptr;
		float* data = nullptr;
		HRESULT hr = S_OK;
		SafeArrayAccess(const SafeArrayAccess&) = delete;
		SafeArrayAccess& operator=(const SafeArrayAccess&) = delete;
	};

	template<typename InputIt>
	SAFEARRAY* CreateSafeArrayFromData(InputIt begin, InputIt end, const char* context = "CreateSafeArrayFromData") {
		using value_type = typename std::iterator_traits<InputIt>::value_type;
		static_assert(std::is_arithmetic_v<value_type>, "Input iterator must yield arithmetic values");

		auto size = std::distance(begin, end);
		if (size < 0 || static_cast<ULONG>(size) > std::numeric_limits<ULONG>::max()) {
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
		for (auto it = begin; it != end; ++it) {
			*pData++ = static_cast<float>(*it);
		}

		return psa;
	}

	SAFEARRAY* CreateSafeArrayFromData(size_t size, float defaultValue, const char* context = "CreateSafeArrayFromData");
}
#pragma endregion


////////////////////////////
// * CONVERSION HELPERS * //
////////////////////////////
#pragma region Conversion Helpers
namespace AWHConvert {
	bool BOOLFromVARIANT(const VARIANT& var, bool defaultValue);
	double DOUBLEFromVARIANT(const VARIANT& var, double defaultValue);
	long INTFromVARIANT(const VARIANT& var, long defaultValue);
	short SHORTFromVARIANT(const VARIANT& var, short defaultValue);
	std::wstring STRINGFromVARIANT(const VARIANT& var, const std::wstring& defaultValue = L"");

	ULONGLONG FileTimeToUll(const FILETIME& ft);
	double MsToSec(int ms);
	int SecToMs(double sec);

	CSize DialogUnitsToPixel(HWND hWnd, int dluX, int dluY);
	int PercentToPixels(double percent, int dimension);
	double PixelsToPercent(int pixels, int dimension);
}
#pragma endregion


///////////////////////////
// * DARK MODE HELPERS * //
///////////////////////////
#pragma region Dark Mode Helpers
namespace AWHDarkMode {
	void AddControls(HWND wnd);
	void AddControlsWithExclude(HWND parent, HWND exclude);
	void AddCtrlAuto(HWND wnd);
	void AddDialog(HWND wnd);
	void AddDialogWithControls(HWND wnd);
	void Cleanup();
	bool IsDark();
	void SetDark(bool dark);
}
#pragma endregion


///////////////////////
// * DEBUG HELPERS * //
///////////////////////
#pragma region Debug Helpers
namespace AWHDebug {
	template<typename... Args>
	void DebugLog(const Args&... args) {
		if (AudioWizardSettings::systemDebugLog) {
			FB2K_console_print("Audio Wizard => ", args...);
		}
	}
}
#pragma endregion


////////////////////////
// * DIALOG HELPERS * //
////////////////////////
#pragma region Dialog Helpers
namespace AWHDialog {
	bool CreateCustomFont(CFont& font, int height, int weight, const TCHAR* faceName = _T("Tahoma"));

	bool GetCheckBoxState(HWND hWnd, int id);
	void SetCheckBox(HWND hWnd, int id, bool checked);

	void SetControlEnableState(HWND hWnd, const std::vector<int>& controlIDs, bool enabled);

	int GetDropDownIndex(HWND hWnd, int id);
	void SetDropDownMenu(HWND hWnd, int id, const std::vector<std::wstring>& items, int selectedItem);

	int GetInputFieldNumber(HWND hWnd, int id, BOOL* pSuccess = nullptr, BOOL signedValue = false);
	void SetInputFieldNumber(HWND hWnd, int id, int value, BOOL signedValue = false);

	pfc::string8 GetInputFieldText(HWND hWnd, int id);
	void SetInputFieldText(HWND hWnd, int id, const std::wstring& text);

	void SetSpinControlRange(HWND hWnd, int id, int minVal, int maxVal);
	LRESULT CALLBACK SpinControlProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	void SpinControlSubclass(HWND hWnd);
};
#pragma endregion


//////////////////////////
// * GRAPHICS HELPERS * //
//////////////////////////
#pragma region Graphics Helpers
namespace AWHGraphics {
	// Deleter for GDI objects
	template<typename GDIType>
	struct GDIDeleter {
		void operator()(GDIType obj) const noexcept {
			if (obj) ::DeleteObject(obj);
		}
	};

	// RAII wrapper for GDI object selection
	struct GDISelector {
		HDC hdc;
		HGDIOBJ oldObj;
		GDISelector(HDC dc, HGDIOBJ obj) : hdc(dc), oldObj(SelectObject(dc, obj)) {}
		~GDISelector() { if (oldObj) SelectObject(hdc, oldObj); }
	};

	// RAII wrapper for GDI objects
	template<typename T>
	using GDI = std::unique_ptr<std::remove_pointer_t<T>, GDIDeleter<T>>;
	using Bitmap = GDI<HBITMAP>;
	using Brush = GDI<HBRUSH>;
	using Font = GDI<HFONT>;
	using Palette = GDI<HPALETTE>;
	using Pen = GDI<HPEN>;
	using Region = GDI<HRGN>;

	class MemoryDC {
		public:
			MemoryDC(HDC hdcRef, int width, int height);
			~MemoryDC();

			HDC GetDC() const { return m_hdcMem; }
			const Bitmap& GetBitmap() const { return m_bitmap; }

			MemoryDC(const MemoryDC&) = delete;
			MemoryDC& operator=(const MemoryDC&) = delete;

		private:
			HDC m_hdcMem;
			Bitmap m_bitmap;
			GDISelector m_bmpSel;
	};

	// RAII wrapper for a window DC obtained via BeginPaint
	class WindowDC {
		public:
			explicit WindowDC(HWND hwnd);
			~WindowDC();

			HDC GetDC() const { return m_hdc; }
			const RECT& GetPaintRect() const { return m_ps.rcPaint; }

			WindowDC(const WindowDC&) = delete;
			WindowDC& operator=(const WindowDC&) = delete;

		private:
			HWND m_hwnd;
			HDC m_hdc;
			PAINTSTRUCT m_ps;
	};

	inline COLORREF ColorBlend(COLORREF foreground, COLORREF background, int alpha) {
		if (alpha <= 0) return background;
		if (alpha >= 255) return foreground;

		const int invAlpha = 255 - alpha;

		return RGB(
			(GetRValue(foreground) * alpha + GetRValue(background) * invAlpha) >> 8,
			(GetGValue(foreground) * alpha + GetGValue(background) * invAlpha) >> 8,
			(GetBValue(foreground) * alpha + GetBValue(background) * invAlpha) >> 8
		);
	}

	Bitmap CreateTheBitmap(int width, int height, UINT planes, UINT bitsPerPixel, const void* data);
	Brush CreateTheSolidBrush(COLORREF color);
	Brush CreateThePatternBrush(HBITMAP bitmap);
	Font CreateTheFont(int height, int width, const wchar_t* face, int weight = FW_NORMAL, DWORD italic = FALSE, DWORD underline = FALSE, DWORD strikeout = FALSE, DWORD charset = DEFAULT_CHARSET);
	Palette CreateThePalette(const LOGPALETTE* palette);
	Pen CreateThePen(int style, int width, COLORREF color);
	Region CreateTheRectRegion(const RECT& rect);

	void DrawTheHorizontalLine(HDC hdc, int xStart, int xEnd, int y, COLORREF color);
	void DrawTheRect(HDC hdc, const RECT& rect, COLORREF fillColor, COLORREF borderColor, int borderWidth = 0);
	void DrawTheText(HDC hdc, const RECT& rect, const CStringW& text, COLORREF color, HFONT font, UINT format);
};
#pragma endregion


//////////////////////
// * MATH HELPERS * //
//////////////////////
#pragma region Math Helpers
namespace AWHMath {
	double CalculateCoefficient(double deltaTime, double timeConstant);
	double CalculateEntropy(const std::vector<double>& features);
	double CalculateIQR(const std::vector<double>& data);
	double CalculateKurtosis(const std::vector<double>& data, double mean, double defaultValue, bool biasCorrected = false);
	double CalculateMean(const std::vector<double>& vec, double invalidValue = -INFINITY);
	double CalculateMedian(std::vector<double> data);
	double CalculatePercentile(const std::vector<double>& data, double percentile);
	double CalculateSmoothingFactor(double deltaTime, double timeConstant);
	double CalculateVariance(const std::vector<double>& data);
	double CalculateVarianceOnline(const std::vector<double>& data);
	double CalculateWeightedAverage(const std::vector<double>& values, const std::vector<double>& weights);

	double CalculateBesselI0(double x);
	double CalculateCosineFadeProgress(double elapsedTime, double fadeDuration);
	double CalculateEaseOutCubic(double t);
	double CalculateEaseOutExpo(double t);
	double CalculateEaseOutQuad(double t);

	double RoundTo(double value, int decimalPlaces);
	bool ValueOverThreshold(double newValue, double oldValue, double threshold);
};
#pragma endregion


//////////////////////
// * META HELPERS * //
//////////////////////
#pragma region Meta Helpers
namespace AWHMeta {
	unsigned GetBitDepth(const metadb_handle_ptr& track);
	pfc::string8 GetDuration(const metadb_handle_ptr& track);
	pfc::string8 GetFileFormat(const metadb_handle_ptr& track);
	pfc::string8 GetMetadataField(const metadb_handle_ptr& track, const char* field);
	pfc::string8 GetTechnicalInfoField(const metadb_handle_ptr& track, const char* field);
}
#pragma endregion


/////////////////////////////
// * PERFORMANCE HELPERS * //
/////////////////////////////
#pragma region Performance Helpers
namespace AWHPerf {
	// CPU Metrics
	struct AWCPU {
		static inline const int numProcessors = std::max(std::thread::hardware_concurrency(), 1u);
		static inline std::mutex foobarMutex;
		static inline DWORD foobarCachedPid = 0;
		static inline ULONGLONG foobarLastSampleTime = 0;
		static inline ULONGLONG foobarLastTotalTime = 0;
		static inline double foobarCachedCpu = 0.0;

		static inline std::mutex systemMutex;
		static inline PDH_HQUERY systemQuery = nullptr;
		static inline PDH_HCOUNTER systemCounter = nullptr;
		static inline ULONGLONG systemLastSampleTime = 0;
		static inline double systemCachedCpu = 0.0;
	};

	// Memory Metrics
	struct AWRAM {
		static inline std::mutex foobarMutex;
		static inline PDH_HQUERY foobarQuery = nullptr;
		static inline PDH_HCOUNTER foobarWSCounter = nullptr;
		static inline PDH_HCOUNTER foobarPBCounter = nullptr;
		static inline ULONGLONG foobarLastSampleTime = 0;
		static inline std::pair<double, double> foobarCachedMemory{ 0.0, 0.0 };

		static inline std::mutex systemMutex;
		static inline ULONGLONG systemLastSampleTime = 0;
		static inline double systemCachedMemory = 0.0;
	};

	HANDLE GetFoobarProcessHandle(bool& pidChanged);
	double GetCpuFoobarUsage(int refreshRate = 1000);
	double GetCpuSystemUsage(int refreshRate = 1000);
	std::pair<double, double> GetMemoryFoobarUsage(int refreshRate = 1000);
	double GetMemorySystemUsage(int refreshRate = 1000);
}
#pragma endregion


////////////////////////
// * STRING HELPERS * //
////////////////////////
#pragma region String Helpers
namespace AWHString {
	bool EqualsIgnoreCase(const std::string& input, const char* compareValue);
	std::wstring FormatSampleRate(const double& sampleRate);
	std::string FormatDate(std::chrono::system_clock::time_point time, const char* format = "%d-%m-%Y");
	CStringA FormatTimestampMs(double ms);
	double GetElapsedTime(const std::chrono::steady_clock::time_point& startTime);
	std::wstring GetProcessingTime(const std::chrono::steady_clock::time_point& startTime, int precision = 2);
	std::wstring GetProcessingSpeed(double totalDuration, const std::chrono::steady_clock::time_point& startTime, int precision = 2);
	CStringW GetWindowTextCStringW(HWND hWnd);
	std::vector<CStringW> SplitString(const CStringW& input, const wchar_t* delimiter);
	pfc::string8 ToFixed(int precision, int value);
	pfc::string8 ToFixed(int precision, double value);
	CStringW ToFixedW(int precision, double value);
	pfc::string8 ToLowerCase(const std::string& input);
	pfc::string8 ToNarrow(const std::wstring& input);
	std::wstring ToWide(const pfc::string8& input);
};
#pragma endregion


//////////////////////
// * TEXT HELPERS * //
//////////////////////
#pragma region Text Helpers
namespace AWHText {
	HFONT CreateScaledFont(HFONT baseFont, HDC hdc, int ptDelta);
	CStringA FormatAlignedTextLine(const std::vector<CStringA>& values, const std::vector<int>& widths,
		const std::vector<bool>& isNumeric, const CStringA& separator = "    "
	);
	int GetFontHeight(HDC hdc, HFONT hFont = nullptr);
	void MeasureText(HDC hdc, const CStringW& text, CSize& size);
	int MeasureTextWidth(HDC hdc, const CStringW& text);
	CStringA WriteFancyHeader(const CStringA& titlePrefix, const CStringA& date = "");
};
#pragma endregion
