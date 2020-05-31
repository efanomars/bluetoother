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
 * File:   hciadapter.h
 */

#ifndef STMI_HCI_ADAPTER_H
#define STMI_HCI_ADAPTER_H


#include <string>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <stdint.h>

namespace stmi
{

//
// The bluetooth.service when started ups the adapter
// The bluetooth.service when stopped downs the adapter
// But whether the bluetooth.service is running or not
// the adapter can be upped and downed with hciconfig
// Only rfkill block can disable the adapter

class HciAdapter
{
public:
	HciAdapter() noexcept = default;

	int32_t getHciId() const noexcept { return m_nHciId; }
	const std::string& getAdapterName() const noexcept { return m_sAdapterName; }
	const std::string& getAddress() const noexcept { return m_sAddress; }
	bool isDown() const noexcept { return m_bIsDown; }
	bool isConnectable() const noexcept { return m_bConnectable && !m_bIsDown; }
	bool isDetectable() const noexcept { return m_bDetectable && !m_bIsDown; }
	const std::string& getLocalName() const noexcept { return m_sLocalName; }
	bool isHardwareEnabled() const noexcept { return m_bHardwareEnabled; }
	bool isSoftwareEnabled() const noexcept { return m_bSoftwareEnabled; }

	// Returns false if error
	bool setConnectable(bool bConnectable) noexcept;
	// Returns false if error
	bool setDetectable(bool bDetectable) noexcept;
	// Returns false if error
	bool setLocalName(const std::string& sName) noexcept;

	// Returns false if error
	bool setSoftwareEnabled(bool bEnabled) noexcept;
	// Returns false if error
	bool setAdapterIsUp(bool bUp) noexcept;

	const std::string& getLastError() const noexcept { return m_sLastError; }
protected:
	// nHciSocket The hci raw socket.
	// nHciId The id of the device.
	// oDevInfo device info from ioctl
	bool reInit(int32_t nHciSocket, int32_t nHciId) noexcept;
private:
	// oDevInfo device info from ioctl
	bool update(const struct ::hci_dev_info& oDevInfo) noexcept;
	bool updateLocalName() noexcept;
	bool updateRfKill() noexcept;
	bool setScan() noexcept;
	bool getRfKillName(uint32_t nRfKillIdx, std::string& sRfKillName) noexcept;
	void setError(const std::string& sText) noexcept;
private:
	int32_t m_nHciSocket = -1;
	int32_t m_nHciId = -1;
	std::string m_sAdapterName; // Ex. hci0
	std::string m_sAddress; // Available even if down(?). Ex. AB:CD:EF:98:76:54
	bool m_bIsDown = true; // The following values only valid if not down
	bool m_bConnectable = false;
	bool m_bDetectable = false;
	std::string m_sLocalName;
	// if rfkill info for the adapter not in the file system
	// assume software and hardware enabled
	bool m_bHardwareEnabled = true;
	bool m_bSoftwareEnabled = true;
	int32_t m_nRfKillIdx = -1;
	//
	std::string m_sLastError;
private:
	HciAdapter(const HciAdapter& oSource) = delete;
	HciAdapter& operator=(const HciAdapter& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_HCI_ADAPTER_H */

