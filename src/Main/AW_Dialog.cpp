/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Dialog Source File                         * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW_Dialog.h"
#include "AW_Helpers.h"


/////////////////////
// * DIALOG BASE * //
/////////////////////
#pragma region Dialog Base
void AudioWizardDialog::InitDarkMode() {
	darkMode = AWHDarkMode::IsDark();
}

void AudioWizardDialog::InitMetricConfig() {
	static constexpr std::array<std::tuple<const wchar_t*, COLORREF, COLORREF>, 5> Colors = {{
		{ L"Blue",    RGB(40, 110, 230), RGB(20, 170, 255)  },
		{ L"Green",   RGB(40, 150, 60),  RGB(80, 240, 140)  },
		{ L"Default", RGB(0, 0, 0),      RGB(240, 240, 240) },
		{ L"Orange",  RGB(230, 120, 20), RGB(255, 165, 55)  },
		{ L"Red",     RGB(230, 25, 45),  RGB(255, 40, 60)   }
	}};

	constexpr double LOWEST = -INFINITY;
	constexpr double HIGHEST = INFINITY;

	auto CreateMetricData = [&](const MetricKey& key, const std::vector<std::tuple<double, double, MetricColor, const wchar_t*>>& ranges) {
		MetricData data;
		data.name = GetMetricName(key.id);
		data.fullName = GetMetricFullName(key.id);
		data.ranges.reserve(ranges.size());

		CStringW tooltip = data.fullName.c_str();
		tooltip += L"\nSEPARATOR\n";

		for (const auto& [min, max, color, desc] : ranges) {
			const auto& [colorName, dark, light] = Colors[static_cast<size_t>(color)];
			MetricRange range;
			range.minRange = min;
			range.maxRange = max;
			range.lightColor = light;
			range.darkColor = dark;
			range.description = desc;
			data.ranges.push_back(range);

			tooltip += GetMetricRangeFormatted(min, max, desc);
		}

		data.tooltip = tooltip;
		metricData[key] = std::move(data);
	};

	// Full-Track Metric Configurations
	const std::vector<std::pair<MetricId, std::vector<std::tuple<double, double, MetricColor, const wchar_t*>>>> fullTrackMetrics = {
		{ MetricId::M_LUFS_FT, {
			{ LOWEST,   -30.0, MetricColor::Blue,    L"Too quiet" },
			{  -30.0,   -23.0, MetricColor::Green,   L"EBU R128 target" },
			{  -23.0,   -18.0, MetricColor::Default, L"Typical range" },
			{  -18.0,   -14.0, MetricColor::Orange,  L"Near limit" },
			{  -14.0, HIGHEST, MetricColor::Red,     L"Overloud" }
		}},
		{ MetricId::S_LUFS_FT, {
			{ LOWEST,   -30.0, MetricColor::Blue,    L"Too quiet" },
			{  -30.0,   -23.0, MetricColor::Green,   L"EBU R128 target" },
			{  -23.0,   -18.0, MetricColor::Default, L"Typical range" },
			{  -18.0,   -14.0, MetricColor::Orange,  L"Near limit" },
			{  -14.0, HIGHEST, MetricColor::Red,     L"Overloud" }
		}},
		{ MetricId::I_LUFS_FT, {
			{ LOWEST,   -24.0, MetricColor::Blue,    L"Too quiet" },
			{  -24.0,   -22.0, MetricColor::Green,   L"EBU R128 compliant" },
			{  -22.0,   -18.0, MetricColor::Default, L"Typical range" },
			{  -18.0,   -14.0, MetricColor::Orange,  L"Near limit" },
			{  -14.0, HIGHEST, MetricColor::Red,     L"Overloud" }
		}},
		{ MetricId::RMS_FT, {
			{ LOWEST,   -30.0, MetricColor::Blue,    L"Below audible RMS" },
			{  -30.0,   -23.0, MetricColor::Green,   L"Target RMS (-23 dB)" },
			{  -23.0,   -18.0, MetricColor::Default, L"Typical RMS" },
			{  -18.0,   -14.0, MetricColor::Orange,  L"High RMS" },
			{  -14.0, HIGHEST, MetricColor::Red,     L"RMS overload" }
		}},
		{ MetricId::SP_FT, {
			{ LOWEST,    -3.0, MetricColor::Blue,    L"Underutilized headroom" },
			{   -3.0,    -1.0, MetricColor::Green,   L"Safe peak" },
			{   -1.0,     0.0, MetricColor::Default, L"Typical peak" },
			{    0.0,     1.0, MetricColor::Orange,  L"Clipping risk" },
			{    1.0, HIGHEST, MetricColor::Red,     L"Clipping" }
		}},
		{ MetricId::TP_FT, {
			{ LOWEST,    -3.0, MetricColor::Blue,    L"Underutilized headroom" },
			{   -3.0,    -1.0, MetricColor::Green,   L"Safe peak" },
			{   -1.0,     0.0, MetricColor::Default, L"Typical peak" },
			{    0.0,     1.0, MetricColor::Orange,  L"Intersample risk" },
			{    1.0, HIGHEST, MetricColor::Red,     L"Clipping" }
		}},
		{ MetricId::PSR_FT, {
			{ LOWEST,    12.0, MetricColor::Red,     L"Overcompressed" },
			{   12.0,    18.0, MetricColor::Orange,  L"Low dynamics" },
			{   18.0,    24.0, MetricColor::Default, L"Typical range" },
			{   24.0,    30.0, MetricColor::Green,   L"Good dynamics" },
			{   30.0, HIGHEST, MetricColor::Blue,    L"High dynamics" }
		}},
		{ MetricId::PLR_FT, {
			{ LOWEST,    12.0, MetricColor::Red,     L"Overcompressed" },
			{   12.0,    18.0, MetricColor::Orange,  L"Low dynamics" },
			{   18.0,    24.0, MetricColor::Default, L"Typical range" },
			{   24.0,    30.0, MetricColor::Green,   L"Good dynamics" },
			{   30.0, HIGHEST, MetricColor::Blue,    L"High dynamics" }
		}},
		{ MetricId::CF_FT, {
			{ LOWEST,     6.0, MetricColor::Red,     L"Overcompressed" },
			{    6.0,    12.0, MetricColor::Orange,  L"Low dynamics" },
			{   12.0,    18.0, MetricColor::Default, L"Typical range" },
			{   18.0,    24.0, MetricColor::Green,   L"Good dynamics" },
			{   24.0, HIGHEST, MetricColor::Blue,    L"High dynamics" }
		}},
		{ MetricId::LRA_FT, {
			{ LOWEST,     5.0, MetricColor::Red,     L"Overcompressed" },
			{    5.0,    10.0, MetricColor::Orange,  L"Low range" },
			{   10.0,    15.0, MetricColor::Default, L"Typical range" },
			{   15.0,    20.0, MetricColor::Green,   L"Good range" },
			{   20.0, HIGHEST, MetricColor::Blue,    L"High range" }
		}},
		{ MetricId::DR_FT, {
			{ LOWEST,     6.0, MetricColor::Red,     L"Overcompressed" },
			{    6.0,     8.0, MetricColor::Orange,  L"Low dynamics" },
			{    8.0,    10.0, MetricColor::Default, L"Typical dynamics" },
			{   10.0,    14.0, MetricColor::Green,   L"Good dynamics" },
			{   14.0, HIGHEST, MetricColor::Blue,    L"High dynamics" }
		}},
		{ MetricId::DR_ALBUM_FT, {
			{ LOWEST,     6.0, MetricColor::Red,     L"Overcompressed" },
			{    6.0,     8.0, MetricColor::Orange,  L"Low dynamics" },
			{    8.0,    10.0, MetricColor::Default, L"Typical dynamics" },
			{   10.0,    14.0, MetricColor::Green,   L"Good dynamics" },
			{   14.0, HIGHEST, MetricColor::Blue,    L"High dynamics" }
		}},
		{ MetricId::PD_FT, {
			{ LOWEST,   2.0, MetricColor::Red,     L"Overcompressed \u00B7 Minimal Dynamics" },
			{  2.0,     4.0, MetricColor::Orange,  L"Stable Loudness \u00B7 Moderate Dynamics" },
			{  4.0,     8.0, MetricColor::Default, L"Balanced Loudness \u00B7 Typical Dynamics" },
			{  8.0,    10.0, MetricColor::Green,   L"Variable Loudness \u00B7 Good Dynamics" },
			{ 10.0, HIGHEST, MetricColor::Blue,    L"Dynamic Loudness \u00B7 High Dynamics" }
		}},
		{ MetricId::PD_ALBUM_FT, {
			{ LOWEST,   2.0, MetricColor::Red,     L"Overcompressed \u00B7 Minimal Dynamics" },
			{  2.0,     4.0, MetricColor::Orange,  L"Stable Loudness \u00B7 Moderate Dynamics" },
			{  4.0,     8.0, MetricColor::Default, L"Balanced Loudness \u00B7 Typical Dynamics" },
			{  8.0,    10.0, MetricColor::Green,   L"Variable Loudness \u00B7 Good Dynamics" },
			{ 10.0, HIGHEST, MetricColor::Blue,    L"Dynamic Loudness \u00B7 High Dynamics" }
		}}
	};

	for (const auto& [id, ranges] : fullTrackMetrics) {
		CreateMetricData({ MetricContext::FullTrack, id }, ranges);
	}

	// Real-Time Metric Configurations
	const std::vector<std::pair<MetricId, std::vector<std::tuple<double, double, MetricColor, const wchar_t*>>>> realTimeMetrics = {
		{ MetricId::MOMENTARY_LUFS_RT, {
			{ LOWEST,   -35.0, MetricColor::Blue,    L"Too quiet" },
			{  -35.0,   -25.0, MetricColor::Green,   L"Low range" },
			{  -25.0,   -15.0, MetricColor::Default, L"Typical range" },
			{  -15.0,   -10.0, MetricColor::Orange,  L"High range" },
			{  -10.0, HIGHEST, MetricColor::Red,     L"Overloud" }
		}},
		{ MetricId::SHORT_TERM_LUFS_RT, {
			{ LOWEST,   -35.0, MetricColor::Blue,    L"Too quiet" },
			{  -35.0,   -25.0, MetricColor::Green,   L"Low range" },
			{  -25.0,   -15.0, MetricColor::Default, L"Typical range" },
			{  -15.0,   -10.0, MetricColor::Orange,  L"High range" },
			{  -10.0, HIGHEST, MetricColor::Red,     L"Overloud" }
		}},
		{ MetricId::RMS_RT, {
			{ LOWEST,   -30.0, MetricColor::Blue,    L"Below audible RMS" },
			{  -30.0,   -23.0, MetricColor::Green,   L"Target RMS (-23 dB)" },
			{  -23.0,   -18.0, MetricColor::Default, L"Typical RMS" },
			{  -18.0,   -14.0, MetricColor::Orange,  L"High RMS" },
			{  -14.0, HIGHEST, MetricColor::Red,     L"RMS overload" }
		}},
		{ MetricId::LEFT_RMS_RT, {
			{ LOWEST,   -35.0, MetricColor::Blue,    L"Below audible" },
			{  -35.0,   -25.0, MetricColor::Green,   L"Low range" },
			{  -25.0,   -15.0, MetricColor::Default, L"Typical range" },
			{  -15.0,   -10.0, MetricColor::Orange,  L"High range" },
			{  -10.0, HIGHEST, MetricColor::Red,     L"Overload" }
		}},
		{ MetricId::RIGHT_RMS_RT, {
			{ LOWEST,   -35.0, MetricColor::Blue,    L"Below audible" },
			{  -35.0,   -25.0, MetricColor::Green,   L"Low range" },
			{  -25.0,   -15.0, MetricColor::Default, L"Typical range" },
			{  -15.0,   -10.0, MetricColor::Orange,  L"High range" },
			{  -10.0, HIGHEST, MetricColor::Red,     L"Overload" }
		}},
		{ MetricId::LEFT_SAMPLE_PEAK_RT, {
			{ LOWEST,   -15.0, MetricColor::Blue,    L"Excessive headroom" },
			{  -15.0,   -10.0, MetricColor::Green,   L"Safe peak" },
			{  -10.0,    -3.0, MetricColor::Default, L"Typical peak" },
			{   -3.0,     0.0, MetricColor::Orange,  L"Intersample risk" },
			{    0.0, HIGHEST, MetricColor::Red,     L"Clipping" }
		}},
		{ MetricId::RIGHT_SAMPLE_PEAK_RT, {
			{ LOWEST,   -15.0, MetricColor::Blue,    L"Excessive headroom" },
			{  -15.0,   -10.0, MetricColor::Green,   L"Safe peak" },
			{  -10.0,    -3.0, MetricColor::Default, L"Typical peak" },
			{   -3.0,     0.0, MetricColor::Orange,  L"Intersample risk" },
			{    0.0, HIGHEST, MetricColor::Red,     L"Clipping" }
		}},
		{ MetricId::TRUE_PEAK_RT, {
			{ LOWEST,   -15.0, MetricColor::Blue,    L"Underutilized headroom" },
			{  -15.0,   -10.0, MetricColor::Green,   L"Safe peak" },
			{  -10.0,    -3.0, MetricColor::Default, L"Typical peak" },
			{   -3.0,     0.0, MetricColor::Orange,  L"Intersample risk" },
			{    0.0, HIGHEST, MetricColor::Red,     L"Clipping" }
		}},
		{ MetricId::PSR_RT, {
			{ LOWEST,     6.0, MetricColor::Red,     L"Overcompressed \u00B7 Minimal Dynamics" },
			{    6.0,    10.0, MetricColor::Orange,  L"Low Dynamics \u00B7 Compressed" },
			{   10.0,    14.0, MetricColor::Default, L"Balanced Dynamics \u00B7 Typical" },
			{   14.0,    18.0, MetricColor::Green,   L"Good Dynamics \u00B7 Broadcast/Music" },
			{   18.0, HIGHEST, MetricColor::Blue,    L"High Dynamics \u00B7 Dynamic Content" }
		}},
		{ MetricId::PLR_RT, {
			{ LOWEST,     6.0, MetricColor::Red,     L"Overcompressed \u00B7 Minimal Dynamics" },
			{    6.0,    10.0, MetricColor::Orange,  L"Low Dynamics \u00B7 Compressed" },
			{   10.0,    14.0, MetricColor::Default, L"Balanced Dynamics \u00B7 Typical" },
			{   14.0,    18.0, MetricColor::Green,   L"Good Dynamics \u00B7 Broadcast/Music" },
			{   18.0, HIGHEST, MetricColor::Blue,    L"High Dynamics \u00B7 Dynamic Content" }
		}},
		{ MetricId::CREST_FACTOR_RT, {
			{ LOWEST,     8.0, MetricColor::Red,     L"Overcompressed \u00B7 Minimal Dynamics" },
			{    8.0,    12.0, MetricColor::Orange,  L"Low Dynamics \u00B7 Compressed" },
			{   12.0,    14.0, MetricColor::Default, L"Balanced Dynamics \u00B7 Typical" },
			{   14.0,    20.0, MetricColor::Green,   L"Good Dynamics \u00B7 Broadcast/Music" },
			{   20.0, HIGHEST, MetricColor::Blue,    L"High Dynamics \u00B7 Dynamic Content" }
		}},
		{ MetricId::DYNAMIC_RANGE_RT, {
			{ LOWEST,     6.0, MetricColor::Red,     L"Overcompressed \u00B7 Minimal Dynamics" },
			{    6.0,     8.0, MetricColor::Orange,  L"Low Dynamics \u00B7 Compressed" },
			{    8.0,    10.0, MetricColor::Default, L"Balanced Dynamics \u00B7 Typical" },
			{   10.0,    14.0, MetricColor::Green,   L"Good Dynamics \u00B7 Broadcast/Music" },
			{   14.0, HIGHEST, MetricColor::Blue,    L"High Dynamics \u00B7 Dynamic Content" }
		}},
		{ MetricId::PURE_DYNAMICS_RT, {
			{ LOWEST,     2.0, MetricColor::Red,     L"Stable Loudness \u00B7 Minimal Dynamics" },
			{    2.0,     4.0, MetricColor::Orange,  L"Balanced Loudness \u00B7 Moderate Dynamics" },
			{    4.0,     8.0, MetricColor::Default, L"Dynamic Loudness \u00B7 Noticeable Dynamics" },
			{    8.0,    10.0, MetricColor::Green,   L"Variable Loudness \u00B7 High Dynamics" },
			{   10.0, HIGHEST, MetricColor::Blue,    L"Fluctuating Loudness \u00B7 Vivid Dynamics" }
		}},
		{ MetricId::PHASE_CORRELATION_RT, {
			{ -1.0,  -0.3, MetricColor::Red,     L"Phase Issues \u00B7 Poor Imaging" },
			{ -0.3,   0.0, MetricColor::Orange,  L"Narrow Stereo \u00B7 Limited Imaging" },
			{  0.0,   0.5, MetricColor::Default, L"Typical Stereo \u00B7 Standard Imaging" },
			{  0.5,   0.9, MetricColor::Green,   L"Good Stereo Image \u00B7 Wide Imaging" },
			{  0.9,   1.0, MetricColor::Blue,    L"Near Mono \u00B7 Minimal Stereo" }
		}},
		{ MetricId::STEREO_WIDTH_RT, {
			{  0.0,  20.0, MetricColor::Blue,    L"Near Mono \u00B7 Minimal Width" },
			{ 20.0,  40.0, MetricColor::Green,   L"Good Stereo Width \u00B7 Balanced Imaging" },
			{ 40.0,  60.0, MetricColor::Default, L"Typical Stereo Width \u00B7 Standard" },
			{ 60.0,  80.0, MetricColor::Orange,  L"Wide Stereo \u00B7 Broad Imaging" },
			{ 80.0, 100.0, MetricColor::Red,     L"Phase Risk \u00B7 Excessive Width" }
		}}
	};

	for (const auto& [id, ranges] : realTimeMetrics) {
		CreateMetricData({ MetricContext::RealTime, id }, ranges);
	}
}

