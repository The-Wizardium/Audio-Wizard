/////////////////////////////////////////////////////////////////////////////////
// * FB2K Component: Audio Wizard                                            * //
// * Description:    Audio Wizard Dialog Full-Track Source File              * //
// * Author:         TT                                                      * //
// * Website:        https://github.com/The-Wizardium/Audio-Wizard           * //
// * Version:        0.1                                                     * //
// * Dev. started:   12-12-2024                                              * //
// * Last change:    22-12-2024                                              * //
/////////////////////////////////////////////////////////////////////////////////


#include "AW_PCH.h"
#include "AW.h"
#include "AW_DialogFullTrack.h"
#include "AW_Helpers.h"
#include "AW_Tag.h"


//////////////////////////////////
// * CONSTRUCTOR & DESTRUCTOR * //
//////////////////////////////////
#pragma region Constructor & Destructor
AudioWizardDialogFullTrack::AudioWizardDialogFullTrack(std::vector<FullTrackResults> results,
	const wchar_t* processingTime, const wchar_t* processingSpeed) :
	data{ std::move(results), CStringW(processingTime), CStringW(processingSpeed), {}, {}, {} } {
	InitColumnsMetadata();
	InitColumnsContent();
}

AudioWizardDialogFullTrack::~AudioWizardDialogFullTrack() {
	DestroyWindow();
}
#pragma endregion


////////////////////////////////////////////
// * DIALOG FULL-TRACK - INITIALIZATION * //
////////////////////////////////////////////
#pragma region Dialog Full Track - Initialization
void AudioWizardDialogFullTrack::InitDarkMode() {
	// There is a bug when using dark mode and using the list control.
	// When using dark mode, the column header sorting will not work.
	// This is a workaround to ignore applying dark mode to the list control.
	// m_dark.AddDialogWithControls(*this); // Problematic
	// m_dark.AddControls(*this); // Problematic
	// m_dark.AddDialog(*this); // OK

	AWHDarkMode::AddDialog(*this);
	AWHDarkMode::AddControlsWithExclude(*this, GetDlgItem(IDC_ANALYSIS_LIST));
}

void AudioWizardDialogFullTrack::InitWindowTitle() {
	CStringW title;

	title.Format(L"Audio Wizard - Analysis Results - Time: %s \u00B7 Speed: %s",
		data.processingTime.GetString(), data.processingSpeed.GetString()
	);

	SetWindowText(title);
}

void AudioWizardDialogFullTrack::InitColumnsMetadata() {
	data.columns.clear();

	auto formatMetric = [](double value) {
		return value == -INFINITY ? CStringW(L"-inf") : CStringW(pfc::format_float(value, 0, 1));
	};

	const std::array<ColumnMetadata, 26> columnConfigs = {{
		{ INDEX_FT, L"", AudioWizardSettings::analysisDisplayIndex, true, false, nullptr, [](const auto&) {
			return L""; }
		},
		{ ARTIST_FT, L"", AudioWizardSettings::analysisDisplayArtist, false, false, nullptr, [](const auto& r) {
			return r.artist.c_str(); }
		},
		{ ALBUM_FT, L"", AudioWizardSettings::analysisDisplayAlbum, false, false, nullptr, [](const auto& r) {
			return r.album.c_str(); }
		},
		{ TRACK_FT, L"", AudioWizardSettings::analysisDisplayTrack, false, false, nullptr, [](const auto& r) {
			return r.title.c_str(); }
		},
		{ DURATION_FT, L"", AudioWizardSettings::analysisDisplayDuration, false, false, nullptr, [](const auto& r) {
			return r.duration.c_str(); }
		},
		{ YEAR_FT, L"", AudioWizardSettings::analysisDisplayYear, false, false, nullptr, [](const auto& r) {
			return r.year.c_str(); }
		},
		{ GENRE_FT, L"", AudioWizardSettings::analysisDisplayGenre, false, false, nullptr, [](const auto& r) {
			return r.genre.c_str(); }
		},
		{ FORMAT_FT, L"", AudioWizardSettings::analysisDisplayFormat, false, false, nullptr, [](const auto& r) {
			return r.format.c_str(); }
		},
		{ CHANNELS_FT, L"", AudioWizardSettings::analysisDisplayChannels, false, false, nullptr, [](const auto& r) {
			return r.channels.c_str(); }
		},
		{ BIT_DEPTH_FT, L"", AudioWizardSettings::analysisDisplayBitDepth, false, false, nullptr, [](const auto& r) {
			return r.bitDepth.c_str(); }
		},
		{ BITRATE_FT, L"", AudioWizardSettings::analysisDisplayBitrate, false, false, nullptr, [](const auto& r) {
			return r.bitrate.c_str(); }
		},
		{ SAMPLE_RATE_FT, L"", AudioWizardSettings::analysisDisplaySampleRate, false, false, nullptr, [](const auto& r) {
			return r.sampleRate.c_str(); }
		},
		{ M_LUFS_FT, L"", AudioWizardSettings::analysisDisplayMomentaryLUFS, false, true, [](const auto& r) {
			return r.momentaryLUFS; }, [formatMetric](const auto& r) { return formatMetric(r.momentaryLUFS); }
		},
		{ S_LUFS_FT, L"", AudioWizardSettings::analysisDisplayShortTermLUFS, false, true, [](const auto& r) {
			return r.shortTermLUFS; }, [formatMetric](const auto& r) { return formatMetric(r.shortTermLUFS); }
		},
		{ I_LUFS_FT, L"", AudioWizardSettings::analysisDisplayIntegratedLUFS, false, true, [](const auto& r) {
			return r.integratedLUFS; }, [formatMetric](const auto& r) { return formatMetric(r.integratedLUFS); }
		},
		{ RMS_FT, L"", AudioWizardSettings::analysisDisplayRMS, false, true, [](const auto& r) {
			return r.RMS; }, [formatMetric](const auto& r) { return formatMetric(r.RMS); }
		},
		{ SP_FT, L"", AudioWizardSettings::analysisDisplaySamplePeak, false, true, [](const auto& r) {
			return r.samplePeak; }, [formatMetric](const auto& r) { return formatMetric(r.samplePeak); }
		},
		{ TP_FT, L"", AudioWizardSettings::analysisDisplayTruePeak, false, true, [](const auto& r) {
			return r.truePeak; }, [formatMetric](const auto& r) { return formatMetric(r.truePeak); }
		},
		{ PSR_FT, L"", AudioWizardSettings::analysisDisplayPSR, false, true, [](const auto& r) {
			return r.PSR; }, [formatMetric](const auto& r) { return formatMetric(r.PSR); }
		},
		{ PLR_FT, L"", AudioWizardSettings::analysisDisplayPLR, false, true, [](const auto& r) {
			return r.PLR; }, [formatMetric](const auto& r) { return formatMetric(r.PLR); }
		},
		{ CF_FT, L"", AudioWizardSettings::analysisDisplayCrestFactor, false, true, [](const auto& r) {
			return r.crestFactor; }, [formatMetric](const auto& r) { return formatMetric(r.crestFactor); }
		},
		{ LRA_FT, L"", AudioWizardSettings::analysisDisplayLoudnessRange, false, true, [](const auto& r) {
			return r.loudnessRange; }, [formatMetric](const auto& r) { return formatMetric(r.loudnessRange); }
		},
		{ DR_FT, L"", AudioWizardSettings::analysisDisplayDynamicRange, false, true, [](const auto& r) {
			return r.dynamicRange; }, [formatMetric](const auto& r) { return formatMetric(r.dynamicRange); }
		},
		{ DR_ALBUM_FT, L"", AudioWizardSettings::analysisDisplayDynamicRangeAlbum, false, true, [](const auto& r) {
			return r.dynamicRangeAlbum; }, [formatMetric](const auto& r) { return formatMetric(r.dynamicRangeAlbum); }
		},
		{ PD_FT, L"", AudioWizardSettings::analysisDisplayPureDynamics, false, true, [](const auto& r) {
			return r.pureDynamics; }, [formatMetric](const auto& r) { return formatMetric(r.pureDynamics); }
		},
		{ PD_ALBUM_FT, L"", AudioWizardSettings::analysisDisplayPureDynamicsAlbum, false, true, [](const auto& r) {
			return r.pureDynamicsAlbum; }, [formatMetric](const auto& r) { return formatMetric(r.pureDynamicsAlbum); }
		}
	}};

	for (const auto& config : columnConfigs) {
		if (config.setting) {
			data.columns.emplace_back(
				config.metricId, AudioWizardDialog::GetMetricName(config.metricId), false,
				config.isIndex, config.isNumeric, config.numericGetter, config.getter
			);
		}
	}
}

