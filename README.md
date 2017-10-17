#Minimal Loopback test for GR718B with Star-Dundee Mk3 Bick

TEST INCLUDED
================
loopback => Generates a loopback test from Link 1 to Link 2.
rmap => Generates rmap write packet to configure GR718B.

BUILDING IUNSTRUCTIONS
======================
autoreconf -vis
env CPPFLAGS='-I/usr/local/STAR-Dundee/STAR-System/inc/star/' LDFLAGS='-L/usr/local/STAR-Dundee/STAR-System/inc/star/' ./configure
make 


BUILD OBJECTIVES
================
make TAGS  => Generate TAGS files.

make distclean

