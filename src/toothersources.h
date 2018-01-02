/*
 * Copyright © 2017  Stefano Marsili, <stemars@gmx.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>
 */
/*
 * File:   toothersources.h
 */

#ifndef STMI_TOOTHER_SOURCES_H
#define STMI_TOOTHER_SOURCES_H

#include <glibmm.h>

#include <cassert>
#include <vector>
#include <string>


namespace stmi
{

/* For polling pipe read events */
class PipeInputSource : public Glib::Source
{
public:
	PipeInputSource(int32_t nPipeFD);
	// Closes file descriptor on destruction
	virtual ~PipeInputSource();

	// A source can have only one callback type, that is the slot given as parameter
	sigc::connection connect(const sigc::slot<void, bool, const std::string&>& oSlot);

	inline int32_t getPipeFD() const { return m_nPipeFD; }
protected:
	bool prepare(int& nTimeout) override;
	bool check() override;
	bool dispatch(sigc::slot_base* oSlot) override;

private:
	int32_t m_nPipeFD;
	std::string m_sErrorStr;
	//
	Glib::PollFD m_oReadPollFD; // The file descriptor is open until destructor is called
	//
	std::string m_sReceiveBuffer;

	static constexpr int32_t s_nMaxJsonStringSize = 64000;
private:
	PipeInputSource() = delete;
	PipeInputSource(const PipeInputSource& oSource) = delete;
	PipeInputSource& operator=(const PipeInputSource& oSource) = delete;
};

} // namespace stmi

#endif /* STMI_TOOTHER_SOURCES_H */
