/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Settings Source File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "resource.h"
#include "AW_Helpers.h"
#include "AW_Settings.h"


////////////////////////
// * STATIC METHODS * //
////////////////////////
#pragma region Static Methods
void AudioWizardSettings::InitAnalysisSettings(HWND hWnd) {
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_COLORS, analysisDisplayColors);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_TOOLTIPS, analysisDisplayTooltips);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_INDEX, analysisDisplayIndex);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_ARTIST, analysisDisplayArtist);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_ALBUM, analysisDisplayAlbum);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_TRACK, analysisDisplayTrack);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_DURATION, analysisDisplayDuration);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_YEAR, analysisDisplayYear);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_GENRE, analysisDisplayGenre);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_FORMAT, analysisDisplayFormat);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_CHANNELS, analysisDisplayChannels);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_BIT_DEPTH, analysisDisplayBitDepth);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_BITRATE, analysisDisplayBitrate);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_SAMPLE_RATE, analysisDisplaySampleRate);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_MOMENTARY_LUFS, analysisDisplayMomentaryLUFS);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_SHORT_TERM_LUFS, analysisDisplayShortTermLUFS);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_INTEGRATED_LUFS, analysisDisplayIntegratedLUFS);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_RMS, analysisDisplayRMS);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_SAMPLE_PEAK, analysisDisplaySamplePeak);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_TRUE_PEAK, analysisDisplayTruePeak);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_PSR, analysisDisplayPSR);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_PLR, analysisDisplayPLR);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_CREST_FACTOR, analysisDisplayCrestFactor);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_LOUDNESS_RANGE, analysisDisplayLoudnessRange);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_DYNAMIC_RANGE, analysisDisplayDynamicRange);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_DYNAMIC_RANGE_ALBUM, analysisDisplayDynamicRangeAlbum);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_PURE_DYNAMICS, analysisDisplayPureDynamics);
	AWHDialog::SetCheckBox(hWnd, IDC_ANALYSIS_CFG_PURE_DYNAMICS_ALBUM, analysisDisplayPureDynamicsAlbum);

	AWHDialog::SetDropDownMenu(hWnd, IDC_ANALYSIS_CFG_METADATA_PRESET, {
		L"All", L"Album", L"User"
	}, analysisDisplayMetadataPreset);

	AWHDialog::SetDropDownMenu(hWnd, IDC_ANALYSIS_CFG_METRICS_PRESET, {
		L"All", L"Loudness", L"Peaks", L"Dynamics", L"User"
	}, analysisDisplayMetricsPreset);
}

