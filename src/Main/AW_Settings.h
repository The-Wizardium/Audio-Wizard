/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Settings Header File                       * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    23-12-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


///////////////////////////////
// * AUDIO WIZARD SETTINGS * //
///////////////////////////////
#pragma region Audio Wizard Settings
class AudioWizardSettings {
public:
	// * GUID DECLARATIONS * //
	static constexpr GUID guid_analysisDisplayColors = { 0x2cb2fc1, 0x7e36, 0x4622, { 0x88, 0xf9, 0x84, 0x9e, 0x86, 0x75, 0xa0, 0x21 } };
	static constexpr GUID guid_analysisDisplayTooltips = { 0xda9d4fec, 0xd243, 0x4fa9, { 0x94, 0xf9, 0x29, 0x1d, 0xbc, 0x91, 0xb9, 0xd } };
	static constexpr GUID guid_analysisDisplayIndex = { 0xd1ab397, 0xb539, 0x43e3, { 0x83, 0x5b, 0xbb, 0x15, 0x89, 0xd9, 0x4f, 0x1c } };
	static constexpr GUID guid_analysisDisplayArtist = { 0x34015270, 0x9ad, 0x46bd, { 0xb8, 0xbb, 0xb6, 0xf4, 0x88, 0xcf, 0xd3, 0x91 } };
	static constexpr GUID guid_analysisDisplayAlbum = { 0x71d8c5bd, 0x69bd, 0x4b02, { 0x98, 0x16, 0xd4, 0xdf, 0xbc, 0x3c, 0xfd, 0xb2 } };
	static constexpr GUID guid_analysisDisplayTrack = { 0x691618ba, 0x6965, 0x4196, { 0xbc, 0xb6, 0xb4, 0x91, 0x72, 0xf6, 0x63, 0xe8 } };
	static constexpr GUID guid_analysisDisplayDuration = { 0xceb2e882, 0xc344, 0x46fc, { 0x97, 0xbf, 0x15, 0x61, 0xb5, 0xaa, 0x8f, 0xbb } };
	static constexpr GUID guid_analysisDisplayYear = { 0x202ce499, 0xdf16, 0x417c, { 0x89, 0x61, 0x2c, 0xdb, 0xff, 0x0, 0x8c, 0x7a } };
	static constexpr GUID guid_analysisDisplayGenre = { 0xe60edc1a, 0xd832, 0x4da4, { 0x93, 0x81, 0x5c, 0xb2, 0xff, 0x4b, 0x23, 0x7e } };
	static constexpr GUID guid_analysisDisplayFormat = { 0x4f8887a2, 0x859f, 0x41de, { 0x92, 0x2e, 0xed, 0xff, 0x4d, 0x3c, 0xe9, 0x85 } };
	static constexpr GUID guid_analysisDisplayChannels = { 0xb0b1ad8e, 0x6dec, 0x4e4d, { 0xbf, 0x6c, 0x7f, 0x29, 0x72, 0x7c, 0x10, 0x1f } };
	static constexpr GUID guid_analysisDisplayBitDepth = { 0x8f62dbf8, 0xc978, 0x4fd8, { 0x88, 0x84, 0x8c, 0x7b, 0xc9, 0xc0, 0x57, 0x7 } };
	static constexpr GUID guid_analysisDisplayBitrate = { 0x7e67823c, 0xec96, 0x4148, { 0xbb, 0xaa, 0xa6, 0x39, 0x3c, 0x74, 0x4f, 0xe3 } };
	static constexpr GUID guid_analysisDisplaySampleRate = { 0x8fef8e8, 0x6e6c, 0x4566, { 0xab, 0xfa, 0x4c, 0x9b, 0xd9, 0xa2, 0xbf, 0x12 } };
	static constexpr GUID guid_analysisDisplayMomentaryLUFS = { 0x552d9e98, 0xf813, 0x4f2f, { 0xb0, 0x39, 0xe9, 0xe4, 0x7e, 0xa3, 0xe2, 0x3e } };
	static constexpr GUID guid_analysisDisplayShortTermLUFS = { 0x14adba63, 0xc3c5, 0x4056, { 0x8f, 0x10, 0x18, 0xe, 0xb5, 0xec, 0x1a, 0x7 } };
	static constexpr GUID guid_analysisDisplayIntegratedLUFS = { 0xa83a3bb6, 0x2aa4, 0x4044, { 0xb3, 0xae, 0xf5, 0xb1, 0x72, 0x1c, 0x8, 0x61 } };
	static constexpr GUID guid_analysisDisplayRMS = { 0xbe35f909, 0xca8, 0x4425, { 0x92, 0x75, 0x0, 0x8e, 0xfc, 0x14, 0xc2, 0xf } };
	static constexpr GUID guid_analysisDisplaySamplePeak = { 0xe0d17aa2, 0x9022, 0x4c35, { 0xae, 0x1f, 0x7b, 0xe9, 0x8b, 0x3b, 0x4a, 0x77 } };
	static constexpr GUID guid_analysisDisplayTruePeak = { 0x979848b3, 0xf8b1, 0x4cd8, { 0xae, 0x0, 0x3c, 0xcb, 0x64, 0x17, 0x8f, 0x8a } };
	static constexpr GUID guid_analysisDisplayPSR = { 0xdc07a526, 0x7f72, 0x444c, { 0x8b, 0xf6, 0x75, 0xca, 0xc2, 0xec, 0xd2, 0xb3 } };
	static constexpr GUID guid_analysisDisplayPLR = { 0x18257579, 0xada6, 0x47e2, { 0x8b, 0xad, 0xf2, 0x7b, 0xab, 0xc8, 0xc2, 0x33 } };
	static constexpr GUID guid_analysisDisplayCrestFactor = { 0xe9748a0c, 0x8e81, 0x4e1b, { 0x80, 0x6d, 0xbc, 0x62, 0x62, 0x9b, 0x57, 0xc0 } };
	static constexpr GUID guid_analysisDisplayLoudnessRange = { 0xe4775df5, 0x4f27, 0x4c6e, { 0xb0, 0xe5, 0xa3, 0x4d, 0xd3, 0x43, 0xf1, 0x7b } };
	static constexpr GUID guid_analysisDisplayDynamicRange = { 0x4fff021d, 0x8f54, 0x4d56, { 0xbe, 0x2e, 0x5f, 0x78, 0xf1, 0x49, 0xaf, 0x3c } };
	static constexpr GUID guid_analysisDisplayDynamicRangeAlbum = { 0x71aae1fa, 0x3471, 0x448a, { 0xbe, 0x3d, 0x73, 0x6f, 0x5c, 0xbc, 0x42, 0x29 } };
	static constexpr GUID guid_analysisDisplayPureDynamics = { 0xd5ff8d86, 0x593e, 0x4c9e, { 0xac, 0x18, 0xdb, 0xb2, 0xb6, 0x4e, 0x89, 0x29 } };
	static constexpr GUID guid_analysisDisplayPureDynamicsAlbum = { 0xd8679586, 0x9d10, 0x4925, { 0x9a, 0x31, 0x93, 0xf3, 0x16, 0x5d, 0x81, 0x9f } };
	static constexpr GUID guid_analysisDisplayMetadataPreset = { 0xf345847d, 0x944d, 0x4be5, { 0xa3, 0xde, 0xf3, 0x98, 0xdc, 0xdb, 0xfd, 0xab } };
	static constexpr GUID guid_analysisDisplayMetricsPreset = { 0x6e434f17, 0x9605, 0x48be, { 0x87, 0x0, 0x9f, 0x41, 0x24, 0x67, 0xde, 0x93 } };
	static constexpr GUID guid_analysisTagsWriting = { 0x5b1cac9f, 0x97c4, 0x49c0, { 0xbb, 0xd4, 0xe6, 0x98, 0x15, 0xc2, 0x81, 0xea } };
	static constexpr GUID guid_analysisTagsClearing = { 0x14ed8fa1, 0x94aa, 0x4eca, { 0xaf, 0xf3, 0xf8, 0x7b, 0x7c, 0x9c, 0xbb, 0xf0 } };