COLORREF AudioWizardDialog::GetColor(MetricId metricId, double value, bool colorize, bool ignoreDarkMode, MetricContext context) {
	bool darkBg = darkMode && !ignoreDarkMode;
	const COLORREF defaultColor = darkBg ? RGB(240, 240, 240) : RGB(32, 32, 32);

	if (!colorize) return defaultColor;

	MetricKey key{ context, metricId };
	auto it = metricData.find(key);
	if (it == metricData.end()) return defaultColor;

	const auto& ranges = it->second.ranges;
	if (ranges.empty()) return defaultColor;

	for (const auto& range : ranges) {
		if (value >= range.minRange && value <= range.maxRange) {
			return darkBg ? range.lightColor : range.darkColor;
		}
	}

	return defaultColor;
}

CStringW AudioWizardDialog::GetMetricValueFormatted(double value) {
	if (std::isinf(value)) {
		return value < 0 ? L"-INF" : L"+INF";
	}

	CStringW str;
	str.Format(value < 0 ? L"%.1f" : L"+%.1f", value);
	return str;
}

CStringW AudioWizardDialog::GetMetricRangeFormatted(double min, double max, const wchar_t* description) {
	CStringW minStr = GetMetricValueFormatted(min);
	CStringW maxStr = GetMetricValueFormatted(max);

	CStringW result;
	result.Format(L"[%s \u2194 %s] %s\n", minStr.GetString(), maxStr.GetString(), description);
	return result;
}

