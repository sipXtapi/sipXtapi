This directory contains scripts and some of the resources needed to build an 
installer using InstallShield Express X.

External Dependencies
---------------------
To build the installer, you must have Microsoft Visual Studio v6.0, Glib, 
Sun's JRE 1.4.2_06, and InstallShield Express X installed.

To specify the locations of each of these, please set the following 
environment variables.  Default locations are assumed.

  SIPXPHONE_JRE_BASE         
     C:\Program Files\Java\j2re1.4.2_06

  SIPXPHONE_INSTALLSHIELD_BASE    
     C:\Program Files\InstallShield X Express\System

  SIPXPHONE_MSVC_BASE
     C:\Program Files\Microsoft Visual Studio\VC98

Build Installer
---------------
Run the following commands in order:

  build_code.bat
  stage_inst.bat
  build_inst.bat

The results are placed in:
  .\sipXphone\Express\SingleImage\DiskImages\DISK1
