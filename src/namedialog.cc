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
 * File:   namedialog.cc
 */

#include "namedialog.h"

#include <cassert>
#include <iostream>


namespace stmi
{

NameDialog::NameDialog(Gtk::Window& oParent, int32_t nMaxLength)
: Gtk::Dialog()
, m_p0LabelNameDesc(nullptr)
, m_p0EntryName(nullptr)
{
	set_transient_for(oParent);
	//
	set_title("Adapter name");
	set_default_size(s_nInitialWindowSizeW, s_nInitialWindowSizeH);
	set_resizable(true);

	add_button("Cancel", s_nRetCancel);
	add_button("Ok", s_nRetOk);
	set_default_response(s_nRetOk);

	m_refEntryBuffer = Gtk::EntryBuffer::create();
		m_refEntryBuffer->set_max_length(nMaxLength);

	Gtk::Box* p0ContentArea = get_content_area();
	assert(p0ContentArea != nullptr);
//std::cout << "NameDialog::NameDialog() content area children: " << p0ContentArea->get_children().size() << '\n';
	Gtk::Box* m_p0VBoxName = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL));
	p0ContentArea->pack_start(*m_p0VBoxName, false, false);
		m_p0VBoxName->set_spacing(5);
		m_p0LabelNameDesc = Gtk::manage(new Gtk::Label(""));
		m_p0VBoxName->pack_start(*m_p0LabelNameDesc, false, false);
			m_p0LabelNameDesc->set_alignment(Gtk::Align::ALIGN_START, m_p0LabelNameDesc->property_valign());
		m_p0EntryName = Gtk::manage(new Gtk::Entry(m_refEntryBuffer));
		m_p0VBoxName->pack_start(*m_p0EntryName, false, false);
			m_p0EntryName->set_activates_default(true);
			m_p0EntryName->signal_changed().connect(
							sigc::mem_fun(*this, &NameDialog::onEntryNameChanged) );

	this->show_all();
}
NameDialog::~NameDialog()
{
}
int NameDialog::run(const std::string& sName, int32_t nSelectedHciId)
{
	assert(!sName.empty());
	m_sOldName = sName;
	m_sName = sName;
	m_p0EntryName->set_text(sName);
	m_sLabelStringNormal = "Bluetooth adapter name (hci" + std::to_string(nSelectedHciId) + ")";
	m_sLabelStringError = m_sLabelStringNormal + "\nCannot be empty!";
	m_p0LabelNameDesc->set_text(m_sLabelStringNormal);
	int nRet;
	do {
		nRet = Gtk::Dialog::run();
		if (nRet != s_nRetOk) {
			break; //--------
		}
		std::string sName = m_p0EntryName->get_text();
		if (!sName.empty()) {
			break; //--------
		}
		// error
		m_p0LabelNameDesc->set_text(m_sLabelStringError);
		Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &NameDialog::onErrorOneSecond), 1000);
	} while (true);
	return nRet;
}
void NameDialog::onErrorOneSecond()
{
	m_p0LabelNameDesc->set_text(m_sLabelStringNormal);
}
const std::string& NameDialog::getName() const
{
	return m_sName;
}
void NameDialog::onEntryNameChanged()
{
//std::cout << "NameDialog::onSpinNameChanged()" << '\n';
	std::string sName = m_p0EntryName->get_text();
	if (!sName.empty()) {
//std::cout << "NameDialog::onEntryNameChanged setting sName=" << sName << '\n';
		m_sName = sName;
	}
}

} // namespace stmi

