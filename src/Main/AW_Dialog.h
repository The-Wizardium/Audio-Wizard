/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Dialog Header File                         * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.3.0                                                   * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    01-09-2025                                              * //
/////////////////////////////////////////////////////////////////////////////////


#pragma once


/////////////////////
// * DIALOG BASE * //
/////////////////////
#pragma region Dialog Base
class AudioWizardDialog {
public:
	enum class MetricColor {
		Blue,
		Green,
		Default,
		Orange,
		Red
	};

	enum class MetricContext {
		FullTrack,
		RealTime
	};

	enum class MetricId {
		INDEX_FT, ARTIST_FT, ALBUM_FT, TRACK_FT, DURATION_FT, YEAR_FT, GENRE_FT, FORMAT_FT, CHANNELS_FT, BIT_DEPTH_FT, BITRATE_FT, SAMPLE_RATE_FT,
		M_LUFS_FT, S_LUFS_FT, I_LUFS_FT, RMS_FT, SP_FT, TP_FT, PSR_FT, PLR_FT, CF_FT, LRA_FT, DR_FT, DR_ALBUM_FT, PD_FT, PD_ALBUM_FT,

		MOMENTARY_LUFS_RT, SHORT_TERM_LUFS_RT,
		LEFT_RMS_RT, RIGHT_RMS_RT,
		LEFT_SAMPLE_PEAK_RT, RIGHT_SAMPLE_PEAK_RT,
		TRUE_PEAK_RT, PSR_RT,
		CREST_FACTOR_RT, PLR_RT,
		DYNAMIC_RANGE_RT, PURE_DYNAMICS_RT,
		PHASE_CORRELATION_RT, STEREO_WIDTH_RT,
		INVALID, RMS_RT
	};

	struct MetricRange {
		double minRange;
		double maxRange;
		COLORREF lightColor;
		COLORREF darkColor;
		std::wstring description;
	};

	struct MetricData {
		std::wstring name;
		std::wstring fullName;
		std::vector<MetricRange> ranges;
		CStringW tooltip;
	};

	struct MetricKey {
		MetricContext context;
		MetricId id;

		bool operator==(const MetricKey& other) const {
			return context == other.context && id == other.id;
		}

		bool operator<(const MetricKey& other) const {
			return std::tie(context, id) < std::tie(other.context, other.id);
		}
	};

	struct MetricKeyHash {
		std::size_t operator()(const MetricKey& key) const {
			return std::hash<std::size_t>{}(static_cast<std::size_t>(key.context)) ^
					std::hash<std::size_t>{}(static_cast<std::size_t>(key.id));
		}
	};

	struct MetricTooltip {
		static constexpr const wchar_t* LEFT_BRACKET = L"[";
		static constexpr const wchar_t* RIGHT_BRACKET = L"]";
		static constexpr const wchar_t* ARROW = L" \u2194 ";
		static constexpr int LINE_SPACING = 5;
		static constexpr int DESC_INDENT = 20;
	};

	static inline std::unordered_map<MetricKey, MetricData, MetricKeyHash> metricData;
	static inline std::unordered_map<MetricKey, CStringW, MetricKeyHash> tooltipCache;
	static inline bool darkMode;

	// * COLOR MANAGEMENT * //
	static void InitDarkMode();
	static void InitMetricConfig();

	static COLORREF GetColor(MetricId metricId, double value, bool colorize, bool ignoreDarkMode, MetricContext context);
	static CStringW GetMetricValueFormatted(double value);
	static CStringW GetMetricRangeFormatted(double min, double max, const wchar_t* description);
	static std::wstring GetMetricName(MetricId metricId);
	static std::wstring GetMetricFullName(MetricId metricId);

	// * TOOLTIP MANAGEMENT * //
	static CStringW GetTooltips(MetricId metricId, MetricContext context);
	static void DrawTooltip(HWND hWnd);
	static void SetTooltips(CToolTipCtrl& tooltip, HWND hWnd, const std::vector<std::pair<HWND, HWND>>* metrics);
	static void SetTooltipSize(HWND hWnd, LPARAM lParam);
	static void CleanTooltips(HWND hWnd);
	static LRESULT CALLBACK TooltipSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR);

private:
	static void DrawTooltipBg(HDC hdc, const RECT& rc);
	static void DrawTooltipText(HDC hdc, const RECT& textRc, const std::vector<COLORREF>* rangeColors, HFONT hFont, const CStringW& text);
};
#pragma endregion
