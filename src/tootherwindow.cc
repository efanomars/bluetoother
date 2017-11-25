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
 * File:   tootherwindow.cc
 */

#include "tootherwindow.h"

#include "config.h"

#include <cassert>
#include <iostream>

namespace stmi
{

TootherWindow::TootherWindow(const std::string& sTitle, BtService& oBtService, HciSocket& oHciSocket)
: m_oBtService(oBtService)
, m_oHciSocket(oHciSocket)
//, m_p0NotebookChoices(nullptr)
//, m_p0TabLabelMain(nullptr)
, m_p0VBoxMain(nullptr)
//, m_p0HBoxRefreshAdapters(nullptr)
, m_p0ButtonRefresh(nullptr)
, m_p0TreeViewAdapters(nullptr)
, m_p0LabelCurrentAdapter(nullptr)
, m_p0LabelCurrentAddress(nullptr)
, m_p0CheckButtonHardwareEnabled(nullptr)
, m_p0CheckButtonSoftwareEnabled(nullptr)
, m_p0CheckButtonAdapterIsUp(nullptr)
//, m_p0HBoxLocalName(nullptr)
//, m_p0LabelLocalName(nullptr)
, m_p0EntryAdapterLocalName(nullptr)
, m_p0CheckButtonAdapterConnectable(nullptr)
, m_p0CheckButtonAdapterDetectable(nullptr)
, m_p0CheckButtonServiceRunning(nullptr)
, m_p0CheckButtonServiceEnabled(nullptr)
, m_p0ButtonTurnAllOn(nullptr)
//, m_p0TabLabelLog(nullptr)
//, m_p0ScrolledLog(nullptr)
, m_p0TextViewLog(nullptr)
//, m_p0TabLabelInfo(nullptr)
//, m_p0ScrolledInfo(nullptr)
//, m_p0LabelInfoText(nullptr)
, m_nTextBufferLogTotLines(0)
, m_nSelectedHciId(-1)
, m_bNeedsRefreshing(false)
, m_bNeedsEnablingEverything(false)
, m_bRefreshing(false)
, m_bRegeneratingAdaptersModel(false)
{
	//
	set_title(sTitle);
	set_default_size(s_nInitialWindowSizeW, s_nInitialWindowSizeH);
	set_resizable(true);

	const std::string sInitialLogText = "";

	const std::string sInfoText =
			"bluetoother\n"
			"===========\n"
			"\n"
			"Version: " + Config::getVersionString() + "\n\n"
			"Copyright © 2017  Stefano Marsili, <stemars@gmx.ch>.\n"
			"Further code copyright holders can be found in the sources.\n\n"
			"Bluetoother is a GUI tool to start/stop the bluetooth adapter\n"
			"and the related systemd service. It also allows to set the\n"
			"local name, the detectability and the connectability of the device.\n\n"
			"It provides some of the functionality of the command line tools\n"
			"rfkill, hciconfig and sytemctl.\n\n"
			"The interface of this program is not reactive. If the settings\n"
			"are changed from another program you need to press the 'Refresh'\n"
			"button to see them. The Log tab mostly only shows errors.";

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection;
	Pango::FontDescription oMonoFont("Mono");

	Gtk::Notebook* m_p0NotebookChoices = Gtk::manage(new Gtk::Notebook());
	Gtk::Window::add(*m_p0NotebookChoices);
		m_p0NotebookChoices->signal_switch_page().connect(
						sigc::mem_fun(*this, &TootherWindow::onNotebookSwitchPage) );

	Gtk::Label* m_p0TabLabelMain = Gtk::manage(new Gtk::Label("Main"));
	m_p0VBoxMain = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	m_aPageIndex[s_nTabMain] = m_p0NotebookChoices->append_page(*m_p0VBoxMain, *m_p0TabLabelMain);
		m_p0VBoxMain->set_spacing(4);

		Gtk::Box* m_p0HBoxRefreshAdapters = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxMain->pack_start(*m_p0HBoxRefreshAdapters, false, false);
			m_p0HBoxRefreshAdapters->set_spacing(6);
			m_p0ButtonRefresh = Gtk::manage(new Gtk::Button("Refresh"));
			m_p0HBoxRefreshAdapters->pack_start(*m_p0ButtonRefresh, false, false);
				m_p0ButtonRefresh->signal_clicked().connect(
								sigc::mem_fun(*this, &TootherWindow::onButtonRefresh) );

			m_refTreeModelAdapters = Gtk::TreeStore::create(m_oAdaptersColumns);
			m_p0TreeViewAdapters = Gtk::manage(new Gtk::TreeView(m_refTreeModelAdapters));
			m_p0HBoxRefreshAdapters->pack_start(*m_p0TreeViewAdapters, true, true);
				m_p0TreeViewAdapters->append_column("Adapter id", m_oAdaptersColumns.m_oColHciName);
				refTreeSelection = m_p0TreeViewAdapters->get_selection();
				refTreeSelection->signal_changed().connect(
								sigc::mem_fun(*this, &TootherWindow::onAdapterSelectionChanged));

		m_p0LabelCurrentAdapter = Gtk::manage(new Gtk::Label("Bluetooth adapter:"));
		m_p0VBoxMain->pack_start(*m_p0LabelCurrentAdapter, false, false);
		m_p0LabelCurrentAddress = Gtk::manage(new Gtk::Label("Address:"));
		m_p0VBoxMain->pack_start(*m_p0LabelCurrentAddress, false, false);

		m_p0CheckButtonHardwareEnabled = Gtk::manage(new Gtk::CheckButton("Hardware enabled (RfKill)"));
		m_p0VBoxMain->pack_start(*m_p0CheckButtonHardwareEnabled, false, false);
			m_p0CheckButtonHardwareEnabled->set_sensitive(false);
		m_p0CheckButtonSoftwareEnabled = Gtk::manage(new Gtk::CheckButton("Software enabled"));
		m_p0VBoxMain->pack_start(*m_p0CheckButtonSoftwareEnabled, false, false);
			m_p0CheckButtonSoftwareEnabled->signal_toggled().connect(
							sigc::mem_fun(*this, &TootherWindow::onSoftwareEnabledToggled) );

		m_p0CheckButtonAdapterIsUp = Gtk::manage(new Gtk::CheckButton("Adapter is up"));
		m_p0VBoxMain->pack_start(*m_p0CheckButtonAdapterIsUp, false, false);
			m_p0CheckButtonAdapterIsUp->set_margin_top(6);
			m_p0CheckButtonAdapterIsUp->signal_toggled().connect(
							sigc::mem_fun(*this, &TootherWindow::onAdapterIsUpToggled) );

		Gtk::Box* m_p0HBoxLocalName = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxMain->pack_start(*m_p0HBoxLocalName, false, false);
			Gtk::Label* m_p0LabelLocalName = Gtk::manage(new Gtk::Label("Local name:"));
			m_p0HBoxLocalName->pack_start(*m_p0LabelLocalName, false, false);
			m_p0EntryAdapterLocalName = Gtk::manage(new Gtk::Entry());
			m_p0HBoxLocalName->pack_start(*m_p0EntryAdapterLocalName, true, true);
				m_p0EntryAdapterLocalName->signal_focus_out_event().connect(
								sigc::mem_fun(*this, &TootherWindow::onAdapterLocalNameChangedFocus) );

		m_p0CheckButtonAdapterConnectable = Gtk::manage(new Gtk::CheckButton("Adapter connectable"));
		m_p0VBoxMain->pack_start(*m_p0CheckButtonAdapterConnectable, false, false);
			m_p0CheckButtonAdapterConnectable->signal_toggled().connect(
							sigc::mem_fun(*this, &TootherWindow::onAdapterConnectableToggled) );
		m_p0CheckButtonAdapterDetectable = Gtk::manage(new Gtk::CheckButton("Adapter detectable"));
		m_p0VBoxMain->pack_start(*m_p0CheckButtonAdapterDetectable, false, false);
			m_p0CheckButtonAdapterDetectable->signal_toggled().connect(
							sigc::mem_fun(*this, &TootherWindow::onAdapterDetectableToggled) );

		m_p0CheckButtonServiceRunning = Gtk::manage(new Gtk::CheckButton("Bluetooth service running"));
		m_p0VBoxMain->pack_start(*m_p0CheckButtonServiceRunning, false, false);
			m_p0CheckButtonServiceRunning->set_margin_top(6);
			m_p0CheckButtonServiceRunning->signal_toggled().connect(
							sigc::mem_fun(*this, &TootherWindow::onServiceRunningToggled) );
		m_p0CheckButtonServiceEnabled = Gtk::manage(new Gtk::CheckButton("Bluetooth service started at boot"));
		m_p0VBoxMain->pack_start(*m_p0CheckButtonServiceEnabled, false, false);
			m_p0CheckButtonServiceEnabled->signal_toggled().connect(
							sigc::mem_fun(*this, &TootherWindow::onServiceEnabledToggled) );

		m_p0ButtonTurnAllOn = Gtk::manage(new Gtk::Button("Turn all on"));
		m_p0VBoxMain->pack_start(*m_p0ButtonTurnAllOn, false, false);
			m_p0ButtonTurnAllOn->signal_clicked().connect(
							sigc::mem_fun(*this, &TootherWindow::onButtonTurnAllOn) );

	Gtk::Label* m_p0TabLabelLog = Gtk::manage(new Gtk::Label("Log"));
	Gtk::ScrolledWindow* m_p0ScrolledLog = Gtk::manage(new Gtk::ScrolledWindow());
	m_aPageIndex[s_nTabLog] = m_p0NotebookChoices->append_page(*m_p0ScrolledLog, *m_p0TabLabelLog);
		m_p0ScrolledLog->set_border_width(5);
		m_p0ScrolledLog->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);