	static constexpr GUID guid_analysisTagsLUFS = { 0xafc0dac, 0x369, 0x4b33, { 0xac, 0xd3, 0x74, 0xa0, 0x18, 0xff, 0xfd, 0x21 } };
	static constexpr GUID guid_analysisTagsRMS = { 0x30faf90c, 0x2464, 0x407c, { 0x95, 0x22, 0xc, 0xc5, 0x1e, 0xb2, 0xd6, 0xe7 } };
	static constexpr GUID guid_analysisTagsSP = { 0xd4fbc405, 0x81c3, 0x4029, { 0x93, 0xe, 0xea, 0x14, 0xef, 0x5d, 0x1e, 0x81 } };
	static constexpr GUID guid_analysisTagsTP = { 0x7a35981d, 0xcdbb, 0x4c5b, { 0x90, 0x37, 0xb3, 0x26, 0xd, 0xb6, 0xda, 0x67 } };
	static constexpr GUID guid_analysisTagsPSR = { 0xc96f172f, 0x24c8, 0x401f, { 0xb9, 0x36, 0xb5, 0xa0, 0x20, 0x95, 0xe7, 0xc0 } };
	static constexpr GUID guid_analysisTagsPLR = { 0x5484b203, 0x319c, 0x41be, { 0x8e, 0x11, 0xb7, 0x24, 0x53, 0xc3, 0x99, 0x49 } };
	static constexpr GUID guid_analysisTagsCF = { 0xcec52d61, 0xeb20, 0x4142, { 0x92, 0xb7, 0x43, 0xae, 0x81, 0x95, 0xfd, 0xbf } };
	static constexpr GUID guid_analysisTagsLRA = { 0x9e3cad3c, 0x19, 0x4473, { 0xab, 0x4, 0xf6, 0xb3, 0x84, 0x2b, 0xde, 0xc3 } };
	static constexpr GUID guid_analysisTagsDR = { 0x81bbe45b, 0x6f05, 0x4e1e, { 0xb1, 0x9e, 0x7d, 0xc6, 0xbe, 0xae, 0xe9, 0x81 } };
	static constexpr GUID guid_analysisTagsDRAlbum = { 0xbf19702, 0x1f5b, 0x4e60, { 0x8e, 0x46, 0xca, 0x7e, 0x8, 0x53, 0x6f, 0x51 } };
	static constexpr GUID guid_analysisTagsPD = { 0x7cf9e841, 0xb9e4, 0x4029, { 0xa1, 0xfe, 0xec, 0x4, 0xef, 0x87, 0xe1, 0xc1 } };
	static constexpr GUID guid_analysisTagsPDAlbum = { 0xa7d594bc, 0x4282, 0x452f, { 0x9e, 0x92, 0xd0, 0xa2, 0x9a, 0x5, 0xe8, 0x61 } };