void AudioWizardDialogFullTrack::InitColumnsContent() {
	data.cachedColumnTexts.resize(data.columns.size());

	for (size_t col = 0; col < data.columns.size(); ++col) {
		const auto& column = data.columns[col];
		auto& cachedTexts = data.cachedColumnTexts[col];
		cachedTexts.clear();
		cachedTexts.reserve(data.results.size());

		for (size_t i = 0; i < data.results.size(); ++i) {
			if (column.isIndex) {
				pfc::string_formatter formatter;
				formatter << (i + 1);
				pfc::stringcvt::string_wide_from_utf8 converter(formatter.c_str());
				cachedTexts.push_back(CStringW(converter.get_ptr()));
			}
			else {
				cachedTexts.push_back(column.getter(data.results[i]));
			}
		}
	}
}

void AudioWizardDialogFullTrack::InitColumnsUI() {
	CClientDC dc(ui.listView);
	HFONT hFont = ui.listView.GetFont();
	AWHGraphics::GDISelector fontSel(dc, hFont);

	for (size_t i = 0; i < data.columns.size(); ++i) {
		int width = GetColumnWidth(dc, i);
		std::wstring columnName = AudioWizardDialog::GetMetricName(data.columns[i].metricId);
		int colIdx = ui.listView.InsertColumn(static_cast<int>(i), columnName.c_str(), LVCFMT_LEFT, width);

		// Store original column index in header's lParam
		HDITEM hdi = { 0 };
		hdi.mask = HDI_LPARAM;
		hdi.lParam = i;
		ui.headerCtrl.SetItem(colIdx, &hdi);
	}
}

void AudioWizardDialogFullTrack::InitListView() {
	ui.listView.Attach(GetDlgItem(IDC_ANALYSIS_LIST));
	ui.listView.GetWindowRect(ui.originalListRect);
	ScreenToClient(&ui.originalListRect);

	ui.listView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
	ui.headerCtrl = ui.listView.GetHeader();

	if (!data.results.empty()) {
		InitColumnsUI();
		PopulateList();
	}
}

void AudioWizardDialogFullTrack::InitButtons() {
	// Attach Group 1 buttons
	ui.openMonitorBtn.Attach(GetDlgItem(IDC_ANALYSIS_OPEN_MONITOR_BUTTON));
	ui.openMonitorBtn.GetWindowRect(ui.originalOpenMonitorRect);
	ScreenToClient(&ui.originalOpenMonitorRect);

	ui.configBtn.Attach(GetDlgItem(IDC_ANALYSIS_CONFIG_BUTTON));
	ui.configBtn.GetWindowRect(ui.originalConfigRect);
	ScreenToClient(&ui.originalConfigRect);

	// Attach Group 2 buttons and combos
	ui.writeTagsBtn.Attach(GetDlgItem(IDC_ANALYSIS_WRITE_TAGS_BUTTON));
	ui.writeTagsBtn.GetWindowRect(ui.originalWriteTagsButtonRect);
	ScreenToClient(&ui.originalWriteTagsButtonRect);

	ui.writeTagsCombo.Attach(GetDlgItem(IDC_ANALYSIS_WRITE_TAGS_COMBOBOX));
	ui.writeTagsCombo.GetWindowRect(ui.originalWriteTagsComboRect);
	ScreenToClient(&ui.originalWriteTagsComboRect);

	ui.clearTagsBtn.Attach(GetDlgItem(IDC_ANALYSIS_CLEAR_TAGS_BUTTON));
	ui.clearTagsBtn.GetWindowRect(ui.originalClearTagsButtonRect);
	ScreenToClient(&ui.originalClearTagsButtonRect);

	ui.clearTagsCombo.Attach(GetDlgItem(IDC_ANALYSIS_CLEAR_TAGS_COMBOBOX));
	ui.clearTagsCombo.GetWindowRect(ui.originalClearTagsComboRect);
	ScreenToClient(&ui.originalClearTagsComboRect);

	ui.setupTagsBtn.Attach(GetDlgItem(IDC_ANALYSIS_SETUP_TAGS_BUTTON));
	ui.setupTagsBtn.GetWindowRect(ui.originalSetupTagsButtonRect);
	ScreenToClient(&ui.originalSetupTagsButtonRect);

	// Attach Group 3 buttons
	ui.saveBtn.Attach(GetDlgItem(IDC_ANALYSIS_SAVE_BUTTON));
	ui.cancelBtn.Attach(GetDlgItem(IDC_ANALYSIS_CLOSE_BUTTON));
	ui.saveBtn.GetWindowRect(ui.originalButtonRect);
	ScreenToClient(ui.originalButtonRect);
}

