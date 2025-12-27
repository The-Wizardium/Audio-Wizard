/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Dialog Full-Track Header File              * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once
#include "resource.h"
#include "AW_Analysis.h"
#include "AW_Dialog.h"


///////////////////////////
// * DIALOG FULL-TRACK * //
///////////////////////////
#pragma region Dialog Full-Track
class AudioWizardDialogFullTrack : public CDialogImpl<AudioWizardDialogFullTrack> {
public:
	// * TYPE ALIASES * //
	using MetricId = AudioWizardDialog::MetricId;
	using enum AudioWizardDialog::MetricId;
	using FullTrackResults = AudioWizardAnalysisFullTrack::FullTrackResults;

	enum { IDD = IDD_ANALYSIS_DIALOG };

	explicit AudioWizardDialogFullTrack(std::vector<FullTrackResults> results,
		const wchar_t* processingTime, const wchar_t* processingSpeed
	);
	~AudioWizardDialogFullTrack() final;

	void InitDialog();

	BEGIN_MSG_MAP(AudioWizardDialogFullTrack)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_NCDESTROY(OnNcDestroy)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		NOTIFY_HANDLER(IDC_ANALYSIS_LIST, LVN_COLUMNCLICK, OnColumnClick)
		NOTIFY_HANDLER(IDC_ANALYSIS_LIST, NM_DBLCLK, OnListDoubleClick)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_OPEN_MONITOR_BUTTON, BN_CLICKED, OnOpenMonitor)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CONFIG_BUTTON, BN_CLICKED, OnConfig)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_WRITE_TAGS_BUTTON, BN_CLICKED, OnWriteTags)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_WRITE_TAGS_COMBOBOX, CBN_SELCHANGE, OnWriteTagsComboSelChange)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CLEAR_TAGS_BUTTON, BN_CLICKED, OnClearTags)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CLEAR_TAGS_COMBOBOX, CBN_SELCHANGE, OnClearTagsComboSelChange)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_SETUP_TAGS_BUTTON, BN_CLICKED, OnSetupTags)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_SAVE_BUTTON, BN_CLICKED, OnSave)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CLOSE_BUTTON, BN_CLICKED, OnClose)
	END_MSG_MAP()