		m_p0TextViewLog = Gtk::manage(new Gtk::TextView());
		m_p0ScrolledLog->add(*m_p0TextViewLog);
			m_refTextBufferLog = Gtk::TextBuffer::create();
			m_refTextBufferMarkBottom = Gtk::TextBuffer::Mark::create("Bottom");
			m_p0TextViewLog->set_editable(false);
			m_p0TextViewLog->set_buffer(m_refTextBufferLog);
			m_refTextBufferLog->set_text(sInitialLogText);
			m_p0TextViewLog->override_font(oMonoFont);
			m_refTextBufferLog->add_mark(m_refTextBufferMarkBottom, m_refTextBufferLog->end());

	Gtk::Label* m_p0TabLabelInfo = Gtk::manage(new Gtk::Label("Info"));
	Gtk::ScrolledWindow* m_p0ScrolledInfo = Gtk::manage(new Gtk::ScrolledWindow());
	m_aPageIndex[s_nTabInfo] = m_p0NotebookChoices->append_page(*m_p0ScrolledInfo, *m_p0TabLabelInfo);
		m_p0ScrolledInfo->set_border_width(5);
		m_p0ScrolledInfo->set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);

		Gtk::Label* m_p0LabelInfoText = Gtk::manage(new Gtk::Label(sInfoText));
		m_p0ScrolledInfo->add(*m_p0LabelInfoText);
			m_p0LabelInfoText->set_line_wrap_mode(Pango::WRAP_WORD_CHAR);


	show_all_children();

	m_refWatchCursor = Gdk::Cursor::create(Gdk::WATCH);

	onButtonRefresh();
}
TootherWindow::~TootherWindow()
{
}
void TootherWindow::onNotebookSwitchPage(Gtk::Widget*, guint nPageNum)
{
	if (nPageNum >= s_nTotPages) {
		return;
	}
}
void TootherWindow::setSensitivityForState()
{
//std::cout << "setSensitivityForState  m_bNeedsRefreshing=" << m_bNeedsRefreshing << '\n';
	if (m_bNeedsRefreshing || m_bNeedsEnablingEverything) {
		m_p0VBoxMain->set_sensitive(false);
		return; //--------------------------------------------------------------
	}
	m_p0VBoxMain->set_sensitive(true);
	m_p0ButtonRefresh->set_sensitive(!m_bRefreshing);
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	const bool bShowListOverride = false;
	//
	const auto& aHciIds = m_oHciSocket.getHciIds();
	const bool bManyAdapters = (aHciIds.size() > 1);
	m_p0TreeViewAdapters->set_visible(bManyAdapters || bShowListOverride);
	m_p0LabelCurrentAdapter->set_visible(!bManyAdapters);

	const bool bAdapterSelected = (m_nSelectedHciId >= 0);
	if (bAdapterSelected) {
		assert(m_oHciSocket.getAdapter(m_nSelectedHciId) != nullptr);
	}
	bool bHardwareEnabled = false;
	bool bSoftwareEnabled = false;
	bool bAdapterEnabled = false;
	bool bIsUp = false;
	if (bAdapterSelected) {
		auto p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
		assert(p0Adapter != nullptr);
		//
		bHardwareEnabled = p0Adapter->isHardwareEnabled();
		bSoftwareEnabled = p0Adapter->isSoftwareEnabled();
		bAdapterEnabled = bHardwareEnabled && bSoftwareEnabled;
		//
		if (bAdapterEnabled) {
			bIsUp = !p0Adapter->isDown();
		}
	}
	m_p0CheckButtonHardwareEnabled->set_sensitive(false);
	m_p0CheckButtonSoftwareEnabled->set_sensitive(bAdapterSelected);
	m_p0CheckButtonAdapterIsUp->set_sensitive(bAdapterEnabled);
	m_p0EntryAdapterLocalName->set_sensitive(bAdapterEnabled && bIsUp);
	m_p0CheckButtonAdapterDetectable->set_sensitive(bAdapterEnabled && bIsUp);
	m_p0CheckButtonAdapterConnectable->set_sensitive(bAdapterEnabled && bIsUp);
	// adapter independent
	m_p0CheckButtonServiceRunning->set_sensitive(true);
	m_p0CheckButtonServiceEnabled->set_sensitive(true);

	m_p0ButtonTurnAllOn->set_sensitive(bHardwareEnabled);

}
void TootherWindow::startRefreshing(int32_t nMillisec)
{
//std::cout << "TootherWindow::startRefreshing() nMillisec" << nMillisec << '\n';
	if (m_bNeedsRefreshing) {
		return;
	}
	m_bNeedsRefreshing = true;
	if (get_window()) {
		m_refStdCursor = get_window()->get_cursor();
	}
	if (get_window()) {
		get_window()->set_cursor(m_refWatchCursor);
	}
	setSensitivityForState();
	Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &TootherWindow::onTimeout), nMillisec);
