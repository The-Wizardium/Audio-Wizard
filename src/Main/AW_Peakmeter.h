/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Peakmeter Header File                      * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


///////////////////
// * PEAKMETER * //
///////////////////
#pragma region Peakmeter
class AudioWizardPeakmeter {
public:
	AudioWizardPeakmeter();
	~AudioWizardPeakmeter();

	// * PUBLIC MAIN METHODS * //
	void ApplyFadeIn();
	void InitCoefficients(double chunkDurationSec);
	void InitHistory(double refreshRateSec);
	void InitPeakmeter();
	void ResetPeakmeter();
	void UpdatePeakmeter();

	// * PUBLIC API METHODS * //
	void GetOffset(double* offsetDb) const;
	void SetOffset(double offsetDb);
	void GetAdjustedLeftRMS(double* leftRMS) const;
	void GetAdjustedRightRMS(double* rightRMS) const;
	void GetAdjustedLeftSamplePeak(double* leftSamplePeak) const;
	void GetAdjustedRightSamplePeak(double* rightSamplePeak) const;
	void UpdateRefreshRate(int refreshRateMs);

private:
	// * CONFIG CONSTANTS * //
	struct Config {
		static constexpr double MANUAL_OFFSET_MIN_DB = -20.0; // Minimum manual offset (dB). Adjusts VU meter reference level. Range: -20.0 to 0.0 dB.
		static constexpr double MANUAL_OFFSET_MAX_DB = 20.0;  // Maximum manual offset (dB). Adjusts VU meter reference level. Range: 0.0 to 20.0 dB.

		static constexpr double FADE_DURATION_SEC = 1.5;      // Fade-in duration at playback start (seconds). Smooths VU meter needle movement. Range: 0.2 to 2.0 seconds.
		static constexpr double TARGET_PEAK_DB = 6.0;        // Target peak level (dB). Sets VU meter level to ~66% display width, with peaks up to 100%. Range: 0.0 to 9.0 dB.
		static constexpr double NOISE_FLOOR_DB = -70.0;      // Noise floor threshold (dB). Ignores signals below this for VU meter display. Range: -90.0 to -40.0 dB.
		static constexpr double QUIET_THRESHOLD_DB = -14.0;   // Quiet signal threshold (dB). Triggers boost for low-level signals on VU meter. Range: -30.0 to -10.0 dB.
		static constexpr double SOFT_LIMIT_THRESHOLD_DB = 20.0; // Soft limit threshold (dB). Caps gain to keep VU meter within 100% display. Range: 0.0 to 6.0 dB.

		static constexpr double NOISE_DECAY_SEC = 0.016;     // Noise decay time (seconds). Controls how quickly noise fades on VU meter. Range: 0.01 to 0.1 seconds.
		static constexpr double GAIN_MAX_CEILING_DB = 25.0;  // Maximum gain ceiling (dB). Limits total gain for VU meter display. Range: 6.0 to 12.0 dB.
		static constexpr double GAIN_ATTACK_SEC = 0.01;      // Gain attack time (seconds). Smooths gain rise for VU meter response. Range: 0.1 to 0.5 seconds.
		static constexpr double GAIN_RELEASE_SEC = 1.0;      // Gain release time (seconds). Smooths gain drop for VU meter response. Range: 0.5 to 2.0 seconds.

		static constexpr double LEVEL_ATTACK_SMOOTHING_SEC = 0.3; // Level attack smoothing time (seconds). Smooths RMS rise for VU meter ballistics. Range: 0.2 to 0.5 seconds.
		static constexpr double LEVEL_RELEASE_SMOOTHING_SEC = 1.0; // Level release smoothing time (seconds). Smooths RMS drop for VU meter ballistics. Range: 0.5 to 1.5 seconds.
		static constexpr double PEAK_HOLD_SEC = 2.0;         // Peak hold time (seconds). Duration peaks are held on VU meter. Range: 1.0 to 5.0 seconds.
		static constexpr double PEAK_DECAY_MIN_DB = -5.0;    // Minimum peak decay rate (dB). Controls decay speed for smaller peaks on VU meter. Range: -10.0 to -1.0 dB.
		static constexpr double PEAK_DECAY_MAX_DB = -1.0;    // Maximum peak decay rate (dB). Controls decay speed for larger peaks on VU meter. Range: -10.0 to -1.0 dB.

		static constexpr double BREAKDOWN_THRESHOLD_DB = -20.0; // Breakdown threshold (dB below average peak). Triggers boost for quiet signals on VU meter. Range: -40.0 to -10.0 dB.
		static constexpr double BREAKDOWN_BOOST_RATE = 5.0;   // Breakdown boost rate (dB per chunk). Controls boost speed for quiet signals on VU meter. Range: 0.1 to 10.0 dB per chunk.
		static constexpr double BREAKDOWN_BOOST_DB = 50.0;    // Maximum breakdown boost (dB). Sets maximum boost for quiet signals on VU meter. Range: 6.0 to 12.0 dB.
		static constexpr double OFFSET_MAX_DECREASE_RATE_DB_PER_SEC = 2.0; // Maximum gain decrease rate (dB/second). Limits gain drop speed for VU meter stability. Range: 1.0 to 20.0 dB/second.
		static constexpr double BOOST_EXTENSION_SEC = 1.0;    // Boost extension time after peak (seconds). Extends boost duration for VU meter visibility. Range: 0.1 to 1.0 seconds.
	};

	// * FADE-IN CONTROL * //
	struct FadeIn {
		std::chrono::steady_clock::time_point fadeStartTime;
		bool fadeActive = false;
	}; FadeIn fade;

	// * PEAK HISTORY * //
	struct History {
		static constexpr double PEAK_HISTORY_WINDOW_SEC = 0.1;
		std::vector<double> peakHistory;
		double peakHistorySum = 0.0;
		double invHistorySize = 1.0;
		size_t historyIndex = 0;
		size_t historySize = 0;
	}; History history;

	// * SMOOTHING COEFFICIENTS * //
	struct Smoothing {
		double gainAttackCoeff;
		double gainReleaseCoeff;
		double levelSmoothingAttack;
		double levelSmoothingRelease;
		double baseDecay;
		double baseDecayDb;
		double noiseDecay;
		double noiseDecayDb;
	}; Smoothing smooth;

	// * PEAKMETER STATE * //
	struct State {
		double chunkDurationSec = 0.0;
		double refreshRateSec = 0.0;
		std::atomic<double> gainOffsetDb = 0.0;
		std::atomic<double> manualOffsetDb = 0.0;
		std::atomic<double> leftRMSDb = -INFINITY;
		std::atomic<double> rightRMSDb = -INFINITY;
		std::atomic<double> leftSamplePeakDb = -INFINITY;
		std::atomic<double> rightSamplePeakDb = -INFINITY;
		double heldLeftSamplePeakDb = -INFINITY;
		double heldRightSamplePeakDb = -INFINITY;
		double breakdownBoost = 0.0;
		double boostTimer = 0.0;
	}; State state;

	// * PRIVATE METHODS * //
	std::pair<double, double> GetAdjustedRMS(double leftRMSDb, double rightRMSDb);
	double GetDynamicGainOffset(double currentPeakDb, double chunkDuration);
	double UpdatePeakHistorySum(double currentSum, double oldPeak, double newPeak) const;
};
#pragma endregion
