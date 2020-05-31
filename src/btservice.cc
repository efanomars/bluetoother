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
 * File:   btservice.cc
 */
#include "btservice.h"

//#include <iostream>
#include <array>

#include <errno.h>
#include <stdio.h>
#include <string.h>

namespace stmi
{

BtService::BtService() noexcept
: m_bUpdatedOnce(false)
, m_bServiceRunning(false)
, m_bServiceEnabled(false)
{
}
bool BtService::isServiceRunning() noexcept
{
	if (!m_bUpdatedOnce) {
		update();
	}
	return m_bServiceRunning;
}
bool BtService::isServiceEnabled() noexcept
{
	if (!m_bUpdatedOnce) {
		update();
	}
	return m_bServiceEnabled;
}
bool BtService::update() noexcept
{
//std::cout << "BtService::update()" << '\n';
	m_sLastError = "";
	m_bUpdatedOnce = true;
	std::string sRes;
	//
	execCmd("systemctl is-enabled bluetooth.service", sRes);
//std::cout << "BtService::update() >>>>> sRes=" << sRes << '\n';
	static const std::string s_sEnabled = "enabled";
	static const std::string s_sDisabled = "disabled";
	const bool bIsEnabled = (sRes.substr(0, s_sEnabled.length()) == s_sEnabled);
	if (! bIsEnabled) {
		const bool bIsDisabled = (sRes.substr(0, s_sDisabled.length()) == s_sDisabled);
		if (! bIsDisabled) {
			m_sLastError = "Bluetooth service is neither enabled nor disabled:\n -> bluez package not installed?";
			return false; //------------------------------------------------
		}
	}
	m_bServiceEnabled = bIsEnabled;
	//
	m_bServiceRunning = execCmd("systemctl is-active bluetooth.service", sRes);
//std::cout << "BtService::update() m_bServiceRunning=" << m_bServiceRunning << "  m_bServiceEnabled=" << m_bServiceEnabled << '\n';
	return true;
}
bool BtService::startService() noexcept
{
	m_sLastError = "";
	std::string sRes;
	const bool bRes = execCmd("systemctl start bluetooth.service", sRes);
	if (!bRes) {
		return false; //--------------------------------------------------------
	}
	m_bServiceRunning = true;
	return true;
}
bool BtService::stopService() noexcept
{
	m_sLastError = "";
	std::string sRes;
	const bool bRes = execCmd("systemctl stop bluetooth.service", sRes);
	if (!bRes) {
		return false; //--------------------------------------------------------
	}
	m_bServiceRunning = false;
	return true;
}
bool BtService::enableService(bool bEnabled) noexcept
{
	m_sLastError = "";
	std::string sRes;
	const bool bRes = execCmd(bEnabled ? "systemctl enable bluetooth.service" : "systemctl disable bluetooth.service", sRes);
	if (!bRes) {
		return false; //--------------------------------------------------------
	}
	m_bServiceEnabled = bEnabled;
	return true;
}

bool BtService::execCmd(const char* sCmd, std::string& sResult) noexcept
{
	sResult.clear();
	std::array<char, 128> aBuffer;
	FILE* pFile = ::popen(sCmd, "r");
	if (pFile == nullptr) {
		m_sLastError = std::string("Error: popen() failed: ") + ::strerror(errno) + "(" + std::to_string(errno) + ")";
		return false; //--------------------------------------------------------
	}
	while (!::feof(pFile)) {
		if (::fgets(aBuffer.data(), sizeof(aBuffer), pFile) != nullptr) {
			sResult += aBuffer.data();
		}
	}
	const auto nRet = ::pclose(pFile);
	return (nRet == 0);
}

} // namespace stmi
