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
#include "jsoncommon.h"

#include <cassert>
#include <iostream>

namespace stmi
{

TootherWindow::TootherWindow(const std::string& sTitle, int nCmdPipeFD, int nReturnPipeFD
							, bool bReadOnly, bool bNoRestrictedWarning)
: Gtk::Window()
, m_bReadOnly(bReadOnly)
, m_bNoRestrictedWarning(bNoRestrictedWarning)
, m_nCmdPipeFD(nCmdPipeFD)
//, m_p0NotebookChoices(nullptr)
//, m_p0TabLabelMain(nullptr)
, m_p0VBoxMain(nullptr)
//, m_p0HBoxRefreshAdapters(nullptr)
, m_p0ButtonRefresh(nullptr)
//, m_p0VBoxTreeAndCurrentAdapter(nullptr)
, m_p0TreeViewAdapters(nullptr)
//, m_p0VBoxCurrentAdapter(nullptr)
, m_p0LabelCurrentAdapter(nullptr)
, m_p0LabelCurrentAddress(nullptr)
, m_p0CheckButtonHardwareEnabled(nullptr)
, m_p0CheckButtonSoftwareEnabled(nullptr)
, m_p0CheckButtonAdapterIsUp(nullptr)
//, m_p0HBoxLocalName(nullptr)
, m_p0ButtonAdapterSetLocalName(nullptr)
, m_p0EntryAdapterLocalName(nullptr)
, m_p0CheckButtonAdapterConnectable(nullptr)
, m_p0CheckButtonAdapterDetectable(nullptr)
, m_p0CheckButtonServiceRunning(nullptr)
, m_p0CheckButtonServiceEnabled(nullptr)
//, m_p0TabLabelLog(nullptr)
//, m_p0ScrolledLog(nullptr)
, m_p0TextViewLog(nullptr)
//, m_p0TabLabelInfo(nullptr)
//, m_p0ScrolledInfo(nullptr)
//, m_p0LabelInfoText(nullptr)
, m_nTextBufferLogTotLines(0)
, m_nSelectedHciId(-1)
, m_nStamp(0)
, m_bWaitingForReturn(false)
, m_bWaitingForState(false)
, m_bSettingWidgetsValues(false)
{
	m_refPipeInputSource = Glib::RefPtr<PipeInputSource>(new PipeInputSource(nReturnPipeFD));
	m_refPipeInputSource->connect(sigc::mem_fun(*this, &TootherWindow::doReceiveString));
	m_refPipeInputSource->attach();
	//
	set_title(sTitle + (m_bReadOnly ? " (restricted)" : ""));
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
			"Bluetoother is a tool to start/stop the bluetooth adapter\n"
			"and the related systemd service. It also allows to set the\n"
			"local name, the detectability and the connectability of the device.\n\n"
			"It provides some of the functionality of the command line tools\n"
			"rfkill, hciconfig and sytemctl.\n\n"
			"The interface of this program is not reactive. If the settings\n"
			"are changed from another program, you need to press the 'Refresh'\n"
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
		m_p0VBoxMain->set_border_width(5);

		Gtk::Box* m_p0HBoxRefreshAdapters = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
		m_p0VBoxMain->pack_start(*m_p0HBoxRefreshAdapters, false, false);
			m_p0HBoxRefreshAdapters->set_spacing(6);

			m_p0ButtonRefresh = Gtk::manage(new Gtk::Button("Refresh"));
			m_p0HBoxRefreshAdapters->pack_start(*m_p0ButtonRefresh, false, false);
				m_p0ButtonRefresh->signal_clicked().connect(
								sigc::mem_fun(*this, &TootherWindow::onButtonRefresh) );

			Gtk::Box* m_p0VBoxTreeAndCurrentAdapter = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
			m_p0HBoxRefreshAdapters->pack_start(*m_p0VBoxTreeAndCurrentAdapter, true, true);
				m_p0VBoxTreeAndCurrentAdapter->set_spacing(6);

				m_refTreeModelAdapters = Gtk::TreeStore::create(m_oAdaptersColumns);
				m_p0TreeViewAdapters = Gtk::manage(new Gtk::TreeView(m_refTreeModelAdapters));
				m_p0VBoxTreeAndCurrentAdapter->pack_start(*m_p0TreeViewAdapters, true, true);
					m_p0TreeViewAdapters->append_column("Adapter id", m_oAdaptersColumns.m_oColHciId);
					m_p0TreeViewAdapters->append_column("name", m_oAdaptersColumns.m_oColHciName);
					refTreeSelection = m_p0TreeViewAdapters->get_selection();
					refTreeSelection->signal_changed().connect(
									sigc::mem_fun(*this, &TootherWindow::onAdapterSelectionChanged));

				Gtk::Box* m_p0VBoxCurrentAdapter = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
				m_p0VBoxTreeAndCurrentAdapter->pack_start(*m_p0VBoxCurrentAdapter, false, false);
					m_p0VBoxCurrentAdapter->set_border_width(5);

					m_p0LabelCurrentAdapter = Gtk::manage(new Gtk::Label("Bluetooth adapter:"));
					m_p0VBoxCurrentAdapter->pack_start(*m_p0LabelCurrentAdapter, false, false);
					m_p0LabelCurrentAddress = Gtk::manage(new Gtk::Label("Address:"));
					m_p0VBoxCurrentAdapter->pack_start(*m_p0LabelCurrentAddress, false, false);

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
			m_p0HBoxLocalName->set_spacing(4);

			m_p0ButtonAdapterSetLocalName = Gtk::manage(new Gtk::Button("Set name"));
			m_p0HBoxLocalName->pack_start(*m_p0ButtonAdapterSetLocalName, false, false);
				m_p0ButtonAdapterSetLocalName->signal_clicked().connect(
								sigc::mem_fun(*this, &TootherWindow::onButtonSetName) );
			m_p0EntryAdapterLocalName = Gtk::manage(new Gtk::Entry());
			m_p0HBoxLocalName->pack_start(*m_p0EntryAdapterLocalName, true, true);
				m_p0EntryAdapterLocalName->set_sensitive(false);

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

	if (m_bReadOnly) {
		printStringToLog("Restricted mode!!!");
		m_p0VBoxMain->set_sensitive(false);
		signal_realize().connect(sigc::mem_fun(*this, &TootherWindow::onWindowRealize));
	}

	show_all_children();

	m_refWatchCursor = Gdk::Cursor::create(Gdk::WATCH);

	if (!m_bReadOnly) {
		onButtonRefresh();
	}
}
TootherWindow::~TootherWindow()
{
}
void TootherWindow::onWindowRealize()
{
	Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &TootherWindow::showReadOnlyWarning), 0);
}
void TootherWindow::showReadOnlyWarning()
{
	assert(m_bReadOnly);
	if (! m_bNoRestrictedWarning) {
		Gtk::MessageDialog oDlg(*this, "The program was started by non root user.\n"
								"Most of the functionality won't work.\n"
								"Please use 'pkexec bluetoother'.", false
								, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, false);
		oDlg.run();
	}
	onButtonRefresh();
}
void TootherWindow::setWidgetsValue()
{
	if (m_bSettingWidgetsValues) {
		return;
	}
	m_bSettingWidgetsValues = true;

	const bool bStateOk = !m_oCurState.is_null();

	m_refTreeModelAdapters->clear();

	int32_t nSelectedIdx = -1;

	if (!bStateOk) {
		m_nSelectedHciId = -1;
		printStringToLog("No bluetooth adapters found!");
	} else {
		const auto& oAdapters = m_oCurState[JsonCommon::s_sKeyModelAdapters];
		assert(oAdapters.is_array());
		const auto nTotAdapters = oAdapters.size();
		if (nTotAdapters > 0) {
			if (m_nSelectedHciId < 0) {
				const auto& oAdapter = oAdapters[0];
				assert(oAdapter.is_object());
				assert(oAdapter[JsonCommon::s_sKeyModelAdapterHciId].is_number_integer());
				m_nSelectedHciId = oAdapter[JsonCommon::s_sKeyModelAdapterHciId];
			} else {
				// test whether the selected id still exists
				const auto ifFind = std::find_if(oAdapters.begin(), oAdapters.end(), [&](const json& oAdapter) {
					assert(oAdapter.is_object());
					assert(oAdapter[JsonCommon::s_sKeyModelAdapterHciId].is_number_integer());
					return (oAdapter[JsonCommon::s_sKeyModelAdapterHciId] == m_nSelectedHciId);
				});
				if (ifFind == oAdapters.end()) {
					// if not set selected to first adapter
					const auto& oAdapter = oAdapters[0];
					assert(oAdapter.is_object());
					assert(oAdapter[JsonCommon::s_sKeyModelAdapterHciId].is_number_integer());
					m_nSelectedHciId = oAdapter[JsonCommon::s_sKeyModelAdapterHciId];
				}
			}
		} else {
			m_nSelectedHciId = -1;
		}
		int32_t nIdx = 0;
		for (const auto& oAdapter : oAdapters) {
			assert(oAdapter.is_object());
			assert(oAdapter[JsonCommon::s_sKeyModelAdapterHciId].is_number_integer());
			const int32_t nHciId = oAdapter[JsonCommon::s_sKeyModelAdapterHciId];
			Gtk::TreeModel::Row oAdapterRow;
			assert(m_refTreeModelAdapters);
			oAdapterRow = *(m_refTreeModelAdapters->append()); //oGameRow.children()
			assert(oAdapter[JsonCommon::s_sKeyModelAdapterName].is_string());
			const std::string& sLocalName = oAdapter[JsonCommon::s_sKeyModelAdapterName];
			std::string sTruncName;
			if (sLocalName.size() > s_nTruncLocalNameSize) {
				sTruncName = sLocalName.substr(0, s_nTruncLocalNameSize) + "...";
			} else {
				sTruncName = sLocalName;
			}
			oAdapterRow[m_oAdaptersColumns.m_oColHciName] = (sTruncName.empty() ? "(unknown)" : sTruncName);
			oAdapterRow[m_oAdaptersColumns.m_oColHciId] = std::string("hci") + std::to_string(nHciId);
			oAdapterRow[m_oAdaptersColumns.m_oColHiddenHciId] = nHciId;
			//
			if (nHciId == m_nSelectedHciId) {
				nSelectedIdx = nIdx;
			}
			++nIdx;
		}
		// set selection on tree view widget
		Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewAdapters->get_selection();
		Gtk::TreeModel::Path oPath;
		if (nSelectedIdx >= 0) {
			oPath.push_back(nSelectedIdx);
		}
//std::cout << "   oPath.size=" << oPath.size() << '\n';
		refTreeSelection->select(oPath);
		//
		if (m_nSelectedHciId < 0) {
			json& oFaultyAdapters = m_oCurState[JsonCommon::s_sKeyModelFaultyAdapters];
			const auto nTotFaultyAdapters = oFaultyAdapters.size();
			if (nTotFaultyAdapters > 0) {
				printStringToLog("No healthy bluetooth adapters found!");
			} else {
				printStringToLog("No bluetooth adapters found!");
			}
		}
	}
	bool bHardwareEnabled = false;
	bool bSoftwareEnabled = false;
	bool bIsUp = false;
	std::string sLocalName;
	bool bIsConnectable = false;
	bool bIsDetectable = false;
	bool bServiceRunning = false;
	bool bServiceEnabled = false;

	if (m_nSelectedHciId >= 0) {
		// There are adapters
		const auto& oAdapters = m_oCurState[JsonCommon::s_sKeyModelAdapters];
		assert(nSelectedIdx >= 0);
		const auto& oAdapter = oAdapters[nSelectedIdx];
		m_p0LabelCurrentAdapter->set_text(std::string("Adapter id: hci") + std::to_string(m_nSelectedHciId));
		assert(oAdapter[JsonCommon::s_sKeyModelAdapterAddress].is_string());
		const std::string& sAddress = oAdapter[JsonCommon::s_sKeyModelAdapterAddress];
		m_p0LabelCurrentAddress->set_text(std::string("Bluetooth address: ") + sAddress);
		//
		assert(oAdapter[JsonCommon::s_sKeyModelAdapterHardwareEnabled].is_boolean());
		bHardwareEnabled = oAdapter[JsonCommon::s_sKeyModelAdapterHardwareEnabled];
		assert(oAdapter[JsonCommon::s_sKeyModelAdapterSoftwareEnabled].is_boolean());
		bSoftwareEnabled = oAdapter[JsonCommon::s_sKeyModelAdapterSoftwareEnabled];
		//
		if (bHardwareEnabled && bSoftwareEnabled) {
			assert(oAdapter[JsonCommon::s_sKeyModelAdapterIsUp].is_boolean());
			bIsUp = oAdapter[JsonCommon::s_sKeyModelAdapterIsUp];
			if (bIsUp) {
				assert(oAdapter[JsonCommon::s_sKeyModelAdapterName].is_string());
				sLocalName = oAdapter[JsonCommon::s_sKeyModelAdapterName];
				assert(oAdapter[JsonCommon::s_sKeyModelAdapterConnectable].is_boolean());
				bIsConnectable = oAdapter[JsonCommon::s_sKeyModelAdapterConnectable];
				assert(oAdapter[JsonCommon::s_sKeyModelAdapterDetectable].is_boolean());
				bIsDetectable = oAdapter[JsonCommon::s_sKeyModelAdapterDetectable];
			}
		}
	} else {
		m_p0LabelCurrentAdapter->set_text("Adapter id:");
		m_p0LabelCurrentAddress->set_text("Bluetooth address:");
	}
	if (bStateOk) {
		assert(m_oCurState[JsonCommon::s_sKeyModelServiceRunning].is_boolean());
		bServiceRunning = m_oCurState[JsonCommon::s_sKeyModelServiceRunning];
		assert(m_oCurState[JsonCommon::s_sKeyModelServiceEnabled].is_boolean());
		bServiceEnabled = m_oCurState[JsonCommon::s_sKeyModelServiceEnabled];
	}
	m_p0CheckButtonHardwareEnabled->set_active(bHardwareEnabled);
	m_p0CheckButtonSoftwareEnabled->set_active(bSoftwareEnabled);
	m_p0CheckButtonAdapterIsUp->set_active(bIsUp);
	m_p0EntryAdapterLocalName->set_text(sLocalName);
	m_p0CheckButtonAdapterConnectable->set_active(bIsConnectable);
	m_p0CheckButtonAdapterDetectable->set_active(bIsDetectable);
	m_p0CheckButtonServiceRunning->set_active(bServiceRunning);
	m_p0CheckButtonServiceEnabled->set_active(bServiceEnabled);

	m_bSettingWidgetsValues = false;
}
void TootherWindow::setWidgetsSensitivity()
{
	if (m_bWaitingForReturn) {
		m_p0VBoxMain->set_sensitive(false);
		return; //--------------------------------------------------------------
	}
	m_p0VBoxMain->set_sensitive(true);

	bool bManyAdapters = false;
	if (! m_oCurState.is_null()) {
		const json& oAdapters = m_oCurState[JsonCommon::s_sKeyModelAdapters];
		assert(oAdapters.is_array());
		const auto nTotAdapters = oAdapters.size();
		bManyAdapters = (nTotAdapters > 1);
	}

	const bool bShowListOverride = false;
	//
	m_p0TreeViewAdapters->set_visible(bManyAdapters || bShowListOverride);
	m_p0LabelCurrentAdapter->set_visible(!bManyAdapters);

	bool bHardwareEnabled = false;
	bool bSoftwareEnabled = false;
	bool bAdapterEnabled = false;
	bool bIsUp = false;
	const bool bAdapterSelected = (m_nSelectedHciId >= 0);
	if (bAdapterSelected) {
		const json& oAdapters = m_oCurState[JsonCommon::s_sKeyModelAdapters];
		const auto ifFind = std::find_if(oAdapters.begin(), oAdapters.end(), [&](const json& oAdapter) {
			assert(oAdapter.is_object());
			assert(oAdapter[JsonCommon::s_sKeyModelAdapterHciId].is_number_integer());
			return (oAdapter[JsonCommon::s_sKeyModelAdapterHciId] == m_nSelectedHciId);
		});
		assert(ifFind != oAdapters.end());
		const auto& oAdapter = *ifFind;
		assert(oAdapter[JsonCommon::s_sKeyModelAdapterHardwareEnabled].is_boolean());
		bHardwareEnabled = oAdapter[JsonCommon::s_sKeyModelAdapterHardwareEnabled];
		assert(oAdapter[JsonCommon::s_sKeyModelAdapterSoftwareEnabled].is_boolean());
		bSoftwareEnabled = oAdapter[JsonCommon::s_sKeyModelAdapterSoftwareEnabled];
		//
		bAdapterEnabled = bHardwareEnabled && bSoftwareEnabled;
		//
		if (bAdapterEnabled) {
			assert(oAdapter[JsonCommon::s_sKeyModelAdapterIsUp].is_boolean());
			bIsUp = oAdapter[JsonCommon::s_sKeyModelAdapterIsUp];
		}
	}
	m_p0CheckButtonHardwareEnabled->set_sensitive(false);
	m_p0CheckButtonSoftwareEnabled->set_sensitive(bAdapterSelected);
	m_p0CheckButtonAdapterIsUp->set_sensitive(bAdapterEnabled);
	m_p0ButtonAdapterSetLocalName->set_sensitive(bAdapterEnabled && bIsUp);
	m_p0CheckButtonAdapterDetectable->set_sensitive(bAdapterEnabled && bIsUp);
	m_p0CheckButtonAdapterConnectable->set_sensitive(bAdapterEnabled && bIsUp);
	// adapter independent
	m_p0CheckButtonServiceRunning->set_sensitive(!m_bReadOnly);
	m_p0CheckButtonServiceEnabled->set_sensitive(!m_bReadOnly);
}
void TootherWindow::startRefreshing(int32_t nMillisec)
{
//std::cout << "TootherWindow::startRefreshing() nMillisec = " << nMillisec << '\n';
	cursorToHourglass();
	m_bWaitingForReturn = true;
	m_bWaitingForState = true;
	setWidgetsSensitivity();
	//
	++m_nStamp;
	json oRoot = json::object();
	oRoot[JsonCommon::s_sKeyCmd] = JsonCommon::s_sValueCmdGetState;
	oRoot[JsonCommon::s_sKeyStamp] = m_nStamp;
	sendCmd(oRoot.dump(4));
	startTimeout(m_nStamp, nMillisec);
}
void TootherWindow::cursorToHourglass()
{
	if (get_window()) {
		m_refStdCursor = get_window()->get_cursor();
		get_window()->set_cursor(m_refWatchCursor);
	}
}
void TootherWindow::cursorToNormal()
{
	if (m_refStdCursor) {
		get_window()->set_cursor(m_refStdCursor);
	} else if (get_window()) {
		m_refStdCursor = Gdk::Cursor::create(Gdk::ARROW);
		get_window()->set_cursor(m_refStdCursor);
	}
}
void TootherWindow::startTimeout(int32_t nStamp, int32_t nMillisec)
{
	Glib::signal_timeout().connect_once(sigc::bind(sigc::mem_fun(*this, &TootherWindow::onTimeout), nStamp, nMillisec)
										, nMillisec);
}
void TootherWindow::doReceiveString(bool bError, const std::string& sStr)
{
//std::cout << "TootherWindow::doReceiveString bError=" << bError << '\n';
	if (bError) {
		quitOnFatalError(std::string("Error: ") + sStr);
		return; //----------------------------------------------------------
	}
	if (!m_bWaitingForReturn) {
		printStringToLog("Error: received timed out data from server");
		//TODO maybe message box warning
		return; //----------------------------------------------------------
	}
	const json oReturnValue = JsonCommon::parseString(sStr);
	assert(oReturnValue.is_object());
	//
	const int32_t nStamp = oReturnValue[JsonCommon::s_sKeyStamp];
	if (nStamp != m_nStamp) {
		printStringToLog("Error: received very timed out data from server");
		//TODO maybe message box warning
		return; //----------------------------------------------------------
	}
	//
	cursorToNormal();
	//
	m_bWaitingForReturn = false;
	const bool bWaitingForState = m_bWaitingForState;
	m_bWaitingForState = false;
	const std::string sRetType = oReturnValue[JsonCommon::s_sKeyReturnType];
	if (sRetType == JsonCommon::s_sValueReturnTypeError) {
		//
		printStringToLog(oReturnValue[JsonCommon::s_sKeyReturnParam]);
		if (bWaitingForState) {
			// Refresh error, set state to null!
			m_oCurState = json{};
			setWidgetsValue();
			setWidgetsSensitivity();
		} else {
			onButtonRefresh();
		}
	} else {
		assert(sRetType == JsonCommon::s_sValueReturnTypeOk);
		if (bWaitingForState) {
			m_oCurState = oReturnValue[JsonCommon::s_sKeyReturnParam];
			setWidgetsValue();
			setWidgetsSensitivity();
		} else {
			onButtonRefresh();
		}
	}
}
void TootherWindow::onTimeout(int32_t nStamp, int32_t nMillisec)
{
//std::cout << "TootherWindow::onTimeout() nMillisec = " << nMillisec << "" << '\n';
	if (nStamp != m_nStamp) {
		// mega old timeout
		return; //----------------------------------------------------------
	}
	if (!m_bWaitingForReturn) {
		// Return json already received
		return; //----------------------------------------------------------
	}
	cursorToNormal();
	//
	printStringToLog("Last command timed out (" + std::to_string(nMillisec) + " millisec)!");
	//TODO message box
	//
	m_bWaitingForReturn = false;
	const bool bWaitingForState = m_bWaitingForState;
	m_bWaitingForState = false;
	if (bWaitingForState) {
		// Refresh error, set state to null!
		m_oCurState = json{};
	}
	setWidgetsValue();
	setWidgetsSensitivity();
}
void TootherWindow::onNotebookSwitchPage(Gtk::Widget*, guint nPageNum)
{
	if (nPageNum >= s_nTotPages) {
		return;
	}
}

