#!/bin/bash

autoreconf -vis

cd build

env CPPFLAGS='-I/usr/local/STAR-Dundee/STAR-System/inc/star/' LDFLAGS='-L/usr/local/STAR-Dundee/STAR-System/inc/star/' ../configure

make