private:
	struct Constants {
		static constexpr int BUTTON_MARGIN = 15;
		static constexpr int LIST_MARGIN = 20;
	};

	struct ColumnSort {
		int lastTooltipColumn = -1;
		int sortColumn = 0;
		bool ascending = true;
	}; ColumnSort columnSort;

	struct ColumnMetadata {
		MetricId metricId;
		std::wstring metricName;
		bool setting;
		bool isIndex;
		bool isNumeric;
		std::function<double(const FullTrackResults&)> numericGetter;
		std::function<CStringW(const FullTrackResults&)> getter;
	};

	struct AnalysisData {
		std::vector<FullTrackResults> results;
		CStringW processingTime;
		CStringW processingSpeed;
		std::vector<MetricId> metrics;
		std::vector<ColumnMetadata> columns;
		std::vector<std::vector<CStringW>> cachedColumnTexts;
	}; AnalysisData data;

	struct UI {
		CSize minSize;
		CListViewCtrl listView;
		CHeaderCtrl headerCtrl;
		CToolTipCtrl tooltipCtrl;
		CStringW currentTooltipText;
		CButton openMonitorBtn;
		CButton configBtn;
		CButton writeTagsBtn;
		CComboBox writeTagsCombo;
		CButton clearTagsBtn;
		CComboBox clearTagsCombo;
		CButton setupTagsBtn;
		CButton saveBtn;
		CButton cancelBtn;
		CRect originalOpenMonitorRect;
		CRect originalConfigRect;
		CRect originalWriteTagsButtonRect;
		CRect originalWriteTagsComboRect;
		CRect originalClearTagsButtonRect;
		CRect originalClearTagsComboRect;
		CRect originalSetupTagsButtonRect;
		CRect originalListRect;
		CRect originalButtonRect;
	}; UI ui;

	// * INITIALIZATION * //
	void InitDarkMode();
	void InitWindowTitle();
	void InitColumnsMetadata();
	void InitColumnsContent();
	void InitColumnsUI();
	void InitListView();
	void InitButtons();
	void InitDropDownMenus();
	void InitTooltips();
	void InitWindowSize();

	// * DIALOG METHODS * //
	int GetColumnWidth(const CClientDC& dc, size_t columnIndex) const;
	void PopulateList();
	void SetListViewPosition(const CRect& clientRect, int groupY);
	void SetButtonMonitorPosition(const CRect&, int groupX, int groupY);
	void SetButtonTagsPosition(const CRect&, int groupX, int groupY);
	void SetButtonSaveClosePosition(const CRect& clientRect, int groupY);
	void UpdateColors(NMLVCUSTOMDRAW* pLVCD);
	void UpdateWindowLayout();
	CStringA WriteAnalysisResultsCsv(const std::vector<ColumnMetadata>& columns,
		const CStringW& processingTime, const CStringW& processingSpeed
	) const;
	CStringA WriteAnalysisResultsTxt(const std::vector<ColumnMetadata>& columns,
		const CStringW& processingTime, const CStringW& processingSpeed
	) const;


	// * EVENT LISTENERS * //
	BOOL OnInitDialog(CWindow, LPARAM);
	LRESULT OnColumnClick(int, LPNMHDR pnmh, BOOL);
	LRESULT OnListDoubleClick(int, LPNMHDR pnmh, BOOL);
	LRESULT OnNotify(UINT, WPARAM, LPARAM lParam, BOOL& bHandled);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const;
	void OnSize(UINT, CSize);
	void OnOpenMonitor(UINT, int, CWindow) const;
	void OnConfig(UINT, int, CWindow);
	void OnWriteTags(UINT, int, CWindow);
	void OnClearTags(UINT, int, CWindow);
	void OnWriteTagsComboSelChange(UINT, int, CWindow) const;
	void OnClearTagsComboSelChange(UINT, int, CWindow) const;
	void OnSetupTags(UINT, int, CWindow);
	void OnSave(UINT, int, CWindow);
	void OnClose(UINT = 0, int = 0, CWindow = nullptr);
	void OnNcDestroy();
};
#pragma endregion


//////////////////////////////////
// * DIALOG FULL-TRACK CONFIG * //
//////////////////////////////////
#pragma region Dialog Full-Track Config
class AudioWizardDialogFullTrackConfig : public CDialogImpl<AudioWizardDialogFullTrackConfig> {
public:
	enum { IDD = IDD_ANALYSIS_CFG_DIALOG };

	explicit AudioWizardDialogFullTrackConfig(AudioWizardDialogFullTrack& parent);

	BEGIN_MSG_MAP(AudioWizardDialogFullTrackConfig)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)

		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_COLORS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_TOOLTIPS, BN_CLICKED, OnSettingChanged)

		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_INDEX, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_ARTIST, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_ALBUM, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_TRACK, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_DURATION, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_YEAR, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_GENRE, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_FORMAT, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_CHANNELS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_BIT_DEPTH, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_BITRATE, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_SAMPLE_RATE, BN_CLICKED, OnSettingChanged)

		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_MOMENTARY_LUFS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_SHORT_TERM_LUFS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_INTEGRATED_LUFS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_RMS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_SAMPLE_PEAK, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_TRUE_PEAK, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_PSR, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_PLR, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_CREST_FACTOR, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_LOUDNESS_RANGE, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_DYNAMIC_RANGE, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_DYNAMIC_RANGE_ALBUM, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_PURE_DYNAMICS, BN_CLICKED, OnSettingChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_PURE_DYNAMICS_ALBUM, BN_CLICKED, OnSettingChanged)

		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_METADATA_PRESET, CBN_SELCHANGE, OnMetadataPresetChanged)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_METRICS_PRESET, CBN_SELCHANGE, OnMetricsPresetChanged)

		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_RESET_BUTTON, BN_CLICKED, OnReset)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_APPLY_BUTTON, BN_CLICKED, OnApply)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_CFG_CLOSE_BUTTON, BN_CLICKED, OnClose)
	END_MSG_MAP()

