MobileDOAS is a software package developed by Chalmers University of Technology in Sweden as an easy to use software package 
for performing gas flux measurements using the mobile DOAS technique. 

Main solution lies in DMSpec.sln

Supported Spectrometer Manufacturers

* OceanOptics spectrometers are supported in all versions.

* Avantes spectrometers were added in version 6.5.0

-- Notice on supported spectrometer manufacturers --
As the software scans for all spectrometers connected through USB at start, 
the device drivers for the supported spectrometer manufacturers needs to be installed
on the computer before the measurement is started. In order to not have to install 
drivers for all supported manufacturers on all computers, special builds of the software 
are created where only one manufacturer is supported. These builds are created from the same
software, but with different definitions.

Conditional support for spectrometers is added in the file:
./SpectrometersLib/pch.h

Support for Avantes spectrometers is added by defining the flag:
MANUFACTURER_SUPPORT_AVANTES

Support for OceanOptics spectrometers is added by defining the flag:
MANUFACTURER_SUPPORT_OCEANOPTICS

