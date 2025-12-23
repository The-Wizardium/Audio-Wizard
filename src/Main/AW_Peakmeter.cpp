/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Peakmeter Source File                      * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.2.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_Helpers.h"
#include "AW_Peakmeter.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardPeakmeter::AudioWizardPeakmeter() {
	InitPeakmeter();
}

AudioWizardPeakmeter::~AudioWizardPeakmeter() = default;
#pragma endregion


/////////////////////////////
// * PUBLIC MAIN METHODS * //
/////////////////////////////
#pragma region Public Main Methods
void AudioWizardPeakmeter::ApplyFadeIn() {
	fade.fadeActive = true;
	fade.fadeStartTime = std::chrono::steady_clock::now();
}

void AudioWizardPeakmeter::InitCoefficients(double chunkDurationSec) {
	smooth.gainAttackCoeff = AWHMath::CalculateCoefficient(chunkDurationSec, Config::GAIN_ATTACK_SEC);
	smooth.gainReleaseCoeff = AWHMath::CalculateCoefficient(chunkDurationSec, Config::GAIN_RELEASE_SEC);
	smooth.levelSmoothingAttack = AWHMath::CalculateSmoothingFactor(chunkDurationSec, Config::LEVEL_ATTACK_SMOOTHING_SEC);
	smooth.levelSmoothingRelease = AWHMath::CalculateSmoothingFactor(chunkDurationSec, Config::LEVEL_RELEASE_SMOOTHING_SEC);
	smooth.baseDecay = AWHMath::CalculateCoefficient(chunkDurationSec, Config::PEAK_HOLD_SEC);
	smooth.noiseDecay = AWHMath::CalculateCoefficient(chunkDurationSec, Config::NOISE_DECAY_SEC);
	smooth.baseDecayDb = AWHAudio::LinearToDb(smooth.baseDecay);
	smooth.noiseDecayDb = AWHAudio::LinearToDb(smooth.noiseDecay);
}

void AudioWizardPeakmeter::InitHistory(double refreshRateSec) {
	refreshRateSec = std::max(refreshRateSec, 0.001);

	constexpr size_t MAX_HISTORY_SIZE = 10000;
	history.historySize = std::max<size_t>(1, std::min<size_t>(MAX_HISTORY_SIZE, static_cast<size_t>(History::PEAK_HISTORY_WINDOW_SEC / refreshRateSec + 0.5)));
	history.peakHistory.assign(history.historySize, Config::NOISE_FLOOR_DB);
	history.historyIndex = 0;
	history.peakHistorySum = Config::NOISE_FLOOR_DB;
	history.invHistorySize = 1.0 / history.historySize;
}

void AudioWizardPeakmeter::InitPeakmeter() {
	state.chunkDurationSec = AWHConvert::MsToSec(AudioWizard::Main()->mainRealTime->monitor.monitorChunkDurationMs);
	InitCoefficients(state.chunkDurationSec);
	state.refreshRateSec = AWHConvert::MsToSec(AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs);
	InitHistory(state.refreshRateSec);
}

void AudioWizardPeakmeter::ResetPeakmeter() {
	state.boostTimer = 0.0;
	state.breakdownBoost = 0.0;
	state.gainOffsetDb.store(0.0);
	state.manualOffsetDb.store(0.0);
	state.heldLeftSamplePeakDb = Config::NOISE_FLOOR_DB;
	state.heldRightSamplePeakDb = Config::NOISE_FLOOR_DB;
	state.leftSamplePeakDb.store(Config::NOISE_FLOOR_DB);
	state.rightSamplePeakDb.store(Config::NOISE_FLOOR_DB);
	state.leftRMSDb.store(Config::NOISE_FLOOR_DB);
	state.rightRMSDb.store(Config::NOISE_FLOOR_DB);
	history.peakHistory.assign(history.historySize, Config::NOISE_FLOOR_DB);
	history.historyIndex = 0;
	history.peakHistorySum = Config::NOISE_FLOOR_DB;
}

