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
 * File:   btservice.h
 */

#ifndef STMI_BT_SERVICE_H
#define STMI_BT_SERVICE_H


//#include "hciadapter.h"

#include <string>
#include <memory>

namespace stmi
{

using std::unique_ptr;

class BtService
{
public:
	BtService() noexcept;

	// returns false if errors, use getLastError()
	bool update() noexcept;

	bool isServiceRunning() noexcept;
	bool isServiceEnabled() noexcept;
	bool startService() noexcept;
	bool stopService() noexcept;
	/** Whether the service should be started at system boot.
	 * @param bEnabled Whether enabled.
	 * @return False is some error.
	 */
	bool enableService(bool bEnabled) noexcept;
	/** The last socket error.
	 * @return The last error or empty.
	 */
	const std::string& getLastError() const noexcept { return m_sLastError; }
private:
	bool execCmd(const char* sCmd, std::string& sResult) noexcept;
private:
	bool m_bUpdatedOnce;
	bool m_bServiceRunning;
	bool m_bServiceEnabled;
	//
	std::string m_sLastError;
private:
	BtService(const BtService& oSource) = delete;
	BtService& operator=(const BtService& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_BT_SERVICE_H */