void AudioWizardDialogFullTrack::InitDropDownMenus() {
	const std::vector<std::wstring> tagOptions = {
		L"All",
		AWHString::ToWide(AudioWizardSettings::analysisTagsLUFS),
		AWHString::ToWide(AudioWizardSettings::analysisTagsRMS),
		AWHString::ToWide(AudioWizardSettings::analysisTagsSP),
		AWHString::ToWide(AudioWizardSettings::analysisTagsTP),
		AWHString::ToWide(AudioWizardSettings::analysisTagsPSR),
		AWHString::ToWide(AudioWizardSettings::analysisTagsPLR),
		AWHString::ToWide(AudioWizardSettings::analysisTagsCF),
		AWHString::ToWide(AudioWizardSettings::analysisTagsLRA),
		AWHString::ToWide(AudioWizardSettings::analysisTagsDR),
		AWHString::ToWide(AudioWizardSettings::analysisTagsDRAlbum),
		AWHString::ToWide(AudioWizardSettings::analysisTagsPD),
		AWHString::ToWide(AudioWizardSettings::analysisTagsPDAlbum)
	};

	for (const auto& option : tagOptions) {
		ui.writeTagsCombo.AddString(option.c_str());
		ui.clearTagsCombo.AddString(option.c_str());
	}

	// Workaround fix for combo box height to be the same size as buttons
	WTL::CFont newFont;
	CSize fontSize = AWHConvert::DialogUnitsToPixel(m_hWnd, 0, 7);
	AWHDialog::CreateCustomFont(newFont, -fontSize.cy, FW_NORMAL, L"Segoe UI");
	ui.writeTagsCombo.SetFont(newFont, TRUE);
	ui.clearTagsCombo.SetFont(newFont, TRUE);
	newFont.Detach();

	// Set current selections
	ui.writeTagsCombo.SetCurSel(AudioWizardSettings::analysisTagsWriting);
	ui.clearTagsCombo.SetCurSel(AudioWizardSettings::analysisTagsClearing);
}

void AudioWizardDialogFullTrack::InitTooltips() {
	std::vector<std::pair<HWND, HWND>> tooltipControls;
	HWND hwndHeader = ui.headerCtrl.m_hWnd;

	if (::IsWindow(hwndHeader)) {
		tooltipControls.emplace_back(hwndHeader, hwndHeader);
	}

	AudioWizardDialog::SetTooltips(ui.tooltipCtrl, m_hWnd, &tooltipControls);
}

void AudioWizardDialogFullTrack::InitWindowSize() {
	const int group1Width = ui.originalOpenMonitorRect.Width() + ui.originalConfigRect.Width();

	const int group2Width =
		ui.originalWriteTagsButtonRect.Width() + ui.originalWriteTagsComboRect.Width() +
		ui.originalClearTagsButtonRect.Width() + ui.originalClearTagsComboRect.Width() +
		ui.originalSetupTagsButtonRect.Width();

	const int group3Width = 2 * ui.originalButtonRect.Width();

	const int totalGapMargin = 12 * Constants::BUTTON_MARGIN;
	const int minWidth = group1Width + group2Width + group3Width + totalGapMargin;
	const int minHeight = minWidth / 2;

	ui.minSize = CSize(minWidth, minHeight);
}
#pragma endregion


/////////////////////////////////////
// * DIALOG FULL-TRACK - METHODS * //
/////////////////////////////////////
#pragma region Dialog Full-Track - Methods
void AudioWizardDialogFullTrack::InitDialog() {
	ui.listView.DeleteAllItems();

	int colCount = ui.listView.GetHeader().GetItemCount();

	for (int i = colCount - 1; i >= 0; i--) {
		ui.listView.DeleteColumn(i);
	}

	InitColumnsMetadata();
	InitColumnsContent();
	InitListView();
}

int AudioWizardDialogFullTrack::GetColumnWidth(const CClientDC& dc, size_t columnIndex) const {
	const ColumnMetadata& column = data.columns[columnIndex];
	CSize maxExtent(0, 0);

	std::wstring columnName = AudioWizardDialog::GetMetricName(column.metricId);
	AWHText::MeasureText(dc, columnName.c_str(), maxExtent);

	for (const auto& text : data.cachedColumnTexts[columnIndex]) {
		CSize textSize;
		AWHText::MeasureText(dc, text, textSize);
		maxExtent.cx = std::max(maxExtent.cx, textSize.cx);
	}

	return maxExtent.cx + Constants::LIST_MARGIN;
}

void AudioWizardDialogFullTrack::PopulateList() {
	ui.listView.SetRedraw(FALSE);

	for (size_t i = 0; i < data.results.size(); ++i) {
		const int itemIdx = ui.listView.InsertItem(static_cast<int>(i), data.cachedColumnTexts[0][i]);
		ui.listView.SetItemData(itemIdx, static_cast<LPARAM>(i));

		for (size_t col = 1; col < data.columns.size(); ++col) {
			ui.listView.SetItemText(itemIdx, static_cast<int>(col), data.cachedColumnTexts[col][i]);
		}
	}

	ui.listView.SetRedraw(TRUE);
	ui.listView.Invalidate();
}

void AudioWizardDialogFullTrack::SetListViewPosition(const CRect& clientRect, int groupY) {
	int listLeft = Constants::LIST_MARGIN;
	int listWidth = clientRect.Width() - 2 * Constants::LIST_MARGIN;
	int listHeight = groupY - ui.originalListRect.top - Constants::LIST_MARGIN;

	ui.listView.SetWindowPos(nullptr,
		listLeft, ui.originalListRect.top,
		listWidth, listHeight,
		SWP_NOZORDER
	);
}