private:
	AudioWizardDialogFullTrack& parent;
	bool hasChanged = false;

	struct UI {
		CSize minSize;
		CRect originalClientRect;
		std::map<int, CRect> originalRects;
	}; UI ui;

	static inline const std::vector<int> controlIds = {
		IDC_ANALYSIS_CFG_COLORS,
		IDC_ANALYSIS_CFG_TOOLTIPS,
		IDC_ANALYSIS_CFG_INDEX,
		IDC_ANALYSIS_CFG_ARTIST,
		IDC_ANALYSIS_CFG_ALBUM,
		IDC_ANALYSIS_CFG_TRACK,
		IDC_ANALYSIS_CFG_DURATION,
		IDC_ANALYSIS_CFG_YEAR,
		IDC_ANALYSIS_CFG_GENRE,
		IDC_ANALYSIS_CFG_FORMAT,
		IDC_ANALYSIS_CFG_CHANNELS,
		IDC_ANALYSIS_CFG_BIT_DEPTH,
		IDC_ANALYSIS_CFG_BITRATE,
		IDC_ANALYSIS_CFG_SAMPLE_RATE,
		IDC_ANALYSIS_CFG_MOMENTARY_LUFS,
		IDC_ANALYSIS_CFG_SHORT_TERM_LUFS,
		IDC_ANALYSIS_CFG_INTEGRATED_LUFS,
		IDC_ANALYSIS_CFG_RMS,
		IDC_ANALYSIS_CFG_SAMPLE_PEAK,
		IDC_ANALYSIS_CFG_TRUE_PEAK,
		IDC_ANALYSIS_CFG_PSR,
		IDC_ANALYSIS_CFG_PLR,
		IDC_ANALYSIS_CFG_CREST_FACTOR,
		IDC_ANALYSIS_CFG_LOUDNESS_RANGE,
		IDC_ANALYSIS_CFG_DYNAMIC_RANGE,
		IDC_ANALYSIS_CFG_DYNAMIC_RANGE_ALBUM,
		IDC_ANALYSIS_CFG_PURE_DYNAMICS,
		IDC_ANALYSIS_CFG_PURE_DYNAMICS_ALBUM,
		IDC_ANALYSIS_CFG_METADATA_PRESET_TEXT,
		IDC_ANALYSIS_CFG_METADATA_PRESET,
		IDC_ANALYSIS_CFG_METRICS_PRESET_TEXT,
		IDC_ANALYSIS_CFG_METRICS_PRESET
	};

	static inline const std::vector<int> buttonIds = {
		IDC_ANALYSIS_CFG_RESET_BUTTON,
		IDC_ANALYSIS_CFG_APPLY_BUTTON,
		IDC_ANALYSIS_CFG_CLOSE_BUTTON
	};

	// * DIALOG METHODS * //
	void InitSettings();
	void SetSettings();
	void InitControlsPosition();
	void UpdateControlsPosition(const std::vector<int>& ids, int targetCenterX, int newY);

	// * EVENT LISTENERS * //
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const;
	void OnSize(UINT, CSize size);
	void OnSettingChanged(UINT, int, CWindow);
	void OnMetadataPresetChanged(UINT, int, CWindow);
	void OnMetricsPresetChanged(UINT, int, CWindow);
	void OnReset(UINT, int, CWindow);
	void OnApply(UINT, int, CWindow);
	void OnClose(UINT = 0, int = 0, CWindow = nullptr);
};
#pragma endregion


//////////////////////////////////////
// * DIALOG FULL-TRACK TAG CONFIG * //
//////////////////////////////////////
#pragma region Dialog Full-Track Config
class AudioWizardDialogFullTrackTagsConfig : public CDialogImpl<AudioWizardDialogFullTrackTagsConfig> {
public:
	enum { IDD = IDD_ANALYSIS_TAGS_CFG_DIALOG };

