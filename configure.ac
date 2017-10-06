AC_INIT([SpW_Loopback_Test], [1.0], [jmgomez@iaa.es])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

dnl LIBSTAR_PATH=/usr/local/STAR-Dundee/STAR-System/inc/star/
dnl LIBSTAR=-lstar_conf_api_brick_mk3 -lstar_conf_api_mk2 -lstar_conf_api_router -lstar-api

AC_PROG_CC
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	])
AC_CHECK_HEADER(star-api.h,
                [AC_DEFINE(HAVE_STAR_H, 1,
		[Define to 1 if you have <star-api.h>.])],
                [AC_MSG_ERROR([Could not locate STAR-API.  Remember is the user's responsibilty to tell the configure script where the libraries are maybe you should set env CPPFLAGS='-I/usr/local/STAR-Dundee/STAR-System/inc/star/' LDFLAGS='-L/usr/local/STAR-Dundee/STAR-System/inc/star/' ./configure
])])
AC_OUTPUT