void AudioWizardSettings::InitAnalysisTagsSettings(HWND hWnd) {
	// Use the stored cfg_string value if it exists and is non-empty; otherwise, fall back to the default value.
	// Explicitly convert default values (const char*) to pfc::string8 to avoid ternary operator ambiguity
	// between pfc::string8 and const char*, ensuring compatibility with AWHString::ToWide.
	pfc::string8 lufs = analysisTagsLUFS.get().length() > 0 ? analysisTagsLUFS.get() : pfc::string8(analysisTagsLUFSDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_LUFS_VALUE, AWHString::ToWide(lufs));

	pfc::string8 rms = analysisTagsRMS.get().length() > 0 ? analysisTagsRMS.get() : pfc::string8(analysisTagsRMSDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_RMS_VALUE, AWHString::ToWide(rms));

	pfc::string8 sp = analysisTagsSP.get().length() > 0 ? analysisTagsSP.get() : pfc::string8(analysisTagsSPDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_SP_VALUE, AWHString::ToWide(sp));

	pfc::string8 tp = analysisTagsTP.get().length() > 0 ? analysisTagsTP.get() : pfc::string8(analysisTagsTPDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_TP_VALUE, AWHString::ToWide(tp));

	pfc::string8 psr = analysisTagsPSR.get().length() > 0 ? analysisTagsPSR.get() : pfc::string8(analysisTagsPSRDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PSR_VALUE, AWHString::ToWide(psr));

	pfc::string8 plr = analysisTagsPLR.get().length() > 0 ? analysisTagsPLR.get() : pfc::string8(analysisTagsPLRDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PLR_VALUE, AWHString::ToWide(plr));

	pfc::string8 cf = analysisTagsCF.get().length() > 0 ? analysisTagsCF.get() : pfc::string8(analysisTagsCFDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_CF_VALUE, AWHString::ToWide(cf));

	pfc::string8 lra = analysisTagsLRA.get().length() > 0 ? analysisTagsLRA.get() : pfc::string8(analysisTagsLRADefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_LRA_VALUE, AWHString::ToWide(lra));

	pfc::string8 dr = analysisTagsDR.get().length() > 0 ? analysisTagsDR.get() : pfc::string8(analysisTagsDRDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_DR_VALUE, AWHString::ToWide(dr));

	pfc::string8 drAlbum = analysisTagsDRAlbum.get().length() > 0 ? analysisTagsDRAlbum.get() : pfc::string8(analysisTagsDRAlbumDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_DR_ALBUM_VALUE, AWHString::ToWide(drAlbum));

	pfc::string8 pd = analysisTagsPD.get().length() > 0 ? analysisTagsPD.get() : pfc::string8(analysisTagsPDDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PD_VALUE, AWHString::ToWide(pd));

	pfc::string8 pdAlbum = analysisTagsPDAlbum.get().length() > 0 ? analysisTagsPDAlbum.get() : pfc::string8(analysisTagsPDAlbumDefault);
	AWHDialog::SetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PD_ALBUM_VALUE, AWHString::ToWide(pdAlbum));
}

void AudioWizardSettings::InitMonitorSettings(HWND hWnd) {
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_TB_METRICS, monitorDisplayTitleBarMetrics);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_TB_METRICS_FRAC, monitorDisplayTitleBarMetricsFraction);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_COLORS, monitorDisplayColors);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_TOOLTIPS, monitorDisplayTooltips);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_MOMENTARY_LUFS, monitorDisplayMomentaryLUFS);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_SHORT_TERM_LUFS, monitorDisplayShortTermLUFS);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_RMS, monitorDisplayRMS);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_LEFT_RMS, monitorDisplayLeftRMS);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_RIGHT_RMS, monitorDisplayRightRMS);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_LEFT_SAMPLE_PEAK, monitorDisplayLeftSamplePeak);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_RIGHT_SAMPLE_PEAK, monitorDisplayRightSamplePeak);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_TRUE_PEAK, monitorDisplayTruePeak);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_PSR, monitorDisplayPSR);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_PLR, monitorDisplayPLR);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_CREST_FACTOR, monitorDisplayCrestFactor);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_DYNAMIC_RANGE, monitorDisplayDynamicRange);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_PURE_DYNAMICS, monitorDisplayPureDynamics);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_PHASE_CORRELATION, monitorDisplayPhaseCorrelation);
	AWHDialog::SetCheckBox(hWnd, IDC_MONITOR_CFG_STEREO_WIDTH, monitorDisplayStereoWidth);

	AWHDialog::SetDropDownMenu(hWnd, IDC_MONITOR_CFG_METRICS_MODE, {
		L"Value", L"Meter", L"Value and Meter"
	}, monitorDisplayMetricsMode);

	AWHDialog::SetDropDownMenu(hWnd, IDC_MONITOR_CFG_REFRESH_RATE, {
		L"  1 fps ~ 1000 ms",
		L"  2 fps ~ 500 ms",
		L"  3 fps ~ 333 ms",
		L"  4 fps ~ 250 ms",
		L"  5 fps ~ 200 ms",
		L"  6 fps ~ 166 ms",
		L"  7 fps ~ 142 ms",
		L"  8 fps ~ 125 ms",
		L"  9 fps ~ 111 ms",
		L"10 fps ~ 100 ms",
		L"12 fps ~  83 ms",
		L"15 fps ~  67 ms",
		L"20 fps ~  50 ms",
		L"25 fps ~  40 ms",
		L"30 fps ~  33 ms",
		L"45 fps ~  22 ms",
		L"60 fps ~  17 ms"
	}, monitorDisplayRefreshRate);
}