std::wstring AudioWizardDialog::GetMetricName(MetricId metricId) {
	static const std::unordered_map<MetricId, std::wstring> displayNames = {
		{ MetricId::INDEX_FT, L"#" },
		{ MetricId::ARTIST_FT, L"Artist" },
		{ MetricId::ALBUM_FT, L"Album" },
		{ MetricId::TRACK_FT, L"Track" },
		{ MetricId::DURATION_FT, L"Duration" },
		{ MetricId::YEAR_FT, L"Year" },
		{ MetricId::GENRE_FT, L"Genre" },
		{ MetricId::FORMAT_FT, L"Format" },
		{ MetricId::CHANNELS_FT, L"Channels" },
		{ MetricId::BIT_DEPTH_FT, L"Bit Depth" },
		{ MetricId::BITRATE_FT, L"Bitrate" },
		{ MetricId::SAMPLE_RATE_FT, L"Sample Rate" },
		{ MetricId::M_LUFS_FT, L"M LUFS" },
		{ MetricId::S_LUFS_FT, L"S LUFS" },
		{ MetricId::I_LUFS_FT, L"I LUFS" },
		{ MetricId::RMS_FT, L"RMS" },
		{ MetricId::SP_FT, L"SP" },
		{ MetricId::TP_FT, L"TP" },
		{ MetricId::PSR_FT, L"PSR" },
		{ MetricId::PLR_FT, L"PLR" },
		{ MetricId::CF_FT, L"CF" },
		{ MetricId::LRA_FT, L"LRA" },
		{ MetricId::DR_FT, L"DR" },
		{ MetricId::DR_ALBUM_FT, L"DR-A" },
		{ MetricId::PD_FT, L"PD" },
		{ MetricId::PD_ALBUM_FT, L"PD-A" },
		{ MetricId::MOMENTARY_LUFS_RT, L"Momentary LUFS" },
		{ MetricId::SHORT_TERM_LUFS_RT, L"Short Term LUFS" },
		{ MetricId::RMS_RT, L"RMS" },
		{ MetricId::LEFT_RMS_RT, L"Left RMS" },
		{ MetricId::RIGHT_RMS_RT, L"Right RMS" },
		{ MetricId::LEFT_SAMPLE_PEAK_RT, L"Left Sample Peak" },
		{ MetricId::RIGHT_SAMPLE_PEAK_RT, L"Right Sample Peak" },
		{ MetricId::TRUE_PEAK_RT, L"True Peak" },
		{ MetricId::PSR_RT, L"PSR" },
		{ MetricId::PLR_RT, L"PLR" },
		{ MetricId::CREST_FACTOR_RT, L"Crest Factor" },
		{ MetricId::PURE_DYNAMICS_RT, L"Pure Dynamics" },
		{ MetricId::DYNAMIC_RANGE_RT, L"Dynamic Range" },
		{ MetricId::PHASE_CORRELATION_RT, L"Phase Correlation" },
		{ MetricId::STEREO_WIDTH_RT, L"Stereo Width" },
	};

	auto it = displayNames.find(metricId);
	return it != displayNames.end() ? it->second : L"Unknown Metric";
}

