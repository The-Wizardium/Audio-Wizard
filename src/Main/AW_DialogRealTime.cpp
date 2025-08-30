/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Dialog Real-Time Source File               * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_DialogRealTime.h"
#include "AW_Helpers.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardDialogRealTime::~AudioWizardDialogRealTime() {
	DestroyWindow();
}
#pragma endregion


///////////////////////////////////////////
// * REAL-TIME DIALOG - INITIALIZATION * //
///////////////////////////////////////////
#pragma region Real-Time Dialog - Initialization
void AudioWizardDialogRealTime::InitDialog() {
	InitMetrics();

	CRect clientRect;
	GetClientRect(&clientRect);
	UpdateFonts(GetFontSize(clientRect));
	InitTooltips();

	HDWP hdwp = BeginDeferWindowPos(static_cast<int>(metrics.size() * 2));
	SetMetrics(clientRect, hdwp);
	EndDeferWindowPos(hdwp);

	UpdateColors();
	UpdateMetrics();
	Invalidate();
}

void AudioWizardDialogRealTime::InitWindowTitle() {
	if (!AudioWizardSettings::monitorDisplayTitleBarMetrics) {
		SetWindowText(L"Audio Wizard - Monitor");
		return;
	}

	const int precision = AudioWizardSettings::monitorDisplayTitleBarMetricsFraction ? 2 : 0;
	const double cpuFoobar = AWHPerf::GetCpuFoobarUsage();
	const double cpuSystem = AWHPerf::GetCpuSystemUsage();
	const auto [ramFoobarWS, ramFoobarPB] = AWHPerf::GetMemoryFoobarUsage();
	const double ramSystem = AWHPerf::GetMemorySystemUsage();

	CStringW cpuFoobarStr = AWHString::ToFixedW(precision, cpuFoobar);
	CStringW cpuSystemStr = AWHString::ToFixedW(precision, cpuSystem);
	CStringW ramFoobarWSStr = AWHString::ToFixedW(precision, ramFoobarWS);
	CStringW ramFoobarPBStr = AWHString::ToFixedW(precision, ramFoobarPB);
	CStringW ramSystemStr = AWHString::ToFixedW(precision, ramSystem);

	CStringW title;
	title.Format(
		L"Audio Wizard - Monitor - CPU: FB2K %s %%\u2009 \u00B7 SYS %s %%\u2009 | RAM: FB2K WS %s MB \u00B7 PB %s MB \u00B7 SYS %s %%",
		(LPCWSTR)cpuFoobarStr, (LPCWSTR)cpuSystemStr, (LPCWSTR)ramFoobarWSStr, (LPCWSTR)ramFoobarPBStr, (LPCWSTR)ramSystemStr
	);

	SetWindowText(title);
}

void AudioWizardDialogRealTime::InitFonts() {
	CRect clientRect;
	GetClientRect(&clientRect);
	UpdateFonts(GetFontSize(clientRect));
}

void AudioWizardDialogRealTime::InitTooltips() {
	col.metricMap.clear();
	std::vector<std::pair<HWND, HWND>> tooltipControls;

	for (const auto& metric : metrics) {
		if (metric.isDisplayed) {
			tooltipControls.emplace_back(metric.label.m_hWnd, metric.label.m_hWnd);
			col.metricMap[metric.label.m_hWnd] = metric.metricId;

			tooltipControls.emplace_back(metric.label.m_hWnd, metric.value.m_hWnd);
			col.metricMap[metric.value.m_hWnd] = metric.metricId;

			tooltipControls.emplace_back(metric.label.m_hWnd, metric.meterBar.m_hWnd);
			col.metricMap[metric.meterBar.m_hWnd] = metric.metricId;
		}
	}

	AudioWizardDialog::SetTooltips(ui.tooltipCtrl, m_hWnd, &tooltipControls);
}

void AudioWizardDialogRealTime::InitMeterBrushes() {
	brush.meterBgBrush = AWHGraphics::CreateTheSolidBrush(
		AudioWizardDialog::darkMode ? RGB(50, 50, 50) : RGB(255, 255, 255)
	);
	brush.meterFrameBrush = AWHGraphics::CreateTheSolidBrush(
		AudioWizardDialog::darkMode ? RGB(0, 0, 0) : RGB(220, 220, 220)
	);
	brush.meterBrushCache.clear();
}

void AudioWizardDialogRealTime::InitColors() {
	col.metricValueColor = AudioWizardDialog::darkMode ? RGB(240, 240, 240) : RGB(32, 32, 32);
	col.metricValueBgColor = AudioWizardDialog::darkMode ? RGB(32, 32, 32) : RGB(240, 240, 240);
	col.meterBgColor = AudioWizardDialog::darkMode ? RGB(50, 50, 50) : RGB(255, 255, 255);
	col.meterBorderColor = AudioWizardDialog::darkMode ? RGB(0, 0, 0) : RGB(220, 220, 220);
}

