
              cards - Package Management Utilities for NuTyX
              a fork of the pkgutils and prt-get tools



Description
-----------
cards is a set of utilities (pkgadd, pkgrm, pkginfo, pkgmk and cards),
which are used for managing software packages in Linux. It was originally developed for
and used by the CRUX distribution (http://crux.nu). This is a fork of this work for the 
NuTyX distribution including the handling of the binaries


Building and installing
-----------------------
 1. To build the lib

$ make libs

 2. To install the lib

$ make install-libs

 3. To build all

$ make all

 4. To build the pkgadd binary

$ make pkgadd

 5. To install the binaries

$ make install
or
$ make DESTDIR=/some/other/path install


 6. To get a quick overview

$ cards help

Copyright
---------

**cards** Copyright (c) 2013-2016 NuTyX team (http://nutyx.org).

Some code based on:

* `pkgutils` Copyright (c) 2000-2005 Per Liden and Copyright (c) 2006-2013 CRUX team (http://crux.nu)
* `pacman` (https://projects.archlinux.org/pacman.git/) Copyright (c) 2015-2016 Pacman Development Team

cards is licensed through the GNU General Public License.
Read the COPYING file for the complete license.