std::wstring AudioWizardDialog::GetMetricFullName(MetricId metricId) {
	static const std::unordered_map<MetricId, std::wstring> fullNames = {
		{ MetricId::M_LUFS_FT, L"Momentary LUFS" },
		{ MetricId::S_LUFS_FT, L"Short Term LUFS" },
		{ MetricId::I_LUFS_FT, L"Integrated LUFS" },
		{ MetricId::RMS_FT, L"Root Mean Square" },
		{ MetricId::SP_FT, L"Sample Peak" },
		{ MetricId::TP_FT, L"True Peak" },
		{ MetricId::PSR_FT, L"Peak to Short Term Loudness Ratio" },
		{ MetricId::PLR_FT, L"Peak to Loudness Ratio" },
		{ MetricId::CF_FT, L"Crest Factor" },
		{ MetricId::LRA_FT, L"Loudness Range" },
		{ MetricId::DR_FT, L"Dynamic Range" },
		{ MetricId::DR_ALBUM_FT, L"Dynamic Range Album" },
		{ MetricId::PD_FT, L"Pure Dynamics" },
		{ MetricId::PD_ALBUM_FT, L"Pure Dynamics Album" },
		{ MetricId::MOMENTARY_LUFS_RT, L"Momentary LUFS" },
		{ MetricId::SHORT_TERM_LUFS_RT, L"Short Term LUFS" },
		{ MetricId::RMS_RT, L"Root Mean Square" },
		{ MetricId::LEFT_RMS_RT, L"Left Channel RMS" },
		{ MetricId::RIGHT_RMS_RT, L"Right Channel RMS" },
		{ MetricId::LEFT_SAMPLE_PEAK_RT, L"Left Channel Sample Peak" },
		{ MetricId::RIGHT_SAMPLE_PEAK_RT, L"Right Channel Sample Peak" },
		{ MetricId::TRUE_PEAK_RT, L"True Peak" },
		{ MetricId::PSR_RT, L"Peak to Short Term Loudness Ratio" },
		{ MetricId::PLR_RT, L"Peak to Loudness Ratio" },
		{ MetricId::CREST_FACTOR_RT, L"Crest Factor" },
		{ MetricId::PURE_DYNAMICS_RT, L"Pure Dynamics" },
		{ MetricId::DYNAMIC_RANGE_RT, L"Dynamic Range" },
		{ MetricId::PHASE_CORRELATION_RT, L"Phase Correlation" },
		{ MetricId::STEREO_WIDTH_RT, L"Stereo Width" },
	};

	auto it = fullNames.find(metricId);
	return it != fullNames.end() ? it->second : L"Unknown Metric";
}