void AudioWizardDialogRealTime::InitMetrics() {
	metrics.clear();
	leftMetrics.clear();
	rightMetrics.clear();
	metricControlValueMap.clear();

	const std::array<MetricConfig, 14> metricControls = {{
		// Left Column
		{ MOMENTARY_LUFS_RT,    IDC_MONITOR_MOMENTARY_LUFS_LABEL,    IDC_MONITOR_MOMENTARY_LUFS_VALUE,    IDC_MONITOR_MOMENTARY_LUFS_METER,    AudioWizardSettings::monitorDisplayMomentaryLUFS,    true },
		{ LEFT_RMS_RT,          IDC_MONITOR_LEFT_RMS_LABEL,          IDC_MONITOR_LEFT_RMS_VALUE,          IDC_MONITOR_LEFT_RMS_METER,          AudioWizardSettings::monitorDisplayLeftRMS,          true },
		{ LEFT_SAMPLE_PEAK_RT,  IDC_MONITOR_LEFT_SAMPLE_PEAK_LABEL,  IDC_MONITOR_LEFT_SAMPLE_PEAK_VALUE,  IDC_MONITOR_LEFT_SAMPLE_PEAK_METER,  AudioWizardSettings::monitorDisplayLeftSamplePeak,   true },
		{ TRUE_PEAK_RT,         IDC_MONITOR_TRUE_PEAK_LABEL,         IDC_MONITOR_TRUE_PEAK_VALUE,         IDC_MONITOR_TRUE_PEAK_METER,         AudioWizardSettings::monitorDisplayTruePeak,         true },
		{ CREST_FACTOR_RT,      IDC_MONITOR_CREST_FACTOR_LABEL,      IDC_MONITOR_CREST_FACTOR_VALUE,      IDC_MONITOR_CREST_FACTOR_METER,      AudioWizardSettings::monitorDisplayCrestFactor,      true },
		{ DYNAMIC_RANGE_RT,     IDC_MONITOR_DYNAMIC_RANGE_LABEL,     IDC_MONITOR_DYNAMIC_RANGE_VALUE,     IDC_MONITOR_DYNAMIC_RANGE_METER,     AudioWizardSettings::monitorDisplayDynamicRange,     true },
		{ PHASE_CORRELATION_RT, IDC_MONITOR_PHASE_CORRELATION_LABEL, IDC_MONITOR_PHASE_CORRELATION_VALUE, IDC_MONITOR_PHASE_CORRELATION_METER, AudioWizardSettings::monitorDisplayPhaseCorrelation, true },

		// Right Column
		{ SHORT_TERM_LUFS_RT,   IDC_MONITOR_SHORT_TERM_LUFS_LABEL,   IDC_MONITOR_SHORT_TERM_LUFS_VALUE,   IDC_MONITOR_SHORT_TERM_LUFS_METER,   AudioWizardSettings::monitorDisplayShortTermLUFS,   false },
		{ RIGHT_RMS_RT,         IDC_MONITOR_RIGHT_RMS_LABEL,         IDC_MONITOR_RIGHT_RMS_VALUE,         IDC_MONITOR_RIGHT_RMS_METER,         AudioWizardSettings::monitorDisplayRightRMS,        false },
		{ RIGHT_SAMPLE_PEAK_RT, IDC_MONITOR_RIGHT_SAMPLE_PEAK_LABEL, IDC_MONITOR_RIGHT_SAMPLE_PEAK_VALUE, IDC_MONITOR_RIGHT_SAMPLE_PEAK_METER, AudioWizardSettings::monitorDisplayRightSamplePeak, false },
		{ PSR_RT,               IDC_MONITOR_PSR_LABEL,               IDC_MONITOR_PSR_VALUE,               IDC_MONITOR_PSR_METER,               AudioWizardSettings::monitorDisplayPSR,             false },
		{ PLR_RT,               IDC_MONITOR_PLR_LABEL,               IDC_MONITOR_PLR_VALUE,               IDC_MONITOR_PLR_METER,               AudioWizardSettings::monitorDisplayPLR,             false },
		{ PURE_DYNAMICS_RT,     IDC_MONITOR_PURE_DYNAMICS_LABEL,     IDC_MONITOR_PURE_DYNAMICS_VALUE,     IDC_MONITOR_PURE_DYNAMICS_METER,     AudioWizardSettings::monitorDisplayPureDynamics,    false },
		{ STEREO_WIDTH_RT,      IDC_MONITOR_STEREO_WIDTH_LABEL,      IDC_MONITOR_STEREO_WIDTH_VALUE,      IDC_MONITOR_STEREO_WIDTH_METER,      AudioWizardSettings::monitorDisplayStereoWidth,     false },
	}};

	for (const auto& config : metricControls) {
		MetricControl metric;
		metric.metricId = config.metricId;
		metric.label.Attach(GetDlgItem(config.labelID));
		metric.value.Attach(GetDlgItem(config.valueID));
		metric.meterBar.Attach(GetDlgItem(config.meterBarID));
		metric.currentColor = RGB(1, 2, 3); // Force initial color update
		metric.currentValue = 0.0;
		metric.previousValue = 0.0;
		metric.isDisplayed = config.setting;
		metric.colorChanged = true;
		metric.targetValue = 0.0;
		metric.animatedValue = 0.0;
		metric.animationStartTime = 0;
		metric.isAnimating = false;

		// Cache display range
		auto [minVal, maxVal] = GetDisplayRange(config.metricId);
		metric.minVal = minVal;
		metric.maxVal = maxVal;

		if (config.setting) {
			(config.isLeft ? leftMetrics : rightMetrics).push_back(metric);
			metric.label.ShowWindow(SW_SHOW);

			switch (static_cast<MetricsMode>(static_cast<int>(AudioWizardSettings::monitorDisplayMetricsMode))) {
				case MetricsMode::Value: {
					metric.value.ShowWindow(SW_SHOW);
					metric.meterBar.ShowWindow(SW_HIDE);
					break;
				}
				case MetricsMode::Meter: {
					metric.value.ShowWindow(SW_HIDE);
					metric.meterBar.ShowWindow(SW_SHOW);
					break;
				}
				case MetricsMode::ValueAndMeter: {
					metric.value.ShowWindow(SW_SHOW);
					metric.meterBar.ShowWindow(SW_SHOW);
					break;
				}
			}
		}
		else {
			metric.label.ShowWindow(SW_HIDE);
			metric.value.ShowWindow(SW_HIDE);
			metric.meterBar.ShowWindow(SW_HIDE);
		}

		size_t index = metrics.size();
		metrics.push_back(metric);
		metricControlValueMap[metric.value.m_hWnd] = index;
		metricControlValueMap[metric.meterBar.m_hWnd] = index;
	}
}

void AudioWizardDialogRealTime::InitButtons() {
	ui.openAnalysisBtn.Attach(GetDlgItem(IDC_MONITOR_OPEN_ANALYSIS_BUTTON));
	ui.configBtn.Attach(GetDlgItem(IDC_MONITOR_CONFIG_BUTTON));
	ui.toggleLogBtn.Attach(GetDlgItem(IDC_MONITOR_TOGGLE_LOG_BUTTON));
	ui.closeBtn.Attach(GetDlgItem(IDC_MONITOR_CLOSE_BUTTON));

	ui.openAnalysisBtn.GetWindowRect(ui.originalOpenAnalysisRect);
	ScreenToClient(&ui.originalOpenAnalysisRect);
	ui.configBtn.GetWindowRect(ui.originalConfigRect);
	ScreenToClient(&ui.originalConfigRect);
	ui.toggleLogBtn.GetWindowRect(ui.originalToggleLogRect);
	ScreenToClient(&ui.originalToggleLogRect);
	ui.closeBtn.GetWindowRect(ui.originalCloseRect);
	ScreenToClient(&ui.originalCloseRect);
}

void AudioWizardDialogRealTime::InitTimers() {
	dlgTitle.monitorTimerId = SetTimer(1, AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs);
	dlgTitle.titleTimerId = SetTimer(2, 1000); // Update every second like task manager
}

void AudioWizardDialogRealTime::InitWindowSize() {
	const int group1Width = ui.originalOpenAnalysisRect.Width();
	const int group2Width = ui.originalConfigRect.Width() + ui.originalToggleLogRect.Width();
	const int group3Width = ui.originalCloseRect.Width();

	const int totalGapMargin = 7 * BUTTON_MARGIN;
	const int minWidth = group1Width + group2Width + group3Width + totalGapMargin;

	auto rows = static_cast<int>((metrics.size() + 1) / 2);
	int rowHeight = ui.labelHeight + BUTTON_MARGIN * 2;
	const int buttonHeight = ui.originalOpenAnalysisRect.Height();
	const int minHeight = rows * rowHeight + buttonHeight + BUTTON_MARGIN;

	ui.minSize = CSize(minWidth, minHeight);
}
#pragma endregion


////////////////////////////////////
// * REAL-TIME DIALOG - METHODS * //
////////////////////////////////////
#pragma region Real-Time Dialog - Methods
void AudioWizardDialogRealTime::DrawMeterBar(DRAWITEMSTRUCT* pDrawItem, double) {
	HDC hdc = pDrawItem->hDC;
	RECT rc = pDrawItem->rcItem;
	int width = rc.right - rc.left;

	AWHGraphics::DrawTheRect(hdc, rc, col.meterBgColor, col.meterBorderColor, 1);

	auto it = metricControlValueMap.find(pDrawItem->hwndItem);
	if (it == metricControlValueMap.end() || !playback_control::get()->is_playing()) return;
	auto& metric = metrics[it->second];

	// Calculate interpolated value if animating
	double valueToDraw = metric.currentValue;
	if (metric.isAnimating) {
		const ULONGLONG timeCurrent = GetTickCount64();
		const ULONGLONG timeElapsed = timeCurrent - metric.animationStartTime;
		const ULONGLONG animationDurationMs = GetMeterBarAnimationDuration();

		if (timeElapsed >= animationDurationMs) {
			metric.animatedValue = metric.targetValue;
			metric.isAnimating = false;
		}
		else {
			double animationProgress = std::clamp(static_cast<double>(timeElapsed) / static_cast<double>(animationDurationMs), 0.0, 1.0);
			animationProgress = AWHMath::CalculateEaseOutCubic(animationProgress); // Apply easing
			metric.animatedValue = metric.currentValue + (metric.targetValue - metric.currentValue) * animationProgress;
		}
		valueToDraw = metric.animatedValue;
	}

	double clampedValue = std::max(metric.minVal, std::min(metric.maxVal, valueToDraw));
	double percentage = (clampedValue - metric.minVal) / (metric.maxVal - metric.minVal);
	if (percentage <= 0) return;

	auto fillWidth = static_cast<int>(percentage * width);
	RECT fillRect = { rc.left, rc.top, rc.left + fillWidth, rc.bottom };
	AWHGraphics::DrawTheRect(hdc, fillRect, metric.currentColor, 0, 0);
}