//std::cout << "TootherWindow::startRefreshing() END" << '\n';
}
void TootherWindow::onTimeout()
{
	if ((!m_bNeedsRefreshing) && !m_bNeedsEnablingEverything) {
		return;
	}
	if (m_bNeedsRefreshing) {
		refresh();
		if (m_refStdCursor) {
			get_window()->set_cursor(m_refStdCursor);
		} else if (get_window()) {
			m_refStdCursor = Gdk::Cursor::create(Gdk::ARROW);
			get_window()->set_cursor(m_refStdCursor);
		}
		m_bNeedsRefreshing = false;
		setSensitivityForState();
	} else {
		assert(m_bNeedsEnablingEverything);
		HciAdapter* p0Adapter = nullptr;
		if (m_nSelectedHciId >= 0) {
			p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
			bool bOk = p0Adapter->setAdapterIsUp(true);
			if (!bOk) {
				printStringToLog(p0Adapter->getLastError());
			}
			bOk = p0Adapter->setDetectable(true);
			if (!bOk) {
				printStringToLog(p0Adapter->getLastError());
			}
			bOk = p0Adapter->setConnectable(true);
			if (!bOk) {
				printStringToLog(p0Adapter->getLastError());
			}
		}
		if (m_refStdCursor) {
			get_window()->set_cursor(m_refStdCursor);
		} else if (get_window()) {
			m_refStdCursor = Gdk::Cursor::create(Gdk::ARROW);
			get_window()->set_cursor(m_refStdCursor);
		}
		m_bNeedsEnablingEverything = false;
		//
		startRefreshing(0);
	}
}
void TootherWindow::refresh()
{
//std::cout << "TootherWindow::refresh()" << '\n';
	//m_bRefreshing means needs refreshing in next timeout!
	if (m_bRefreshing) {
		return;
	}
	const auto& aHciIds = m_oHciSocket.getHciIds();
	const auto& aFaultyHcis = m_oHciSocket.getFaultyHciIds();
	m_bRefreshing = true;
	if (!m_oHciSocket.update()) {
		printStringToLog(m_oHciSocket.getLastError());
	} else {
		if (!aFaultyHcis.empty()) {
			for (int32_t nHciId : aFaultyHcis) {
				auto p0Adapter = m_oHciSocket.getAdapter(nHciId);
				assert(p0Adapter != nullptr);
				printStringToLog(p0Adapter->getLastError());
			}
		}
	}
	if (!m_oBtService.update()) {
		printStringToLog(m_oBtService.getLastError());
	}
	if (aHciIds.size() > 0) {
		if (m_nSelectedHciId < 0) {
			m_nSelectedHciId = aHciIds[0];
		} else { // test whether the selected id still exists
			const auto itFound = std::find(aHciIds.begin(), aHciIds.end(), m_nSelectedHciId);
			if (itFound == aHciIds.end()) {
				m_nSelectedHciId = aHciIds[0];
			}
		}
	} else {
		m_nSelectedHciId = -1;
		if (!aFaultyHcis.empty()) {
			printStringToLog("No healthy bluetooth adapters found!");
		} else {
			printStringToLog("No bluetooth adapters found!");
		}
	}
	if (m_nSelectedHciId >= 0) {
		auto p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
		assert(p0Adapter != nullptr);
		m_p0LabelCurrentAdapter->set_text(std::string("Adapter id: hci") + std::to_string(m_nSelectedHciId));
		m_p0LabelCurrentAddress->set_text(std::string("Bluetooth address: ") + p0Adapter->getAddress());
		m_p0CheckButtonHardwareEnabled->set_active(p0Adapter->isHardwareEnabled());
		m_p0CheckButtonSoftwareEnabled->set_active(p0Adapter->isSoftwareEnabled());
		m_p0CheckButtonAdapterIsUp->set_active(!p0Adapter->isDown());
		m_p0EntryAdapterLocalName->set_text(p0Adapter->getLocalName());
		m_p0CheckButtonAdapterDetectable->set_active(p0Adapter->isDetectable());
		m_p0CheckButtonAdapterConnectable->set_active(p0Adapter->isConnectable());
	}
	m_p0CheckButtonServiceRunning->set_active(m_oBtService.isServiceRunning());
	const bool bServiceEnabled = m_oBtService.isServiceEnabled();
	m_p0CheckButtonServiceEnabled->set_active(bServiceEnabled);
	//
	regenerateAdaptersModel();
	m_bRefreshing = false;
}
void TootherWindow::regenerateAdaptersModel()
{
	if (m_bRegeneratingAdaptersModel) {
		return;
	}
	m_bRegeneratingAdaptersModel = true;
	m_refTreeModelAdapters->clear();

	int32_t nSelectedIdx = -1;
	const auto& aHciIds = m_oHciSocket.getHciIds();
	const auto nTotHciIds = static_cast<int32_t>(aHciIds.size());
	for (int32_t nIdx = 0; nIdx < nTotHciIds; ++nIdx) {
		const int32_t nHciId = aHciIds[nIdx];
		auto p0Adapter = m_oHciSocket.getAdapter(nHciId);
		assert(p0Adapter != nullptr);
		Gtk::TreeModel::Row oAdapterRow;
		assert(m_refTreeModelAdapters);
		oAdapterRow = *(m_refTreeModelAdapters->append()); //oGameRow.children()
		oAdapterRow[m_oAdaptersColumns.m_oColHciName] = p0Adapter->getAdapterName();
		oAdapterRow[m_oAdaptersColumns.m_oColHiddenHciId] = nHciId;
		if (nHciId == m_nSelectedHciId) {
			nSelectedIdx = nIdx;
		}
		++nIdx;
	}

	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewAdapters->get_selection();
	Gtk::TreeModel::Path oPath;
	if (nSelectedIdx >= 0) {
		oPath.push_back(nSelectedIdx);
	}
//std::cout << "   oPath.size=" << oPath.size() << '\n';
	refTreeSelection->select(oPath);

	m_bRegeneratingAdaptersModel = false;
}

