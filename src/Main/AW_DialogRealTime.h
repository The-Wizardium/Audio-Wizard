/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Dialog Real-Time Header File               * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "resource.h"
#include "AW_Dialog.h"
#include "AW_Helpers.h"
#include "AW_Settings.h"


//////////////////////////
// * DIALOG REAL-TIME * //
//////////////////////////
#pragma region Dialog Real-Time
class AudioWizardDialogRealTime : public CDialogImpl<AudioWizardDialogRealTime> {
public:
	// * TYPE ALIASES * //
	using MetricId = AudioWizardDialog::MetricId;
	using enum AudioWizardDialog::MetricId;

	enum { IDD = IDD_MONITOR_DIALOG };

	enum class MetricsMode {
		Value = 0,
		Meter = 1,
		ValueAndMeter = 2
	};

	AudioWizardDialogRealTime() = default;
	~AudioWizardDialogRealTime() final;

	void InitDialog();

	BEGIN_MSG_MAP(AudioWizardDialogRealTime)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_SIZE(OnSize)
		MSG_WM_TIMER(OnTimer)
		MSG_WM_CLOSE(OnClose)
		MESSAGE_HANDLER(WM_USER + 2, OnRefreshRateChanged)

		MSG_WM_NCDESTROY(OnNcDestroy)
		MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		COMMAND_HANDLER_EX(IDC_MONITOR_OPEN_ANALYSIS_BUTTON, BN_CLICKED, OnOpenAnalysis)
		COMMAND_HANDLER_EX(IDC_MONITOR_CONFIG_BUTTON, BN_CLICKED, OnConfig)
		COMMAND_HANDLER_EX(IDC_MONITOR_TOGGLE_LOG_BUTTON, BN_CLICKED, OnToggleLog)
		COMMAND_HANDLER_EX(IDC_MONITOR_CLOSE_BUTTON, BN_CLICKED, OnClose)
	END_MSG_MAP()

private:
	// * LIST LAYOUT CONSTANTS * //
	static constexpr int BUTTON_MARGIN = 15;

	struct UI {
		int labelHeight;
		int valueHeight;
		int columnWidth;
		CSize minSize;
		CButton openAnalysisBtn;
		CButton configBtn;
		CButton toggleLogBtn;
		CButton closeBtn;
		CRect originalOpenAnalysisRect;
		CRect originalConfigRect;
		CRect originalToggleLogRect;
		CRect originalCloseRect;
		CToolTipCtrl tooltipCtrl;
		std::unordered_map<HWND, CStringW> tooltipMap;
		CStringW currentTooltipText;
	}; UI ui;

	struct Font {
		AWHGraphics::Font labelFont;
		AWHGraphics::Font valueFont;
	}; Font font;

	struct MetricConfig {
		MetricId metricId;
		int labelID;
		int valueID;
		int meterBarID;
		bool setting;
		bool isLeft;
	};

	struct MetricControl {
		MetricId metricId;
		CStatic label;
		CStatic value;
		CStatic meterBar;
		COLORREF currentColor;
		double currentValue;
		double previousValue;
		double minVal;
		double maxVal;
		bool isDisplayed;
		bool colorChanged;
		double targetValue;
		double animatedValue;
		ULONGLONG animationStartTime;
		bool isAnimating;
	};
	std::vector<MetricControl> metrics;
	std::vector<MetricControl> leftMetrics;
	std::vector<MetricControl> rightMetrics;
	std::unordered_map<HWND, size_t> metricControlValueMap;

	// * DIALOG TITLE * //
	struct DialogTitle {
		ULONGLONG prevFoobarTime = 0;
		ULONGLONG prevSystemIdle = 0;
		ULONGLONG prevSystemKernel = 0;
		ULONGLONG prevSystemUser = 0;
		ULONGLONG prevTotalTime = 0;
		UINT_PTR monitorTimerId = 0;
		UINT_PTR titleTimerId = 0;
	}; DialogTitle dlgTitle;

	// * BRUSHES * //
	struct Brush {
		AWHGraphics::Brush meterBgBrush;
		AWHGraphics::Brush meterFrameBrush;
		mutable std::unordered_map<COLORREF, AWHGraphics::Brush> meterBrushCache;
	}; Brush brush;

