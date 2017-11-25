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
 * File:   btservice.cc
 */
#include "btservice.h"

#include <cassert>
#include <iostream>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <unistd.h>

namespace stmi
{

BtService::BtService()
: m_bUpdatedOnce(false)
, m_bServiceRunning(false)
, m_bServiceEnabled(false)
{
}
bool BtService::isServiceRunning()
{
	if (!m_bUpdatedOnce) {
		update();
	}
	return m_bServiceRunning;
}
bool BtService::isServiceEnabled()
{
	if (!m_bUpdatedOnce) {
		update();
	}
	return m_bServiceEnabled;
}
bool BtService::update()
{
//std::cout << "BtService::update()" << '\n';
	m_sLastError = "";
	m_bUpdatedOnce = true;
	std::string sRes;
	{
		const bool bRes = execCmd("systemctl is-active bluetooth.service", sRes);
		if (!bRes) {
			return false; //----------------------------------------------------
		}
	}
	static const std::string s_sActive = "active";
	m_bServiceRunning = (sRes.substr(0, s_sActive.length()) == s_sActive);
	//
	const bool bRes = execCmd("systemctl is-enabled bluetooth.service", sRes);
	if (!bRes) {
		return false; //--------------------------------------------------------
	}
//std::cout << "BtService::update() >>>>> sRes=" << sRes << '\n';
	static const std::string s_sEnabled = "enabled";
	m_bServiceEnabled = (sRes.substr(0, s_sEnabled.length()) == s_sEnabled);
//std::cout << "BtService::update() m_bServiceRunning=" << m_bServiceRunning << "  m_bServiceEnabled=" << m_bServiceEnabled << '\n';
	return true;
}
bool BtService::startService()
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
bool BtService::stopService()
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
bool BtService::enableService(bool bEnabled)
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

bool BtService::execCmd(const char* sCmd, std::string& sResult)
{
	sResult.clear();
	std::array<char, 128> aBuffer;
	std::shared_ptr<FILE> refPipe(popen(sCmd, "r"), pclose);
	if (!refPipe) {
		m_sLastError = std::string("Error: popen() failed: ") + strerror(errno) + "(" + std::to_string(errno) + ")";
		return false; //--------------------------------------------------------
	}
	while (!::feof(refPipe.get())) {
		if (::fgets(aBuffer.data(), sizeof(aBuffer), refPipe.get()) != nullptr) {
			sResult += aBuffer.data();
		}
	}
	return true;
}

} // namespace stmi
