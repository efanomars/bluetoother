/*
 * Copyright © 2017-2020  Stefano Marsili, <stemars@gmx.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   tootherwindow.h
 */

#ifndef STMI_TOOTHER_WINDOW_H
#define STMI_TOOTHER_WINDOW_H

#include "toothersources.h"
#include "jsoncommon.h"
#include "namedialog.h"

#include <gtkmm.h>

#include <string>

#include <stdint.h>

namespace stmi
{

class TootherWindow : public Gtk::Window
{
public:
	TootherWindow(const std::string& sTitle, int nCmdPipeFD, int nReturnPipeFD
				, bool bReadOnly, bool bNoRestrictedWarning) noexcept;

private:
	void onNotebookSwitchPage(Gtk::Widget*, guint nPageNum) noexcept;

	void onAdapterSelectionChanged() noexcept;
	void onSoftwareEnabledToggled() noexcept;

	void onAdapterIsUpToggled() noexcept;
	void onAdapterDetectableToggled() noexcept;
	void onAdapterConnectableToggled() noexcept;
	void onServiceRunningToggled() noexcept;
	void onServiceEnabledToggled() noexcept;

	void onButtonRefresh() noexcept;
	void onButtonSetName() noexcept;

	void doReceiveString(bool bError, const std::string& sStr) noexcept;

	void startRefreshing(int32_t nMillisec) noexcept;
	void startTimeout(int32_t nStamp, int32_t nMillisec) noexcept;
	void onTimeout(int32_t nStamp, int32_t nMillisec) noexcept;

	void setWidgetsValue() noexcept;
	void setWidgetsSensitivity() noexcept;
	void execAdapterBoolCmd(const std::string& sCmd, bool bEnabled, int32_t nTimeout) noexcept;
	void execAdapterStringCmd(const std::string& sCmd, const std::string& sStr, int32_t nTimeout) noexcept;
	void sendCmd(const std::string& sStr) noexcept;

	void cursorToHourglass() noexcept;
	void cursorToNormal() noexcept;

	void printStringToLog(const std::string& sStr) noexcept;

	void quitOnFatalError(const std::string& sErr) noexcept;

	void onWindowRealize() noexcept;
	void showReadOnlyWarning() noexcept;
private:
	bool m_bReadOnly;
	bool m_bNoRestrictedWarning;
	int m_nCmdPipeFD;

	Glib::RefPtr<PipeInputSource> m_refPipeInputSource;

	static constexpr const int32_t s_nTotPages = 3;
	int32_t m_aPageIndex[s_nTotPages];

	//Gtk::Notebook* m_p0NotebookChoices;

		static constexpr const int32_t s_nTabMain = 0;
		//Gtk::Label* m_p0TabLabelMain;
		Gtk::Box* m_p0VBoxMain;

			//Gtk::Box* m_p0HBoxRefreshAdapters;
				Gtk::Button* m_p0ButtonRefresh;
				//Gtk::Box* m_p0VBoxTreeAndCurrentAdapter
					Gtk::TreeView* m_p0TreeViewAdapters;
					//Gtk::Box* m_p0VBoxCurrentAdapter
						Gtk::Label* m_p0LabelCurrentAdapter;
						Gtk::Label* m_p0LabelCurrentAddress;

			Gtk::CheckButton* m_p0CheckButtonHardwareEnabled;
			Gtk::CheckButton* m_p0CheckButtonSoftwareEnabled;

			Gtk::CheckButton* m_p0CheckButtonAdapterIsUp;

			//Gtk::Box* m_p0HBoxLocalName;
				Gtk::Button* m_p0ButtonAdapterSetLocalName;
				Gtk::Entry* m_p0EntryAdapterLocalName;

			Gtk::CheckButton* m_p0CheckButtonAdapterConnectable;
			Gtk::CheckButton* m_p0CheckButtonAdapterDetectable;

			Gtk::CheckButton* m_p0CheckButtonServiceRunning;
			Gtk::CheckButton* m_p0CheckButtonServiceEnabled;

		static constexpr const int32_t s_nTabLog = 1;
		//Gtk::Label* m_p0TabLabelLog;
		//Gtk::ScrolledWindow* m_p0ScrolledLog;
			Gtk::TextView* m_p0TextViewLog;

		static constexpr const int32_t s_nTabInfo = 2;
		//Gtk::Label* m_p0TabLabelInfo;
		//Gtk::ScrolledWindow* m_p0ScrolledInfo;
			//Gtk::Label* m_p0LabelInfoText;

	class AdaptersColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		AdaptersColumns() noexcept
		{
			add(m_oColHciName);
			add(m_oColHciId);
			add(m_oColHiddenHciId);
		}
		Gtk::TreeModelColumn<std::string> m_oColHciName;
		Gtk::TreeModelColumn<std::string> m_oColHciId;
		Gtk::TreeModelColumn<int32_t> m_oColHiddenHciId;
	};
	AdaptersColumns m_oAdaptersColumns;
	Glib::RefPtr<Gtk::TreeStore> m_refTreeModelAdapters;

	Glib::RefPtr<Gtk::TextBuffer> m_refTextBufferLog;
	Glib::RefPtr<Gtk::TextBuffer::Mark> m_refTextBufferMarkBottom;
	int32_t m_nTextBufferLogTotLines;
	static constexpr const int32_t s_nTextBufferLogMaxLines = 100;

	Glib::RefPtr<Gtk::TextBuffer> m_refTextBufferInfo;

	Glib::RefPtr<NameDialog> m_refNameDialog;

	int32_t m_nSelectedHciId;

	int32_t m_nStamp;
	bool m_bWaitingForReturn;
	bool m_bWaitingForState; // (m_bWaitingForState == true) implies (m_bWaitingForReturn == true)
	json m_oCurState;
	bool m_bSettingWidgetsValues;

	Glib::RefPtr<Gdk::Cursor> m_refStdCursor;
	Glib::RefPtr<Gdk::Cursor> m_refWatchCursor;

	static constexpr const int32_t s_nInitialWindowSizeW = 350;
	static constexpr const int32_t s_nInitialWindowSizeH = 230;

	static constexpr const int32_t s_nMaxLocalNameSize = 100;
	static constexpr const int32_t s_nTruncLocalNameSize = 25;

	static constexpr const int32_t s_nCmdRefreshTimeout = 3000; // millisec
	static constexpr const int32_t s_nCmdSetAdapterBoolTimeout = 2000; // millisec
	static constexpr const int32_t s_nCmdSetAdapterNameTimeout = 2000; // millisec
	static constexpr const int32_t s_nCmdSetServiceRunningTimeout = 5000; // millisec
	static constexpr const int32_t s_nCmdSetServiceEnabledTimeout = 4000; // millisec
	static constexpr const int32_t s_nCmdTurnAllOnTimeout = 10000; // millisec
private:
	TootherWindow() = delete;
};

} // namespace stmi

#endif /* STMI_TOOTHER_WINDOW_H */

