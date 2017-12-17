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
 * File:   hcisocket.h
 */

#ifndef STMI_HCI_SOCKET_H
#define STMI_HCI_SOCKET_H


#include "hciadapter.h"

#include <memory>
#include <vector>

namespace stmi
{

using std::unique_ptr;

class HciSocket
{
public:
	HciSocket();
	~HciSocket();

	/** Update.
	 * If errors occur sets last error.
	 * @return False if some error.
	 */
	bool update();
	
	/** The healthy hci ids.
	 * ex. the 0 in "hci0:".
	 * Calls update() if not called yet.
	 * @return The set.
	 */
	const std::vector<int32_t>& getHciIds();
	/** The faulty hci ids.
	 * ex. the 0 in "hci0:".
	 * Calls update() if not called yet.
	 * @return The set.
	 */
	const std::vector<int32_t>& getFaultyHciIds();

	/** Return an adapter by id.
	 * For both healthy and faulty hcis.
	 * @param nHciId The id.
	 * @return null if hci not found.
	 */
	HciAdapter* getAdapter(int32_t nHciId);

	/** The last socket error.
	 * Faulty adapters don't cause this error to be set.
	 * @return The last error or empty.
	 */
	const std::string& getLastError() const { return m_sLastError; }
private:
	class MyHciAdapter : public HciAdapter
	{
	public:
		MyHciAdapter() = default;
		bool reInit(int32_t nHciSocket, int32_t nHciId)
		{
			return HciAdapter::reInit(nHciSocket, nHciId);
		}
	};
private:
	int32_t m_nHciSocket;
	bool m_bUpdatedOnce;
	std::vector<unique_ptr<MyHciAdapter>> m_aAdapters; // all healthy
	std::vector<unique_ptr<MyHciAdapter>> m_aFaultyAdapters; // all faulty
	std::vector<unique_ptr<MyHciAdapter>> m_aUnusedAdapters; // recyclable
	std::vector<int32_t> m_aHciIds; // all healthy, Size: m_aAdapters.size()
	std::vector<int32_t> m_aFaultyHciIds; // all faulty, Size: m_aFaultyAdapters.size()
	//
	std::string m_sLastError;
private:
	HciSocket(const HciSocket& oSource) = delete;
	HciSocket& operator=(const HciSocket& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_HCI_SOCKET_H */