	explicit AudioWizardDialogFullTrackTagsConfig(AudioWizardDialogFullTrack& parent);

	BEGIN_MSG_MAP(AudioWizardDialogFullTrackTagsConfig)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_SIZE(OnSize)
		MSG_WM_CLOSE(OnClose)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_TAGS_CFG_RESET_BUTTON, BN_CLICKED, OnReset)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_TAGS_CFG_APPLY_BUTTON, BN_CLICKED, OnApply)
		COMMAND_HANDLER_EX(IDC_ANALYSIS_TAGS_CFG_CLOSE_BUTTON, BN_CLICKED, OnClose)
	END_MSG_MAP()

private:
	AudioWizardDialogFullTrack& parent;
	bool hasChanged = false;

	struct UI {
		CSize minSize;
		CRect originalClientRect;
		std::map<int, CRect> originalRects;
	}; UI ui;

	static inline const std::vector<int> controlIds = {
		IDC_ANALYSIS_TAGS_CFG_LUFS_LABEL,
		IDC_ANALYSIS_TAGS_CFG_LUFS_VALUE,
		IDC_ANALYSIS_TAGS_CFG_RMS_LABEL,
		IDC_ANALYSIS_TAGS_CFG_RMS_VALUE,
		IDC_ANALYSIS_TAGS_CFG_SP_LABEL,
		IDC_ANALYSIS_TAGS_CFG_SP_VALUE,
		IDC_ANALYSIS_TAGS_CFG_TP_LABEL,
		IDC_ANALYSIS_TAGS_CFG_TP_VALUE,
		IDC_ANALYSIS_TAGS_CFG_PSR_LABEL,
		IDC_ANALYSIS_TAGS_CFG_PSR_VALUE,
		IDC_ANALYSIS_TAGS_CFG_PLR_LABEL,
		IDC_ANALYSIS_TAGS_CFG_PLR_VALUE,
		IDC_ANALYSIS_TAGS_CFG_CF_LABEL,
		IDC_ANALYSIS_TAGS_CFG_CF_VALUE,
		IDC_ANALYSIS_TAGS_CFG_LRA_LABEL,
		IDC_ANALYSIS_TAGS_CFG_LRA_VALUE,
		IDC_ANALYSIS_TAGS_CFG_DR_LABEL,
		IDC_ANALYSIS_TAGS_CFG_DR_VALUE,
		IDC_ANALYSIS_TAGS_CFG_DR_ALBUM_LABEL,
		IDC_ANALYSIS_TAGS_CFG_DR_ALBUM_VALUE,
		IDC_ANALYSIS_TAGS_CFG_PD_LABEL,
		IDC_ANALYSIS_TAGS_CFG_PD_VALUE,
		IDC_ANALYSIS_TAGS_CFG_PD_ALBUM_LABEL,
		IDC_ANALYSIS_TAGS_CFG_PD_ALBUM_VALUE
	};

	static inline const std::vector<int> buttonIds = {
		IDC_ANALYSIS_TAGS_CFG_RESET_BUTTON,
		IDC_ANALYSIS_TAGS_CFG_APPLY_BUTTON,
		IDC_ANALYSIS_TAGS_CFG_CLOSE_BUTTON
	};

	// * DIALOG METHODS * //
	void InitSettings();
	void SetSettings();
	void InitControlsPosition();
	void UpdateControlsPosition(const std::vector<int>& ids, int targetCenterX, int newY);

	// * EVENT LISTENERS * //
	BOOL OnInitDialog(CWindow, LPARAM);
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const;
	void OnSize(UINT, CSize size);
	void OnSettingChanged(UINT, int, CWindow);
	void OnReset(UINT, int, CWindow);
	void OnApply(UINT, int, CWindow);
	void OnClose(UINT = 0, int = 0, CWindow = nullptr);
};
#pragma endregion
