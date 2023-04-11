The SpectrometersLib collects the availabe spectrometer drivers into one single location
This makes it possible to build MobileDoas with support for different spectrometer manufacturers

The different manufacturers are built conditionally (based on a definition-flag) meaning that it is 
    possible to build this library also on a computer where not all drivers are installed.
