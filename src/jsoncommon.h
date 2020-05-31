/*
 * Copyright © 2017-2019  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   jsoncommon.h
 */

#ifndef STMI_JSON_COMMON_H
#define STMI_JSON_COMMON_H

#include "nlohmann/json.hpp"

#include <string>

#include <stdint.h>


namespace stmi
{

using json = nlohmann::json;

class JsonCommon
{
public:
	static json buildCmd(int32_t nStamp, const std::string& sCmd) noexcept;
	static json buildCmd(int32_t nStamp, const std::string& sCmd, int32_t nAdapter, const bool bParam) noexcept;
	static json buildCmd(int32_t nStamp, const std::string& sCmd, int32_t nAdapter, const std::string& sParam) noexcept;

	static json buildReturnOk(int32_t nStamp) noexcept;
	static json buildReturnOk(int32_t nStamp, json&& oState) noexcept;
	static json buildReturnError(int32_t nStamp, const std::string& sError) noexcept;

	static json parseString(const std::string& sStr) noexcept;
public:
	static std::string s_sKeyStamp;
	static std::string s_sKeyCmd;
	static std::string s_sValueCmdGetState;
	static std::string s_sValueCmdSetAdapterSoftwareEnabled;
	static std::string s_sValueCmdSetAdapterIsUp;
	static std::string s_sValueCmdSetAdapterName;
	static std::string s_sValueCmdSetAdapterConnectable;
	static std::string s_sValueCmdSetAdapterDetectable;
	static std::string s_sValueCmdSetServiceRunning;
	static std::string s_sValueCmdSetServiceEnabled;
	static std::string s_sKeyCmdAdapter;
	static std::string s_sKeyCmdParam;
	static std::string s_sKeyReturnType;
	static std::string s_sValueReturnTypeOk;
	static std::string s_sValueReturnTypeError;
	static std::string s_sKeyReturnParam;

	static std::string s_sKeyModelAdapters;
	static std::string s_sKeyModelAdapterHciId;
	static std::string s_sKeyModelAdapterAddress;
	static std::string s_sKeyModelAdapterHardwareEnabled;
	static std::string s_sKeyModelAdapterSoftwareEnabled;
	static std::string s_sKeyModelAdapterIsUp;
	static std::string s_sKeyModelAdapterName;
	static std::string s_sKeyModelAdapterConnectable;
	static std::string s_sKeyModelAdapterDetectable;
	static std::string s_sKeyModelServiceRunning;
	static std::string s_sKeyModelServiceEnabled;

	static std::string s_sKeyModelFaultyAdapters;
	static std::string s_sKeyModelFaultyAdapterHciId;
	static std::string s_sKeyModelFaultyAdapterError;
private:
	JsonCommon() = delete;
	JsonCommon(const JsonCommon& oSource) = delete;
	JsonCommon& operator=(const JsonCommon& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_JSON_COMMON_H */