void TootherWindow::onButtonRefresh()
{
//std::cout << "TootherWindow::onButtonRefresh()" << '\n';
	startRefreshing(0);
}
void TootherWindow::onAdapterSelectionChanged()
{
//std::cout << "TootherWindow::onAdapterSelectionChanged()" << '\n';
	if (m_bRegeneratingAdaptersModel) {
		return;
	}
	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewAdapters->get_selection();
	Gtk::TreeModel::iterator it = refTreeSelection->get_selected();
	if (it)	{
		Gtk::TreeModel::Row oRow = *it;
		const int32_t nHciId = oRow[m_oAdaptersColumns.m_oColHiddenHciId];
		m_nSelectedHciId = nHciId;
	} else {
		m_nSelectedHciId = -1;
	}
	startRefreshing(0);
}
void TootherWindow::onSoftwareEnabledToggled()
{
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	auto p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
	if (p0Adapter == nullptr) {
		return; //--------------------------------------------------------------
	}
	const bool bEnabled = m_p0CheckButtonSoftwareEnabled->get_active();
	const bool bOk = p0Adapter->setSoftwareEnabled(bEnabled);
	if (!bOk) {
		printStringToLog(p0Adapter->getLastError());
	}
	startRefreshing(0);
}
void TootherWindow::onAdapterIsUpToggled()
{
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	auto p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
	if (p0Adapter == nullptr) {
		return; //--------------------------------------------------------------
	}
	const bool bIsUp = m_p0CheckButtonAdapterIsUp->get_active();
	const bool bOk = p0Adapter->setAdapterIsUp(bIsUp);
	if (!bOk) {
		printStringToLog(p0Adapter->getLastError());
	}
	startRefreshing(0);
}
bool TootherWindow::onAdapterLocalNameChangedFocus(GdkEventFocus* /*p0Event*/)
{
	if (m_bRefreshing) {
		return true; //---------------------------------------------------------
	}
	if (m_nSelectedHciId < 0) {
		return true; //---------------------------------------------------------
	}
	auto p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
	if (p0Adapter == nullptr) {
		return true; //---------------------------------------------------------
	}
	const std::string sLocalName = m_p0EntryAdapterLocalName->get_text();
	if (sLocalName.empty() || (sLocalName.size() > s_nMaxLocalNameSize)) {
		const auto& sOldName = p0Adapter->getLocalName();
		m_p0EntryAdapterLocalName->set_text(sOldName);
		return true; //---------------------------------------------------------
	}
	const bool bOk = p0Adapter->setLocalName(sLocalName);
	if (!bOk) {
		printStringToLog(p0Adapter->getLastError());
	}
	startRefreshing(0);
	return true;
}
void TootherWindow::onAdapterDetectableToggled()
{
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	auto p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
	if (p0Adapter == nullptr) {
		return; //--------------------------------------------------------------
	}
	const bool bIsDetectable = m_p0CheckButtonAdapterDetectable->get_active();
	const bool bOk = p0Adapter->setDetectable(bIsDetectable);
	if (!bOk) {
		printStringToLog(p0Adapter->getLastError());
	}
	startRefreshing(0);
}
void TootherWindow::onAdapterConnectableToggled()
{
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	auto p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
	if (p0Adapter == nullptr) {
		return; //--------------------------------------------------------------
	}
	const bool bIsConnectable = m_p0CheckButtonAdapterConnectable->get_active();
	const bool bOk = p0Adapter->setConnectable(bIsConnectable);
	if (!bOk) {
		printStringToLog(p0Adapter->getLastError());
	}
	startRefreshing(0);
}
void TootherWindow::onServiceRunningToggled()
{
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	const bool bServiceRunning = m_p0CheckButtonServiceRunning->get_active();
	const bool bOk = (bServiceRunning ? m_oBtService.startService() : m_oBtService.stopService());
	if (!bOk) {
		printStringToLog(m_oBtService.getLastError());
	}
	startRefreshing(s_nRefreshingWaitServiceRunning);
}
void TootherWindow::onServiceEnabledToggled()
{
//std::cout << "TootherWindow::onServiceEnabledToggled()------------------" << '\n';
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	const bool bServiceEnabled = m_p0CheckButtonServiceEnabled->get_active();
	const bool bOk = m_oBtService.enableService(bServiceEnabled);
	if (!bOk) {
		printStringToLog(m_oBtService.getLastError());
	}
	startRefreshing(s_nRefreshingWaitServiceEnabling);
}