	static constexpr GUID guid_monitorDisplayTitleBarMetrics = { 0x6dcff1ab, 0x8045, 0x4dfd, { 0x9d, 0x9c, 0x6f, 0xa5, 0xac, 0xe5, 0x65, 0x33 } };
	static constexpr GUID guid_monitorDisplayTitleBarMetricsFraction = { 0xc40232db, 0xcf61, 0x479f, { 0xa5, 0x18, 0xc2, 0x36, 0x7f, 0x4, 0x7d, 0x8f } };
	static constexpr GUID guid_monitorDisplayColors = { 0x6b6d3699, 0x9ecd, 0x47ef, { 0xa7, 0xb7, 0x30, 0xa0, 0x4d, 0xd6, 0x8e, 0x13 } };
	static constexpr GUID guid_monitorDisplayTooltips = { 0x52b71759, 0x6809, 0x41ff, { 0xab, 0xe2, 0x1b, 0xa5, 0x49, 0xaf, 0xa6, 0x4d } };
	static constexpr GUID guid_monitorDisplayMomentaryLUFS = { 0x4f430b6d, 0x369d, 0x42a4, { 0xa0, 0xcc, 0xea, 0x1b, 0x31, 0x3d, 0x84, 0x91 } };
	static constexpr GUID guid_monitorDisplayShortTermLUFS = { 0x29d67563, 0x9d1a, 0x4af0, { 0xaf, 0x3, 0xad, 0x1, 0xa2, 0x78, 0x30, 0x6b } };
	static constexpr GUID guid_monitorDisplayRMS = { 0xeb08cdb9, 0x2180, 0x4d0f, { 0xaf, 0x58, 0xb6, 0xe9, 0x47, 0xac, 0x61, 0x84 } };
	static constexpr GUID guid_monitorDisplayLeftRMS = { 0x5331a534, 0x478c, 0x4316, { 0xa8, 0xb2, 0xe4, 0x6d, 0xfa, 0x8f, 0xbf, 0x5b } };
	static constexpr GUID guid_monitorDisplayRightRMS = { 0xdd85abcd, 0x65d2, 0x416f, { 0xbd, 0x72, 0x24, 0x9a, 0x49, 0xb, 0x3a, 0x73 } };
	static constexpr GUID guid_monitorDisplayLeftSamplePeak = { 0xbf906666, 0x4188, 0x4a1a, { 0xad, 0xee, 0xfd, 0x54, 0x8, 0xad, 0xbd, 0x64 } };
	static constexpr GUID guid_monitorDisplayRightSamplePeak = { 0xfd31df99, 0xe0fa, 0x46ec, { 0xa1, 0x46, 0x85, 0xcc, 0x47, 0xd, 0x35, 0x7 } };
	static constexpr GUID guid_monitorDisplayTruePeak = { 0x5c55e104, 0xb675, 0x4da0, { 0xbb, 0x31, 0xa0, 0xda, 0x56, 0xc2, 0xdc, 0xdd } };
	static constexpr GUID guid_monitorDisplayPSR = { 0x7cf8eb34, 0x36fd, 0x479d, { 0xa7, 0xdf, 0xc0, 0xc2, 0x50, 0x8a, 0x58, 0x9a } };
	static constexpr GUID guid_monitorDisplayPLR = { 0x9dfe056a, 0x3987, 0x4b9a, { 0x97, 0x40, 0xb3, 0x8e, 0xc9, 0x2a, 0xb5, 0xe3 } };
	static constexpr GUID guid_monitorDisplayCrestFactor = { 0x373c5ce6, 0xa29e, 0x4a9e, { 0xad, 0xf6, 0xee, 0xad, 0xe7, 0x92, 0xb7, 0x25 } };
	static constexpr GUID guid_monitorDisplayDynamicRange = { 0xf5bba50f, 0x74df, 0x43a1, { 0xb4, 0xde, 0x5, 0x16, 0xb5, 0x15, 0x71, 0x8c } };
	static constexpr GUID guid_monitorDisplayPureDynamics = { 0xbffbcbf7, 0xa435, 0x4f3e, { 0xb3, 0xa8, 0xb2, 0xbe, 0xc8, 0xd3, 0x27, 0x35 } };
	static constexpr GUID guid_monitorDisplayPhaseCorrelation = { 0xfb970c09, 0x76ad, 0x4a39, { 0x8c, 0x86, 0x86, 0xb9, 0x8d, 0x26, 0x74, 0x6d } };
	static constexpr GUID guid_monitorDisplayStereoWidth = { 0x5c73c99a, 0x11de, 0x4f0d, { 0x8b, 0xe3, 0x9c, 0xca, 0xca, 0x4, 0xde, 0xd9 } };
	static constexpr GUID guid_monitorDisplayMetricsMode = { 0x4021f07e, 0xce1f, 0x40bd, { 0xac, 0xbd, 0x66, 0xec, 0x8f, 0xdf, 0x9f, 0x2b } };
	static constexpr GUID guid_monitorDisplayRefreshRate = { 0x2d86e9f8, 0x28b3, 0x42dd, { 0xb1, 0xb3, 0x2, 0x11, 0xe, 0x49, 0xdd, 0xb8 } };