void AudioWizardDialogFullTrack::SetButtonMonitorPosition(const CRect&, int groupX, int groupY) {
	const int buttonHeight = ui.originalOpenMonitorRect.Height();
	const int yPos = groupY + (ui.originalButtonRect.Height() - buttonHeight) / 2;

	// Position Open Monitor
	ui.openMonitorBtn.SetWindowPos(nullptr, groupX, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

	// Position Configure button
	int configureX = groupX + ui.originalOpenMonitorRect.Width() + Constants::BUTTON_MARGIN;
	ui.configBtn.SetWindowPos(nullptr, configureX, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void AudioWizardDialogFullTrack::SetButtonTagsPosition(const CRect&, int groupX, int groupY) {
	static const int comboBoxOffsetFix = 1;
	int currentX = groupX;

	ui.writeTagsBtn.SetWindowPos(nullptr, currentX, groupY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	currentX += ui.originalWriteTagsButtonRect.Width() + Constants::BUTTON_MARGIN;
	ui.writeTagsCombo.SetWindowPos(nullptr, currentX, groupY + comboBoxOffsetFix, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	currentX += ui.originalWriteTagsComboRect.Width() + Constants::BUTTON_MARGIN;
	ui.clearTagsBtn.SetWindowPos(nullptr, currentX, groupY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	currentX += ui.originalClearTagsButtonRect.Width() + Constants::BUTTON_MARGIN;
	ui.clearTagsCombo.SetWindowPos(nullptr, currentX, groupY + comboBoxOffsetFix, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	currentX += ui.originalClearTagsComboRect.Width() + Constants::BUTTON_MARGIN;
	ui.setupTagsBtn.SetWindowPos(nullptr, currentX, groupY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void AudioWizardDialogFullTrack::SetButtonSaveClosePosition(const CRect& clientRect, int groupY) {
	int listRight = clientRect.Width() - Constants::LIST_MARGIN;
	int group3Width = 2 * ui.originalButtonRect.Width() + Constants::BUTTON_MARGIN;
	int groupX = listRight - group3Width;

	ui.saveBtn.SetWindowPos(nullptr, groupX, groupY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	groupX += ui.originalButtonRect.Width() + Constants::BUTTON_MARGIN;
	ui.cancelBtn.SetWindowPos(nullptr, groupX, groupY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void AudioWizardDialogFullTrack::UpdateColors(NMLVCUSTOMDRAW* pLVCD) {
	const int col = pLVCD->iSubItem;
	if (col < 0 || col >= static_cast<int>(data.columns.size())) return;

	const ColumnMetadata& column = data.columns[col];
	if (!column.isNumeric || column.metricId == INVALID) return;

	const auto row = static_cast<int>(pLVCD->nmcd.dwItemSpec);
	const auto originalIndex = static_cast<int>(ui.listView.GetItemData(row));
	if (originalIndex < 0 || originalIndex >= static_cast<int>(data.results.size())) return;

	pLVCD->clrText = AudioWizardDialog::GetColor(column.metricId, column.numericGetter(data.results[originalIndex]),
		AudioWizardSettings::analysisDisplayColors, true, AudioWizardDialog::MetricContext::FullTrack
	);
}

void AudioWizardDialogFullTrack::UpdateWindowLayout() {
	CRect clientRect;
	GetClientRect(&clientRect);

	const int group1Width =
		ui.originalOpenMonitorRect.Width() + ui.originalConfigRect.Width() + Constants::BUTTON_MARGIN;

	const int group2Width =
		ui.originalWriteTagsButtonRect.Width() + ui.originalWriteTagsComboRect.Width() +
		ui.originalClearTagsButtonRect.Width() + ui.originalClearTagsComboRect.Width() +
		ui.originalSetupTagsButtonRect.Width() + 4 * Constants::BUTTON_MARGIN;

	const int group3Width = 2 * ui.originalButtonRect.Width() + Constants::BUTTON_MARGIN;

	// Group 1
	const int groupX = Constants::LIST_MARGIN;
	const int groupY = clientRect.bottom - ui.originalButtonRect.Height() - Constants::LIST_MARGIN;
	SetButtonMonitorPosition(clientRect, groupX, groupY);
	const int group1Right = groupX + group1Width;

	// Group 3
	const int group3X = clientRect.Width() - group3Width - Constants::LIST_MARGIN;
	SetButtonSaveClosePosition(clientRect, groupY);

	// Group 2
	const int availableWidth = group3X - group1Right;
	const int gap = std::max(Constants::BUTTON_MARGIN, (availableWidth - group2Width) / 2);
	const int group2X = group1Right + gap;
	SetButtonTagsPosition(clientRect, group2X, groupY);

	// Update ListView
	SetListViewPosition(clientRect, groupY);
}

CStringA AudioWizardDialogFullTrack::WriteAnalysisResultsCsv(const std::vector<ColumnMetadata>& columns,
	const CStringW& processingTime, const CStringW& processingSpeed) const {

	CStringA content = AWHText::WriteFancyHeader(
		"AUDIO WIZARD - ANALYSIS RESULTS - ", CStringA(static_cast<const char*>(CW2A(CA2W(
			AWHString::FormatDate(std::chrono::system_clock::now()).c_str(), CP_UTF8), CP_UTF8
		)))
	);

	// Processing Time and Speed (as comments)
	content.AppendFormat("# Processing Time: %s\r\n# Processing Speed: %s\r\n\r\n",
		static_cast<const char*>(CW2A(processingTime, CP_UTF8)),
		static_cast<const char*>(CW2A(processingSpeed, CP_UTF8))
	);

	// CSV Column Headers
	CStringA headerLine;
	for (size_t col = 0; col < columns.size(); ++col) {
		CW2A headerA(columns[col].metricName.c_str(), CP_UTF8);
		CStringA escapedHeader;
		for (const char* p = headerA; *p; ++p) {
			if (*p == '"') escapedHeader.AppendChar('"');
			escapedHeader.AppendChar(*p);
		}
		headerLine.AppendFormat("\"%s\"", static_cast<const char*>(escapedHeader));
		if (col < columns.size() - 1) {
			headerLine.Append(",");
		}
	}
	content += headerLine + "\r\n";

	// CSV Data
	int itemCount = ui.listView.GetItemCount();
	for (int row = 0; row < itemCount; ++row) {
		CStringA line;
		for (size_t col = 0; col < columns.size(); ++col) {
			CStringW cellW;
			ui.listView.GetItemText(row, static_cast<int>(col), cellW.GetBuffer(256), 256);
			cellW.ReleaseBuffer();
			CW2A cellA(cellW, CP_UTF8);
			if (columns[col].isNumeric && cellW == L"-inf") {
				line.Append(cellA);
			}
			else if (!columns[col].isNumeric) {
				line.AppendFormat("\"%s\"", static_cast<const char*>(cellA));
			}
			else {
				line.Append(cellA);
			}
			if (col < columns.size() - 1) {
				line.Append(",");
			}
		}
		content += line + "\r\n";
	}

	return content;
}

CStringA AudioWizardDialogFullTrack::WriteAnalysisResultsTxt(const std::vector<ColumnMetadata>& columns,
	const CStringW& processingTime, const CStringW& processingSpeed) const {

	CStringA content = AWHText::WriteFancyHeader(
		"AUDIO WIZARD - ANALYSIS RESULTS - ", CStringA(static_cast<const char*>(CW2A(CA2W(
			AWHString::FormatDate(std::chrono::system_clock::now()).c_str(), CP_UTF8), CP_UTF8
		)))
	);

	// Processing Time and Speed
	content.AppendFormat("Processing Time: %s\r\nProcessing Speed: %s\r\n\r\n",
		static_cast<const char*>(CW2A(processingTime, CP_UTF8)),
		static_cast<const char*>(CW2A(processingSpeed, CP_UTF8))
	);

	// Collect data and calculate widths
	std::vector<CStringA> headersA;
	std::vector<int> colWidths(columns.size());
	std::vector<bool> isNumeric(columns.size());
	std::vector<std::vector<CStringA>> dataColumns(columns.size());
	int itemCount = ui.listView.GetItemCount();

	for (size_t col = 0; col < columns.size(); ++col) {
		headersA.emplace_back(CW2A(columns[col].metricName.c_str(), CP_UTF8));
		colWidths[col] = headersA.back().GetLength();
		isNumeric[col] = columns[col].isNumeric;
		dataColumns[col].reserve(itemCount);
	}

	for (int row = 0; row < itemCount; ++row) {
		for (size_t col = 0; col < columns.size(); ++col) {
			CStringW cellW;
			ui.listView.GetItemText(row, static_cast<int>(col), cellW.GetBuffer(256), 256);
			cellW.ReleaseBuffer();
			dataColumns[col].emplace_back(CW2A(cellW, CP_UTF8));
			colWidths[col] = std::max(colWidths[col], dataColumns[col].back().GetLength());
		}
	}

	// Header and Separator
	content += AWHText::FormatAlignedTextLine(headersA, colWidths, isNumeric);
	std::vector<CStringA> separator(columns.size());
	for (size_t col = 0; col < columns.size(); ++col) {
		separator[col] = CStringA('-', colWidths[col]);
	}
	content += AWHText::FormatAlignedTextLine(separator, colWidths, isNumeric);

	// Data Rows
	for (int row = 0; row < itemCount; ++row) {
		std::vector<CStringA> rowData(columns.size());
		for (size_t col = 0; col < columns.size(); ++col) {
			rowData[col] = dataColumns[col][row];
		}
		content += AWHText::FormatAlignedTextLine(rowData, colWidths, isNumeric);
	}

	return content;
}
#pragma endregion


////////////////////////////////////////////
// * DIALOG FULL-TRACK - EVENT HANDLERS * //
////////////////////////////////////////////
#pragma region Dialog Full-Track - Event Handlers
LRESULT AudioWizardDialogFullTrack::OnColumnClick(int, LPNMHDR pnmh, BOOL) {
	auto pNMLV = reinterpret_cast<LPNMLISTVIEW>(pnmh);
	const int column = pNMLV->iSubItem;

	if (column < 0 || column >= static_cast<int>(data.columns.size())) {
		return 0;
	}

	if (column == columnSort.sortColumn) {
		columnSort.ascending = !columnSort.ascending;
	}
	else {
		columnSort.sortColumn = column;
		columnSort.ascending = true;
	}

	ui.listView.SortItems([](LPARAM l1, LPARAM l2, LPARAM lpThis) {
		const auto* pThis = reinterpret_cast<AudioWizardDialogFullTrack*>(lpThis);
		const auto idx1 = static_cast<int>(l1);
		const auto idx2 = static_cast<int>(l2);
		const auto& col = pThis->data.columns[pThis->columnSort.sortColumn];

		if (col.isIndex) {
			return pThis->columnSort.ascending ? (idx1 - idx2) : (idx2 - idx1);
		}

		if (col.isNumeric) {
			const double v1 = col.numericGetter(pThis->data.results[idx1]);
			const double v2 = col.numericGetter(pThis->data.results[idx2]);
			const int cmp = (v1 < v2) ? -1 : (v1 > v2) ? 1 : 0;

			return pThis->columnSort.ascending ? cmp : -cmp;
		}

		const int cmp = pThis->data.cachedColumnTexts[pThis->columnSort.sortColumn][idx1]
			.CompareNoCase(pThis->data.cachedColumnTexts[pThis->columnSort.sortColumn][idx2]);

		return pThis->columnSort.ascending ? cmp : -cmp;

	}, reinterpret_cast<DWORD_PTR>(this));

	return 0;
}

LRESULT AudioWizardDialogFullTrack::OnListDoubleClick(int, LPNMHDR pnmh, BOOL) {
	auto const* pItem = reinterpret_cast<NMITEMACTIVATE*>(pnmh);
	if (pItem->iItem == -1) return 0; // No item selected

	auto originalIndex = static_cast<int>(ui.listView.GetItemData(pItem->iItem));
	const auto& result = data.results[originalIndex];
	metadb_handle_ptr track = result.handle;

	if (!track.is_valid()) {
		MessageBox(L"Track not found in library", L"Error", MB_ICONERROR);
		return 0;
	}

	playlist_manager::ptr playlist = playlist_manager::get();
	t_size index;

	if (!playlist->activeplaylist_find_item(track, index)) {
		metadb_handle_list tracks;
		tracks.add_item(track);
		playlist->activeplaylist_add_items(tracks, bit_array_true());
		index = playlist->activeplaylist_get_item_count() - 1;
	}

	t_size activePlaylist = playlist->get_active_playlist();
	playlist->playlist_set_focus_item(activePlaylist, index);
	playback_control::get()->start(playback_control::track_command_settrack, false);

	return 0;
}

LRESULT AudioWizardDialogFullTrack::OnNotify(UINT, WPARAM, LPARAM lParam, BOOL& bHandled) {
	auto* pnmh = reinterpret_cast<LPNMHDR>(lParam);

	// Clear the old tooltip
	if (pnmh->hwndFrom == ui.headerCtrl.m_hWnd) {
		ui.tooltipCtrl.Pop();
		bHandled = TRUE;
		return 0;
	}

	// Set the tooltip
	if (pnmh->code == TTN_GETDISPINFOW && pnmh->hwndFrom == ui.tooltipCtrl.m_hWnd) {
		auto* pInfo = reinterpret_cast<NMTTDISPINFOW*>(pnmh);
		if (pInfo->hdr.idFrom != reinterpret_cast<UINT_PTR>(ui.headerCtrl.m_hWnd)) return 0;

		CPoint pt;
		GetCursorPos(&pt);
		ui.headerCtrl.ScreenToClient(&pt);

		HDHITTESTINFO hdhti = { { pt.x, pt.y }, 0 };
		HDITEM hdi = { HDI_LPARAM };
		const int item = ui.headerCtrl.HitTest(&hdhti);

		if (item == -1 || !ui.headerCtrl.GetItem(item, &hdi)) {
			bHandled = TRUE;
			return 0;
		}

		const auto originalColumnIndex = static_cast<int>(hdi.lParam);

		if (originalColumnIndex < 0 || originalColumnIndex >= static_cast<int>(data.columns.size()) || !data.columns[originalColumnIndex].isNumeric) {
			static CStringW emptyTooltip;
			pInfo->lpszText = emptyTooltip.GetBuffer();
			bHandled = TRUE;
			return 0;
		}

		// Tooltip text
		static CStringW emptyTooltip;
		const auto metricId = data.columns[originalColumnIndex].metricId;

		if (AudioWizardSettings::analysisDisplayTooltips) {
			std::wstring fullName = AudioWizardDialog::GetMetricFullName(metricId);
			CStringW tooltipText = fullName.c_str();
			tooltipText += L"\nSEPARATOR\n";
			tooltipText += AudioWizardDialog::GetTooltips(metricId, AudioWizardDialog::MetricContext::FullTrack);
			pInfo->lpszText = tooltipText.GetBuffer();
		}
		else {
			pInfo->lpszText = emptyTooltip.GetBuffer();
		}

		// Tooltip color ranges
		if (AudioWizardSettings::analysisDisplayColors) {
			AudioWizardDialog::MetricKey key{ AudioWizardDialog::MetricContext::FullTrack, metricId };
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

		bHandled = TRUE;
		return 0;
	}

	// Set the colors in the list
	if (pnmh->code == NM_CUSTOMDRAW && pnmh->hwndFrom == ui.listView.m_hWnd) {
		auto* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pnmh);
		const DWORD drawStage = pLVCD->nmcd.dwDrawStage;

		if (drawStage == CDDS_PREPAINT) {
			return CDRF_NOTIFYITEMDRAW;
		}
		else if (drawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
			UpdateColors(pLVCD);
		}
		else if (drawStage == CDDS_ITEMPREPAINT) {
			return CDRF_NOTIFYSUBITEMDRAW;
		}

		return CDRF_DODEFAULT;
	}

	bHandled = FALSE;
	return 0;
}

BOOL AudioWizardDialogFullTrack::OnInitDialog(CWindow, LPARAM) {
	AudioWizardDialog::InitDarkMode();
	AudioWizardDialog::InitMetricConfig();

	InitDarkMode();
	InitListView();
	InitTooltips();
	InitButtons();
	InitDropDownMenus();
	InitWindowSize();
	InitWindowTitle();
	CenterWindow();

	return TRUE;
}

void AudioWizardDialogFullTrack::OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const {
	lpMMI->ptMinTrackSize = CPoint(ui.minSize.cx, ui.minSize.cy);
}

void AudioWizardDialogFullTrack::OnSize(UINT, CSize) {
	if (!ui.listView.IsWindow()) return;
	UpdateWindowLayout();
	Invalidate();
}

void AudioWizardDialogFullTrack::OnOpenMonitor(UINT, int, CWindow) const {
	auto dlg = std::make_unique<AudioWizardDialogRealTime>();
	auto* rawDlg = dlg.get();
	rawDlg->SetWindowLongPtr(GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dlg.release()));

	rawDlg->Create(core_api::get_main_window());
	rawDlg->CenterWindow();
	rawDlg->ShowWindow(SW_SHOW);
}

void AudioWizardDialogFullTrack::OnConfig(UINT, int, CWindow) {
	AudioWizardDialogFullTrackConfig dlg(*this);
	dlg.DoModal(m_hWnd);
}

void AudioWizardDialogFullTrack::OnWriteTags(UINT, int, CWindow) {
	int sel = ui.writeTagsCombo.GetCurSel();
	if (sel == CB_ERR) return;

	const std::vector<std::pair<std::string, std::function<double(const FullTrackResults&)>>> tagMapping = {
		{ AudioWizardSettings::analysisTagsLUFS.get().c_str(),    [](const auto& r) { return r.integratedLUFS; }},
		{ AudioWizardSettings::analysisTagsRMS.get().c_str(),     [](const auto& r) { return r.RMS; }},
		{ AudioWizardSettings::analysisTagsSP.get().c_str(),      [](const auto& r) { return r.samplePeak; }},
		{ AudioWizardSettings::analysisTagsTP.get().c_str(),      [](const auto& r) { return r.truePeak; }},
		{ AudioWizardSettings::analysisTagsPSR.get().c_str(),     [](const auto& r) { return r.PSR; }},
		{ AudioWizardSettings::analysisTagsPLR.get().c_str(),     [](const auto& r) { return r.PLR; }},
		{ AudioWizardSettings::analysisTagsCF.get().c_str(),      [](const auto& r) { return r.crestFactor; }},
		{ AudioWizardSettings::analysisTagsLRA.get().c_str(),     [](const auto& r) { return r.loudnessRange; }},
		{ AudioWizardSettings::analysisTagsDR.get().c_str(),      [](const auto& r) { return r.dynamicRange; }},
		{ AudioWizardSettings::analysisTagsDRAlbum.get().c_str(), [](const auto& r) { return r.dynamicRangeAlbum; }},
		{ AudioWizardSettings::analysisTagsPD.get().c_str(),      [](const auto& r) { return r.pureDynamics; }},
		{ AudioWizardSettings::analysisTagsPDAlbum.get().c_str(), [](const auto& r) { return r.pureDynamicsAlbum; }}
	};

	if (sel == 0) { // "All Tags"
		if (::MessageBox(m_hWnd, L"Write all tags?", L"Confirm Tag Writing", MB_YESNO | MB_ICONINFORMATION) != IDYES) {
			return;
		}
		auto completionHandler = [] {
			::MessageBox(core_api::get_main_window(), L"All Tags have been successfully written.", L"Success", MB_OK | MB_ICONINFORMATION);
		};
		AudioWizardTag::WriteMultipleTags(tagMapping, data.results, 1, 1, completionHandler);
	}
	else {
		const auto& [tag, getter] = tagMapping[static_cast<size_t>(sel) - 1];
		if (!tag.empty()) { // Skip if custom tag name is empty
			AudioWizardTag::WriteTag(tag.c_str(), data.results, getter, 1, 1);
		}
	}
}

void AudioWizardDialogFullTrack::OnWriteTagsComboSelChange(UINT, int, CWindow) const {
	int sel = ui.writeTagsCombo.GetCurSel();
	if (sel != CB_ERR) {
		AudioWizardSettings::analysisTagsWriting = sel;
	}
}

void AudioWizardDialogFullTrack::OnClearTags(UINT, int, CWindow) {
	int sel = ui.clearTagsCombo.GetCurSel();
	if (sel == CB_ERR) return;

	metadb_handle_list handles;
	for (const auto& result : data.results) {
		handles.add_item(result.handle.get_ptr());
	}

	const std::vector<std::string> tagNames = {
		AudioWizardSettings::analysisTagsLUFS.get().c_str(),
		AudioWizardSettings::analysisTagsRMS.get().c_str(),
		AudioWizardSettings::analysisTagsSP.get().c_str(),
		AudioWizardSettings::analysisTagsTP.get().c_str(),
		AudioWizardSettings::analysisTagsPSR.get().c_str(),
		AudioWizardSettings::analysisTagsPLR.get().c_str(),
		AudioWizardSettings::analysisTagsCF.get().c_str(),
		AudioWizardSettings::analysisTagsLRA.get().c_str(),
		AudioWizardSettings::analysisTagsDR.get().c_str(),
		AudioWizardSettings::analysisTagsDRAlbum.get().c_str(),
		AudioWizardSettings::analysisTagsPD.get().c_str(),
		AudioWizardSettings::analysisTagsPDAlbum.get().c_str()
	};

	if (sel == 0) { // "All Tags"
		if (::MessageBox(m_hWnd, L"Clear all tags?", L"Confirm Tag Clearing", MB_YESNO | MB_ICONWARNING) != IDYES) {
			return;
		}
		auto completionHandler = [] {
			::MessageBox(core_api::get_main_window(), L"All Tags have been successfully cleared.", L"Success", MB_OK | MB_ICONINFORMATION);
		};
		// Filter out empty tags
		std::vector<std::string> nonEmptyTags;
		for (const auto& tag : tagNames) {
			if (!tag.empty()) {
				nonEmptyTags.push_back(tag);
			}
		}
		AudioWizardTag::ClearMultipleTags(nonEmptyTags, handles, completionHandler);
	}
	else {
		const std::string& tag = tagNames[static_cast<size_t>(sel) - 1];
		if (!tag.empty()) { // Skip if custom tag name is empty
			AudioWizardTag::ClearTag(tag.c_str(), handles);
		}
	}
}

void AudioWizardDialogFullTrack::OnClearTagsComboSelChange(UINT, int, CWindow) const {
	int sel = ui.clearTagsCombo.GetCurSel();
	if (sel != CB_ERR) {
		AudioWizardSettings::analysisTagsClearing = sel;
	}
}

void AudioWizardDialogFullTrack::OnSetupTags(UINT, int, CWindow) {
	AudioWizardDialogFullTrackTagsConfig dlg(*this);
	dlg.DoModal(m_hWnd);
}

void AudioWizardDialogFullTrack::OnSave(UINT, int, CWindow) {
	CComPtr<IFileDialog> pFileDialog;
	if (FAILED(pFileDialog.CoCreateInstance(CLSID_FileSaveDialog))) {
		MessageBox(L"Failed to create file dialog", L"Error", MB_ICONERROR);
		return;
	}

	std::array fileTypes = {
		COMDLG_FILTERSPEC{ L"Text Files", L"*.txt" },
		COMDLG_FILTERSPEC{ L"CSV Files", L"*.csv" },
		COMDLG_FILTERSPEC{ L"All Files", L"*.*" }
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

	// Write formatted content
	CStringA content;
	if (filePath.Right(4).CompareNoCase(L".csv") == 0) {
		content = WriteAnalysisResultsCsv(data.columns, data.processingTime, data.processingSpeed);
	}
	else {
		content = WriteAnalysisResultsTxt(data.columns, data.processingTime, data.processingSpeed);
	}

	// Write to file
	HANDLE hFile = CreateFileW(
		filePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr
	);

	if (hFile == INVALID_HANDLE_VALUE) {
		CStringW msg;
		msg.Format(L"Error %d: Failed to create file", GetLastError());
		MessageBox(msg, L"Error", MB_ICONERROR);
		return;
	}

	DWORD written;
	BOOL writeResult = WriteFile(hFile, content.GetString(), content.GetLength(), &written, nullptr);
	CloseHandle(hFile);

	if (!writeResult) {
		CStringW msg;
		msg.Format(L"Error %d: Failed to write file", GetLastError());
		MessageBox(msg, L"Error", MB_ICONERROR);
	}
	else {
		MessageBox(L"Analysis results have been saved successfully!", L"Success", MB_OK | MB_ICONINFORMATION);
	}
}

void AudioWizardDialogFullTrack::OnClose(UINT, int, CWindow) {
	AudioWizard::Main()->StopFullTrackAnalysis();
	DestroyWindow();
}

void AudioWizardDialogFullTrack::OnNcDestroy() {
	SetMsgHandled(FALSE);

	std::unique_ptr<AudioWizardDialogFullTrack>(
		reinterpret_cast<AudioWizardDialogFullTrack*>(GetWindowLongPtr(GWLP_USERDATA))
	);
}
#pragma endregion


////////////////////////////////////////////
// * DIALOG FULL-TRACK CONFIG - METHODS * //
////////////////////////////////////////////
#pragma region Dialog Full-Track Config - Methods
AudioWizardDialogFullTrackConfig::AudioWizardDialogFullTrackConfig(AudioWizardDialogFullTrack& parent) :
	parent(parent) {
}

void AudioWizardDialogFullTrackConfig::InitControlsPosition() {
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

	if (ui.originalRects.contains(IDC_ANALYSIS_CFG_RESET_BUTTON)) {
		int buttonHeight = ui.originalRects[IDC_ANALYSIS_CFG_RESET_BUTTON].Height();
		ui.minSize.cy = std::max(ui.minSize.cy, AWHConvert::DialogUnitsToPixel(m_hWnd, 0, 300).cy + 3 * buttonHeight);
	}
}

void AudioWizardDialogFullTrackConfig::InitSettings() {
	AudioWizardSettings::InitAnalysisSettings(m_hWnd);
	AudioWizardSettings::GetAnalysisPreset(m_hWnd);
	hasChanged = false;
}

void AudioWizardDialogFullTrackConfig::SetSettings() {
	hasChanged = true;
	AudioWizardSettings::SetAnalysisSettings(m_hWnd);
	hasChanged = false;
}

void AudioWizardDialogFullTrackConfig::UpdateControlsPosition(const std::vector<int>& ids, int targetCenterX, int newY) {
	if (ids.empty() || ui.originalRects.empty()) return;

	// Calculate original horizontal bounds
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


///////////////////////////////////////////////////
// * DIALOG FULL-TRACK CONFIG - EVENT HANDLERS * //
///////////////////////////////////////////////////
#pragma region Dialog Full-Track Config - Event Handlers
BOOL AudioWizardDialogFullTrackConfig::OnInitDialog(CWindow, LPARAM) {
	AWHDarkMode::AddDialogWithControls(*this);

	InitControlsPosition();
	InitSettings();

	return TRUE;
}

void AudioWizardDialogFullTrackConfig::OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const {
	lpMMI->ptMinTrackSize = CPoint(ui.minSize.cx, ui.minSize.cy);
}

void AudioWizardDialogFullTrackConfig::OnSize(UINT, CSize size) {
	if (ui.originalClientRect.IsRectEmpty()) return;

	const int cx = size.cx;
	const int cy = size.cy;
	const int buttonHeight = ui.originalRects[IDC_ANALYSIS_CFG_RESET_BUTTON].Height();

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

void AudioWizardDialogFullTrackConfig::OnSettingChanged(UINT, int, CWindow) {
	SetSettings();
}

void AudioWizardDialogFullTrackConfig::OnMetadataPresetChanged(UINT, int, CWindow) {
	int sel = AWHDialog::GetDropDownIndex(m_hWnd, IDC_ANALYSIS_CFG_METADATA_PRESET);
	if (sel == CB_ERR) return;

	AudioWizardSettings::SetAnalysisMetadataPreset(sel);

	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_INDEX, AudioWizardSettings::analysisDisplayIndex);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_ARTIST, AudioWizardSettings::analysisDisplayArtist);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_ALBUM, AudioWizardSettings::analysisDisplayAlbum);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_TRACK, AudioWizardSettings::analysisDisplayTrack);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_DURATION, AudioWizardSettings::analysisDisplayDuration);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_YEAR, AudioWizardSettings::analysisDisplayYear);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_GENRE, AudioWizardSettings::analysisDisplayGenre);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_FORMAT, AudioWizardSettings::analysisDisplayFormat);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_CHANNELS, AudioWizardSettings::analysisDisplayChannels);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_BIT_DEPTH, AudioWizardSettings::analysisDisplayBitDepth);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_BITRATE, AudioWizardSettings::analysisDisplayBitrate);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_SAMPLE_RATE, AudioWizardSettings::analysisDisplaySampleRate);

	hasChanged = true;
}

void AudioWizardDialogFullTrackConfig::OnMetricsPresetChanged(UINT, int, CWindow) {
	int sel = AWHDialog::GetDropDownIndex(m_hWnd, IDC_ANALYSIS_CFG_METRICS_PRESET);
	if (sel == CB_ERR) return;

	AudioWizardSettings::SetAnalysisMetricsPreset(sel);

	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_MOMENTARY_LUFS, AudioWizardSettings::analysisDisplayMomentaryLUFS);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_SHORT_TERM_LUFS, AudioWizardSettings::analysisDisplayShortTermLUFS);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_INTEGRATED_LUFS, AudioWizardSettings::analysisDisplayIntegratedLUFS);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_LOUDNESS_RANGE, AudioWizardSettings::analysisDisplayLoudnessRange);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_RMS, AudioWizardSettings::analysisDisplayRMS);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_SAMPLE_PEAK, AudioWizardSettings::analysisDisplaySamplePeak);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_TRUE_PEAK, AudioWizardSettings::analysisDisplayTruePeak);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_PSR, AudioWizardSettings::analysisDisplayPSR);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_PLR, AudioWizardSettings::analysisDisplayPLR);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_CREST_FACTOR, AudioWizardSettings::analysisDisplayCrestFactor);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_DYNAMIC_RANGE, AudioWizardSettings::analysisDisplayDynamicRange);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_DYNAMIC_RANGE_ALBUM, AudioWizardSettings::analysisDisplayDynamicRangeAlbum);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_PURE_DYNAMICS, AudioWizardSettings::analysisDisplayPureDynamics);
	AWHDialog::SetCheckBox(m_hWnd, IDC_ANALYSIS_CFG_PURE_DYNAMICS_ALBUM, AudioWizardSettings::analysisDisplayPureDynamicsAlbum);

	hasChanged = true;
}

