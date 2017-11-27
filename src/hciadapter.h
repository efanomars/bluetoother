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
 * File:   hciadapter.h
 */

#ifndef STMI_HCI_ADAPTER_H
#define STMI_HCI_ADAPTER_H


#include <vector>
#include <string>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

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
	HciAdapter() = default;

	int32_t getHciId() const { return m_nHciId; }
	const std::string& getAdapterName() const { return m_sAdapterName; }
	const std::string& getAddress() const { return m_sAddress; }
	bool isDown() const { return m_bIsDown; }
	bool isConnectable() const { return m_bConnectable && !m_bIsDown; }
	bool isDetectable() const { return m_bDetectable && !m_bIsDown; }
	const std::string& getLocalName() const { return m_sLocalName; }
	bool isHardwareEnabled() const { return m_bHardwareEnabled; }
	bool isSoftwareEnabled() const { return m_bSoftwareEnabled; }

	// Returns false if error
	bool setConnectable(bool bConnectable);
	// Returns false if error
	bool setDetectable(bool bDetectable);
	// Returns false if error
	bool setLocalName(const std::string& sName);

	// Returns false if error
	bool setSoftwareEnabled(bool bEnabled);
	// Returns false if error
	bool setAdapterIsUp(bool bUp);

	const std::string& getLastError() const { return m_sLastError; }
protected:
	// nHciSocket The hci raw socket.
	// nHciId The id of the device.
	// oDevInfo device info from ioctl
	bool reInit(int32_t nHciSocket, int32_t nHciId);
private:
	// oDevInfo device info from ioctl
	bool update(const struct hci_dev_info& oDevInfo);
	bool updateLocalName();
	bool updateRfKill();
	bool setScan();
	bool getRfKillName(uint32_t nRfKillIdx, std::string& sRfKillName);
	void setError(const std::string& sText);
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