	static constexpr GUID guid_systemDebugLog = { 0x20d9e4ee, 0xc4ee, 0x4b29, { 0x80, 0xc7, 0x6d, 0x8c, 0x26, 0x17, 0x61, 0x0 } };

	// * STATIC MEMBER DEFAULTS * //
	static constexpr bool analysisDisplayColorsDefault = true;
	static constexpr bool analysisDisplayTooltipsDefault = true;
	static constexpr bool analysisDisplayIndexDefault = true;
	static constexpr bool analysisDisplayArtistDefault = true;
	static constexpr bool analysisDisplayAlbumDefault = true;
	static constexpr bool analysisDisplayTrackDefault = true;
	static constexpr bool analysisDisplayDurationDefault = true;
	static constexpr bool analysisDisplayYearDefault = true;
	static constexpr bool analysisDisplayGenreDefault = true;
	static constexpr bool analysisDisplayFormatDefault = true;
	static constexpr bool analysisDisplayChannelsDefault = true;
	static constexpr bool analysisDisplayBitDepthDefault = true;
	static constexpr bool analysisDisplayBitrateDefault = true;
	static constexpr bool analysisDisplaySampleRateDefault = true;
	static constexpr bool analysisDisplayMomentaryLUFSDefault = true;
	static constexpr bool analysisDisplayShortTermLUFSDefault = true;
	static constexpr bool analysisDisplayIntegratedLUFSDefault = true;
	static constexpr bool analysisDisplayRMSDefault = true;
	static constexpr bool analysisDisplaySamplePeakDefault = true;
	static constexpr bool analysisDisplayTruePeakDefault = true;
	static constexpr bool analysisDisplayPSRDefault = true;
	static constexpr bool analysisDisplayPLRDefault = true;
	static constexpr bool analysisDisplayCrestFactorDefault = true;
	static constexpr bool analysisDisplayLoudnessRangeDefault = true;
	static constexpr bool analysisDisplayDynamicRangeDefault = true;
	static constexpr bool analysisDisplayDynamicRangeAlbumDefault = true;
	static constexpr bool analysisDisplayPureDynamicsDefault = true;
	static constexpr bool analysisDisplayPureDynamicsAlbumDefault = true;
	static constexpr int analysisDisplayMetadataPresetDefault = 0;
	static constexpr int analysisDisplayMetricsPresetDefault = 0;

