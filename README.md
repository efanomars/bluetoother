bluetoother
===========

bluetoother is a GUI tool to configure bluetooth adapter connectivity
for systemd and bluez (libbluetooth).

It needs to be run with root privileges:

- from a terminal:          sudo bluetoother

- from a desktop launcher:  gksu bluetoother

Functionality:

- start/stop the bluetooth adapter

- start/stop systemd bluetooth.service

- set local name

- set detectability and connectability

It provides some of the functionality of the command line tools
rfkill, hciconfig and sytemctl.

Read the INSTALL file for installation instructions.