void AudioWizardPeakmeter::UpdatePeakmeter() {
	double leftRMSDb = AudioWizard::Main()->mainRealTime->metrics.leftRMS.load();
	double rightRMSDb = AudioWizard::Main()->mainRealTime->metrics.rightRMS.load();
	double leftSamplePeakDb = AudioWizard::Main()->mainRealTime->metrics.leftSamplePeak.load();
	double rightSamplePeakDb = AudioWizard::Main()->mainRealTime->metrics.rightSamplePeak.load();

	// Apply decay to held peaks
	AWHAudioDSP::ApplyDecayToHeldValueDb(state.heldLeftSamplePeakDb, leftSamplePeakDb, smooth.noiseDecayDb, smooth.baseDecayDb, Config::NOISE_FLOOR_DB);
	AWHAudioDSP::ApplyDecayToHeldValueDb(state.heldRightSamplePeakDb, rightSamplePeakDb, smooth.noiseDecayDb, smooth.baseDecayDb, Config::NOISE_FLOOR_DB);
	double currentPeakDb = std::max(state.heldLeftSamplePeakDb, state.heldRightSamplePeakDb);

	// Update peak history
	double oldPeak = history.peakHistory[history.historyIndex];
	history.peakHistory[history.historyIndex] = currentPeakDb;
	history.peakHistorySum = UpdatePeakHistorySum(history.peakHistorySum, oldPeak, currentPeakDb);
	history.historyIndex = (history.historyIndex + 1) % history.historySize;

	// Apply dynamic offset
	const double smoothedOffsetDb = GetDynamicGainOffset(currentPeakDb, state.chunkDurationSec);
	state.gainOffsetDb.store(smoothedOffsetDb);

	// Apply offset gain and update levels/peaks
	auto [leftRMSAdjusted, rightRMSAdjusted] = GetAdjustedRMS(leftRMSDb, rightRMSDb);
	double prevLeftRMS = state.leftRMSDb.load();
	double prevRightRMS = state.rightRMSDb.load();
	double prevLeftSamplePeak = state.leftSamplePeakDb.load();
	double prevRightSamplePeak = state.rightSamplePeakDb.load();

	state.leftRMSDb.store(prevLeftRMS == Config::NOISE_FLOOR_DB ? leftRMSAdjusted :
		AWHAudioDSP::SmoothValue(prevLeftRMS, leftRMSAdjusted, smooth.levelSmoothingAttack, smooth.levelSmoothingRelease)
	);
	state.rightRMSDb.store(prevRightRMS == Config::NOISE_FLOOR_DB ? rightRMSAdjusted :
		AWHAudioDSP::SmoothValue(prevRightRMS, rightRMSAdjusted, smooth.levelSmoothingAttack, smooth.levelSmoothingRelease)
	);

	// Adaptive peak decay
	double peakMagnitude = std::max(leftRMSAdjusted, rightRMSAdjusted);
	double decayDb = Config::PEAK_DECAY_MIN_DB + (Config::PEAK_DECAY_MAX_DB - Config::PEAK_DECAY_MIN_DB) *
		((peakMagnitude - Config::NOISE_FLOOR_DB) / (Config::GAIN_MAX_CEILING_DB - Config::NOISE_FLOOR_DB));

	state.leftSamplePeakDb.store(prevLeftSamplePeak == Config::NOISE_FLOOR_DB ? leftRMSAdjusted :
		std::max(leftRMSAdjusted, prevLeftSamplePeak + decayDb)
	);
	state.rightSamplePeakDb.store(prevRightSamplePeak == Config::NOISE_FLOOR_DB ? rightRMSAdjusted :
		std::max(rightRMSAdjusted, prevRightSamplePeak + decayDb)
	);
}
#pragma endregion


////////////////////////////
// * PUBLIC API METHODS * //
////////////////////////////
#pragma region Public API Methods
void AudioWizardPeakmeter::GetOffset(double* offsetDb) const {
	*offsetDb = state.manualOffsetDb.load();
}

void AudioWizardPeakmeter::SetOffset(double offsetDb) {
	offsetDb = std::clamp(offsetDb, Config::MANUAL_OFFSET_MIN_DB, Config::MANUAL_OFFSET_MAX_DB);
	state.manualOffsetDb.store(offsetDb);
}

void AudioWizardPeakmeter::GetAdjustedLeftRMS(double* leftRMS) const {
	*leftRMS = state.leftRMSDb.load();
}

void AudioWizardPeakmeter::GetAdjustedRightRMS(double* rightRMS) const {
	*rightRMS = state.rightRMSDb.load();
}

void AudioWizardPeakmeter::GetAdjustedLeftSamplePeak(double* leftSamplePeak) const {
	*leftSamplePeak = state.leftSamplePeakDb.load();
}

void AudioWizardPeakmeter::GetAdjustedRightSamplePeak(double* rightSamplePeak) const {
	*rightSamplePeak = state.rightSamplePeakDb.load();
}

void AudioWizardPeakmeter::UpdateRefreshRate(int refreshRateMs) {
	state.refreshRateSec = AWHConvert::MsToSec(refreshRateMs);
	size_t newHistorySize = std::max<size_t>(1, static_cast<size_t>(History::PEAK_HISTORY_WINDOW_SEC / state.refreshRateSec + 0.5));

	InitCoefficients(state.refreshRateSec);

	if (newHistorySize != history.historySize) {
		InitHistory(state.refreshRateSec);
	}
}
#pragma endregion