	static constexpr int analysisTagsWritingDefault = 0;
	static constexpr int analysisTagsClearingDefault = 0;
	static constexpr const char* analysisTagsLUFSDefault = "AW_LUFS";
	static constexpr const char* analysisTagsRMSDefault = "AW_RMS";
	static constexpr const char* analysisTagsSPDefault = "AW_SP";
	static constexpr const char* analysisTagsTPDefault = "AW_TP";
	static constexpr const char* analysisTagsPSRDefault = "AW_PSR";
	static constexpr const char* analysisTagsPLRDefault = "AW_PLR";
	static constexpr const char* analysisTagsCFDefault = "AW_CF";
	static constexpr const char* analysisTagsLRADefault = "AW_LRA";
	static constexpr const char* analysisTagsDRDefault = "AW_DR";
	static constexpr const char* analysisTagsDRAlbumDefault = "AW_DR_ALBUM";
	static constexpr const char* analysisTagsPDDefault = "AW_PD";
	static constexpr const char* analysisTagsPDAlbumDefault = "AW_PD_ALBUM";

	static constexpr bool monitorDisplayTitleBarMetricsDefault = true;
	static constexpr bool monitorDisplayTitleBarMetricsFractionDefault = true;
	static constexpr bool monitorDisplayColorsDefault = true;
	static constexpr bool monitorDisplayTooltipsDefault = true;
	static constexpr bool monitorDisplayMomentaryLUFSDefault = true;
	static constexpr bool monitorDisplayShortTermLUFSDefault = true;
	static constexpr bool monitorDisplayRMSDefault = true;
	static constexpr bool monitorDisplayLeftRMSDefault = true;
	static constexpr bool monitorDisplayRightRMSDefault = true;
	static constexpr bool monitorDisplayLeftSamplePeakDefault = true;
	static constexpr bool monitorDisplayRightSamplePeakDefault = true;
	static constexpr bool monitorDisplayTruePeakDefault = true;
	static constexpr bool monitorDisplayPSRDefault = true;
	static constexpr bool monitorDisplayPLRDefault = true;
	static constexpr bool monitorDisplayCrestFactorDefault = true;
	static constexpr bool monitorDisplayDynamicRangeDefault = true;
	static constexpr bool monitorDisplayPureDynamicsDefault = true;
	static constexpr bool monitorDisplayPhaseCorrelationDefault = true;
	static constexpr bool monitorDisplayStereoWidthDefault = true;
	static constexpr int monitorDisplayMetricsModeDefault = 4;
	static constexpr int monitorDisplayRefreshRateDefault = 33;

