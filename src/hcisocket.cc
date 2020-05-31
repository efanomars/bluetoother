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
/* Parts of code modified from bluez package
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 */
/*
 * File:   hcisocket.cc
 */
#include "hcisocket.h"

#include <cassert>
//#include <iostream>
#include <cstdint>
#include <type_traits>
#include <utility>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <unistd.h>

namespace stmi
{

HciSocket::HciSocket() noexcept
: m_nHciSocket(-1)
, m_bUpdatedOnce(false)
{
}
std::string HciSocket::init() noexcept
{
	m_nHciSocket = ::socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI);
	if (m_nHciSocket < 0) {
		return "Can't open HCI socket";
	}
	return "";
}
HciSocket::~HciSocket() noexcept
{
	if (m_nHciSocket >= 0) {
		::close(m_nHciSocket);
	}
}

const std::vector<int32_t>& HciSocket::getHciIds() noexcept
{
	if (!m_bUpdatedOnce) {
		update();
	}
	return m_aHciIds;
}
const std::vector<int32_t>& HciSocket::getFaultyHciIds() noexcept
{
	if (!m_bUpdatedOnce) {
		update();
	}
	return m_aFaultyHciIds;
}
bool HciSocket::update() noexcept
{
	assert(m_nHciSocket >= 0);
	m_bUpdatedOnce = true;
	m_sLastError.clear();
	//
	for (auto& refAdapter : m_aAdapters) {
		m_aUnusedAdapters.push_back(std::move(refAdapter));
	}
	m_aAdapters.clear();
	for (auto& refAdapter : m_aFaultyAdapters) {
		m_aUnusedAdapters.push_back(std::move(refAdapter));
	}
	m_aFaultyAdapters.clear();

	m_aHciIds.clear();
	m_aFaultyHciIds.clear();

	struct hci_dev_list_req aReqs[HCI_MAX_DEV + 2];

	aReqs[0].dev_num = HCI_MAX_DEV;
	struct hci_dev_req* p0Req = aReqs[0].dev_req;

	const auto nRes = ::ioctl(m_nHciSocket, HCIGETDEVLIST, static_cast<void *>(&(aReqs[0])));
	if (nRes < 0) {
		m_sLastError = "Can't get HCI adapter list";
		return false;  //-------------------------------------------------------
	}

	const auto nTotDevices = aReqs[0].dev_num;
//std::cout << "HciSocket::update() nTotDevices=" << nTotDevices << '\n';
	for (std::remove_const<decltype(nTotDevices)>::type nCount = 0; nCount < nTotDevices; ++nCount) {
		const auto nHciId = (p0Req + nCount)->dev_id;
//std::cout << "HciSocket::update() nHciId=" << nHciId << '\n';
		m_aAdapters.push_back({});
		m_aHciIds.push_back(nHciId);
		unique_ptr<MyHciAdapter>& refAdapter = m_aAdapters.back();
		if (m_aUnusedAdapters.empty()) {
			refAdapter = std::make_unique<MyHciAdapter>();
		} else {
			refAdapter = std::move(m_aUnusedAdapters.back());
			m_aUnusedAdapters.pop_back();
		}
		//
		const bool bOk = refAdapter->reInit(m_nHciSocket, nHciId);
		if (!bOk) {
//std::cout << "HciSocket::update() nHciId=" << nHciId << " reInit failed" << '\n';
//std::cout << refAdapter->getLastError() << '\n';
			m_aFaultyAdapters.push_back(std::move(refAdapter));
			m_aFaultyHciIds.push_back(nHciId);
			m_aAdapters.pop_back();
			m_aHciIds.pop_back();
		}
	}
	return true;
}

HciAdapter* HciSocket::getAdapter(int32_t nHciId) noexcept
{
	for (auto& refAdapter : m_aAdapters) {
		if (nHciId == refAdapter->getHciId()) {
			return refAdapter.get();
		}
	}
	for (auto& refAdapter : m_aFaultyAdapters) {
		if (nHciId == refAdapter->getHciId()) {
			return refAdapter.get();
		}
	}
	return nullptr;
}

} // namespace stmi
