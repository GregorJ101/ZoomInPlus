# ZoomIn++
ZoomIn++ is based on the ZoomIn app published by Brian Friesen, which is based on the ZoomIn app provided by Microsoft included with Visual Studio.

This solution is based the ZoomIn application found online.  It has been extended in the following ways:
- Added code to save viewed area size and position on desktop to registry
- Added code to create named settings in registry using the command-line argument string
- Added theme name to caption bar after the app title "ZoomIn++"
- Added code to not save window size & position if minimized
- Added code to start app minimized
- Updated the About box with build timestamp
- Fixed issue with selection rectangle starting at top left instead of actual window position
- Added code to protect against emtpy caption strings
- Changed default start position to top left, and show-grid option to false
- Replaced code for drawing selection rectangles with circle-and-crosshairs cursor
- Added code to enable mouse scroll-wheel for adjusting magnification
- Added menu option for using selection rectangle instead of cross-hair cursor, default to false
- Added code to persist selection rectangle setting in registry
