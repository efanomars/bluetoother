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
 * File:   tootherserver.h
 */

#ifndef STMI_TOOTHER_SERVER_H
#define STMI_TOOTHER_SERVER_H


#include "hcisocket.h"
#include "btservice.h"
#include "hciadapter.h"
#include "toothersources.h"

#include "jsoncommon.h"

#include "nlohmann/json.hpp"

#include <glibmm.h>

#include <string>

#include <stdint.h>

namespace stmi
{

using json = nlohmann::json;

class TootherServer
{
public:
	TootherServer(int nCmdPipeFD, int nReturnPipeFD, bool bPrintOutServerErrors) noexcept;
	int run() noexcept;

private:
	void sendReturn(const std::string& sStr) noexcept;
	void doReceiveString(bool bError, const std::string& sStr) noexcept;

	void startTimeout(int32_t nStamp, int32_t nMillisec, bool bSendState) noexcept;
	void onTimeout(int32_t nStamp, bool bSendState) noexcept;

	void cmdGetState(int32_t nStamp) noexcept;
	void cmdSetAdapterSoftwareEnabled(int32_t nStamp, int32_t nAdapter, bool bEnabled) noexcept;
	void cmdSetAdapterIsUp(int32_t nStamp, int32_t nAdapter, bool bIsUp) noexcept;
	void cmdSetAdapterName(int32_t nStamp, int32_t nAdapter, const std::string& sName) noexcept;
	void cmdSetAdapterConnectable(int32_t nStamp, int32_t nAdapter, bool bConnectable) noexcept;
	void cmdSetAdapterDetectable(int32_t nStamp, int32_t nAdapter, bool bDetectable) noexcept;
	void cmdServiceRunning(int32_t nStamp, bool bRun) noexcept;
	void cmdServiceEnabled(int32_t nStamp, bool bEnable) noexcept;

	template<typename TCapa>
	void cmdSetAdapterProperty(int32_t nStamp, int32_t nAdapter, int32_t nMillisec, TCapa oCapa) noexcept
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
		startTimeout(nStamp, nMillisec, false);
	}

	json getState() noexcept;
private:
	int m_nCmdPipeFD;
	int m_nReturnPipeFD;
	bool m_bPrintOutServerErrors;

	Glib::RefPtr<Glib::MainLoop> m_refML;

	// service
	BtService m_oBtService;
	// hci model
	HciSocket m_oHciSocket;

	Glib::RefPtr<PipeInputSource> m_refPipeInputSource;


	static constexpr const int32_t s_nCmdSetSoftwareEnabled = 300; // millisec
	static constexpr const int32_t s_nCmdSetServiceRunningTimeout = 2500; // millisec
	static constexpr const int32_t s_nCmdSetServiceEnabledTimeout = 1500; // millisec
private:
	TootherServer(const TootherServer& oSource) = delete;
	TootherServer& operator=(const TootherServer& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_TOOTHER_SERVER_H */