bool AudioWizardSettings::InitMetadataPreset(int presetIndex) {
	bool index = AudioWizardSettings::analysisDisplayIndex;
	bool artist = AudioWizardSettings::analysisDisplayArtist;
	bool album = AudioWizardSettings::analysisDisplayAlbum;
	bool track = AudioWizardSettings::analysisDisplayTrack;
	bool duration = AudioWizardSettings::analysisDisplayDuration;
	bool year = AudioWizardSettings::analysisDisplayYear;
	bool genre = AudioWizardSettings::analysisDisplayGenre;
	bool format = AudioWizardSettings::analysisDisplayFormat;
	bool channels = AudioWizardSettings::analysisDisplayChannels;
	bool bitDepth = AudioWizardSettings::analysisDisplayBitDepth;
	bool bitrate = AudioWizardSettings::analysisDisplayBitrate;
	bool sampleRate = AudioWizardSettings::analysisDisplaySampleRate;

	switch (presetIndex) {
		case 0: { // "All"
			return index && artist && album && track && duration && year && genre && format && channels && bitDepth && bitrate && sampleRate;
		}
		case 1: { // "Album"
			return index && !artist && !album && track && !duration && !year && !genre && !format && !channels && !bitDepth && !bitrate && !sampleRate;
		}
		default: { // No match for "User" or invalid index
			return false;
		}
	}
}

bool AudioWizardSettings::InitMetricsPreset(int presetIndex) {
	bool MLUFS = AudioWizardSettings::analysisDisplayMomentaryLUFS;
	bool SLUFS = AudioWizardSettings::analysisDisplayShortTermLUFS;
	bool ILUFS = AudioWizardSettings::analysisDisplayIntegratedLUFS;
	bool LRA = AudioWizardSettings::analysisDisplayLoudnessRange;
	bool RMS = AudioWizardSettings::analysisDisplayRMS;
	bool SP = AudioWizardSettings::analysisDisplaySamplePeak;
	bool TP = AudioWizardSettings::analysisDisplayTruePeak;
	bool PSR = AudioWizardSettings::analysisDisplayPSR;
	bool PLR = AudioWizardSettings::analysisDisplayPLR;
	bool CF = AudioWizardSettings::analysisDisplayCrestFactor;
	bool DR = AudioWizardSettings::analysisDisplayDynamicRange;
	bool DRA = AudioWizardSettings::analysisDisplayDynamicRangeAlbum;
	bool PD = AudioWizardSettings::analysisDisplayPureDynamics;
	bool PDA = AudioWizardSettings::analysisDisplayPureDynamicsAlbum;

	switch (presetIndex) {
		case 0: { // "All"
			return MLUFS && SLUFS && ILUFS && RMS && SP && TP && PSR && PLR && CF && LRA && DR && DRA && PD && PDA;
		}
		case 1: { // "Loudness"
			return MLUFS && SLUFS && ILUFS && RMS && !SP && !TP && !PSR && !PLR && !CF && !LRA && !DR && !DRA && !PD && !PDA;
		}
		case 2: { // "Peaks"
			return !MLUFS && !SLUFS && !ILUFS && !RMS && SP && TP && PSR && PLR && !CF && !LRA && !DR && !DRA && !PD && !PDA;
		}
		case 3: { // "Dynamics"
			return !MLUFS && !SLUFS && !ILUFS && !RMS && !SP && !TP && !PSR && !PLR && CF && LRA && DR && DRA && PD && PDA;
		}
		default: { // No match for "User" or invalid index
			return false;
		}
	}
}