CStringW AudioWizardDialog::GetTooltips(MetricId metricId, MetricContext context) {
	MetricKey key{ context, metricId };
	if (auto it = tooltipCache.find(key); it != tooltipCache.end()) {
		return it->second;
	}

	auto dataIt = metricData.find(key);
	if (dataIt == metricData.end()) return {};

	CStringW desc;
	const auto& data = dataIt->second;
	for (const auto& range : data.ranges) {
		desc += GetMetricRangeFormatted(range.minRange, range.maxRange, range.description.c_str());
	}

	tooltipCache[key] = desc;
	return desc;
}

void AudioWizardDialog::DrawTooltipBg(HDC hdc, const RECT & rc) {
	AWHGraphics::DrawTheRect(hdc, rc, darkMode ? RGB(0, 0, 0) : RGB(255, 255, 255), 0, 0);
}

void AudioWizardDialog::DrawTooltipText(HDC hdc, const RECT& textRc, const std::vector<COLORREF>* rangeColors, HFONT hFont, const CStringW& text) {
	const COLORREF defaultColor = darkMode ? RGB(255, 255, 255) : RGB(0, 0, 0);
	const COLORREF separatorColor = darkMode ? RGB(64, 64, 64) : RGB(192, 192, 192);
	const int lineHeight = AWHText::GetFontHeight(hdc, hFont) + MetricTooltip::LINE_SPACING;

	std::vector<CStringW> lines = AWHString::SplitString(text, L"\n");
	if (lines.empty()) return;

	AWHGraphics::GDISelector fontSel(hdc, hFont);
	SetBkMode(hdc, TRANSPARENT);

	int currentY = textRc.top;
	AWHGraphics::Font largeFont(AWHText::CreateScaledFont(hFont, hdc, 1));
	HFONT fontToUse = largeFont.get() ? largeFont.get() : hFont;

	AWHGraphics::GDISelector headlineFontSel(hdc, fontToUse);
	int headlineHeight = AWHText::GetFontHeight(hdc, fontToUse) + MetricTooltip::LINE_SPACING;
	RECT rcHeadline{ textRc.left, currentY, textRc.right, currentY + headlineHeight };
	AWHGraphics::DrawTheText(hdc, rcHeadline, lines[0], defaultColor, fontToUse, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
	currentY += headlineHeight;

	bool hasSeparator = lines.size() > 1 && lines[1] == L"SEPARATOR";
	if (hasSeparator) {
		AWHGraphics::DrawTheHorizontalLine(hdc, textRc.left, textRc.right, currentY + lineHeight / 2, separatorColor);
		currentY += lineHeight;
	}

	const size_t rangeStart = hasSeparator ? 2 : 1;
	if (rangeStart >= lines.size()) return;

	CSize leftBracket;
	CSize arrow;
	CSize rightBracket;
	AWHText::MeasureText(hdc, MetricTooltip::LEFT_BRACKET, leftBracket);
	AWHText::MeasureText(hdc, MetricTooltip::ARROW, arrow);
	AWHText::MeasureText(hdc, MetricTooltip::RIGHT_BRACKET, rightBracket);

	int maxMinW = 0;
	int maxMaxW = 0;
	for (size_t i = rangeStart; i < lines.size(); ++i) {
		const CStringW& line = lines[i];
		int arrowPos = line.Find(MetricTooltip::ARROW, 1);
		int bracketEnd = line.Find(MetricTooltip::RIGHT_BRACKET);
		if (arrowPos == -1 || bracketEnd == -1 || arrowPos >= bracketEnd) continue;

		CSize sz;
		AWHText::MeasureText(hdc, line.Mid(1, arrowPos - 1).Trim(), sz);
		maxMinW = std::max(maxMinW, static_cast<int>(sz.cx));
		AWHText::MeasureText(hdc, line.Mid(arrowPos + static_cast<int>(wcslen(MetricTooltip::ARROW)), bracketEnd - arrowPos - static_cast<int>(wcslen(MetricTooltip::ARROW))).Trim(), sz);
		maxMaxW = std::max(maxMaxW, static_cast<int>(sz.cx));
	}

	const int baseX = textRc.left;
	const int descX = baseX + leftBracket.cx + maxMinW + arrow.cx + maxMaxW + rightBracket.cx + MetricTooltip::DESC_INDENT;

	for (size_t i = rangeStart, colorIdx = 0; i < lines.size(); ++i, ++colorIdx) {
		const CStringW& line = lines[i];
		int arrowPos = line.Find(MetricTooltip::ARROW, 1);
		int bracketEnd = line.Find(MetricTooltip::RIGHT_BRACKET);
		if (arrowPos == -1 || bracketEnd == -1 || arrowPos >= bracketEnd) continue;

		CStringW minStr = line.Mid(1, arrowPos - 1).Trim();
		CStringW maxStr = line.Mid(arrowPos + static_cast<int>(wcslen(MetricTooltip::ARROW)), bracketEnd - arrowPos - static_cast<int>(wcslen(MetricTooltip::ARROW))).Trim();
		CStringW desc = line.Mid(bracketEnd + 1).Trim();
		COLORREF color = rangeColors && colorIdx < rangeColors->size() ? (*rangeColors)[colorIdx] : defaultColor;

		int minX = baseX + leftBracket.cx;
		int arrowX = minX + maxMinW;
		int maxX = arrowX + arrow.cx;
		int rightBracketX = maxX + maxMaxW;

		RECT rcLeftBracket{ baseX, currentY, baseX + leftBracket.cx, currentY + lineHeight };
		AWHGraphics::DrawTheText(hdc, rcLeftBracket, MetricTooltip::LEFT_BRACKET, color, hFont, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		RECT rcMin{ baseX + leftBracket.cx, currentY, arrowX, currentY + lineHeight };
		AWHGraphics::DrawTheText(hdc, rcMin, minStr, color, hFont, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

		RECT rcArrow{ arrowX, currentY, arrowX + arrow.cx, currentY + lineHeight };
		AWHGraphics::DrawTheText(hdc, rcArrow, MetricTooltip::ARROW, color, hFont, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		RECT rcMax{ maxX, currentY, rightBracketX, currentY + lineHeight };
		AWHGraphics::DrawTheText(hdc, rcMax, maxStr, color, hFont, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);

		RECT rcRightBracket{ rightBracketX, currentY, rightBracketX + rightBracket.cx, currentY + lineHeight };
		AWHGraphics::DrawTheText(hdc, rcRightBracket, MetricTooltip::RIGHT_BRACKET, color, hFont, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		if (!desc.IsEmpty()) {
			RECT rcDesc{ descX, currentY, textRc.right, currentY + lineHeight };
			AWHGraphics::DrawTheText(hdc, rcDesc, desc, defaultColor, hFont, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
		}

		currentY += lineHeight;
	}
}

void AudioWizardDialog::DrawTooltip(HWND hWnd) {
	AWHGraphics::WindowDC winDC(hWnd);
	RECT rc = winDC.GetPaintRect();
	AWHGraphics::MemoryDC memDC(winDC.GetDC(), rc.right, rc.bottom);

	std::unique_ptr<std::vector<COLORREF>> rangeColors(
		static_cast<std::vector<COLORREF>*>(GetProp(hWnd, L"RANGE_COLORS"))
	);

	DrawTooltipBg(memDC.GetDC(), rc);

	RECT textRc = rc;
	InflateRect(&textRc, -20, -20);

	auto hFont = reinterpret_cast<HFONT>(SendMessage(hWnd, WM_GETFONT, 0, 0));
	DrawTooltipText(memDC.GetDC(), textRc, rangeColors.get(), hFont, AWHString::GetWindowTextCStringW(hWnd));

	if (rangeColors) {
		RemoveProp(hWnd, L"RANGE_COLORS");
		RemoveProp(hWnd, L"METRIC_COLORS");
	}

	BitBlt(winDC.GetDC(), 0, 0, rc.right, rc.bottom, memDC.GetDC(), 0, 0, SRCCOPY);
}

void AudioWizardDialog::SetTooltips(CToolTipCtrl& tooltip, HWND hWnd, const std::vector<std::pair<HWND, HWND>>* metrics) {
	if (!::IsWindow(tooltip)) {
		tooltip.Create(hWnd);
		CFont tooltipFont;
		tooltipFont.CreatePointFont(100, L"Segoe UI");
		tooltip.SetFont(tooltipFont.Detach());
		tooltip.SetMaxTipWidth(9999);
		tooltip.Activate(TRUE);
		tooltip.SetDelayTime(TTDT_AUTOPOP, 0x7FFF); // Max value (32767 ms, ~32 seconds) for tooltip visibility
		tooltip.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowSubclass(tooltip, TooltipSubclassProc, 0, 0);
	}

	for (const auto& [labelHwnd, tooltipHwnd] : *metrics) {
		if (!::IsWindow(labelHwnd) || !::IsWindow(tooltipHwnd)) continue;
		TOOLINFO ti = { sizeof(ti) };
		ti.hwnd = hWnd;
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.uId = (UINT_PTR)tooltipHwnd;
		ti.lpszText = LPSTR_TEXTCALLBACK;
		tooltip.AddTool(&ti);
	}
}

void AudioWizardDialog::SetTooltipSize(HWND hWnd, LPARAM lParam) {
	auto* wp = (WINDOWPOS*)lParam;
	if (wp->flags & SWP_NOSIZE) return;

	HDC hdc = GetDC(hWnd);
	auto hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0);
	AWHGraphics::GDISelector fontSel(hdc, hFont);

	// Parse text
	CStringW text = AWHString::GetWindowTextCStringW(hWnd);
	std::vector<CStringW> lines = AWHString::SplitString(text, L"\n");

	int maxWidth = 0;
	int totalHeight = 0;
	const int lineHeight = AWHText::GetFontHeight(hdc, hFont) + MetricTooltip::LINE_SPACING;

	if (!lines.empty()) {
		// Measure headline with larger font
		AWHGraphics::Font largeFont(AWHText::CreateScaledFont(hFont, hdc, 1));
		HFONT fontToUse = largeFont.get() ? largeFont.get() : hFont;
		AWHGraphics::GDISelector headlineFontSel(hdc, fontToUse);

		maxWidth = AWHText::MeasureTextWidth(hdc, lines[0]);
		totalHeight = AWHText::GetFontHeight(hdc, fontToUse) + MetricTooltip::LINE_SPACING;

		// Handle separator and ranges
		bool hasSeparator = lines.size() > 1 && lines[1] == L"SEPARATOR";
		size_t rangeStart = hasSeparator ? 2 : 1;
		if (hasSeparator) totalHeight += lineHeight;

		if (rangeStart < lines.size()) {
			CSize leftBracket;
			CSize arrow;
			CSize rightBracket;
			AWHText::MeasureText(hdc, MetricTooltip::LEFT_BRACKET, leftBracket);
			AWHText::MeasureText(hdc, MetricTooltip::ARROW, arrow);
			AWHText::MeasureText(hdc, MetricTooltip::RIGHT_BRACKET, rightBracket);

			// Calculate max range part widths
			int maxMinW = 0;
			int maxMaxW = 0;
			int maxDescW = 0;
			for (size_t i = rangeStart; i < lines.size(); ++i) {
				const CStringW& line = lines[i];
				const int arrowPos = line.Find(MetricTooltip::ARROW, 1);
				const int bracketEnd = line.Find(MetricTooltip::RIGHT_BRACKET);
				if (arrowPos == -1 || bracketEnd == -1 || arrowPos >= bracketEnd) continue;

				// Measure min part
				CSize sz;
				AWHText::MeasureText(hdc, line.Mid(1, arrowPos - 1).Trim(), sz);
				maxMinW = std::max(maxMinW, static_cast<int>(sz.cx));

				// Measure max part
				int maxLength = bracketEnd - arrowPos - static_cast<int>(wcslen(MetricTooltip::ARROW));
				AWHText::MeasureText(hdc, line.Mid(arrowPos + static_cast<int>(wcslen(MetricTooltip::ARROW)), maxLength).Trim(), sz);
				maxMaxW = std::max(maxMaxW, static_cast<int>(sz.cx));

				// Measure description
				CStringW desc = line.Mid(bracketEnd + 1).Trim();
				if (!desc.IsEmpty()) {
					AWHText::MeasureText(hdc, desc, sz);
					maxDescW = std::max(maxDescW, static_cast<int>(sz.cx));
				}
			}

			// Calculate total range width
			int baseRangeWidth = leftBracket.cx + maxMinW + arrow.cx + maxMaxW + rightBracket.cx;
			int fullRangeWidth = baseRangeWidth + (maxDescW > 0 ? MetricTooltip::DESC_INDENT + maxDescW : 0);
			maxWidth = std::max(maxWidth, fullRangeWidth);
			totalHeight += static_cast<int>((lines.size() - rangeStart) * lineHeight);
		}
	}

	// Apply final size with increased padding
	wp->cx = std::max(maxWidth + 40, 200);
	wp->cy = std::max(totalHeight + 40, 100);

	ReleaseDC(hWnd, hdc);
}

void AudioWizardDialog::CleanTooltips(HWND hWnd) {
	if (auto* rangeColors = static_cast<std::vector<COLORREF>*>(GetProp(hWnd, L"RANGE_COLORS"))) {
		std::unique_ptr<std::vector<COLORREF>> cleanup(rangeColors);
		RemoveProp(hWnd, L"RANGE_COLORS");
	}

	RemoveProp(hWnd, L"METRIC_COLORS");
	RemoveWindowSubclass(hWnd, TooltipSubclassProc, 0);
}

LRESULT CALLBACK AudioWizardDialog::TooltipSubclassProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR, DWORD_PTR) {
	switch (msg) {
		case WM_PAINT: {
			DrawTooltip(hWnd);
			return 0;
		}

		case WM_WINDOWPOSCHANGING: {
			SetTooltipSize(hWnd, lParam);
			break;
		}

		case WM_NCDESTROY: {
			CleanTooltips(hWnd);
			break;
		}
	}

	return DefSubclassProc(hWnd, msg, wParam, lParam);
}
#pragma endregion