/////////////////////////
// * PRIVATE METHODS * //
/////////////////////////
#pragma region Private Methods
std::pair<double, double> AudioWizardPeakmeter::GetAdjustedRMS(double leftRMSDb, double rightRMSDb) {
	const double offsetGainDb = state.gainOffsetDb.load() + state.manualOffsetDb.load();

	if (leftRMSDb > Config::NOISE_FLOOR_DB) {
		leftRMSDb += offsetGainDb;
	}
	if (rightRMSDb > Config::NOISE_FLOOR_DB) {
		rightRMSDb += offsetGainDb;
	}

	if (fade.fadeActive) {
		const auto now = std::chrono::steady_clock::now();
		const double elapsed = std::chrono::duration<double>(now - fade.fadeStartTime).count();
		if (elapsed >= Config::FADE_DURATION_SEC) {
			fade.fadeActive = false;
		}
		else {
			const double fadeDb = Config::NOISE_FLOOR_DB * (1.0 - AWHMath::CalculateCosineFadeProgress(elapsed, Config::FADE_DURATION_SEC));
			leftRMSDb = leftRMSDb > Config::NOISE_FLOOR_DB ? leftRMSDb + fadeDb : fadeDb;
			rightRMSDb = rightRMSDb > Config::NOISE_FLOOR_DB ? rightRMSDb + fadeDb : fadeDb;
		}
	}

	return { leftRMSDb, rightRMSDb };
}

double AudioWizardPeakmeter::GetDynamicGainOffset(double currentPeakDb, double chunkDuration) {
	double avgPeakDb = AWHAudioDSP::CalculateAveragePeakDb(history.peakHistory, history.historySize);
	bool isLoudTransient = currentPeakDb > Config::SOFT_LIMIT_THRESHOLD_DB;

	if (currentPeakDb < avgPeakDb + Config::BREAKDOWN_THRESHOLD_DB && currentPeakDb < Config::QUIET_THRESHOLD_DB) {
		state.breakdownBoost = std::min(state.breakdownBoost + Config::BREAKDOWN_BOOST_RATE, Config::BREAKDOWN_BOOST_DB);
		state.boostTimer = 0.0;
	}
	else {
		state.boostTimer += chunkDuration;
		if (state.boostTimer > Config::BOOST_EXTENSION_SEC) {
			double decayRate = isLoudTransient ? 0.5 * Config::BREAKDOWN_BOOST_RATE : Config::BREAKDOWN_BOOST_RATE * 2;
			state.breakdownBoost = std::max(state.breakdownBoost * std::exp(-decayRate * chunkDuration), 0.0);
		}
	}

	const double desiredGainDb = AWHAudioDSP::CalculateDesiredGainDb(currentPeakDb, Config::TARGET_PEAK_DB, Config::GAIN_MAX_CEILING_DB, Config::NOISE_FLOOR_DB);
	double boostedGainDb = desiredGainDb + state.breakdownBoost;

	if (boostedGainDb > Config::SOFT_LIMIT_THRESHOLD_DB) {
		boostedGainDb = Config::SOFT_LIMIT_THRESHOLD_DB + (Config::GAIN_MAX_CEILING_DB - Config::SOFT_LIMIT_THRESHOLD_DB) *
			(1.0 - std::exp(-(boostedGainDb - Config::SOFT_LIMIT_THRESHOLD_DB)));
	}

	double currentOffsetDb = state.gainOffsetDb.load();
	double smoothedOffsetDb = AWHAudioDSP::SmoothValue(
		currentOffsetDb, boostedGainDb, (1.0 - smooth.gainAttackCoeff), (1.0 - smooth.gainReleaseCoeff)
	);

	const double maxDecreaseDbPerChunk = Config::OFFSET_MAX_DECREASE_RATE_DB_PER_SEC * chunkDuration;
	const double minAllowedDb = currentOffsetDb - maxDecreaseDbPerChunk;
	return std::max(smoothedOffsetDb, minAllowedDb);
}

double AudioWizardPeakmeter::UpdatePeakHistorySum(double currentSum, double oldPeak, double newPeak) const {
	double adjustedNewPeak = std::max(newPeak, Config::NOISE_FLOOR_DB);
	double adjustedOldPeak = std::max(oldPeak, Config::NOISE_FLOOR_DB);
	return std::max(currentSum + adjustedNewPeak - adjustedOldPeak, Config::NOISE_FLOOR_DB);
}
#pragma endregion