	static constexpr bool systemDebugLogDefault = false;

	// * STATIC MEMBERS * //
	static inline cfg_bool analysisDisplayColors{ guid_analysisDisplayColors, analysisDisplayColorsDefault };
	static inline cfg_bool analysisDisplayTooltips{ guid_analysisDisplayTooltips, analysisDisplayTooltipsDefault };
	static inline cfg_bool analysisDisplayIndex{ guid_analysisDisplayIndex, analysisDisplayIndexDefault };
	static inline cfg_bool analysisDisplayArtist{ guid_analysisDisplayArtist, analysisDisplayArtistDefault };
	static inline cfg_bool analysisDisplayAlbum{ guid_analysisDisplayAlbum, analysisDisplayAlbumDefault };
	static inline cfg_bool analysisDisplayTrack{ guid_analysisDisplayTrack, analysisDisplayTrackDefault };
	static inline cfg_bool analysisDisplayDuration{ guid_analysisDisplayDuration, analysisDisplayDurationDefault };
	static inline cfg_bool analysisDisplayYear{ guid_analysisDisplayYear, analysisDisplayYearDefault };
	static inline cfg_bool analysisDisplayGenre{ guid_analysisDisplayGenre, analysisDisplayGenreDefault };
	static inline cfg_bool analysisDisplayFormat{ guid_analysisDisplayFormat, analysisDisplayFormatDefault };
	static inline cfg_bool analysisDisplayChannels{ guid_analysisDisplayChannels, analysisDisplayChannelsDefault };
	static inline cfg_bool analysisDisplayBitDepth{ guid_analysisDisplayBitDepth, analysisDisplayBitDepthDefault };
	static inline cfg_bool analysisDisplayBitrate{ guid_analysisDisplayBitrate, analysisDisplayBitrateDefault };
	static inline cfg_bool analysisDisplaySampleRate{ guid_analysisDisplaySampleRate, analysisDisplaySampleRateDefault };
	static inline cfg_bool analysisDisplayMomentaryLUFS{ guid_analysisDisplayMomentaryLUFS, analysisDisplayMomentaryLUFSDefault };
	static inline cfg_bool analysisDisplayShortTermLUFS{ guid_analysisDisplayShortTermLUFS, analysisDisplayShortTermLUFSDefault };
	static inline cfg_bool analysisDisplayIntegratedLUFS{ guid_analysisDisplayIntegratedLUFS, analysisDisplayIntegratedLUFSDefault };
	static inline cfg_bool analysisDisplayRMS{ guid_analysisDisplayRMS, analysisDisplayRMSDefault };
	static inline cfg_bool analysisDisplaySamplePeak{ guid_analysisDisplaySamplePeak, analysisDisplaySamplePeakDefault };
	static inline cfg_bool analysisDisplayTruePeak{ guid_analysisDisplayTruePeak, analysisDisplayTruePeakDefault };
	static inline cfg_bool analysisDisplayPSR{ guid_analysisDisplayPSR, analysisDisplayPSRDefault };
	static inline cfg_bool analysisDisplayPLR{ guid_analysisDisplayPLR, analysisDisplayPLRDefault };
	static inline cfg_bool analysisDisplayCrestFactor{ guid_analysisDisplayCrestFactor, analysisDisplayCrestFactorDefault };
	static inline cfg_bool analysisDisplayLoudnessRange{ guid_analysisDisplayLoudnessRange, analysisDisplayLoudnessRangeDefault };
	static inline cfg_bool analysisDisplayDynamicRange{ guid_analysisDisplayDynamicRange, analysisDisplayDynamicRangeDefault };
	static inline cfg_bool analysisDisplayDynamicRangeAlbum{ guid_analysisDisplayDynamicRangeAlbum, analysisDisplayDynamicRangeAlbumDefault };
	static inline cfg_bool analysisDisplayPureDynamics{ guid_analysisDisplayPureDynamics, analysisDisplayPureDynamicsDefault };
	static inline cfg_bool analysisDisplayPureDynamicsAlbum{ guid_analysisDisplayPureDynamicsAlbum, analysisDisplayPureDynamicsAlbumDefault };
	static inline cfg_int analysisDisplayMetadataPreset{ guid_analysisDisplayMetadataPreset, analysisDisplayMetadataPresetDefault };
	static inline cfg_int analysisDisplayMetricsPreset{ guid_analysisDisplayMetricsPreset, analysisDisplayMetricsPresetDefault };