void AudioWizardSettings::GetAnalysisPreset(HWND hWnd) {
	// Metadata preset options
	const std::vector<std::wstring> metadataOptions = { L"All", L"Album", L"User" };
	int metadataIndex = 2; // Default to "User"

	if (InitMetadataPreset(0)) { // Matches "All"
		metadataIndex = 0;
	}
	else if (InitMetadataPreset(1)) { // Matches "Album"
		metadataIndex = 1;
	}

	analysisDisplayMetadataPreset = metadataIndex;
	AWHDialog::SetDropDownMenu(hWnd, IDC_ANALYSIS_CFG_METADATA_PRESET, metadataOptions, metadataIndex);

	// Metrics preset options
	const std::vector<std::wstring> metricsOptions = { L"All", L"Loudness", L"Peaks", L"Dynamics", L"User" };
	int metricsIndex = 4; // Default to "User"

	if (InitMetricsPreset(0)) { // Matches "All"
		metricsIndex = 0;
	}
	else if (InitMetricsPreset(1)) { // Matches "Loudness"
		metricsIndex = 1;
	}
	else if (InitMetricsPreset(2)) { // Matches "Peaks"
		metricsIndex = 2;
	}
	else if (InitMetricsPreset(3)) { // Matches "Dynamics"
		metricsIndex = 3;
	}

	analysisDisplayMetricsPreset = metricsIndex;
	AWHDialog::SetDropDownMenu(hWnd, IDC_ANALYSIS_CFG_METRICS_PRESET, metricsOptions, metricsIndex);
}

void AudioWizardSettings::SetAnalysisMetadataPreset(int presetIndex) {
	switch (presetIndex) {
		case 0: { // "All"
			analysisDisplayIndex = true;
			analysisDisplayArtist = true;
			analysisDisplayAlbum = true;
			analysisDisplayTrack = true;
			analysisDisplayDuration = true;
			analysisDisplayYear = true;
			analysisDisplayGenre = true;
			analysisDisplayFormat = true;
			analysisDisplayChannels = true;
			analysisDisplayBitDepth = true;
			analysisDisplayBitrate = true;
			analysisDisplaySampleRate = true;
			break;
		}
		case 1: { // "Album"
			analysisDisplayIndex = true;
			analysisDisplayArtist = false;
			analysisDisplayAlbum = false;
			analysisDisplayTrack = true;
			analysisDisplayDuration = false;
			analysisDisplayYear = false;
			analysisDisplayGenre = false;
			analysisDisplayFormat = false;
			analysisDisplayChannels = false;
			analysisDisplayBitDepth = false;
			analysisDisplayBitrate = false;
			analysisDisplaySampleRate = false;
			break;
		}
		case 2: { // "User"
			break; // No action needed; user settings are already custom
		}
	}
}