ULONGLONG AudioWizardDialogRealTime::GetMeterBarAnimationDuration() const {
	constexpr int TARGET_FRAMES = 7;
	return AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs.load() * TARGET_FRAMES;
}

double AudioWizardDialogRealTime::GetMeterBarUpdateThreshold() const {
	return AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs.load() <= 33 ? 0.002 : 0.005;
}

double AudioWizardDialogRealTime::GetMetricValueUpdateThreshold() const {
	int refreshRateMs = AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs.load();
	if (refreshRateMs <= 33) return 0.005; // High refresh rate
	if (refreshRateMs <= 66) return 0.01;  // Standard refresh rate
	return 0.02;                           // Low refresh rate
}

std::pair<double, double> AudioWizardDialogRealTime::GetDisplayRange(MetricId metricId) const {
	static const std::unordered_map<MetricId, std::pair<double, double>> displayRanges = {
		{ MOMENTARY_LUFS_RT,    { -70.0,   0.0 }},
		{ SHORT_TERM_LUFS_RT,   { -70.0,   0.0 }},
		{ LEFT_RMS_RT,          { -70.0,   0.0 }},
		{ RIGHT_RMS_RT,         { -70.0,   0.0 }},
		{ LEFT_SAMPLE_PEAK_RT,  { -70.0,   3.0 }},
		{ RIGHT_SAMPLE_PEAK_RT, { -70.0,   3.0 }},
		{ TRUE_PEAK_RT,         { -70.0,   3.0 }},
		{ PSR_RT,               {   0.0,  20.0 }},
		{ PLR_RT,               {   0.0,  20.0 }},
		{ CREST_FACTOR_RT,      {   0.0,  20.0 }},
		{ PURE_DYNAMICS_RT,     {   0.0,  20.0 }},
		{ DYNAMIC_RANGE_RT,     {   0.0,  20.0 }},
		{ PHASE_CORRELATION_RT, {  -1.0,   1.0 }},
		{ STEREO_WIDTH_RT,      {   0.0, 100.0 }},
	};

	auto it = displayRanges.find(metricId);
	return it != displayRanges.end() ? it->second : std::pair{ 0.0, 1.0 };
}

int AudioWizardDialogRealTime::GetFontSize(const CRect& clientRect) const {
	const int baseSize = clientRect.Height() / 28;
	return std::clamp(baseSize, 10, 36);
}

void AudioWizardDialogRealTime::SetButtonAnalysisPosition(HDWP hdwp, int groupX, int groupY) const {
	CRect rect(groupX, groupY,
		groupX + ui.originalOpenAnalysisRect.Width(),
		groupY + ui.originalOpenAnalysisRect.Height()
	);

	::DeferWindowPos(hdwp, ui.openAnalysisBtn, nullptr,
		rect.left, rect.top, rect.Width(), rect.Height(),
		SWP_NOZORDER | SWP_NOACTIVATE
	);
}

