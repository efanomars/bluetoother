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
 * File:   tootherserver.cc
 */
#include "tootherserver.h"

#include <cassert>
#include <iostream>

#include <unistd.h>

namespace stmi
{

TootherServer::TootherServer(int nCmdPipeFD, int nReturnPipeFD)
: m_nReturnPipeFD(nReturnPipeFD)
{
	m_refML = Glib::MainLoop::create();

	m_refPipeInputSource = Glib::RefPtr<PipeInputSource>(new PipeInputSource(nCmdPipeFD));
	m_refPipeInputSource->connect(sigc::mem_fun(*this, &TootherServer::doReceiveString));
	m_refPipeInputSource->attach();
}
int TootherServer::run()
{
	m_refML->run();
	return EXIT_SUCCESS;
}
void TootherServer::doReceiveString(bool bError, const std::string& sStr)
{
//std::cout << "TootherServer::doReceiveString bError=" << bError << " " << sStr << '\n';
	if (bError) {
		m_refML->quit();
		return; //----------------------------------------------------------
	}
	//
	json oCmdValue = JsonCommon::parseString(sStr);
	assert(! oCmdValue.is_null());
	const int32_t nStamp = oCmdValue.at(JsonCommon::s_sKeyStamp).get<int32_t>();
	const std::string sCmd = oCmdValue.at(JsonCommon::s_sKeyCmd).get<std::string>();
	int32_t nAdapter = -1;
	try {
		nAdapter = oCmdValue.at(JsonCommon::s_sKeyCmdAdapter).get<int32_t>();
	} catch (json::out_of_range) {
	}
	json oParam;
	try {
		oParam = oCmdValue.at(JsonCommon::s_sKeyCmdParam);
	} catch (json::out_of_range) {
	}
	//
	if (sCmd == JsonCommon::s_sValueCmdGetState) {
		cmdGetState(nStamp);
	} else if(sCmd == JsonCommon::s_sValueCmdSetAdapterSoftwareEnabled) {
		cmdSetAdapterSoftwareEnabled(nStamp, nAdapter, oParam.get<bool>());
	} else if(sCmd == JsonCommon::s_sValueCmdSetAdapterIsUp) {
		cmdSetAdapterIsUp(nStamp, nAdapter, oParam.get<bool>());
	} else if(sCmd == JsonCommon::s_sValueCmdSetAdapterName) {
		cmdSetAdapterName(nStamp, nAdapter, oParam.get<std::string>());
	} else if(sCmd == JsonCommon::s_sValueCmdSetAdapterConnectable) {
		cmdSetAdapterConnectable(nStamp, nAdapter, oParam.get<bool>());
	} else if(sCmd == JsonCommon::s_sValueCmdSetAdapterDetectable) {
		cmdSetAdapterDetectable(nStamp, nAdapter, oParam.get<bool>());
	} else if(sCmd == JsonCommon::s_sValueCmdSetServiceRunning) {
		cmdServiceRunning(nStamp, oParam.get<bool>());
	} else if(sCmd == JsonCommon::s_sValueCmdSetServiceEnabled) {
		cmdServiceEnabled(nStamp, oParam.get<bool>());
	} else {
		assert(false);
	}
}
void TootherServer::sendReturn(const std::string& sStr)
{
//std::cout << "TootherServer::sendReturn:\n" << sStr << '\n';
	int32_t nCount = 0;
	auto nToWrite = sStr.length() + 1; // include the 0
	auto p0Str = sStr.c_str();
	do {
//std::cout << "TootherServer::sendReturn pipe:" << m_nReturnPipeFD << " count:" << nCount << "  nToWrite=" << nToWrite << '\n';
		const auto nWritten = ::write(m_nReturnPipeFD, p0Str, nToWrite);
		if (nWritten < 0) {
			std::cerr << "Error: could not send return value (error " << errno << ")" << '\n';
			m_refML->quit();
			return; //------------------------------------------------------
		}
		p0Str += nWritten;
		nToWrite -= nWritten;
		++nCount;
		if (nCount >= 10) {
			std::cerr << "Error: could not send return value (cannot write)" << '\n';
			m_refML->quit();
			return; //------------------------------------------------------
		}
	} while (nToWrite > 0);
}
json TootherServer::getState()
{
	json oRoot = json::object();
	json oHciArr = json::array();
	const auto& aHciIds = m_oHciSocket.getHciIds();
	for (int32_t nHciId : aHciIds) {
		json oAdapter = json::object();
		oAdapter[JsonCommon::s_sKeyModelAdapterHciId] = nHciId;
		auto p0Adapter = m_oHciSocket.getAdapter(nHciId);
		assert(p0Adapter != nullptr);
		//
		oAdapter[JsonCommon::s_sKeyModelAdapterAddress] = p0Adapter->getAddress();
		oAdapter[JsonCommon::s_sKeyModelAdapterHardwareEnabled] = p0Adapter->isHardwareEnabled();
		oAdapter[JsonCommon::s_sKeyModelAdapterSoftwareEnabled] = p0Adapter->isSoftwareEnabled();
		oAdapter[JsonCommon::s_sKeyModelAdapterIsUp] = !p0Adapter->isDown();
		oAdapter[JsonCommon::s_sKeyModelAdapterName] = p0Adapter->getLocalName();
		oAdapter[JsonCommon::s_sKeyModelAdapterConnectable] = p0Adapter->isConnectable();
		oAdapter[JsonCommon::s_sKeyModelAdapterDetectable] = p0Adapter->isDetectable();
		//
		oHciArr.push_back(oAdapter);
	}
	oRoot[JsonCommon::s_sKeyModelAdapters] = oHciArr;
	//
	json oFaultyHciArr = json::array();
	const auto& aFaultyHciIds = m_oHciSocket.getFaultyHciIds();
	for (int32_t nHciId : aFaultyHciIds) {
		json oAdapter = json::object();
		oAdapter[JsonCommon::s_sKeyModelFaultyAdapterHciId] = nHciId;
		auto p0Adapter = m_oHciSocket.getAdapter(nHciId);
		assert(p0Adapter != nullptr);
		oAdapter[JsonCommon::s_sKeyModelFaultyAdapterError] = p0Adapter->getLastError();
	}
	oRoot[JsonCommon::s_sKeyModelFaultyAdapters] = oFaultyHciArr;
	//
	oRoot[JsonCommon::s_sKeyModelServiceRunning] = m_oBtService.isServiceRunning();
	oRoot[JsonCommon::s_sKeyModelServiceEnabled] = m_oBtService.isServiceEnabled();

	return oRoot;
}
void TootherServer::startTimeout(int32_t nStamp, int32_t nMillisec, bool bSendState)
{
	Glib::signal_timeout().connect_once(sigc::bind(sigc::mem_fun(*this, &TootherServer::onTimeout), nStamp, bSendState)
										, nMillisec);
}
void TootherServer::onTimeout(int32_t nStamp, bool bSendState)
{
//std::cout << "TootherServer::onTimeout() nStamp=" << nStamp << '\n';
	json oReturn;
	if (bSendState) {
		oReturn = JsonCommon::buildReturnOk(nStamp, getState());
	} else {
		oReturn = JsonCommon::buildReturnOk(nStamp);
	}
	sendReturn(oReturn.dump(4));
}
void TootherServer::cmdGetState(int32_t nStamp)
{
//std::cout << "TootherServer::cmdGetState" << '\n';
	const bool bSocketOk = m_oHciSocket.update();
	const bool bServiceOk = m_oBtService.update();
	std::string sError;
	if (!bSocketOk) {
		sError = m_oHciSocket.getLastError();
	}
	if (!bServiceOk) {
		sError = sError + (bSocketOk ? "" : "\n") + m_oBtService.getLastError();
	}
	if (! (bSocketOk && bServiceOk)) {
		sendReturn(JsonCommon::buildReturnError(nStamp, sError).dump(4));
		return; //----------------------------------------------------------
	}
	startTimeout(nStamp, 0, true);
}
void TootherServer::cmdSetAdapterSoftwareEnabled(int32_t nStamp, int32_t nAdapter, bool bEnabled)
{
//std::cout << "TootherServer::cmdSetAdapterSoftwareEnabled bEnabled=" << bEnabled << '\n';
	cmdSetAdapterProperty(nStamp, nAdapter, [&](HciAdapter* p0Adapter) {
		return p0Adapter->setSoftwareEnabled(bEnabled);
	});
}
void TootherServer::cmdSetAdapterIsUp(int32_t nStamp, int32_t nAdapter, bool bIsUp)
{
	cmdSetAdapterProperty(nStamp, nAdapter, [&](HciAdapter* p0Adapter) {
		return p0Adapter->setAdapterIsUp(bIsUp);
	});
}
void TootherServer::cmdSetAdapterName(int32_t nStamp, int32_t nAdapter, const std::string& sName)
{
	cmdSetAdapterProperty(nStamp, nAdapter, [&](HciAdapter* p0Adapter) {
		return p0Adapter->setLocalName(sName);
	});
}
void TootherServer::cmdSetAdapterConnectable(int32_t nStamp, int32_t nAdapter, bool bConnectable)
{
	cmdSetAdapterProperty(nStamp, nAdapter, [&](HciAdapter* p0Adapter) {
		return p0Adapter->setConnectable(bConnectable);
	});
}
void TootherServer::cmdSetAdapterDetectable(int32_t nStamp, int32_t nAdapter, bool bDetectable)
{
	cmdSetAdapterProperty(nStamp, nAdapter, [&](HciAdapter* p0Adapter) {
		return p0Adapter->setDetectable(bDetectable);
	});
}
void TootherServer::cmdServiceRunning(int32_t nStamp, bool bRun)
{
	const bool bOk = (bRun ? m_oBtService.startService() : m_oBtService.stopService());
	if (!bOk) {
		sendReturn(JsonCommon::buildReturnError(nStamp, m_oBtService.getLastError()).dump(4));
		return; //----------------------------------------------------------
	}
	startTimeout(nStamp, s_nCmdSetServiceRunningTimeout, false);
}
void TootherServer::cmdServiceEnabled(int32_t nStamp, bool bEnable)
{
	const bool bOk = m_oBtService.enableService(bEnable);
	json oReturnValue;
	if (!bOk) {
		sendReturn(JsonCommon::buildReturnError(nStamp, m_oBtService.getLastError()).dump(4));
		return; //----------------------------------------------------------
	}
	startTimeout(nStamp, s_nCmdSetServiceEnabledTimeout, false);
}

} // namespace stmi