void AudioWizardSettings::SetAnalysisMetricsPreset(int presetIndex) {
	switch (presetIndex) {
		case 0: { // "All"
			analysisDisplayMomentaryLUFS = true;
			analysisDisplayShortTermLUFS = true;
			analysisDisplayIntegratedLUFS = true;
			analysisDisplayRMS = true;
			analysisDisplaySamplePeak = true;
			analysisDisplayTruePeak = true;
			analysisDisplayPSR = true;
			analysisDisplayPLR = true;
			analysisDisplayCrestFactor = true;
			analysisDisplayLoudnessRange = true;
			analysisDisplayDynamicRange = true;
			analysisDisplayDynamicRangeAlbum = true;
			analysisDisplayPureDynamics = true;
			analysisDisplayPureDynamicsAlbum = true;
			break;
		}
		case 1: { // "Loudness"
			analysisDisplayMomentaryLUFS = true;
			analysisDisplayShortTermLUFS = true;
			analysisDisplayIntegratedLUFS = true;
			analysisDisplayRMS = true;
			analysisDisplaySamplePeak = false;
			analysisDisplayTruePeak = false;
			analysisDisplayPSR = false;
			analysisDisplayPLR = false;
			analysisDisplayCrestFactor = false;
			analysisDisplayLoudnessRange = false;
			analysisDisplayDynamicRange = false;
			analysisDisplayDynamicRangeAlbum = false;
			analysisDisplayPureDynamics = false;
			analysisDisplayPureDynamicsAlbum = false;
			break;
		}
		case 2: { // "Peaks"
			analysisDisplayMomentaryLUFS = false;
			analysisDisplayShortTermLUFS = false;
			analysisDisplayIntegratedLUFS = false;
			analysisDisplayRMS = false;
			analysisDisplaySamplePeak = true;
			analysisDisplayTruePeak = true;
			analysisDisplayPSR = true;
			analysisDisplayPLR = true;
			analysisDisplayCrestFactor = false;
			analysisDisplayLoudnessRange = false;
			analysisDisplayDynamicRange = false;
			analysisDisplayDynamicRangeAlbum = false;
			analysisDisplayPureDynamics = false;
			analysisDisplayPureDynamicsAlbum = false;
			break;
		}
		case 3: { // "Dynamics"
			analysisDisplayMomentaryLUFS = false;
			analysisDisplayShortTermLUFS = false;
			analysisDisplayIntegratedLUFS = false;
			analysisDisplayRMS = false;
			analysisDisplaySamplePeak = false;
			analysisDisplayTruePeak = false;
			analysisDisplayPSR = false;
			analysisDisplayPLR = false;
			analysisDisplayCrestFactor = true;
			analysisDisplayLoudnessRange = true;
			analysisDisplayDynamicRange = true;
			analysisDisplayDynamicRangeAlbum = true;
			analysisDisplayPureDynamics = true;
			analysisDisplayPureDynamicsAlbum = true;
			break;
		}
		case 4: { // "User"
			break; // No action needed; user settings are already custom
		}
	}
}

void AudioWizardSettings::SetAnalysisSettings(HWND hWnd) {
	analysisDisplayColors = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_COLORS);
	analysisDisplayTooltips = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_TOOLTIPS);
	analysisDisplayIndex = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_INDEX);
	analysisDisplayArtist = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_ARTIST);
	analysisDisplayAlbum = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_ALBUM);
	analysisDisplayTrack = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_TRACK);
	analysisDisplayDuration = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_DURATION);
	analysisDisplayYear = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_YEAR);
	analysisDisplayGenre = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_GENRE);
	analysisDisplayFormat = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_FORMAT);
	analysisDisplayChannels = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_CHANNELS);
	analysisDisplayBitDepth = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_BIT_DEPTH);
	analysisDisplayBitrate = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_BITRATE);
	analysisDisplaySampleRate = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_SAMPLE_RATE);
	analysisDisplayMomentaryLUFS = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_MOMENTARY_LUFS);
	analysisDisplayShortTermLUFS = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_SHORT_TERM_LUFS);
	analysisDisplayIntegratedLUFS = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_INTEGRATED_LUFS);
	analysisDisplayRMS = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_RMS);
	analysisDisplaySamplePeak = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_SAMPLE_PEAK);
	analysisDisplayTruePeak = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_TRUE_PEAK);
	analysisDisplayPSR = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_PSR);
	analysisDisplayPLR = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_PLR);
	analysisDisplayCrestFactor = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_CREST_FACTOR);
	analysisDisplayLoudnessRange = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_LOUDNESS_RANGE);
	analysisDisplayDynamicRange = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_DYNAMIC_RANGE);
	analysisDisplayDynamicRangeAlbum = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_DYNAMIC_RANGE_ALBUM);
	analysisDisplayPureDynamics = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_PURE_DYNAMICS);
	analysisDisplayPureDynamicsAlbum = AWHDialog::GetCheckBoxState(hWnd, IDC_ANALYSIS_CFG_PURE_DYNAMICS_ALBUM);
	analysisDisplayMetadataPreset = AWHDialog::GetDropDownIndex(hWnd, IDC_ANALYSIS_CFG_METADATA_PRESET);
	analysisDisplayMetricsPreset = AWHDialog::GetDropDownIndex(hWnd, IDC_ANALYSIS_CFG_METRICS_PRESET);

	GetAnalysisPreset(hWnd);
}

