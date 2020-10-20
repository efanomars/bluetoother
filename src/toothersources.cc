/*
 * Copyright © 2017-2020  Stefano Marsili, <stemars@gmx.ch>
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
 * File:   toothersources.cc
 */

#include "toothersources.h"

//#include <cassert>
#include <cstring>

#include <unistd.h>

namespace stmi
{

PipeInputSource::PipeInputSource(int32_t nPipeFD) noexcept
: m_nPipeFD(nPipeFD)
{
	static_assert(sizeof(int) <= sizeof(int32_t), "");
	static_assert(false == FALSE, "");
	static_assert(true == TRUE, "");
	//
	m_oReadPollFD.set_fd(nPipeFD);
	m_oReadPollFD.set_events(Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR | Glib::IO_NVAL);
	add_poll(m_oReadPollFD);
	set_can_recurse(false);
}
PipeInputSource::~PipeInputSource() noexcept
{
//std::cout << "PipeInputSource::~PipeInputSource()" << '\n';
	if (m_nPipeFD >= 0) {
		::close(m_nPipeFD);
	}
}
sigc::connection PipeInputSource::connect(const sigc::slot<void, bool, const std::string&>& oSlot) noexcept
{
	if (m_nPipeFD < 0) {
		// Error, return an empty connection
		return sigc::connection();
	}
	return connect_generic(oSlot);
}
bool PipeInputSource::prepare(int& nTimeout) noexcept
{
	nTimeout = -1;

	return false;
}
bool PipeInputSource::check() noexcept
{
	bool bRet = false;

	if ((m_oReadPollFD.get_revents() & (Glib::IO_IN | Glib::IO_HUP | Glib::IO_ERR | Glib::IO_NVAL)) != 0) {
		bRet = true;
	}

	return bRet;
}
bool PipeInputSource::dispatch(sigc::slot_base* p0Slot) noexcept
{
	const bool bContinue = true;

	if (p0Slot == nullptr) {
		// Shouldn't happen
		return bContinue;
	}
//std::cout << "PipeInputSource::dispatch" << '\n';

	const auto nIoEvents = m_oReadPollFD.get_revents();
	if ((nIoEvents & (Glib::IO_ERR | Glib::IO_NVAL)) != 0) {
		// close pipe
		const bool bPipeError = ((nIoEvents & Glib::IO_ERR) != 0);
		(*static_cast<sigc::slot<void, bool, const std::string&>*>(p0Slot))(true, (bPipeError ? "Pipe error" : "Pipe invalid"));
		return !bContinue; // --------------------------------------------------
	}
	const bool bClientHangedUp = ((nIoEvents & Glib::IO_HUP) != 0);
	if (bClientHangedUp) {
		(*static_cast<sigc::slot<void, bool, const std::string&>*>(p0Slot))(true, "Client hanged up");
		return !bContinue; // --------------------------------------------------
	}
	if ((nIoEvents & G_IO_IN) != 0) {
		char aStreamBuf[s_nMaxJsonStringSize];
		const int32_t nBufLen = sizeof aStreamBuf;
		const int32_t nReadLen = ::read(m_oReadPollFD.get_fd(), aStreamBuf, nBufLen);
		if (nReadLen == 0) {
			// EOF: can a pipe cause it?
			return !bContinue; //-------------------------------------------
		} else if (nReadLen < 0) {
			// Since this read originated from a epoll it's not supposed to fail
			return !bContinue; //-------------------------------------------
		}
		// rest
		char* p0CurPos = aStreamBuf;
		int32_t nRestLen = nReadLen;
		do {
			auto p0Zero = static_cast<char*>(std::memchr(p0CurPos, 0, nRestLen));
			if (p0Zero == nullptr) {
				m_sReceiveBuffer += std::string(p0CurPos, nRestLen);
				break; // do ----
			}
			//           111111111122222
			// 0123456789012345678901234
			// xxxxxxxxxxxxxxxx0yyyyyyyy
			// nRestLen = 25, nAttach = 16 => nRestLen = 8
			const int32_t nAttach = p0Zero - p0CurPos;
			nRestLen = nRestLen - 1 - nAttach;
			m_sReceiveBuffer += std::string(p0CurPos, nAttach);
			(*static_cast<sigc::slot<void, bool, const std::string&>*>(p0Slot))(false, m_sReceiveBuffer);
			m_sReceiveBuffer.clear();
			p0CurPos = p0Zero + 1;
		} while (nRestLen > 0);
	}
	return bContinue;
}

} // namespace stmi