	// * COLORS * //
	struct Color {
		std::map<HWND, COLORREF> colorMap;
		std::map<HWND, MetricId> metricMap;
		CBrush bgBrush;
		COLORREF lastBgColor;
		COLORREF metricValueColor;
		COLORREF metricValueBgColor;
		COLORREF meterBgColor;
		COLORREF meterBorderColor;
	}; Color col;

	// * LOG * //
	struct LogEntry {
		CStringW trackTitle;
		double timestamp;
		double duration;
		double momentaryLUFS;
		double shortTermLUFS;
		double leftRMS;
		double rightRMS;
		double leftSamplePeak;
		double rightSamplePeak;
		double truePeak;
		double RMS;
		double PSR;
		double PLR;
		double crestFactor;
		double dynamicRange;
		double pureDynamics;
		double phaseCorrelation;
		double stereoWidth;
	};

	struct LogColumnDefinition {
		MetricId metricId;
		const char* header;
		const char* abbreviation;
		std::function<CStringA(const LogEntry&)> formatter;
		int width = 0;
	};

	struct LogState {
		AWHAudioBuffer::RingBuffer<LogEntry> logBuffer { 66176 }; // ~8.5MB, ~15min at 17ms
		std::vector<LogEntry> logBackup;
		CStringW trackTitle;
		metadb_handle_ptr initialTrack;
		std::atomic<bool> isLogging = false;
		CStringW lastLogPath;
		ULONGLONG lastTimestamp = 0;
		ULONGLONG logStartTime;
	}; LogState log;

	// * INITIALIZATION * //
	void InitWindowTitle();
	void InitFonts();
	void InitTooltips();
	void InitMeterBrushes();
	void InitColors();
	void InitMetrics();
	void InitButtons();
	void InitTimers();
	void InitWindowSize();

	// * DIALOG METHODS * //
	void DrawMeterBar(DRAWITEMSTRUCT* pDrawItem, double value);
	ULONGLONG GetMeterBarAnimationDuration() const;
	double GetMeterBarUpdateThreshold() const;
	double GetMetricValueUpdateThreshold() const;
	std::pair<double, double> GetDisplayRange(MetricId metricId) const;
	int GetFontSize(const CRect& clientRect) const;
	void SetButtonAnalysisPosition(HDWP hdwp, int groupX, int groupY) const;
	void SetButtonLogPosition(HDWP hdwp, int groupX, int groupY) const;
	void SetButtonClosePosition(HDWP hdwp, int groupX, int groupY) const;
	void SetMetrics(const CRect& clientRect, HDWP hdwp) const;
	void UpdateMetrics();
	void UpdateColors();
	void UpdateFonts(int fontSize);
	void UpdateWindowLayout(HDWP hdwp) const;

	// * LOG * //
	void StartLogging();
	void StopLogging();
	void UpdateLogging();
	std::vector<LogColumnDefinition> GetLogColumnDefinitions() const;
	CStringA WriteLogCsv() const;
	CStringA WriteLogTxt() const;
	void SaveLog();

	// * EVENT HANDLERS * //
	BOOL OnInitDialog(CWindow, LPARAM);
	LRESULT OnDrawItem(UINT, WPARAM, LPARAM lParam, BOOL);
	LRESULT OnNotify(UINT, WPARAM, LPARAM lParam, BOOL);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const;
	void OnSize(UINT, CSize);
	void OnTimer(UINT_PTR id);
	void OnOpenAnalysis(UINT, int, CWindow) const;
	void OnConfig(UINT, int, CWindow);
	void OnToggleLog(UINT, int, CWindow);
	void OnClose(UINT = 0, int = 0, CWindow = nullptr);
	void OnNcDestroy();
	LRESULT OnRefreshRateChanged(UINT, WPARAM, LPARAM, BOOL);
};
#pragma endregion


/////////////////////////////////
// * DIALOG REAL-TIME CONFIG * //
/////////////////////////////////
#pragma region Dialog Real-Time Config
class AudioWizardDialogRealTimeConfig : public CDialogImpl<AudioWizardDialogRealTimeConfig> {
public:
	enum { IDD = IDD_MONITOR_CFG_DIALOG };

	explicit AudioWizardDialogRealTimeConfig(AudioWizardDialogRealTime& parent);