void TootherWindow::onButtonRefresh()
{
//std::cout << "TootherWindow::onButtonRefresh()" << '\n';
	assert(!m_bWaitingForReturn);
	startRefreshing(s_nCmdRefreshTimeout);
}
void TootherWindow::onAdapterSelectionChanged()
{
//std::cout << "TootherWindow::onAdapterSelectionChanged()" << '\n';
	if (m_bSettingWidgetsValues) {
		return;
	}
	assert(!m_bWaitingForReturn);
	Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_p0TreeViewAdapters->get_selection();
	Gtk::TreeModel::iterator it = refTreeSelection->get_selected();
	if (it)	{
		Gtk::TreeModel::Row oRow = *it;
		const int32_t nHciId = oRow[m_oAdaptersColumns.m_oColHiddenHciId];
		m_nSelectedHciId = nHciId;
	} else {
		m_nSelectedHciId = -1;
	}
	setWidgetsValue();
	setWidgetsSensitivity();
}
void TootherWindow::execAdapterBoolCmd(const std::string sCmd, bool bEnabled, int32_t nTimeout)
{
	cursorToHourglass();

	m_bWaitingForReturn = true;
	m_bWaitingForState = false;
	setWidgetsSensitivity();
	//
	++m_nStamp;
	const json oRoot = JsonCommon::buildCmd(m_nStamp, sCmd, m_nSelectedHciId, bEnabled);
	sendCmd(oRoot.dump(4));
	startTimeout(m_nStamp, nTimeout);
}
void TootherWindow::execAdapterStringCmd(const std::string sCmd, const std::string& sStr, int32_t nTimeout)
{
	cursorToHourglass();

	m_bWaitingForReturn = true;
	m_bWaitingForState = false;
	setWidgetsSensitivity();
	//
	++m_nStamp;
	const json oRoot = JsonCommon::buildCmd(m_nStamp, sCmd, m_nSelectedHciId, sStr);
	sendCmd(oRoot.dump(4));
	startTimeout(m_nStamp, nTimeout);
}
void TootherWindow::onSoftwareEnabledToggled()
{
	if (m_bSettingWidgetsValues) {
		return;
	}
	assert(!m_bWaitingForReturn);
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	const bool bEnabled = m_p0CheckButtonSoftwareEnabled->get_active();
	execAdapterBoolCmd(JsonCommon::s_sValueCmdSetAdapterSoftwareEnabled, bEnabled, s_nCmdSetAdapterBoolTimeout);
}
void TootherWindow::onAdapterIsUpToggled()
{
	if (m_bSettingWidgetsValues) {
		return;
	}
	assert(!m_bWaitingForReturn);
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	const bool bIsUp = m_p0CheckButtonAdapterIsUp->get_active();
	execAdapterBoolCmd(JsonCommon::s_sValueCmdSetAdapterIsUp, bIsUp, s_nCmdSetAdapterBoolTimeout);
}
void TootherWindow::onButtonSetName()
{
	if (m_bSettingWidgetsValues) {
		return; //--------------------------------------------------------------
	}
	assert(!m_bWaitingForReturn);
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	std::string sLocalName = m_p0EntryAdapterLocalName->get_text();
	if (!m_refNameDialog) {
		m_refNameDialog = Glib::RefPtr<NameDialog>(new NameDialog(*this, s_nMaxLocalNameSize));
	}
	m_refNameDialog->set_position(Gtk::WIN_POS_CENTER_ON_PARENT);
//std::cout << "-------> sName=" << sName << '\n';
	const int nRet = m_refNameDialog->run(sLocalName, m_nSelectedHciId);
	m_refNameDialog->hide();
	if (nRet == NameDialog::s_nRetOk) {
		sLocalName = m_refNameDialog->getName();
//std::cout << sLocalName << '\n';
		execAdapterStringCmd(JsonCommon::s_sValueCmdSetAdapterName, sLocalName, s_nCmdSetAdapterNameTimeout);
	}
}
void TootherWindow::onAdapterConnectableToggled()
{
	if (m_bSettingWidgetsValues) {
		return;
	}
	assert(!m_bWaitingForReturn);
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	const bool bIsConnectable = m_p0CheckButtonAdapterConnectable->get_active();
	execAdapterBoolCmd(JsonCommon::s_sValueCmdSetAdapterConnectable, bIsConnectable, s_nCmdSetAdapterBoolTimeout);
}
void TootherWindow::onAdapterDetectableToggled()
{
	if (m_bSettingWidgetsValues) {
		return;
	}
	assert(!m_bWaitingForReturn);
	if (m_nSelectedHciId < 0) {
		return; //--------------------------------------------------------------
	}
	const bool bIsDetectable = m_p0CheckButtonAdapterDetectable->get_active();
	execAdapterBoolCmd(JsonCommon::s_sValueCmdSetAdapterDetectable, bIsDetectable, s_nCmdSetAdapterBoolTimeout);
}
void TootherWindow::onServiceRunningToggled()
{
	if (m_bSettingWidgetsValues) {
		return;
	}
	assert(!m_bWaitingForReturn);
	const bool bServiceRunning = m_p0CheckButtonServiceRunning->get_active();
	execAdapterBoolCmd(JsonCommon::s_sValueCmdSetServiceRunning, bServiceRunning, s_nCmdSetServiceRunningTimeout);
}
void TootherWindow::onServiceEnabledToggled()
{
	if (m_bSettingWidgetsValues) {
		return;
	}
	assert(!m_bWaitingForReturn);
	const bool bServiceEnabled = m_p0CheckButtonServiceEnabled->get_active();
	execAdapterBoolCmd(JsonCommon::s_sValueCmdSetServiceEnabled, bServiceEnabled, s_nCmdSetServiceEnabledTimeout);
}

void TootherWindow::sendCmd(const std::string& sStr)
{
	int32_t nCount = 0;
	auto nToWrite = sStr.length() + 1; // include the 0
	auto p0Str = sStr.c_str();
	do {
		const auto nWritten = ::write(m_nCmdPipeFD, p0Str, nToWrite);
		if (nWritten < 0) {
			quitOnFatalError(std::string("Error: could not send cmd (error ") + std::to_string(errno) + ")");
			return; //------------------------------------------------------
		}
		p0Str += nWritten;
		nToWrite -= nWritten;
		++nCount;
		if (nCount >= 10) {
			quitOnFatalError("Error: could not send cmd (cannot write to pipe)");
			return; //------------------------------------------------------
		}
	} while (nToWrite > 0);
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
void TootherWindow::quitOnFatalError(const std::string& sErr)
{
	std::cerr << sErr << '\n';
	Gtk::MessageDialog oDlg(sErr, false
							, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, false);
	oDlg.set_transient_for(*this);
	oDlg.run();
	TootherWindow::close();
}

} // namespace stmi