void AudioWizardSettings::SetAnalysisTagsSettings(HWND hWnd) {
	analysisTagsLUFS = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_LUFS_VALUE);
	analysisTagsRMS = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_RMS_VALUE);
	analysisTagsSP = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_SP_VALUE);
	analysisTagsTP = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_TP_VALUE);
	analysisTagsPSR = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PSR_VALUE);
	analysisTagsPLR = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PLR_VALUE);
	analysisTagsCF = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_CF_VALUE);
	analysisTagsLRA = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_LRA_VALUE);
	analysisTagsDR = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_DR_VALUE);
	analysisTagsDRAlbum = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_DR_ALBUM_VALUE);
	analysisTagsPD = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PD_VALUE);
	analysisTagsPDAlbum = AWHDialog::GetInputFieldText(hWnd, IDC_ANALYSIS_TAGS_CFG_PD_ALBUM_VALUE);
}

void AudioWizardSettings::SetMonitorSettings(HWND hWnd) {
	monitorDisplayTitleBarMetrics = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_TB_METRICS);
	monitorDisplayTitleBarMetricsFraction = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_TB_METRICS_FRAC);
	monitorDisplayColors = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_COLORS);
	monitorDisplayTooltips = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_TOOLTIPS);
	monitorDisplayMomentaryLUFS = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_MOMENTARY_LUFS);
	monitorDisplayShortTermLUFS = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_SHORT_TERM_LUFS);
	monitorDisplayRMS = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_RMS);
	monitorDisplayLeftRMS = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_LEFT_RMS);
	monitorDisplayRightRMS = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_RIGHT_RMS);
	monitorDisplayLeftSamplePeak = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_LEFT_SAMPLE_PEAK);
	monitorDisplayRightSamplePeak = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_RIGHT_SAMPLE_PEAK);
	monitorDisplayTruePeak = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_TRUE_PEAK);
	monitorDisplayPSR = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_PSR);
	monitorDisplayPLR = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_PLR);
	monitorDisplayCrestFactor = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_CREST_FACTOR);
	monitorDisplayDynamicRange = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_DYNAMIC_RANGE);
	monitorDisplayPureDynamics = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_PURE_DYNAMICS);
	monitorDisplayPhaseCorrelation = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_PHASE_CORRELATION);
	monitorDisplayStereoWidth = AWHDialog::GetCheckBoxState(hWnd, IDC_MONITOR_CFG_STEREO_WIDTH);
	monitorDisplayMetricsMode = AWHDialog::GetDropDownIndex(hWnd, IDC_MONITOR_CFG_METRICS_MODE);
	monitorDisplayRefreshRate = AWHDialog::GetDropDownIndex(hWnd, IDC_MONITOR_CFG_REFRESH_RATE);
}

