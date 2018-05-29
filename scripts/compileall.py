#!/usr/bin/env python3

#  Copyright © 2017-2018  Stefano Marsili, <stemars@gmx.ch>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, see <http://www.gnu.org/licenses/>

# File:   compileall.py

# - compiles and installs the project contained in this source package
#   for both Debug and Release configurations
# - makes sure no line in the code starts with std::cout
# - optionally calls clang-tidy

import sys
import os
import subprocess


def testAll(sBuildType, sDestDir, sSudo):

	subprocess.check_call("./scripts/uninstall_bluetoother.py -r -y --destdir {}  {}".format(sDestDir, sSudo).split())

	subprocess.check_call("./scripts/install_bluetoother.py -b {}  --destdir {}  {}"\
			.format(sBuildType, sDestDir, sSudo).split())



def checkTidy():
	#
	subprocess.check_call("./scripts/checktidy.py".split())
	

def main():

	import argparse
	oParser = argparse.ArgumentParser(description="Uninstall, compile, reinstall the bluetoother project")
	oParser.add_argument("-y", "--no-prompt", help="no prompt comfirmation", action="store_true"\
						, default=False, dest="bNoPrompt")
	oParser.add_argument("--destdir", help="install dir (default=/usr/local)", metavar='DESTDIR'\
						, default="/usr/local", dest="sDestDir")
	oParser.add_argument("--no-sudo", help="don't use sudo to (un)install", action="store_true"\
						, default=False, dest="bDontSudo")
	oParser.add_argument("-b", "--buildtype", help="build type (default=Both)", choices=['Debug', 'Release', 'Both']\
						, default="Both", dest="sBuildType")
	oParser.add_argument("--tidy", help="apply clang-tidy to source code", action="store_true"\
						, default=False, dest="bTidy")
	oArgs = oParser.parse_args()

	while not oArgs.bNoPrompt:
		sAnswer = input("Are you sure? (yes/no) >").strip()
		if sAnswer.casefold() == "yes":
			break
		elif sAnswer.casefold() == "no":
			return #-----------------------------------------------------------

	sDestDir = os.path.abspath(oArgs.sDestDir)

	sScriptDir = os.path.dirname(os.path.abspath(__file__))
	os.chdir(sScriptDir)
	os.chdir("..")

	subprocess.check_call("./scripts/checkcode.py".split())

	if oArgs.bDontSudo:
		sSudo = "--no-sudo"
	else:
		sSudo = ""

	if oArgs.sBuildType == "Both":
		testAll("Debug", sDestDir, sSudo)
		if oArgs.bTidy:
			checkTidy()
		#
		testAll("Release", sDestDir, sSudo)
		if oArgs.bTidy:
			checkTidy()
	else:
		testAll(oArgs.sBuildType, sDestDir, sSudo)
		if oArgs.bTidy:
			checkTidy()

	print("------------------------------------")
	print("compileall.py finished successfully!")
	print("------------------------------------")


if __name__ == "__main__":
	main()