void TootherWindow::onButtonTurnAllOn()
{
//std::cout << "TootherWindow::onButtonTurnAllOn()  m_nSelectedHciId=" << m_nSelectedHciId << '\n';
	if (m_bNeedsEnablingEverything) {
		return; //--------------------------------------------------------------
	}
	if (m_bNeedsRefreshing) {
		return; //--------------------------------------------------------------
	}
	if (m_bRefreshing) {
		return; //--------------------------------------------------------------
	}
	HciAdapter* p0Adapter = nullptr;
	if (m_nSelectedHciId >= 0) {
		p0Adapter = m_oHciSocket.getAdapter(m_nSelectedHciId);
	}
//std::cout << "TootherWindow::onButtonTurnAllOn()  m_nSelectedHciId=" << m_nSelectedHciId << '\n';
	if (p0Adapter != nullptr) {
		const bool bOk = p0Adapter->setSoftwareEnabled(true);
		if (!bOk) {
			printStringToLog(p0Adapter->getLastError());
		}
	}
	bool bOk = m_oBtService.startService();
	if (!bOk) {
		printStringToLog(m_oBtService.getLastError());
	}
	bOk = m_oBtService.enableService(true);
	if (!bOk) {
		printStringToLog(m_oBtService.getLastError());
	}

	m_bNeedsEnablingEverything = true;
	if (get_window()) {
		m_refStdCursor = get_window()->get_cursor();
	}
	if (get_window()) {
		get_window()->set_cursor(m_refWatchCursor);
	}
	setSensitivityForState();
	Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &TootherWindow::onTimeout), s_nRefreshingWaitServiceRunning);
//std::cout << "TootherWindow::onButtonTurnAllOn() END" << '\n';
}


void TootherWindow::printStringToLog(const std::string& sStr)
{
//std::cout << "TootherWindow::printStringToLog " << sStr << '\n';
	if (m_nTextBufferLogTotLines >= s_nTextBufferLogMaxLines) {
		auto itLine1 = m_refTextBufferLog->get_iter_at_line(1);
		m_refTextBufferLog->erase(m_refTextBufferLog->begin(), itLine1);
		--m_nTextBufferLogTotLines;
	}
	m_refTextBufferLog->insert(m_refTextBufferLog->end(), sStr + "\n");
	++m_nTextBufferLogTotLines;

	m_refTextBufferLog->place_cursor(m_refTextBufferLog->end());
	m_refTextBufferLog->move_mark(m_refTextBufferMarkBottom, m_refTextBufferLog->end());
	m_p0TextViewLog->scroll_to(m_refTextBufferMarkBottom, 0.1);
}

} // namespace stmi