	static inline cfg_int analysisTagsWriting{ guid_analysisTagsWriting, analysisTagsWritingDefault };
	static inline cfg_int analysisTagsClearing{ guid_analysisTagsClearing, analysisTagsClearingDefault };
	static inline cfg_string analysisTagsLUFS{ guid_analysisTagsLUFS, analysisTagsLUFSDefault };
	static inline cfg_string analysisTagsRMS{ guid_analysisTagsRMS, analysisTagsRMSDefault };
	static inline cfg_string analysisTagsSP{ guid_analysisTagsSP, analysisTagsSPDefault };
	static inline cfg_string analysisTagsTP{ guid_analysisTagsTP, analysisTagsTPDefault };
	static inline cfg_string analysisTagsPSR{ guid_analysisTagsPSR, analysisTagsPSRDefault };
	static inline cfg_string analysisTagsPLR{ guid_analysisTagsPLR, analysisTagsPLRDefault };
	static inline cfg_string analysisTagsCF{ guid_analysisTagsCF, analysisTagsCFDefault };
	static inline cfg_string analysisTagsLRA{ guid_analysisTagsLRA, analysisTagsLRADefault };
	static inline cfg_string analysisTagsDR{ guid_analysisTagsDR, analysisTagsDRDefault };
	static inline cfg_string analysisTagsDRAlbum{ guid_analysisTagsDRAlbum, analysisTagsDRAlbumDefault };
	static inline cfg_string analysisTagsPD{ guid_analysisTagsPD, analysisTagsPDDefault };
	static inline cfg_string analysisTagsPDAlbum{ guid_analysisTagsPDAlbum, analysisTagsPDAlbumDefault };

