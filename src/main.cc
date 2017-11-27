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
 * File:   main.cc
 */

#include "hcisocket.h"
#include "btservice.h"

#include "config.h"
#include "tootherwindow.h"

#include <gtkmm.h>

#include <cassert>
#include <iostream>

#include <unistd.h>

namespace stmi
{

void printVersion()
{
	std::cout << Config::getVersionString() << '\n';
}
void printUsage()
{
	std::cout << "Usage: bluetoother" << '\n';
	std::cout << "Setup bluetooth adpter connectivity." << '\n';
	std::cout << "Option:" << '\n';
	std::cout << "  -h --help              Prints this message." << '\n';
	std::cout << "  -v --version           Prints version." << '\n';
}

int bluetootherMain(int argc, char** argv)
{
	char* argvZeroSave = ((argc >= 1) ? argv[0] : nullptr);
	while (argc >= 2) {
		auto nOldArgC = argc;
		if ((strcmp("--version", argv[1]) == 0) || (strcmp("-v", argv[1]) == 0)) {
			printVersion();
			return EXIT_SUCCESS; //---------------------------------------------
		}
		if ((strcmp("--help", argv[1]) == 0) || (strcmp("-h", argv[1]) == 0)) {
			printUsage();
			return EXIT_SUCCESS; //---------------------------------------------
		}
		//bool bOk = evalArg(argc, argv, "--interval", "-i", nInterval);
		//if (!bOk) {
		//	return EXIT_FAILURE; //---------------------------------------------
		//}
		if (nOldArgC == argc) {
			std::cout << "Unknown argument: " << std::string(argv[1]) << '\n';
			return EXIT_FAILURE; //---------------------------------------------
		}
		argv[0] = argvZeroSave;
	}

	const Glib::ustring sAppName = "com.github.efanomars.bluetoother";
	const Glib::ustring sWindoTitle = "bluetoother " + Config::getVersionString();

	Glib::RefPtr<Gtk::Application> refApp;
	try {
		//
		refApp = Gtk::Application::create(argc, argv, sAppName);
	} catch (const std::runtime_error& oErr) {
		std::cout << "Error: " << oErr.what() << '\n';
		return EXIT_FAILURE; //-------------------------------------------------
	}
	if (getuid() != 0) {
		// not root
		
		// Gtk::MessageDialog oDlg("Starting \"bluetoother\" without administrative privileges.\n"
		// 						"Some of the functions might not work correctly.\n"
		// 						"It's recommended to use sudo or gksu to start this program.", false
		// 						, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, false);
 		Gtk::MessageDialog oDlg("This program needs administrative privileges.\n"
								"Please start it with either\n"
								"'sudo bluetoother' or 'gksu bluetoother'.", false
 								, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, false);
		oDlg.run();
 		return EXIT_FAILURE; //-------------------------------------------------
	}
	// service
	BtService oService{};
	// hci model
	HciSocket oSocket{};

	TootherWindow oWindow(sWindoTitle, oService, oSocket);
	const auto nRet = refApp->run(oWindow);
	return nRet;
}

} // namespace stmi

int main(int argc, char** argv)
{
	return stmi::bluetootherMain(argc, argv);
}

