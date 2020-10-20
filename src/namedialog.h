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
 * File:   namedialog.h
 */

#ifndef STMI_NAME_DIALOG_H
#define STMI_NAME_DIALOG_H

#include <gtkmm.h>

#include <string>

#include <stdint.h>

namespace stmi
{

class NameDialog : public Gtk::Dialog
{
public:
	NameDialog(Gtk::Window& oParent, int32_t nMaxLength) noexcept;

	// Call this instead of run()
	int run(const std::string& sName, int32_t nSelectedHciId) noexcept;

	// Call this after run(...) has returned s_nRetOk
	// The returned name is always valid
	const std::string& getName() const noexcept;

	static constexpr const int s_nRetOk = 1;
	static constexpr const int s_nRetCancel = 0;
private:
	void onEntryNameChanged() noexcept;
	void onErrorOneSecond() noexcept;
private:
	//Gtk::Box* m_p0VBoxName;
		Gtk::Label* m_p0LabelNameDesc;
		Gtk::Entry* m_p0EntryName;

	Glib::RefPtr<Gtk::EntryBuffer> m_refEntryBuffer;
	////////////
	std::string m_sOldName;
	std::string m_sName;

	std::string m_sLabelStringNormal;
	std::string m_sLabelStringError;

	static constexpr const int32_t s_nInitialWindowSizeW = 200;
	static constexpr const int32_t s_nInitialWindowSizeH = 100;
};

} // namespace stmi

#endif /* STMI_NAME_DIALOG_H */

