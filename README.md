bluetoother
===========

bluetoother is a GUI tool to configure bluetooth adapter connectivity
for systemd and bluez (libbluetooth).

It needs to be run with root privileges and uses PolKit for authentication.
The child GUI process is run with user privileges to make it compatible with
Wayland's strict security requirements.

Execute it with command 'pkexec bluetoother'.

Functionality:

- start/stop the bluetooth adapter

- start/stop systemd bluetooth.service

- set local name

- set detectability and connectability

It provides some of the functionality of the command line tools
rfkill, hciconfig and sytemctl.

Read the INSTALL file for installation instructions.
