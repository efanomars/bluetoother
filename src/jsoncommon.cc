/*
 * Copyright © 2017  Stefano Marsili, <stemars@gmx.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   jsoncommon.cc
 */

#include "jsoncommon.h"

#include <iostream>
#include <cassert>
#include <sstream>


namespace stmi
{

std::string JsonCommon::s_sKeyStamp = "Stamp";

std::string JsonCommon::s_sKeyCmd = "Cmd";
std::string JsonCommon::s_sValueCmdGetState = "GET_STATE";
std::string JsonCommon::s_sValueCmdSetAdapterSoftwareEnabled = "SET_ADAPTER_ENABLED";
std::string JsonCommon::s_sValueCmdSetAdapterIsUp = "SET_ADAPTER_IS_UP";
std::string JsonCommon::s_sValueCmdSetAdapterName = "SET_ADAPTER_NAME";
std::string JsonCommon::s_sValueCmdSetAdapterConnectable = "SET_ADAPTER_CONNECTABLE";
std::string JsonCommon::s_sValueCmdSetAdapterDetectable = "SET_ADAPTER_DETECTABLE";
std::string JsonCommon::s_sValueCmdSetServiceRunning = "SET_SERVICE_RUNNING";
std::string JsonCommon::s_sValueCmdSetServiceEnabled = "SET_SERVICE_ENABLED";
std::string JsonCommon::s_sKeyCmdAdapter = "AdapterNr";
std::string JsonCommon::s_sKeyCmdParam = "Param";

std::string JsonCommon::s_sKeyReturnType = "Type";
std::string JsonCommon::s_sValueReturnTypeOk = "OK";
std::string JsonCommon::s_sValueReturnTypeError = "ERROR";
std::string JsonCommon::s_sKeyReturnParam = "Param";

std::string JsonCommon::s_sKeyModelAdapters = "Adapters";
std::string JsonCommon::s_sKeyModelAdapterHciId = "HciId";
std::string JsonCommon::s_sKeyModelAdapterAddress = "Address";
std::string JsonCommon::s_sKeyModelAdapterHardwareEnabled = "HardwareEnabled";
std::string JsonCommon::s_sKeyModelAdapterSoftwareEnabled = "SoftwareEnabled";
std::string JsonCommon::s_sKeyModelAdapterIsUp = "IsUp";
std::string JsonCommon::s_sKeyModelAdapterName = "Name";
std::string JsonCommon::s_sKeyModelAdapterConnectable = "Connectable";
std::string JsonCommon::s_sKeyModelAdapterDetectable = "Detectable";
std::string JsonCommon::s_sKeyModelServiceRunning = "ServiceRunning";
std::string JsonCommon::s_sKeyModelServiceEnabled = "ServiceEnabled";

std::string JsonCommon::s_sKeyModelFaultyAdapters = "FaultyAdapters";
std::string JsonCommon::s_sKeyModelFaultyAdapterHciId = "HciId";
std::string JsonCommon::s_sKeyModelFaultyAdapterError = "Error";

json JsonCommon::parseString(const std::string& sStr)
{
	std::istringstream oStrStream;
	oStrStream.str(sStr);
	//
	try {
		return json::parse(oStrStream);
		//
	} catch (json::parse_error& e) {
		json oRoot;
		return oRoot;
	}
}
json JsonCommon::buildCmd(int32_t nStamp, const std::string& sCmd)
{
	assert(!sCmd.empty());
	json oCmdValue = json::object();
	oCmdValue[s_sKeyStamp] = nStamp;
	oCmdValue[s_sKeyCmd] = sCmd;
	return oCmdValue;
}
json JsonCommon::buildCmd(int32_t nStamp, const std::string& sCmd, int32_t nAdapter, bool bParam)
{
	assert(!sCmd.empty());
	json oCmdValue = json::object();
	oCmdValue[s_sKeyStamp] = nStamp;
	oCmdValue[s_sKeyCmd] = sCmd;
	if (nAdapter >= 0) {
		oCmdValue[s_sKeyCmdAdapter] = nAdapter;
	}
	oCmdValue[s_sKeyCmdParam] = bParam;
	return oCmdValue;
}
json JsonCommon::buildCmd(int32_t nStamp, const std::string& sCmd, int32_t nAdapter, const std::string& sParam)
{
	assert(!sCmd.empty());
	json oCmdValue = json::object();
	oCmdValue[s_sKeyStamp] = nStamp;
	oCmdValue[s_sKeyCmd] = sCmd;
	if (nAdapter >= 0) {
		oCmdValue[s_sKeyCmdAdapter] = nAdapter;
	}
	oCmdValue[s_sKeyCmdParam] = sParam;
	return oCmdValue;
}
json JsonCommon::buildReturnError(int32_t nStamp, const std::string& sError)
{
	json oReturnValue = json::object();
	oReturnValue[s_sKeyStamp] = nStamp;
	oReturnValue[s_sKeyReturnType] = s_sValueReturnTypeError;
	oReturnValue[s_sKeyReturnParam] = sError;
	return oReturnValue;
}
json JsonCommon::buildReturnOk(int32_t nStamp)
{
	json oReturnValue = json::object();
	oReturnValue[s_sKeyStamp] = nStamp;
	oReturnValue[JsonCommon::s_sKeyReturnType] = s_sValueReturnTypeOk;
	oReturnValue[JsonCommon::s_sKeyReturnParam] = json{};
	return oReturnValue;
}
json JsonCommon::buildReturnOk(int32_t nStamp, json&& oState)
{
	json oReturnValue = json::object();
	oReturnValue[s_sKeyStamp] = nStamp;
	oReturnValue[JsonCommon::s_sKeyReturnType] = s_sValueReturnTypeOk;
	oReturnValue[JsonCommon::s_sKeyReturnParam] = std::move(oState);
	return oReturnValue;
}

} // namespace stmi