void AudioWizardDialogFullTrackConfig::OnReset(UINT, int, CWindow) {
	AudioWizardSettings::ResetAnalysisSettings();
	InitSettings();
}

void AudioWizardDialogFullTrackConfig::OnApply(UINT, int, CWindow) {
	SetSettings();
	parent.InitDialog();
}

void AudioWizardDialogFullTrackConfig::OnClose(UINT, int, CWindow) {
	EndDialog(IDCANCEL);
}
#pragma endregion


/////////////////////////////////////////////////
// * DIALOG FULL-TRACK TAGS CONFIG - METHODS * //
/////////////////////////////////////////////////
#pragma region Dialog Full-Track Tags Config - Methods
AudioWizardDialogFullTrackTagsConfig::AudioWizardDialogFullTrackTagsConfig(AudioWizardDialogFullTrack& parent) :
	parent(parent) {
}

void AudioWizardDialogFullTrackTagsConfig::InitControlsPosition() {
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

	if (ui.originalRects.contains(IDC_ANALYSIS_TAGS_CFG_RESET_BUTTON)) {
		int buttonHeight = ui.originalRects[IDC_ANALYSIS_TAGS_CFG_RESET_BUTTON].Height();
		ui.minSize.cy = std::max(ui.minSize.cy, AWHConvert::DialogUnitsToPixel(m_hWnd, 0, 300).cy + 3 * buttonHeight);
	}
}

