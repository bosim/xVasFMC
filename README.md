xVasFMC
================

This project is a fork from the well-known FMC for MSFS/X-Plane vasfmc. The objectives
of this project are:

* A destilled version of VasFMC, with only the goal of working in X-Plane. 
* Assisting more than overtaking control.
* Only be a CDU and nothing else.

Compiling
=================
* Tested with Microsoft Visual Studio C++ 2010 Express and Qt SDK 1.2.1. Can be done
by simply open the project files in Qt Creator and building, first vaslib, then vasfmc
and then xpfmcconn21.

License
=================
vasfmc is distributed under GNU GPL version 2. See LICENSE.txt for details. Few files are
not GPL, however redistribution has been granted. Please take notice of that when/if editing. 

Known bugs / TODO
=================
* LNAV can cause flight reset (i.e. the plane is back at the runway). This will be fixed very soon. 
* It is not possible to turn heading steering of, but will be fixed really soon.
* VNAV handling is not yet implemented.
* Fuel calculation is not present.
* VASFMC console is still there, we will get rid of it.
* Get rid of debug information.

Author(s)
=================

Past developers of VasFMC:

* Alex Wemmer,
* Philipp Münzel for the X-Plane plugin, 
* the VAS project team.

Current maintainer of the Code:

* Bo Simonsen <bo@geekworld.dk>

This is a spare-time project for me, which means that i am only dedicating a few hours per
week for this project, so don't expect miracles.