	static inline cfg_bool monitorDisplayTitleBarMetrics{ guid_monitorDisplayTitleBarMetrics, monitorDisplayTitleBarMetricsDefault };
	static inline cfg_bool monitorDisplayTitleBarMetricsFraction{ guid_monitorDisplayTitleBarMetricsFraction, monitorDisplayTitleBarMetricsFractionDefault };
	static inline cfg_bool monitorDisplayColors{ guid_monitorDisplayColors, monitorDisplayColorsDefault };
	static inline cfg_bool monitorDisplayTooltips{ guid_monitorDisplayTooltips, monitorDisplayTooltipsDefault };
	static inline cfg_bool monitorDisplayMomentaryLUFS{ guid_monitorDisplayMomentaryLUFS, monitorDisplayMomentaryLUFSDefault };
	static inline cfg_bool monitorDisplayShortTermLUFS{ guid_monitorDisplayShortTermLUFS, monitorDisplayShortTermLUFSDefault };
	static inline cfg_bool monitorDisplayRMS{ guid_monitorDisplayRMS, monitorDisplayRMSDefault };
	static inline cfg_bool monitorDisplayLeftRMS{ guid_monitorDisplayLeftRMS, monitorDisplayLeftRMSDefault };
	static inline cfg_bool monitorDisplayRightRMS{ guid_monitorDisplayRightRMS, monitorDisplayRightRMSDefault };
	static inline cfg_bool monitorDisplayLeftSamplePeak{ guid_monitorDisplayLeftSamplePeak, monitorDisplayLeftSamplePeakDefault };
	static inline cfg_bool monitorDisplayRightSamplePeak{ guid_monitorDisplayRightSamplePeak, monitorDisplayRightSamplePeakDefault };
	static inline cfg_bool monitorDisplayTruePeak{ guid_monitorDisplayTruePeak, monitorDisplayTruePeakDefault };
	static inline cfg_bool monitorDisplayPSR{ guid_monitorDisplayPSR, monitorDisplayPSRDefault };
	static inline cfg_bool monitorDisplayPLR{ guid_monitorDisplayPLR, monitorDisplayPLRDefault };
	static inline cfg_bool monitorDisplayCrestFactor{ guid_monitorDisplayCrestFactor, monitorDisplayCrestFactorDefault };
	static inline cfg_bool monitorDisplayDynamicRange{ guid_monitorDisplayDynamicRange, monitorDisplayDynamicRangeDefault };
	static inline cfg_bool monitorDisplayPureDynamics{ guid_monitorDisplayPureDynamics, monitorDisplayPureDynamicsDefault };
	static inline cfg_bool monitorDisplayPhaseCorrelation{ guid_monitorDisplayPhaseCorrelation, monitorDisplayPhaseCorrelationDefault };
	static inline cfg_bool monitorDisplayStereoWidth{ guid_monitorDisplayStereoWidth, monitorDisplayStereoWidthDefault };
	static inline cfg_int monitorDisplayMetricsMode{ guid_monitorDisplayMetricsMode, monitorDisplayMetricsModeDefault };
	static inline cfg_int monitorDisplayRefreshRate{ guid_monitorDisplayRefreshRate, monitorDisplayRefreshRateDefault };

	static inline cfg_bool systemDebugLog{ guid_systemDebugLog, systemDebugLogDefault };

	// * STATIC METHODS * //
	static void InitAnalysisSettings(HWND hWnd);
	static void InitAnalysisTagsSettings(HWND hWnd);
	static void InitMonitorSettings(HWND hWnd);
	static bool InitMetadataPreset(int presetIndex);
	static bool InitMetricsPreset(int presetIndex);
	static void GetAnalysisPreset(HWND hWnd);
	static void SetAnalysisMetadataPreset(int presetIndex);
	static void SetAnalysisMetricsPreset(int presetIndex);
	static void SetAnalysisSettings(HWND hWnd);
	static void SetAnalysisTagsSettings(HWND hWnd);
	static void SetMonitorSettings(HWND hWnd);
	static void ResetAnalysisSettings();
	static void ResetAnalysisTagsSettings();
	static void ResetMonitorSettings();
};
#pragma endregion
