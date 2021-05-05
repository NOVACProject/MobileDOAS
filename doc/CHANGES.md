MobileDOAS Release Notes
-----------------------------------------------------
Version 6.3.1 (May 2021)
* Revisions to new attributes added to STD files in 6.3 (#154)

Version 6.3 (April 2021)

New features
* Option to read and analyze spectra files from a directory (#137)
* Add GPS status, course, and speed to STD files (#151)

Bug fixes
* Fix traverse display in postflux dialog when passing midnight UTC (#148)

Other
* Improved method to determine whether spectrum is dark (#146)


MobileDOAS Release Notes
-----------------------------------------------------
Version 6.2 (August 2019)

New features
* Warn user when trying to view spectrometer output from slave channel if second channel is not enabled in configuration (#126)
* Option to skip dark measurement (#128)
* Increase max traverse from 4096 to 16384 (#132)

Bug fixes
* Fix reading of GPS altitude

-----------------------------------------------------
Version 6.1 (April 2019)

Critical changes
* Enable configuration and evaluation of second channel for spectrometers that support it (#8)
* Removed support for text format configuration file.

New features
* Start GPS - for reading GPS data; in debug mode only
* Multiple reference files can be addeded or removed simultaneously through evaluation configuration dialogs
* Legend for real-time route dialog (#67)
* Remove extended format attributes in STD that were previously coded with 0. (#71)
* Update Plume Height and Wind Measurement dialogs to read two evaluation logs (#73)
* Warn user (audio and visual) when GPS connection is lost (#78)
* Reduce number of spectra collected for characterization of the offset in adaptive mode (#87)
* Option to disable audio in spectrometer configuration (#88)
* Alter pitch based on column density, instead of volume, if supported by audio device (#88)
* Use same tone for negative column values (#88)
* Limit maximum exposure time in 'adaptive' mode to time-resolution of measurement (#100)
* Support for QE65000 (#101)
* Support user configurable range for saturation ratio in adaptive mode (#109)

Bug fixes
* Ensure evaluation configuration dialog updates properly when adding/removing a new fit window
* Attempt to reconnect when GPS connection is lost (#77)
* Added GPS lat/lon validation (#81)
* Fix for bad GPS date in STD files (#86)
* Fix date in STD file from View Spectrometer Output (#104)
* Restore 'Dark' tab in ReEvaluate dialog (#116)

-----------------------------------------------------
Version 6.0 (April 2018)

Critical changes
* Support for 64-bit OS

New features
* Add option to remove points with intensity higher than a certain threshold (#17)
* Use another color (cyan) for fit resolve in Fit Window (#19)
* Auto-scale real-time route (#20)
* Automatically change flux calculations when unit is changed (#23)
* Display absolute values for final plume width and flux calculations (#24, #25)
* Add support for re-evaluation of measurements taken in adaptive mode (#26)
* Add Altitude to .STD files (#32)

Bug fixes
* Fix issue with list of references not updating in Configuration Dialog->Evalute (#11)
* Validate lat/lon values after focus is removed from field in Post Flux Calculation dialog (#14)
* Fix issue with intensity slide bar on main page not always showing up (#21)
* Fix date problem with measurements spanning midnight UTC (#27)
* Fix Altitude showing up as 0.0 in evaluation logs (#29)
* Fix "Could not communicate with the GPS" error popping up behind main UI (#31)
* Fix multiples of same tab being generated in Post Flux Calculation dialog when 'File->ReEvaluate This Log File' is selected (#34)
* Add message for user when Post-Wind calculations are not supported (#39)
* Fix compatibility with high DPI display (#47)
* Fix issue with reading GPS data off of COM ports above 9 (#53)

Removal of unused features
* Remove option 3 from 'Sky' tab in ReEvaluation dialog (#9)
* Remove 'Dark' tab from ReEvaluation dialog (#10)
* Disable WindSpeed Measurement and PlumeHeight Measurement menu items (until dual-beam support can be implemented)
* Remove Change Exposure Time menu option (#43)