void AudioWizardDialogRealTime::SetButtonLogPosition(HDWP hdwp, int groupX, int groupY) const {
	int currentX = groupX;

	CRect rect(currentX, groupY, currentX + ui.originalConfigRect.Width(), groupY + ui.originalConfigRect.Height());
	::DeferWindowPos(hdwp, ui.configBtn, nullptr, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	currentX += ui.originalConfigRect.Width() + BUTTON_MARGIN;

	rect = CRect(currentX, groupY, currentX + ui.originalToggleLogRect.Width(), groupY + ui.originalToggleLogRect.Height());
	::DeferWindowPos(hdwp, ui.toggleLogBtn, nullptr, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
}

void AudioWizardDialogRealTime::SetButtonClosePosition(HDWP hdwp, int groupX, int groupY) const {
	CRect rect(groupX, groupY,
		groupX + ui.originalCloseRect.Width(),
		groupY + ui.originalCloseRect.Height()
	);

	::DeferWindowPos(hdwp, ui.closeBtn, nullptr,
		rect.left, rect.top, rect.Width(), rect.Height(),
		SWP_NOZORDER | SWP_NOACTIVATE
	);
}

void AudioWizardDialogRealTime::SetMetrics(const CRect& clientRect, HDWP hdwp) const {
	// Precompute client dimensions
	const int clientWidth = clientRect.Width();
	const int clientHeight = clientRect.Height();

	// Calculate spacing values
	const int margin = AWHConvert::PercentToPixels(0.05, clientWidth);
	const int columnSpacing = AWHConvert::PercentToPixels(0.05, clientWidth);
	const int labelValueSpacing = AWHConvert::PercentToPixels(0.02, clientWidth);
	const int rowSpacing = AWHConvert::PercentToPixels(0.02, clientHeight);

	// Calculate column dimensions
	const int columnWidth = (clientWidth - 2 * margin - columnSpacing) / 2;

	// Calculate meter dimensions
	const bool isMeterMode1 = AudioWizardSettings::monitorDisplayMetricsMode == 1;
	const int meterBarHeight = isMeterMode1
		? static_cast<int>(ui.valueHeight * 0.5)
		: static_cast<int>(ui.valueHeight * 0.25);

	// Calculate max label width
	CClientDC dc(*this);
	CFontHandle oldFont = dc.SelectFont(font.labelFont.get());
	int maxLabelWidth = 0;

	for (const auto* metricSet : { &leftMetrics, &rightMetrics }) {
		for (const auto& metric : *metricSet) {
			CStringW text;
			metric.label.GetWindowText(text);
			CSize sz;
			dc.GetTextExtent(text, text.GetLength(), &sz);
			maxLabelWidth = std::max(static_cast<LONG>(maxLabelWidth), sz.cx);
		}
	}
	dc.SelectFont(oldFont);

	// Calculate label and value widths
	const int labelWidth = std::min(static_cast<int>(maxLabelWidth * 1.1), columnWidth - labelValueSpacing - 40);
	const int valueWidth = columnWidth - labelWidth - labelValueSpacing;

	// Calculate row height based on display mode
	const int valueMeterSpacing = 5;
	const auto fixedMeterHeight = static_cast<int>(ui.valueHeight * 0.25);
	int rowHeight = ui.valueHeight + valueMeterSpacing + fixedMeterHeight;

	// Positioning lambda
	auto PositionColumn = [&](const std::vector<MetricControl>& columnMetrics, int x) {
		int y = margin;
		for (const auto& metric : columnMetrics) {
			// Position label
			::DeferWindowPos(hdwp, metric.label.m_hWnd, nullptr,
				x, y + (ui.valueHeight - ui.labelHeight) / 2,
				labelWidth, ui.labelHeight,
				SWP_NOZORDER | SWP_NOACTIVATE);

			// Position value/meter components
			const int valueX = x + labelWidth + labelValueSpacing;
			int contentY = y;

			switch (AudioWizardSettings::monitorDisplayMetricsMode) {
				case 0: { // Metrics value mode
					::DeferWindowPos(hdwp, metric.value.m_hWnd, nullptr,
						valueX, contentY, valueWidth, ui.valueHeight,
						SWP_NOZORDER | SWP_NOACTIVATE);
					break;
				}
				case 1: { // Meter bar only mode
					const int labelBaselineY = y + (ui.valueHeight + ui.labelHeight) / 2;
					::DeferWindowPos(hdwp, metric.meterBar.m_hWnd, nullptr,
						valueX, labelBaselineY - static_cast<int>(meterBarHeight * 1.5),
						valueWidth, meterBarHeight,
						SWP_NOZORDER | SWP_NOACTIVATE);
					break;
				}
				case 2: { // Metrics value and meter bar mode
					::DeferWindowPos(hdwp, metric.value.m_hWnd, nullptr,
						valueX, contentY, valueWidth, ui.valueHeight,
						SWP_NOZORDER | SWP_NOACTIVATE);
					::DeferWindowPos(hdwp, metric.meterBar.m_hWnd, nullptr,
						valueX, contentY + ui.valueHeight + valueMeterSpacing,
						valueWidth, meterBarHeight,
						SWP_NOZORDER | SWP_NOACTIVATE);
					break;
				}
			}
			y += rowHeight + rowSpacing;
		}
	};

	// Position columns
	PositionColumn(leftMetrics, margin);
	PositionColumn(rightMetrics, margin + columnWidth + columnSpacing);
}

void AudioWizardDialogRealTime::UpdateMetrics() {
	if (!AudioWizard::Main()) return;

	struct MetricGetter {
		void (AudioWizardMain::* getter)(double*) const;
		double valueThresholdMultiplier;
		double meterThresholdMultiplier;
	};

	static const std::unordered_map<MetricId, MetricGetter> metricGetters = {
		{ MOMENTARY_LUFS_RT,    { &AudioWizardMain::GetMomentaryLUFS,    1.0, 1.0 }},
		{ SHORT_TERM_LUFS_RT,   { &AudioWizardMain::GetShortTermLUFS,    1.0, 1.0 }},
		{ LEFT_RMS_RT,          { &AudioWizardMain::GetLeftRMS,          1.0, 1.0 }},
		{ RIGHT_RMS_RT,         { &AudioWizardMain::GetRightRMS,         1.0, 1.0 }},
		{ LEFT_SAMPLE_PEAK_RT,  { &AudioWizardMain::GetLeftSamplePeak,   1.0, 1.0 }},
		{ RIGHT_SAMPLE_PEAK_RT, { &AudioWizardMain::GetRightSamplePeak,  1.0, 1.0 }},
		{ TRUE_PEAK_RT,         { &AudioWizardMain::GetTruePeak,         1.0, 1.0 }},
		{ PSR_RT,               { &AudioWizardMain::GetPSR,              2.0, 2.0 }},
		{ PLR_RT,               { &AudioWizardMain::GetPLR,              2.0, 2.0 }},
		{ CREST_FACTOR_RT,      { &AudioWizardMain::GetCrestFactor,      1.0, 1.0 }},
		{ DYNAMIC_RANGE_RT,     { &AudioWizardMain::GetDynamicRange,     1.0, 1.0 }},
		{ PURE_DYNAMICS_RT,     { &AudioWizardMain::GetPureDynamics,     1.0, 1.0 }},
		{ PHASE_CORRELATION_RT, { &AudioWizardMain::GetPhaseCorrelation, 8.0, 8.0 }},
		{ STEREO_WIDTH_RT,      { &AudioWizardMain::GetStereoWidth,      8.0, 8.0 }},
	};

	const int mode = AudioWizardSettings::monitorDisplayMetricsMode;
	const bool updateValue = (mode != 1);
	const bool updateMeter = (mode != 0);
	const double metricValueThresholdBase = GetMetricValueUpdateThreshold();
	const double meterThresholdBase = GetMeterBarUpdateThreshold();

	for (auto& metric : metrics) {
		if (!metric.isDisplayed) continue;

		auto it = metricGetters.find(metric.metricId);
		if (it == metricGetters.end()) continue;

		const auto& getter = it->second;
		double val = 0.0;
		(AudioWizard::Main()->*getter.getter)(&val);

		const double metricValueThreshold = metricValueThresholdBase * getter.valueThresholdMultiplier;
		if (std::abs(val - metric.previousValue) < metricValueThreshold) {
			metric.currentValue = val;
			continue;
		}

		// Update metric state
		metric.previousValue = metric.currentValue;
		metric.currentValue = val;
		metric.targetValue = val;
		metric.animatedValue = val;
		metric.animationStartTime = GetTickCount64();
		metric.isAnimating = true;

		// Update displayed value
		if (updateValue) {
			wchar_t textBuffer[16];
			swprintf_s(textBuffer, L"%+.1f", val);
			metric.value.SetWindowTextW(textBuffer);
		}

		// Update meter bar
		if (updateMeter) {
			const double range = metric.maxVal - metric.minVal;
			const double meterThreshold = meterThresholdBase * getter.meterThresholdMultiplier;
			if (range != 0.0 && (metric.colorChanged || std::abs((val - metric.previousValue) / range) >= meterThreshold)) {
				metric.meterBar.Invalidate();
			}
		}
	}
}

void AudioWizardDialogRealTime::UpdateColors() {
	const bool useColors = AudioWizardSettings::monitorDisplayColors;
	const int mode = AudioWizardSettings::monitorDisplayMetricsMode;
	const bool updateValue = mode != 1;
	const bool updateMeter = mode != 0;

	for (auto& metric : metrics) {
		if (!metric.isDisplayed) continue;

		COLORREF newColor = AudioWizardDialog::GetColor(
			metric.metricId, metric.currentValue, useColors, false, AudioWizardDialog::MetricContext::RealTime
		);

		if (newColor != metric.currentColor) {
			metric.currentColor = newColor;
			metric.colorChanged = true;
			col.colorMap[metric.value.m_hWnd] = newColor;

			if (updateValue) metric.value.Invalidate();
			if (updateMeter) metric.meterBar.Invalidate();
		}
		else {
			metric.colorChanged = false;
		}
	}
}

void AudioWizardDialogRealTime::UpdateFonts(int fontSize) {
	font.labelFont = AWHGraphics::CreateTheFont(
		-fontSize, 0, L"Segoe UI", FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET
	);
	font.valueFont = AWHGraphics::CreateTheFont(
		-fontSize, 0, L"Segoe UI", FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET
	);

	// Update font metrics
	CClientDC dc(*this);
	AWHGraphics::GDISelector labelFontSel(dc, font.labelFont.get());
	ui.labelHeight = AWHText::GetFontHeight(dc, font.labelFont.get()) + 8;

	AWHGraphics::GDISelector valueFontSel(dc, font.valueFont.get());
	ui.valueHeight = AWHText::GetFontHeight(dc, font.valueFont.get()) + 8;

	// Apply fonts
	for (auto& metric : metrics) {
		metric.label.SetFont(font.labelFont.get());
		metric.value.SetFont(font.valueFont.get());
	}
}

void AudioWizardDialogRealTime::UpdateWindowLayout(HDWP hdwp) const {
	CRect clientRect;
	GetClientRect(&clientRect);
	const int groupY = clientRect.bottom - ui.originalOpenAnalysisRect.Height() - BUTTON_MARGIN;

	// Group 1
	SetButtonAnalysisPosition(hdwp, BUTTON_MARGIN, groupY);

	// Group 3
	const int group3X = clientRect.right - ui.originalCloseRect.Width() - BUTTON_MARGIN;
	SetButtonClosePosition(hdwp, group3X, groupY);

	// Group 2
	const int group1Right = BUTTON_MARGIN + ui.originalOpenAnalysisRect.Width();
	const int group2Width = ui.originalConfigRect.Width() + ui.originalToggleLogRect.Width() + BUTTON_MARGIN;
	const int availableWidth = group3X - group1Right;
	const int gap = std::max(BUTTON_MARGIN, (availableWidth - group2Width) / 2);
	const int group2X = group1Right + gap;
	SetButtonLogPosition(hdwp, group2X, groupY);
}

void AudioWizardDialogRealTime::StartLogging() {
	if (!AudioWizard::Main() || !log.isLogging) return;

	metadb_handle_ptr currentTrack;
	if (!playback_control::get()->get_now_playing(currentTrack) || currentTrack != log.initialTrack) {
		StopLogging();
		return;
	}

	constexpr size_t MAX_LOG_MEMORY_BYTES = 1'073'741'824; // 1GB
	constexpr size_t ENTRY_SIZE = sizeof(LogEntry); // ~112 bytes
	const size_t maxEntries = MAX_LOG_MEMORY_BYTES / ENTRY_SIZE; // ~524,288

	if (log.logBackup.size() >= maxEntries) {
		StopLogging();
		MessageBox(L"Reached maximum log size (1GB)", L"Logging Stopped", MB_ICONWARNING);
		return;
	}

	LogEntry entry = {};
	ULONGLONG currentTimestamp = GetTickCount64() - log.logStartTime;
	if (log.lastTimestamp && currentTimestamp == log.lastTimestamp) return; // Skip duplicates
	entry.timestamp = static_cast<double>(currentTimestamp);
	entry.duration = log.lastTimestamp ? static_cast<double>(currentTimestamp - log.lastTimestamp) / 1000.0 :
		static_cast<double>(AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs) / 1000.0;
	log.lastTimestamp = currentTimestamp;

	AudioWizard::Main()->GetMomentaryLUFS(&entry.momentaryLUFS);
	AudioWizard::Main()->GetShortTermLUFS(&entry.shortTermLUFS);
	AudioWizard::Main()->GetLeftRMS(&entry.leftRMS);
	AudioWizard::Main()->GetRightRMS(&entry.rightRMS);
	AudioWizard::Main()->GetLeftSamplePeak(&entry.leftSamplePeak);
	AudioWizard::Main()->GetRightSamplePeak(&entry.rightSamplePeak);
	AudioWizard::Main()->GetTruePeak(&entry.truePeak);
	AudioWizard::Main()->GetRMS(&entry.RMS);
	AudioWizard::Main()->GetPSR(&entry.PSR);
	AudioWizard::Main()->GetPLR(&entry.PLR);
	AudioWizard::Main()->GetCrestFactor(&entry.crestFactor);
	AudioWizard::Main()->GetDynamicRange(&entry.dynamicRange);
	AudioWizard::Main()->GetPureDynamics(&entry.pureDynamics);
	AudioWizard::Main()->GetPhaseCorrelation(&entry.phaseCorrelation);
	AudioWizard::Main()->GetStereoWidth(&entry.stereoWidth);

	if (!log.logBuffer.write(&entry, 1)) {
		FB2K_console_formatter() << "AudioWizard => RingBuffer write failed: Buffer overflow";
		StopLogging();
		return;
	}

	try {
		log.logBackup.push_back(entry);
	}
	catch (const std::bad_alloc&) {
		StopLogging();
		MessageBox(L"Failed to continue logging: Out of memory.", L"Error", MB_ICONERROR);
	}
}

void AudioWizardDialogRealTime::StopLogging() {
	if (!log.isLogging) return;
	log.isLogging = false;
	ui.toggleLogBtn.SetWindowText(L"Start Log");

	if (!log.logBackup.empty()) {
		int response = MessageBox(L"Save the log?", L"Logging Stopped", MB_YESNO | MB_ICONQUESTION);
		if (response == IDYES) SaveLog();
	}

	log.logBuffer.clear();
	log.logBackup.clear();
	log.initialTrack.reset();
	log.trackTitle.Empty();
	log.logStartTime = 0;
}

void AudioWizardDialogRealTime::UpdateLogging() {
	if (!log.isLogging) return;

	metadb_handle_ptr currentTrack;
	bool hasTrack = playback_control::get()->get_now_playing(currentTrack);

	if (!hasTrack || currentTrack != log.initialTrack) {
		StopLogging();
		return;
	}

	StartLogging();
}

std::vector<AudioWizardDialogRealTime::LogColumnDefinition> AudioWizardDialogRealTime::GetLogColumnDefinitions() const {
	auto FormatTimestamp = [](const LogEntry& e) {
		return CStringA("TS: ") + AWHString::FormatTimestampMs(e.timestamp);
	};

	auto FormatNumber = [](const char* abbrev, auto member) {
		return [abbrev, member](const LogEntry& e) {
			int precision = (strcmp(abbrev, "DUR") == 0) ? 3 : 1;
			int width = (strcmp(abbrev, "DUR") == 0) ? 3 : 2;
			return CStringA(abbrev) + ": " + pfc::format_float(e.*member, width, precision).c_str();
		};
	};

	return {
		 { INVALID,               "Timestamp",         "TS",     FormatTimestamp },
		 { INVALID,               "Duration",          "DUR",    FormatNumber("DUR",    &LogEntry::duration)},
		 { MOMENTARY_LUFS_RT,     "Momentary LUFS",    "M LUFS", FormatNumber("M LUFS", &LogEntry::momentaryLUFS)},
		 { SHORT_TERM_LUFS_RT,    "Short Term LUFS",   "S LUFS", FormatNumber("S LUFS", &LogEntry::shortTermLUFS)},
		 { LEFT_RMS_RT,           "Left RMS",          "L RMS",  FormatNumber("L RMS",  &LogEntry::leftRMS)},
		 { RIGHT_RMS_RT,          "Right RMS",         "R RMS",  FormatNumber("R RMS",  &LogEntry::rightRMS)},
		 { LEFT_SAMPLE_PEAK_RT,   "Left Sample Peak",  "L SP",   FormatNumber("L SP",   &LogEntry::leftSamplePeak)},
		 { RIGHT_SAMPLE_PEAK_RT,  "Right Sample Peak", "R SP",   FormatNumber("R SP",   &LogEntry::rightSamplePeak)},
		 { TRUE_PEAK_RT,          "True Peak",         "TP",     FormatNumber("TP",     &LogEntry::truePeak)},
		 { PSR_RT,                "PSR",               "PSR",    FormatNumber("PSR",    &LogEntry::PSR)},
		 { PLR_RT,                "PLR",               "PLR",    FormatNumber("PLR",    &LogEntry::PLR)},
		 { CREST_FACTOR_RT,       "Crest Factor",      "CF",     FormatNumber("CF",     &LogEntry::crestFactor)},
		 { DYNAMIC_RANGE_RT,      "Dynamic Range",     "DR",     FormatNumber("DR",     &LogEntry::dynamicRange)},
		 { PURE_DYNAMICS_RT,      "Pure Dynamics",     "PD",     FormatNumber("PD",     &LogEntry::pureDynamics)},
		 { PHASE_CORRELATION_RT,  "Phase Correlation", "PC",     FormatNumber("PC",     &LogEntry::phaseCorrelation)},
		 { STEREO_WIDTH_RT,       "Stereo Width",      "SW",     FormatNumber("SW",     &LogEntry::stereoWidth)},
	};
}

CStringA AudioWizardDialogRealTime::WriteLogCsv() const {
	if (log.logBackup.empty()) return "";

	CStringA content = AWHText::WriteFancyHeader(
		"AUDIO WIZARD - REAL TIME LOG - ", CStringA(static_cast<const char*>(CW2A(CA2W(
			AWHString::FormatDate(std::chrono::system_clock::now()).c_str(), CP_UTF8), CP_UTF8
		)))
	);

	// Track Metadata (as comments)
	if (log.initialTrack.is_valid()) {
		file_info_impl info;
		if (log.initialTrack->get_info(info)) {
			auto getMeta = [&info](const char* field) { return info.meta_get(field, 0) ? info.meta_get(field, 0) : "Unknown"; };
			content.AppendFormat("# Artist: %s\r\n# Album: %s\r\n# Track: %s\r\n\r\n",
				getMeta("artist"), getMeta("album"),
				log.trackTitle.IsEmpty() ? getMeta("title") : CW2A(log.trackTitle, CP_UTF8));
		}
	}

	// Get displayed columns
	auto allColumns = GetLogColumnDefinitions();
	std::vector<LogColumnDefinition> columns;
	for (const auto& column : allColumns) {
		if (column.metricId == INVALID) {
			columns.push_back(column);
			continue;
		}
		for (const auto& metric : metrics) {
			if (metric.isDisplayed && metric.metricId == column.metricId) {
				columns.push_back(column);
				break;
			}
		}
	}

	// CSV Header
	CStringA headerLine;
	for (size_t i = 0; i < columns.size(); ++i) {
		CStringA escapedHeader = columns[i].header;
		escapedHeader.Replace("\"", "\"\"");
		headerLine.AppendFormat("\"%s\"%s", escapedHeader.GetString(), i < columns.size() - 1 ? "," : "\r\n");
	}
	content += headerLine;

	// CSV Data
	for (const auto& entry : log.logBackup) {
		CStringA line;
		for (size_t i = 0; i < columns.size(); ++i) {
			CStringA cell = columns[i].formatter(entry);
			int pos = cell.Find(':');
			if (pos != -1) cell = cell.Mid(pos + 2);
			bool isNumeric = (columns[i].metricId != INVALID);
			line.AppendFormat(isNumeric ? "%s" : "\"%s\"", cell.GetString());
			if (i < columns.size() - 1) line.Append(",");
		}
		content += line + "\r\n";
	}

	return content;
}

CStringA AudioWizardDialogRealTime::WriteLogTxt() const {
	if (log.logBackup.empty()) return "";

	CStringA content = AWHText::WriteFancyHeader(
		"AUDIO WIZARD - REAL TIME LOG - ", CStringA(static_cast<const char*>(CW2A(CA2W(
			AWHString::FormatDate(std::chrono::system_clock::now()).c_str(), CP_UTF8), CP_UTF8
		)))
	);

	// Track Metadata
	if (log.initialTrack.is_valid()) {
		file_info_impl info;
		if (log.initialTrack->get_info(info)) {
			auto GetMeta = [&](const char* field) {
				const char* value = info.meta_get(field, 0);
				return value ? value : "Unknown";
			};
			content.AppendFormat(
				"Artist: %s\r\nAlbum: %s\r\nTrack: %s\r\n\r\n",
				GetMeta("artist"), GetMeta("album"),
				log.trackTitle.IsEmpty() ? GetMeta("title") : CW2A(log.trackTitle, CP_UTF8)
			);
		}
	}

	// Column Setup
	auto columns = GetLogColumnDefinitions();
	std::vector<int> colWidths(columns.size());
	std::vector<bool> isNumeric(columns.size());
	std::vector<CStringA> headersA(columns.size());

	for (size_t i = 0; i < columns.size(); ++i) {
		headersA[i] = columns[i].header;
		colWidths[i] = headersA[i].GetLength();
		isNumeric[i] = (columns[i].metricId != INVALID);
		for (const auto& entry : log.logBackup) {
			CStringA val = columns[i].formatter(entry);
			colWidths[i] = std::max(colWidths[i], val.GetLength());
		}
	}

	// Header and Separator
	content += AWHText::FormatAlignedTextLine(headersA, colWidths, isNumeric);
	std::vector<CStringA> separator(columns.size());
	for (size_t i = 0; i < columns.size(); ++i) {
		separator[i] = CStringA('-', colWidths[i]);
	}
	content += AWHText::FormatAlignedTextLine(separator, colWidths, isNumeric);

	// Data Rows
	for (const auto& entry : log.logBackup) {
		std::vector<CStringA> rowData(columns.size());
		for (size_t i = 0; i < columns.size(); ++i) {
			rowData[i] = columns[i].formatter(entry);
		}
		content += AWHText::FormatAlignedTextLine(rowData, colWidths, isNumeric);
	}

	return content;
}

void AudioWizardDialogRealTime::SaveLog() {
	CComPtr<IFileDialog> pFileDialog;
	if (FAILED(pFileDialog.CoCreateInstance(CLSID_FileSaveDialog))) {
		MessageBox(L"Failed to create file dialog", L"Error", MB_ICONERROR);
		return;
	}

	std::array<COMDLG_FILTERSPEC, 3> fileTypes = {
		{ { L"Text Files", L"*.txt" }, { L"CSV Files", L"*.csv" }, { L"All Files", L"*.*" } }
	};
	pFileDialog->SetFileTypes(static_cast<UINT>(fileTypes.size()), fileTypes.data());
	pFileDialog->SetDefaultExtension(L"txt");

	if (FAILED(pFileDialog->Show(nullptr))) return;

	CComPtr<IShellItem> pItem;
	if (FAILED(pFileDialog->GetResult(&pItem))) {
		MessageBox(L"Failed to get file path", L"Error", MB_ICONERROR);
		return;
	}

	PWSTR pszPath = nullptr;
	if (FAILED(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
		MessageBox(L"Failed to retrieve file path", L"Error", MB_ICONERROR);
		return;
	}
	CStringW filePath(pszPath);
	CoTaskMemFree(pszPath);

	// Determine file type based on extension
	CStringA content;
	if (filePath.Right(4).CompareNoCase(L".csv") == 0) {
		content = WriteLogCsv();
	}
	else {
		content = WriteLogTxt();
	}

	if (content.IsEmpty()) {
		MessageBox(L"No log data to save", L"Error", MB_ICONERROR);
		return;
	}

	HANDLE hFile = CreateFileW(filePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE) {
		CStringW msg;
		msg.Format(L"Error %d: Failed to create file", GetLastError());
		MessageBox(msg, L"Error", MB_ICONERROR);
		return;
	}

	DWORD written;
	if (!WriteFile(hFile, content.GetString(), content.GetLength(), &written, nullptr)) {
		CStringW msg;
		msg.Format(L"Error %d: Failed to write file", GetLastError());
		MessageBox(msg, L"Error", MB_ICONERROR);
	}
	else {
		log.lastLogPath = filePath;
		CStringW message;
		message.Format(L"Log saved successfully!\nOpen the saved log?");
		int response = MessageBox(message, L"Success", MB_YESNO | MB_ICONQUESTION);
		if (response == IDYES) {
			ShellExecute(nullptr, L"open", filePath, nullptr, nullptr, SW_SHOWNORMAL);
		}
	}

	CloseHandle(hFile);
}
#pragma endregion


///////////////////////////////////////////
// * REAL-TIME DIALOG - EVENT HANDLERS * //
///////////////////////////////////////////
#pragma region Real-Time Dialog - Event Handlers
BOOL AudioWizardDialogRealTime::OnInitDialog(CWindow, LPARAM) {
	AWHDarkMode::AddDialogWithControls(*this);

	AudioWizard::Main()->mainRealTime->realTimeDialogHwnd.store(m_hWnd, std::memory_order_release);
	AudioWizard::Main()->StartRealTimeMonitoring(
		AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs,
		AudioWizard::Main()->mainRealTime->monitor.monitorChunkDurationMs
	);
	AudioWizardDialog::InitDarkMode();
	AudioWizardDialog::InitMetricConfig();

	InitColors();
	InitWindowTitle();
	InitMetrics();
	InitFonts();
	InitTooltips();
	InitMeterBrushes();
	InitButtons();
	InitTimers();
	InitWindowSize();

	return TRUE;
}

LRESULT AudioWizardDialogRealTime::OnDrawItem(UINT, WPARAM, LPARAM lParam, BOOL) {
	auto* pDrawItem = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
	if (pDrawItem->CtlType != ODT_STATIC) return 0;

	HWND hWnd = pDrawItem->hwndItem;
	HDC hdc = pDrawItem->hDC;
	const RECT& rc = pDrawItem->rcItem;
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	// Create double-buffered MemoryDC
	AWHGraphics::MemoryDC memDC(hdc, width, height);
	HDC memHdc = memDC.GetDC();
	if (!memHdc) return 1;

	auto it = metricControlValueMap.find(pDrawItem->hwndItem);
	if (it == metricControlValueMap.end()) return 1;

	const auto& metric = metrics[it->second];

	if (pDrawItem->hwndItem == metric.meterBar.m_hWnd) {
		// Draw meter bar
		DRAWITEMSTRUCT tempDrawItem = *pDrawItem;
		tempDrawItem.hDC = memHdc;
		tempDrawItem.rcItem = { 0, 0, width, height };
		DrawMeterBar(&tempDrawItem, metric.currentValue);
	}
	else {
		// Draw value text
		AWHGraphics::DrawTheRect(memHdc, { 0, 0, width, height }, col.metricValueBgColor, col.metricValueBgColor, 0);

		RECT textRect = { 0, 0, width, height };
		CStringW text = AWHString::GetWindowTextCStringW(hWnd);
		COLORREF textColor = col.colorMap[hWnd];
		AWHGraphics::DrawTheText(memHdc, textRect, text, textColor, font.valueFont.get(), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
	}

	BitBlt(hdc, rc.left, rc.top, width, height, memHdc, 0, 0, SRCCOPY);
	return 1;
}

LRESULT AudioWizardDialogRealTime::OnNotify(UINT, WPARAM, LPARAM lParam, BOOL) {
	auto* pInfo = reinterpret_cast<NMTTDISPINFOW*>(lParam);
	if (pInfo->hdr.code != TTN_GETDISPINFOW) return 0;

	const auto hWndTool = reinterpret_cast<HWND>(pInfo->hdr.idFrom);
	auto it = metricControlValueMap.find(hWndTool);
	if (it == metricControlValueMap.end()) return 0;

	MetricId metricId = metrics[it->second].metricId;
	static CStringW emptyTooltip;

	if (AudioWizardSettings::monitorDisplayTooltips) {
		std::wstring fullName = AudioWizardDialog::GetMetricFullName(metricId);
		CStringW tooltipText = fullName.c_str();
		tooltipText += L"\nSEPARATOR\n";
		tooltipText += AudioWizardDialog::GetTooltips(metricId, AudioWizardDialog::MetricContext::RealTime);
		pInfo->lpszText = tooltipText.GetBuffer();
	}
	else {
		pInfo->lpszText = emptyTooltip.GetBuffer();
	}

	if (AudioWizardSettings::monitorDisplayColors) {
		AudioWizardDialog::MetricKey key{ AudioWizardDialog::MetricContext::RealTime, metricId };
		auto dataIt = AudioWizardDialog::metricData.find(key);

		if (dataIt != AudioWizardDialog::metricData.end()) {
			std::vector<COLORREF> colors;
			colors.reserve(dataIt->second.ranges.size());

			for (const auto& range : dataIt->second.ranges) {
				colors.push_back(AudioWizardDialog::darkMode ? range.lightColor : range.darkColor);
			}

			SetProp(pInfo->hdr.hwndFrom, L"RANGE_COLORS", std::make_unique<std::vector<COLORREF>>(colors).release());
		}
	}

	return 0;
}

void AudioWizardDialogRealTime::OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const {
	lpMMI->ptMinTrackSize = CPoint(ui.minSize.cx, ui.minSize.cy);
}

void AudioWizardDialogRealTime::OnSize(UINT, CSize) {
	CRect clientRect;
	GetClientRect(&clientRect);

	// Start deferred window positioning to minimize flicker when resizing - Metrics (labels + values) + Buttons
	const int numControls = static_cast<int>(metrics.size() * 2) + 5;
	HDWP hdwp = BeginDeferWindowPos(numControls);

	if (hdwp) {
		UpdateFonts(GetFontSize(clientRect));
		SetMetrics(clientRect, hdwp);
		UpdateWindowLayout(hdwp);
		EndDeferWindowPos(hdwp);
	}

	RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASENOW);
}

void AudioWizardDialogRealTime::OnTimer(UINT_PTR id) {
	if (id == 1) {
		UpdateMetrics();
		UpdateColors();
		UpdateLogging();
	}
	else if (id == 2) {
		InitWindowTitle();
	}
}

void AudioWizardDialogRealTime::OnOpenAnalysis(UINT, int, CWindow) const {
	AudioWizard::Main()->GetFullTrackAnalysis();
}

void AudioWizardDialogRealTime::OnConfig(UINT, int, CWindow) {
	AudioWizardDialogRealTimeConfig dlg(*this);
	dlg.DoModal(m_hWnd);
}

void AudioWizardDialogRealTime::OnToggleLog(UINT, int, CWindow) {
	if (log.isLogging) {
		StopLogging();
		return;
	}

	metadb_handle_ptr track;
	if (!playback_control::get()->get_now_playing(track)) {
		MessageBox(L"No track is currently playing.", L"Logging Error", MB_ICONERROR);
		return;
	}

	constexpr double DEFAULT_DURATION_SEC = 15.0 * 60; // 15 minutes
	constexpr double BUFFER_MULTIPLIER = 1.25;
	constexpr int MIN_ENTRIES = 100;
	constexpr size_t MAX_LOG_MEMORY_BYTES = 1'073'741'824; // 1GB
	constexpr size_t ENTRY_SIZE = sizeof(LogEntry); // ~112 bytes

	// Get track duration
	double trackDuration = DEFAULT_DURATION_SEC;
	file_info_impl info;
	if (track->get_info(info)) {
		double retrievedDuration = info.get_length();
		if (retrievedDuration > 0) trackDuration = retrievedDuration;
	}

	// Get track title
	log.trackTitle = L"Unknown";
	if (track->get_info(info)) {
		const char* title = info.meta_get("title", 0);
		if (title) {
			pfc::string8 title_utf8(title);
			log.trackTitle = pfc::stringcvt::string_wide_from_utf8(title_utf8).get_ptr();
		}
	}

	// Calculate buffer size
	const int refreshRate = AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs;
	const double totalMilliseconds = trackDuration * 1000.0;
	const double calculatedEntries = (totalMilliseconds / refreshRate) * BUFFER_MULTIPLIER;
	const size_t entriesNeeded = std::max(static_cast<size_t>(std::round(calculatedEntries)), static_cast<size_t>(MIN_ENTRIES));
	const size_t maxEntries = MAX_LOG_MEMORY_BYTES / ENTRY_SIZE; // ~524,288

	// Initialize logging
	try {
		log.logBuffer.clear();
		log.logBackup.clear();
		log.logBackup.reserve(std::min(entriesNeeded, maxEntries));
		log.initialTrack = track;
		log.logStartTime = GetTickCount64();
		log.lastTimestamp = 0;
		log.isLogging = true;
		ui.toggleLogBtn.SetWindowText(L"Stop Log");
	}
	catch (const std::bad_alloc&) {
		FB2K_console_formatter() << "AudioWizard => Failed to initialize logging: Out of memory";
		MessageBox(L"Failed to initialize logging: Out of memory", L"Error", MB_ICONERROR);
	}
}

void AudioWizardDialogRealTime::OnClose(UINT, int, CWindow) {
	AudioWizard::Main()->StopRealTimeMonitoring();
	AudioWizard::Main()->mainRealTime->realTimeDialogHwnd.store(nullptr, std::memory_order_release);

	col.metricMap.clear();
	ui.tooltipMap.clear();
	ui.tooltipCtrl.DestroyWindow();

	StopLogging();
	KillTimer(dlgTitle.monitorTimerId);
	KillTimer(dlgTitle.titleTimerId);
	DestroyWindow();
}

void AudioWizardDialogRealTime::OnNcDestroy() {
	SetMsgHandled(FALSE);

	std::unique_ptr<AudioWizardDialogRealTime>(
		reinterpret_cast<AudioWizardDialogRealTime*>(GetWindowLongPtr(GWLP_USERDATA))
	);
}

LRESULT AudioWizardDialogRealTime::OnRefreshRateChanged(UINT, WPARAM, LPARAM, BOOL) {
	KillTimer(dlgTitle.monitorTimerId);
	dlgTitle.monitorTimerId = SetTimer(1, AudioWizard::Main()->mainRealTime->monitor.monitorRefreshRateMs.load());
	return 0;
}
#pragma endregion


///////////////////////////////////////////
// * REAL-TIME CONFIG DIALOG - METHODS * //
///////////////////////////////////////////
#pragma region Real-Time Config Dialog - Methods
AudioWizardDialogRealTimeConfig::AudioWizardDialogRealTimeConfig(AudioWizardDialogRealTime& parent)
	: parent(parent) {
}

void AudioWizardDialogRealTimeConfig::InitSettings() {
	AudioWizardSettings::InitMonitorSettings(m_hWnd);
	InitRefreshRate();
}

void AudioWizardDialogRealTimeConfig::SetSettings() {
	hasChanged = true;
	AudioWizardSettings::SetMonitorSettings(m_hWnd);
	hasChanged = false;
}

void AudioWizardDialogRealTimeConfig::InitControlsPosition() {
	GetClientRect(&ui.originalClientRect);

	for (int id : controlIds) {
		CWindow ctrl = GetDlgItem(id);
		CRect rect;
		ctrl.GetWindowRect(&rect);
		ScreenToClient(&rect);
		ui.originalRects[id] = rect;
	}

	for (int id : buttonIds) {
		CWindow ctrl = GetDlgItem(id);
		CRect rect;
		ctrl.GetWindowRect(&rect);
		ScreenToClient(&rect);
		ui.originalRects[id] = rect;
	}

	CWindow groupBox = GetDlgItem(0);
	if (groupBox) {
		CRect rect;
		groupBox.GetWindowRect(&rect);
		ScreenToClient(&rect);
		ui.originalRects[0] = rect;
	}

	ui.minSize = AWHConvert::DialogUnitsToPixel(m_hWnd, 320, 320);

	if (ui.originalRects.count(IDC_MONITOR_CFG_RESET_BUTTON)) {
		int buttonHeight = ui.originalRects[IDC_MONITOR_CFG_RESET_BUTTON].Height();
		ui.minSize.cy = std::max(ui.minSize.cy, AWHConvert::DialogUnitsToPixel(m_hWnd, 0, 300).cy + 3 * buttonHeight);
	}
}

void AudioWizardDialogRealTimeConfig::InitRefreshRate() {
	int currentRefreshRate = AudioWizardSettings::monitorDisplayRefreshRate;

	auto it = std::find(refreshRates.begin(), refreshRates.end(), currentRefreshRate);
	auto index = (it != refreshRates.end()) ? std::distance(refreshRates.begin(), it) : 9; // Default to 100 ms
	::SendDlgItemMessage(m_hWnd, IDC_MONITOR_CFG_REFRESH_RATE, CB_SETCURSEL, index, 0);

	hasChanged = false;
}

void AudioWizardDialogRealTimeConfig::UpdateRefreshRate() {
	auto selectedIndex = static_cast<int>(SendDlgItemMessage(IDC_MONITOR_CFG_REFRESH_RATE, CB_GETCURSEL, 0, 0));

	if (selectedIndex != CB_ERR && selectedIndex >= 0 && static_cast<size_t>(selectedIndex) < refreshRates.size()) {
		int newRefreshRate = refreshRates[selectedIndex];
		AudioWizard::Main()->SetMonitoringRefreshRate(newRefreshRate);
	}
}

void AudioWizardDialogRealTimeConfig::UpdateControlsPosition(const std::vector<int>& ids, int targetCenterX, int newY) {
	if (ids.empty() || ui.originalRects.empty()) return;

	// Calculate original horizontal bounds in a single pass
	int minX = INT_MAX;
	int maxX = INT_MIN;

	for (int id : ids) {
		auto it = ui.originalRects.find(id);
		if (it != ui.originalRects.end()) {
			const CRect& rect = it->second;
			minX = std::min(minX, static_cast<int>(rect.left));
			maxX = std::max(maxX, static_cast<int>(rect.right));
		}
	}

	if (minX == INT_MAX) return;

	// Calculate positioning offset
	const int offsetX = targetCenterX - (minX + (maxX - minX) / 2);

	// Reposition controls
	for (int id : ids) {
		auto it = ui.originalRects.find(id);
		if (it != ui.originalRects.end()) {
			const CRect& rect = it->second;
			const int y = (newY == -1) ? rect.top : newY;
			GetDlgItem(id).SetWindowPos(nullptr, rect.left + offsetX, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
	}
}
#pragma endregion


//////////////////////////////////////////////////
// * REAL-TIME CONFIG DIALOG - EVENT HANDLERS * //
//////////////////////////////////////////////////
#pragma region Real-Time Config Dialog - Event Handlers
void AudioWizardDialogRealTimeConfig::OnSize(UINT, CSize size) {
	if (ui.originalClientRect.IsRectEmpty()) return;

	const int cx = size.cx;
	const int cy = size.cy;
	const int buttonHeight = ui.originalRects[IDC_MONITOR_CFG_RESET_BUTTON].Height();

	// Resize and adjust group box
	if (ui.originalRects.count(0)) {
		CRect newGroupRect = ui.originalRects[0];
		newGroupRect.right = cx - (ui.originalClientRect.Width() - newGroupRect.right);
		newGroupRect.bottom = cy - 3 * buttonHeight;

		CWindow groupBox = GetDlgItem(0);
		groupBox.SetWindowPos(nullptr, newGroupRect.left, newGroupRect.top,
			newGroupRect.Width(), newGroupRect.Height(), SWP_NOZORDER
		);

		// Center controls inside the group box
		UpdateControlsPosition(controlIds, newGroupRect.CenterPoint().x, -1);
	}

	// Center buttons at the bottom
	UpdateControlsPosition(buttonIds, cx / 2, cy - 2 * buttonHeight);
}

BOOL AudioWizardDialogRealTimeConfig::OnInitDialog(CWindow, LPARAM) {
	AWHDarkMode::AddDialogWithControls(*this);

	InitControlsPosition();
	InitSettings();

	return TRUE;
}

void AudioWizardDialogRealTimeConfig::OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const {
	lpMMI->ptMinTrackSize = CPoint(ui.minSize.cx, ui.minSize.cy);
}

void AudioWizardDialogRealTimeConfig::OnSettingChanged(UINT, int, CWindow) {
	SetSettings();
}

void AudioWizardDialogRealTimeConfig::OnReset(UINT, int, CWindow) {
	AudioWizardSettings::ResetMonitorSettings();
	InitSettings();
}

void AudioWizardDialogRealTimeConfig::OnApply(UINT, int, CWindow) {
	SetSettings();
	UpdateRefreshRate();
	parent.InitDialog();
}

void AudioWizardDialogRealTimeConfig::OnClose(UINT, int, CWindow) {
	EndDialog(IDCANCEL);
}

LRESULT AudioWizardDialogRealTimeConfig::OnRefreshRateChanged(UINT, WPARAM, LPARAM, BOOL) {
	InitRefreshRate();
	return 0;
}
#pragma endregion