	BEGIN_MSG_MAP(AudioWizardDialogRealTimeConfig)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)
		MESSAGE_HANDLER(WM_USER + 2, OnRefreshRateChanged)

		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_COLORS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_TOOLTIPS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_MOMENTARY_LUFS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_SHORT_TERM_LUFS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_LEFT_RMS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_RIGHT_RMS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_LEFT_SAMPLE_PEAK, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_RIGHT_SAMPLE_PEAK, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_TRUE_PEAK, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_PSR, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_PLR, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_CREST_FACTOR, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_DYNAMIC_RANGE, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_PURE_DYNAMICS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_PHASE_CORRELATION, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_STEREO_WIDTH, BN_CLICKED, OnSettingChanged)

		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_RESET_BUTTON, BN_CLICKED, OnReset)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_APPLY_BUTTON, BN_CLICKED, OnApply)
		COMMAND_HANDLER_EX(IDC_MONITOR_CFG_CLOSE_BUTTON, BN_CLICKED, OnClose)
	END_MSG_MAP()

private:
	AudioWizardDialogRealTime& parent;
	bool hasChanged = false;

	struct UI {
		CSize minSize;
		CRect originalClientRect;
		std::map<int, CRect> originalRects;
	}; UI ui;

	static inline const std::vector<int> controlIds = {
		IDC_MONITOR_CFG_TB_METRICS,
		IDC_MONITOR_CFG_TB_METRICS_FRAC,
		IDC_MONITOR_CFG_COLORS,
		IDC_MONITOR_CFG_TOOLTIPS,
		IDC_MONITOR_CFG_MOMENTARY_LUFS,
		IDC_MONITOR_CFG_SHORT_TERM_LUFS,
		IDC_MONITOR_CFG_LEFT_RMS,
		IDC_MONITOR_CFG_RIGHT_RMS,
		IDC_MONITOR_CFG_LEFT_SAMPLE_PEAK,
		IDC_MONITOR_CFG_RIGHT_SAMPLE_PEAK,
		IDC_MONITOR_CFG_TRUE_PEAK,
		IDC_MONITOR_CFG_PSR,
		IDC_MONITOR_CFG_PLR,
		IDC_MONITOR_CFG_CREST_FACTOR,
		IDC_MONITOR_CFG_DYNAMIC_RANGE,
		IDC_MONITOR_CFG_PURE_DYNAMICS,
		IDC_MONITOR_CFG_PHASE_CORRELATION,
		IDC_MONITOR_CFG_STEREO_WIDTH,
		IDC_MONITOR_CFG_METRICS_MODE_TEXT,
		IDC_MONITOR_CFG_METRICS_MODE,
		IDC_MONITOR_CFG_REFRESH_RATE_TEXT,
		IDC_MONITOR_CFG_REFRESH_RATE
	};

	static inline const std::vector<int> buttonIds = {
		IDC_MONITOR_CFG_RESET_BUTTON,
		IDC_MONITOR_CFG_APPLY_BUTTON,
		IDC_MONITOR_CFG_CLOSE_BUTTON
	};

	static inline const std::vector<int> refreshRates = {
		1000, 500, 333, 250, 200, 166, 142, 125, 111, 100, 83, 67, 50, 40, 33, 22, 17
	};

	static inline const std::map<int, int> refresRateToChunkDuration = {
		{ 1000, 500 }, { 500, 500 }, { 333, 333 }, { 250, 250 }, { 200, 200 },
		{  166, 200 }, { 142, 200 }, { 125, 200 }, { 111, 200 }, { 100, 200 },
		{   83, 200 }, {  67, 200 }, {  50, 200 }, {  40, 200 }, {  33, 200 },
		{   22, 100 }, {  17, 100 }
	};

	// * DIALOG METHODS * //
	void InitSettings();
	void SetSettings();
	void InitControlsPosition();
	void UpdateControlsPosition(const std::vector<int>& ids, int targetCenterX, int newY);
	void InitRefreshRate();
	void UpdateRefreshRate();

	// * EVENT HANDLERS * //
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const;
	void OnSize(UINT, CSize size);
	void OnSettingChanged(UINT, int, CWindow);
	void OnReset(UINT, int, CWindow);
	void OnApply(UINT, int, CWindow);
	void OnClose(UINT = 0, int = 0, CWindow = nullptr);
	LRESULT OnRefreshRateChanged(UINT, WPARAM, LPARAM, BOOL);
};
#pragma endregion