void AudioWizardSettings::ResetAnalysisSettings() {
	analysisDisplayColors = analysisDisplayColorsDefault;
	analysisDisplayTooltips = analysisDisplayTooltipsDefault;
	analysisDisplayIndex = analysisDisplayIndexDefault;
	analysisDisplayArtist = analysisDisplayArtistDefault;
	analysisDisplayAlbum = analysisDisplayAlbumDefault;
	analysisDisplayTrack = analysisDisplayTrackDefault;
	analysisDisplayDuration = analysisDisplayDurationDefault;
	analysisDisplayYear = analysisDisplayYearDefault;
	analysisDisplayGenre = analysisDisplayGenreDefault;
	analysisDisplayFormat = analysisDisplayFormatDefault;
	analysisDisplayChannels = analysisDisplayChannelsDefault;
	analysisDisplayBitDepth = analysisDisplayBitDepthDefault;
	analysisDisplayBitrate = analysisDisplayBitrateDefault;
	analysisDisplaySampleRate = analysisDisplaySampleRateDefault;
	analysisDisplayMomentaryLUFS = analysisDisplayMomentaryLUFSDefault;
	analysisDisplayShortTermLUFS = analysisDisplayShortTermLUFSDefault;
	analysisDisplayIntegratedLUFS = analysisDisplayIntegratedLUFSDefault;
	analysisDisplayRMS = analysisDisplayRMSDefault;
	analysisDisplaySamplePeak = analysisDisplaySamplePeakDefault;
	analysisDisplayTruePeak = analysisDisplayTruePeakDefault;
	analysisDisplayPSR = analysisDisplayPSRDefault;
	analysisDisplayPLR = analysisDisplayPLRDefault;
	analysisDisplayCrestFactor = analysisDisplayCrestFactorDefault;
	analysisDisplayLoudnessRange = analysisDisplayLoudnessRangeDefault;
	analysisDisplayDynamicRange = analysisDisplayDynamicRangeDefault;
	analysisDisplayDynamicRangeAlbum = analysisDisplayDynamicRangeAlbumDefault;
	analysisDisplayPureDynamics = analysisDisplayPureDynamicsDefault;
	analysisDisplayPureDynamicsAlbum = analysisDisplayPureDynamicsAlbumDefault;
	analysisDisplayMetadataPreset = analysisDisplayMetadataPresetDefault;
	analysisDisplayMetricsPreset = analysisDisplayMetricsPresetDefault;
}

void AudioWizardSettings::ResetAnalysisTagsSettings() {
	analysisTagsLUFS = analysisTagsLUFSDefault;
	analysisTagsRMS = analysisTagsRMSDefault;
	analysisTagsSP = analysisTagsSPDefault;
	analysisTagsTP = analysisTagsTPDefault;
	analysisTagsPSR = analysisTagsPSRDefault;
	analysisTagsPLR = analysisTagsPLRDefault;
	analysisTagsCF = analysisTagsCFDefault;
	analysisTagsLRA = analysisTagsLRADefault;
	analysisTagsDR = analysisTagsDRDefault;
	analysisTagsDRAlbum = analysisTagsDRAlbumDefault;
	analysisTagsPD = analysisTagsPDDefault;
	analysisTagsPDAlbum = analysisTagsPDAlbumDefault;
}

void AudioWizardSettings::ResetMonitorSettings() {
	monitorDisplayTitleBarMetrics = monitorDisplayTitleBarMetricsDefault;
	monitorDisplayTitleBarMetricsFraction = monitorDisplayTitleBarMetricsFractionDefault;
	monitorDisplayColors = monitorDisplayColorsDefault;
	monitorDisplayTooltips = monitorDisplayTooltipsDefault;
	monitorDisplayMomentaryLUFS = monitorDisplayMomentaryLUFSDefault;
	monitorDisplayShortTermLUFS = monitorDisplayShortTermLUFSDefault;
	monitorDisplayRMS = monitorDisplayRMSDefault;
	monitorDisplayLeftRMS = monitorDisplayLeftRMSDefault;
	monitorDisplayRightRMS = monitorDisplayRightRMSDefault;
	monitorDisplayLeftSamplePeak = monitorDisplayLeftSamplePeakDefault;
	monitorDisplayRightSamplePeak = monitorDisplayRightSamplePeakDefault;
	monitorDisplayTruePeak = monitorDisplayTruePeakDefault;
	monitorDisplayPSR = monitorDisplayPSRDefault;
	monitorDisplayPLR = monitorDisplayPLRDefault;
	monitorDisplayCrestFactor = monitorDisplayCrestFactorDefault;
	monitorDisplayDynamicRange = monitorDisplayDynamicRangeDefault;
	monitorDisplayPureDynamics = monitorDisplayPureDynamicsDefault;
	monitorDisplayPhaseCorrelation = monitorDisplayPhaseCorrelationDefault;
	monitorDisplayStereoWidth = monitorDisplayStereoWidthDefault;
	monitorDisplayMetricsMode = monitorDisplayMetricsModeDefault;
	monitorDisplayRefreshRate = monitorDisplayRefreshRateDefault;
}
#pragma endregion
