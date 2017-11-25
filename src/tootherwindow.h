/*
 * Copyright © 2017  Stefano Marsili, <stemars@gmx.ch>
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   tootherwindow.h
 */

#ifndef STMI_TOOTHER_WINDOW_H
#define STMI_TOOTHER_WINDOW_H

#include "btservice.h"
#include "hcisocket.h"

#include <gtkmm.h>

#include <string>

namespace stmi
{

class TootherWindow : public Gtk::Window
{
public:
	TootherWindow(const std::string& sTitle, BtService& oBtService, HciSocket& oHciSocket);
	virtual ~TootherWindow();

private:
	void onNotebookSwitchPage(Gtk::Widget*, guint nPageNum);

	void onAdapterSelectionChanged();
	void onSoftwareEnabledToggled();

	void onAdapterIsUpToggled();
	bool onAdapterLocalNameChangedFocus(GdkEventFocus* /*p0Event*/);
	void onAdapterDetectableToggled();
	void onAdapterConnectableToggled();
	void onServiceRunningToggled();
	void onServiceEnabledToggled();

	void onButtonRefresh();
	void onButtonTurnAllOn();

	void startRefreshing(int32_t nMillisec);
	void onTimeout();
	void refresh();

	void setSensitivityForState();

	void regenerateAdaptersModel();

	void printStringToLog(const std::string& sStr);

private:
	BtService& m_oBtService;
	HciSocket& m_oHciSocket;

	static constexpr int32_t s_nTotPages = 3;
	int32_t m_aPageIndex[s_nTotPages];

	//Gtk::Notebook* m_p0NotebookChoices;

		static const int32_t s_nTabMain = 0;
		//Gtk::Label* m_p0TabLabelMain;
		Gtk::Box* m_p0VBoxMain;

			//Gtk::Box* m_p0HBoxRefreshAdapters;
				Gtk::Button* m_p0ButtonRefresh;
				Gtk::TreeView* m_p0TreeViewAdapters;

			Gtk::Label* m_p0LabelCurrentAdapter;
			Gtk::Label* m_p0LabelCurrentAddress;

			Gtk::CheckButton* m_p0CheckButtonHardwareEnabled;
			Gtk::CheckButton* m_p0CheckButtonSoftwareEnabled;

			Gtk::CheckButton* m_p0CheckButtonAdapterIsUp;

			//Gtk::Box* m_p0HBoxLocalName;
				//Gtk::Label* m_p0LabelLocalName;
				Gtk::Entry* m_p0EntryAdapterLocalName;

			Gtk::CheckButton* m_p0CheckButtonAdapterConnectable;
			Gtk::CheckButton* m_p0CheckButtonAdapterDetectable;

			Gtk::CheckButton* m_p0CheckButtonServiceRunning;
			Gtk::CheckButton* m_p0CheckButtonServiceEnabled;

			Gtk::Button* m_p0ButtonTurnAllOn;

		static const int32_t s_nTabLog = 1;
		//Gtk::Label* m_p0TabLabelLog;
		//Gtk::ScrolledWindow* m_p0ScrolledLog;
			Gtk::TextView* m_p0TextViewLog;

		static const int32_t s_nTabInfo = 2;
		//Gtk::Label* m_p0TabLabelInfo;
		//Gtk::ScrolledWindow* m_p0ScrolledInfo;
			//Gtk::Label* m_p0LabelInfoText;

	class AdaptersColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
		AdaptersColumns()
		{
			add(m_oColHciName);
			add(m_oColHiddenHciId);
		}
		Gtk::TreeModelColumn<std::string> m_oColHciName;
		Gtk::TreeModelColumn<int32_t> m_oColHiddenHciId;
	};
	AdaptersColumns m_oAdaptersColumns;
	Glib::RefPtr<Gtk::TreeStore> m_refTreeModelAdapters;

	Glib::RefPtr<Gtk::TextBuffer> m_refTextBufferLog;
	Glib::RefPtr<Gtk::TextBuffer::Mark> m_refTextBufferMarkBottom;
	int32_t m_nTextBufferLogTotLines;
	static constexpr int32_t s_nTextBufferLogMaxLines = 100;

	Glib::RefPtr<Gtk::TextBuffer> m_refTextBufferInfo;

	int32_t m_nSelectedHciId;

	bool m_bNeedsRefreshing;
	bool m_bNeedsEnablingEverything;

	bool m_bRefreshing;
	bool m_bRegeneratingAdaptersModel;

	Glib::RefPtr<Gdk::Cursor> m_refStdCursor;
	Glib::RefPtr<Gdk::Cursor> m_refWatchCursor;

	static constexpr int32_t s_nInitialWindowSizeW = 350;
	static constexpr int32_t s_nInitialWindowSizeH = 250;

	static constexpr int32_t s_nMaxLocalNameSize = 200;

	static constexpr int32_t s_nRefreshingWaitServiceRunning = 2500; // millisec
	static constexpr int32_t s_nRefreshingWaitServiceEnabling = 500; // millisec
private:
	TootherWindow() = delete;
};

} // namespace stmi

#endif /* STMI_TOOTHER_WINDOW_H */

