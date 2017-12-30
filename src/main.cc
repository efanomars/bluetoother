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

#include "config.h"
#include "tootherwindow.h"
#include "tootherserver.h"

#include <gtkmm.h>

#include <cassert>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

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

int startGUIClient(bool bReadOnly, int nCmdPipe, int nReturnPipe, int nArgC, char** aArgV)
{
	const Glib::ustring sAppName = "com.github.efanomars.bluetoother";
	const Glib::ustring sWindoTitle = "bluetoother " + Config::getVersionString();

	Glib::RefPtr<Gtk::Application> refApp;
	try {
		//
		refApp = Gtk::Application::create(nArgC, aArgV, sAppName);
	} catch (const std::runtime_error& oErr) {
		std::cout << "Error: " << oErr.what() << '\n';
		return EXIT_FAILURE; //-------------------------------------------------
	}
	TootherWindow oWindow(sWindoTitle, nCmdPipe, nReturnPipe, bReadOnly);
	const auto nRet = refApp->run(oWindow);
	return nRet;
}
int startRootServer(int nCmdPipe, int nReturnPipe, bool bPrintOutServerErrors)
{
	TootherServer oServer(nCmdPipe, nReturnPipe, bPrintOutServerErrors);
	return oServer.run();
}

int bluetootherMain(int nArgC, char** aArgV)
{
	bool bPrintOutServerErrors = false;
	char* p0ArgVZeroSave = ((nArgC >= 1) ? aArgV[0] : nullptr);
	while (nArgC >= 2) {
		auto nOldArgC = nArgC;
		if (aArgV[1] != nullptr) {
			if ((strcmp("--version", aArgV[1]) == 0) || (strcmp("-v", aArgV[1]) == 0)) {
				printVersion();
				return EXIT_SUCCESS; //---------------------------------------------
			}
			if ((strcmp("--help", aArgV[1]) == 0) || (strcmp("-h", aArgV[1]) == 0)) {
				printUsage();
				return EXIT_SUCCESS; //---------------------------------------------
			}
			if (strcmp("--print-server-error", aArgV[1]) == 0) {
				--nArgC;
				++aArgV;
				bPrintOutServerErrors = true;
			}
			if (nOldArgC == nArgC) {
				std::cerr << "Unknown argument: " << std::string(aArgV[1]) << '\n';
				return EXIT_FAILURE; //---------------------------------------------
			}
		} else {
			--nArgC;
			++aArgV;
		}
		aArgV[0] = p0ArgVZeroSave;
	}

	const auto nEffectiveUID = ::geteuid();

	bool bReadOnly = false;
	int32_t nUID = -1;
	int32_t nGID = -1;
	if (nEffectiveUID == 0) {
		// run as root
		// look if pkexec was used
		char* p0UserId = getenv("PKEXEC_UID");
		if (p0UserId != nullptr) {
			try {
				nUID = std::stoi(p0UserId);
			} catch (std::invalid_argument ) {
				std::cerr << "Error: PKEXEC_UID contains invalid uid." << '\n';
			} catch (std::out_of_range ) {
				std::cerr << "Error: PKEXEC_UID contains out of range integer (invalid uid)." << '\n';
			}
		} else {
			std::cerr << "Error: PKEXEC_UID not defined." << '\n';
		}
		if (nUID < 0) {
			std::cerr << "Error: PKEXEC_UID invalid value. You should start this program with pkexec." << '\n';
			nUID = 0;
			//return EXIT_FAILURE; //-------------------------------------------------
		} else if (nUID == 0) {
			std::cout << "Warning: pkexec was called by root user. Will fail on Wayland." << '\n';
		}
		const auto p0Passw = getpwuid(nUID);
		if (p0Passw == nullptr) {
			std::cerr << "Error: getpwuid couldn't find user '" << nUID << "'" << '\n';
			return EXIT_FAILURE; //-------------------------------------------------
		}
		nGID = p0Passw->pw_gid;
	} else {
		// unprivileged user: read-only mode
		nUID = nEffectiveUID;
		nGID = ::getegid();
		bReadOnly = true;
	}
	pid_t oPid;
	int aCmdPipe[2];    // 0: read (Server)  1: write (GUI)
	int aReturnPipe[2]; // 0: read (GUI)     1: write (Server)

	// Create the pipes.
	if (::pipe(aCmdPipe)) {
		std::cerr << "Error: 'Cmd pipe' failed." << '\n';
		return EXIT_FAILURE; //-------------------------------------------------
	}
	if (::pipe(aReturnPipe)) {
		std::cerr << "Error: 'Return pipe' failed." << '\n';
		return EXIT_FAILURE; //-------------------------------------------------
	}

	// Create the child process.
	oPid = ::fork();
	if (oPid == static_cast<pid_t>(0)) {
		// This is the child process.
		// Close other ends first.
		::close(aCmdPipe[1]);
		::close(aReturnPipe[0]);
		//::setuid(nEffectiveUID); // not really necessary?
		auto nRet = startRootServer(aCmdPipe[0], aReturnPipe[1], bPrintOutServerErrors);
		return nRet; //---------------------------------------------------------
	} else if (oPid < static_cast<pid_t>(0)) {
		// The fork failed.
		std::cerr << "Error: fork failed." << '\n';
		return EXIT_FAILURE; //-------------------------------------------------
	} else {
		// This is the parent process.
		// Close other ends first.
		::close(aCmdPipe[0]);
		::close(aReturnPipe[1]);
		//
		auto nRet = ::setgid(nGID);
		if (nRet == -1) {
			std::cerr << "Error: " << errno << ". Couldn't set group id to " << nGID << "." << '\n';
			return EXIT_FAILURE; //---------------------------------------------
		}
		nRet = ::setuid(nUID);
		if (nRet == -1) {
			std::cerr << "Error: " << errno << ". Couldn't set user id to " << nUID << "." << '\n';
			return EXIT_FAILURE; //---------------------------------------------
		}
		//
		return startGUIClient(bReadOnly, aCmdPipe[1], aReturnPipe[0], nArgC, aArgV); //----
	}
}

} // namespace stmi

int main(int nArgC, char** aArgV)
{
	return stmi::bluetootherMain(nArgC, aArgV);
}

