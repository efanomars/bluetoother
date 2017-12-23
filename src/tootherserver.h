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
 * File:   tootherserver.h
 */

#ifndef STMI_TOOTHER_SERVER_H
#define STMI_TOOTHER_SERVER_H


#include "hcisocket.h"
#include "btservice.h"

#include "toothersources.h"

#include "jsoncommon.h"

#include <glibmm.h>

#include <vector>
#include <stdexcept>

namespace stmi
{

using json = nlohmann::json;

class TootherServer
{
public:
	TootherServer(int nCmdPipeFD, int nReturnPipeFD, bool bPrintOutServerErrors);
	int run();

private:
	void sendReturn(const std::string& sStr);
	void doReceiveString(bool bError, const std::string& sStr);

	void startTimeout(int32_t nStamp, int32_t nMillisec, bool bSendState);
	void onTimeout(int32_t nStamp, bool bSendState);

	void cmdGetState(int32_t nStamp);
	void cmdSetAdapterSoftwareEnabled(int32_t nStamp, int32_t nAdapter, bool bEnabled);
	void cmdSetAdapterIsUp(int32_t nStamp, int32_t nAdapter, bool bIsUp);
	void cmdSetAdapterName(int32_t nStamp, int32_t nAdapter, const std::string& sName);
	void cmdSetAdapterConnectable(int32_t nStamp, int32_t nAdapter, bool bConnectable);
	void cmdSetAdapterDetectable(int32_t nStamp, int32_t nAdapter, bool bDetectable);
	void cmdServiceRunning(int32_t nStamp, bool bRun);
	void cmdServiceEnabled(int32_t nStamp, bool bEnable);
// 	void cmdAllOn(int32_t nStamp);
//	void cmdDisconnect();

	template<typename TCapa>
	void cmdSetAdapterProperty(int32_t nStamp, int32_t nAdapter, TCapa oCapa)
	{
		auto p0Adapter = m_oHciSocket.getAdapter(nAdapter);
		if (p0Adapter == nullptr) {
			json oReturnValue = JsonCommon::buildReturnError(nStamp
					, "Fatal error: adapter with id " + std::to_string(nAdapter) + " not found!");
			sendReturn(oReturnValue.dump(4));
			return; //--------------------------------------------------------------
		}
		const bool bOk = oCapa(p0Adapter);
		json oReturnValue;
		if (!bOk) {
			sendReturn(JsonCommon::buildReturnError(nStamp, p0Adapter->getLastError()).dump(4));
			return; //--------------------------------------------------------------
		}
		startTimeout(nStamp, 0, false);
	}

	json getState();
private:
	int m_nReturnPipeFD;
	bool m_bPrintOutServerErrors;

	Glib::RefPtr<Glib::MainLoop> m_refML;

	// service
	BtService m_oBtService;
	// hci model
	HciSocket m_oHciSocket;

	Glib::RefPtr<PipeInputSource> m_refPipeInputSource;

// 	// set to true when shutting down to avoid printing errors
// 	bool m_bDisconnected;

	static constexpr int32_t s_nCmdSetServiceRunningTimeout = 2500; // millisec
	static constexpr int32_t s_nCmdSetServiceEnabledTimeout = 1500; // millisec
private:
	TootherServer(const TootherServer& oSource) = delete;
	TootherServer& operator=(const TootherServer& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_TOOTHER_SERVER_H */