void AudioWizardDialogFullTrackTagsConfig::InitSettings() {
	AudioWizardSettings::InitAnalysisTagsSettings(m_hWnd);
	hasChanged = false;
}

void AudioWizardDialogFullTrackTagsConfig::SetSettings() {
	hasChanged = true;
	AudioWizardSettings::SetAnalysisTagsSettings(m_hWnd);
	hasChanged = false;
}

void AudioWizardDialogFullTrackTagsConfig::UpdateControlsPosition(const std::vector<int>& ids, int targetCenterX, int newY) {
	if (ids.empty() || ui.originalRects.empty()) return;

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

	const int offsetX = targetCenterX - (minX + (maxX - minX) / 2);

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


////////////////////////////////////////////////////////
// * DIALOG FULL-TRACK TAGS CONFIG - EVENT HANDLERS * //
////////////////////////////////////////////////////////
#pragma region Dialog Full-Track Tags Config - Event Handlers
BOOL AudioWizardDialogFullTrackTagsConfig::OnInitDialog(CWindow, LPARAM) {
	AWHDarkMode::AddDialogWithControls(*this);

	InitControlsPosition();
	InitSettings();

	// Set the "Apply" button as the default button
	// This allows applying input field changes on pressing "Enter"
	SendMessage(DM_SETDEFID, IDC_ANALYSIS_TAGS_CFG_APPLY_BUTTON, 0);

	return TRUE;
}

void AudioWizardDialogFullTrackTagsConfig::OnGetMinMaxInfo(LPMINMAXINFO lpMMI) const {
	lpMMI->ptMinTrackSize = CPoint(ui.minSize.cx, ui.minSize.cy);
}

void AudioWizardDialogFullTrackTagsConfig::OnSize(UINT, CSize size) {
	if (ui.originalClientRect.IsRectEmpty()) return;

	const int cx = size.cx;
	const int cy = size.cy;
	const int buttonHeight = ui.originalRects[IDC_ANALYSIS_TAGS_CFG_RESET_BUTTON].Height();

	if (ui.originalRects.count(0)) {
		CRect newGroupRect = ui.originalRects[0];
		newGroupRect.right = cx - (ui.originalClientRect.Width() - newGroupRect.right);
		newGroupRect.bottom = cy - 3 * buttonHeight;

		CWindow groupBox = GetDlgItem(0);
		groupBox.SetWindowPos(nullptr, newGroupRect.left, newGroupRect.top,
			newGroupRect.Width(), newGroupRect.Height(), SWP_NOZORDER);

		UpdateControlsPosition(controlIds, newGroupRect.CenterPoint().x, -1);
	}

	UpdateControlsPosition(buttonIds, cx / 2, cy - 2 * buttonHeight);
}

void AudioWizardDialogFullTrackTagsConfig::OnSettingChanged(UINT, int, CWindow) {
	SetSettings();
}

void AudioWizardDialogFullTrackTagsConfig::OnReset(UINT, int, CWindow) {
	AudioWizardSettings::ResetAnalysisTagsSettings();
	InitSettings();
}

void AudioWizardDialogFullTrackTagsConfig::OnApply(UINT, int, CWindow) {
	SetSettings();
}

void AudioWizardDialogFullTrackTagsConfig::OnClose(UINT, int, CWindow) {
	EndDialog(IDCANCEL);
}
