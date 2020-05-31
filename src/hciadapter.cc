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
/* Parts of code from bluez package
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 */
/* Parts of code from rfkill package
 * Copyright 2009 Johannes Berg <johannes@sipsolutions.net>
 * Copyright 2009 Marcel Holtmann <marcel@holtmann.org>
 * Copyright 2009 Tim Gardner <tim.gardner@canonical.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * File:   hciadapter.cc
 */
#include "hciadapter.h"

#include <cassert>
#include <iostream>

#include <sys/ioctl.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

namespace stmi
{

namespace Private
{
// See rfkill.h (rfkill package)
enum rfkill_type {
	RFKILL_TYPE_ALL = 0,
	RFKILL_TYPE_WLAN,
	RFKILL_TYPE_BLUETOOTH,
	RFKILL_TYPE_UWB,
	RFKILL_TYPE_WIMAX,
	RFKILL_TYPE_WWAN,
	RFKILL_TYPE_GPS,
	RFKILL_TYPE_FM,
	RFKILL_TYPE_NFC,
	NUM_RFKILL_TYPES,
};
enum rfkill_operation {
	RFKILL_OP_ADD = 0,
	RFKILL_OP_DEL,
	RFKILL_OP_CHANGE,
	RFKILL_OP_CHANGE_ALL,
};

struct rfkill_event {
	uint32_t idx; //was __u32 idx;
	uint8_t type; //was __u8  type;
	uint8_t  op; //was __u8  op;
	uint8_t  soft, hard; //was __u8  soft, hard;
} /*__packed*/;

#define RFKILL_EVENT_SIZE_V1	8

} // namespace Private

using namespace Private;

bool HciAdapter::reInit(int32_t nHciSocket, int32_t nHciId) noexcept
{
//std::cout << "HciAdapter::reInit()  nHciSocket=" << nHciSocket << "  nHciId=" << nHciId << '\n';
	m_sLastError.clear();
	assert(nHciSocket >= 0);
	assert(nHciId >= 0);
	m_nHciId = nHciId;
	struct ::hci_dev_info oDevInfo;
	oDevInfo.dev_id = nHciId;
	if (::ioctl(nHciSocket, HCIGETDEVINFO, static_cast<void *>(&oDevInfo)) < 0) {
		setError("Can't get info on");
		m_nHciId = -1;
		return false; //--------------------------------------------------------
	}
	m_nHciSocket = nHciSocket;
	m_sAdapterName = oDevInfo.name; // "hci0"
	char sAddr[18];
	::ba2str(&oDevInfo.bdaddr, sAddr);
	m_sAddress = sAddr;
	return update(oDevInfo);
}
bool HciAdapter::update(const struct ::hci_dev_info& oDevInfo) noexcept
{
	uint32_t nFlags = oDevInfo.flags;
	m_bIsDown = !::hci_test_bit(HCI_UP, &nFlags);
	if (!m_bIsDown) {
		m_bConnectable = ::hci_test_bit(HCI_PSCAN, &nFlags);
		m_bDetectable = ::hci_test_bit(HCI_ISCAN, &nFlags);
		if (!updateLocalName()) {
			return false; //----------------------------------------------------
		}
	} else {
		m_bConnectable = false;
		m_bDetectable = false;
		m_sLocalName.clear();
	}
	return updateRfKill();
}
bool HciAdapter::setConnectable(bool bConnectable) noexcept
{
	m_bConnectable = bConnectable;
	return setScan();
}
bool HciAdapter::setDetectable(bool bDetectable) noexcept
{
	m_bDetectable = bDetectable;
	return setScan();
}
bool HciAdapter::setScan() noexcept
{
	m_sLastError.clear();
	// See char *::hci_dflagstostr(uint32_t flags) in lib/hci.c (bluez package)
	uint32_t nScan = SCAN_DISABLED;
	if (m_bConnectable && m_bDetectable) {
		nScan = SCAN_PAGE | SCAN_INQUIRY;
	} else if (m_bConnectable) {
		nScan = SCAN_PAGE;
	} else if (m_bDetectable) {
		nScan = SCAN_INQUIRY;
	}

	struct ::hci_dev_req oDr;

	oDr.dev_id  = m_nHciId;
	oDr.dev_opt = nScan;

	if (::ioctl(m_nHciSocket, HCISETSCAN, reinterpret_cast<unsigned long>(&oDr)) < 0) {
		setError("Can't set scan mode on");
		return false;
	}
	return true;
}
bool HciAdapter::setLocalName(const std::string& sName) noexcept
{
	m_sLastError.clear();
	// See cmd_name(int ctl, int hdev, char *opt) in tools/hciconfig.c (bluez package)
	auto nDd = ::hci_open_dev(m_nHciId);
	if (nDd < 0) {
		setError("Can't open device");
		return false; //--------------------------------------------------------
	}

	if (::hci_write_local_name(nDd, sName.c_str(), 2000) < 0) {
		setError("Can't change local name on");
		::hci_close_dev(nDd);
		return false; //--------------------------------------------------------
	}
	
	::hci_close_dev(nDd);
	return true;
}
bool HciAdapter::updateLocalName() noexcept
{
//std::cout << "HciAdapter::updateLocalName()" << '\n';
	// See cmd_name(int ctl, int hdev, char *opt) in tools/hciconfig.c (bluez package)
	auto nDd = ::hci_open_dev(m_nHciId);
	if (nDd < 0) {
		setError("Can't open device");
		return false; //--------------------------------------------------------
	}

	char sName[249];

	if (::hci_read_local_name(nDd, sizeof(sName), sName, 1000) < 0) {
		setError("Can't read local name on");
		::hci_close_dev(nDd);
		return false; //--------------------------------------------------------
	}

	for (int32_t nI = 0; nI < 248; nI++) {
		const auto nC = static_cast<unsigned char>(sName[nI]);
		if (nC == 0) {
			break; // for----
		}
		if ((nC < 32) || (nC == 127)) {
			sName[nI] = '.';
		}
	}
	sName[sizeof(sName) - 1] = '\0';
	m_sLocalName = sName;
	::hci_close_dev(nDd);
//std::cout << "HciAdapter::updateLocalName() m_sLocalName=" << m_sLocalName << '\n';
	return true;
}
// See int rfkill_list(const char *param) in rfkill.c (rfkill package)
bool HciAdapter::updateRfKill() noexcept
{
//std::cout << "HciAdapter::updateRfKill()" << '\n';
	m_bHardwareEnabled = true;
	m_bSoftwareEnabled = true;
	m_nRfKillIdx = -1;

	struct rfkill_event oEvent __attribute__ ((aligned(__alignof__(rfkill_event))));

	const auto nFd = ::open("/dev/rfkill", O_RDONLY);
	if (nFd < 0) {
		m_sLastError = "Can't open RFKILL control device";
		return false; //--------------------------------------------------------
	}

	if (::fcntl(nFd, F_SETFL, O_NONBLOCK) < 0) {
		m_sLastError = "Can't set RFKILL control device to non-blocking";
		::close(nFd);
		return false; //--------------------------------------------------------
	}

	while (1) {
		const auto nLen = ::read(nFd, &oEvent, sizeof(oEvent));
		if (nLen < 0) {
			if (errno == EAGAIN) {
				break; // while ----
			}
			m_sLastError = "Reading of RFKILL events failed";
			::close(nFd);
			return false; //----------------------------------------------------
		}

		if (nLen != RFKILL_EVENT_SIZE_V1) {
			std::cerr << "Wrong size of RFKILL event" << '\n';
			continue; // while ----
		}

		if (oEvent.op != RFKILL_OP_ADD)
			continue; // while ----

		if (oEvent.type != RFKILL_TYPE_BLUETOOTH) {
			continue; // while ----
		}

		std::string sRfKillName;
		const bool bOk = getRfKillName(oEvent.idx, sRfKillName);
		if (!bOk) {
			continue; // while ----
		}
		if (sRfKillName == ("hci" + std::to_string(m_nHciId))) {
			m_bHardwareEnabled = (!oEvent.hard);
			m_bSoftwareEnabled = (!oEvent.soft);
			m_nRfKillIdx = oEvent.idx;
			break; // while ----
		}
	}

//std::cout << "HciAdapter::updateRfKill() m_bHardwareEnabled=" << m_bHardwareEnabled << "  m_bSoftwareEnabled=" << m_bSoftwareEnabled << '\n';
	::close(nFd);
	return true;
}
bool HciAdapter::getRfKillName(uint32_t nRfKillIdx, std::string& sRfKillName) noexcept
{
	sRfKillName = "";
	std::string sFilename = "/sys/class/rfkill/rfkill" + std::to_string(nRfKillIdx) + "/name";
	const auto nFd = ::open(sFilename.c_str(), O_RDONLY);
	if (nFd < 0) {
		m_sLastError = "Couldn't open file '" + sFilename + "': "
					+ strerror(errno) + " (" + std::to_string(errno);
		return false; //--------------------------------------------------------
	}
	char sName[128];
	memset(sName, 0, sizeof(sName));
	const auto nRead = ::read(nFd, sName, sizeof(sName) - 1);
	if (nRead < 0) {
		m_sLastError = "Couldn't read file '" + sFilename + "': "
					+ strerror(errno) + " (" + std::to_string(errno);
		::close(nFd);
		return false; //--------------------------------------------------------
	}
	::close(nFd);
	//
	sName[nRead] = '\0';
	sRfKillName = sName;
	const auto nFoundPos = sRfKillName.find('\n');
	if (nFoundPos != std::string::npos) {
		sRfKillName = sRfKillName.substr(0, nFoundPos);
	}
//std::cout << "HciAdapter::getRfKillName()  sRfKillName=" << sRfKillName << '\n';
	return true;
}
bool HciAdapter::setSoftwareEnabled(bool bEnabled) noexcept
{
	m_sLastError.clear();
	assert(m_nRfKillIdx >= 0);

	const auto nFd = ::open("/dev/rfkill", O_RDWR);
	if (nFd < 0) {
		setError("Can't open rfkill control device for");
		return false; //--------------------------------------------------------
	}

	struct rfkill_event oEvent __attribute__ ((aligned(__alignof__(rfkill_event))));
	memset(&oEvent, 0, sizeof(oEvent));
	oEvent.op = RFKILL_OP_CHANGE;
	oEvent.idx = m_nRfKillIdx;
	oEvent.soft = !bEnabled;

	const auto nLen = write(nFd, &oEvent, sizeof(oEvent));
	if (nLen < 0) {
		::close(nFd);
		setError("Failed to change rfkill state for");
		return false; //--------------------------------------------------------
	}

	::close(nFd);
	return true;
}
bool HciAdapter::setAdapterIsUp(bool bUp) noexcept
{
//std::cout << "HciAdapter::setAdapterIsUp" << '\n';
	m_sLastError.clear();
	// See cmd_up(int ctl, int hdev, char *opt) in tools/hciconfig.c (bluez package)
	// See cmd_down(int ctl, int hdev, char *opt) in tools/hciconfig.c (bluez package)
	if (bUp) {
		if (::ioctl(m_nHciSocket, HCIDEVUP, m_nHciId) < 0) {
			if (errno == EALREADY) {
				return true; //-------------------------------------------------
			}
			setError("Can't up device");
			return false; //----------------------------------------------------
		}
	} else {
		if (::ioctl(m_nHciSocket, HCIDEVDOWN, m_nHciId) < 0) {
			setError("Can't down device");
			return false; //----------------------------------------------------
		}		
	}
	return true;
}

void HciAdapter::setError(const std::string& sText) noexcept
{
	m_sLastError = sText + " hci" + std::to_string(m_nHciId) + ": "
					+ strerror(errno) + " (" + std::to_string(errno) + ")";
}


} // namespace stmi
