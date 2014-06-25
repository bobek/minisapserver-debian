dnl Autoconf settings for mini-SAP-server
dnl $Id: configure.ac 344 2009-01-11 18:48:12Z courmisch $

AC_COPYRIGHT([Copyright (C) the VideoLAN team 1998-2009])

AC_INIT(miniSAPserver,0.3.6,streaming@videolan.org)
AC_PREREQ(2.50)

AC_CONFIG_SRCDIR(configure.ac)
AC_CONFIG_AUX_DIR(admin)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_HEADERS(config.h)

dnl ****************************
dnl        General checks
dnl ****************************
AC_PROG_CC
AC_PROG_CXX
AC_C_CONST
AC_HEADER_STDBOOL

AC_CHECK_FUNCS([clearenv])
RDC_REPLACE_FUNC_GETOPT_LONG


dnl ****************************
dnl        Features
dnl ****************************

AM_INIT_AUTOMAKE([check-news dist-bzip2 no-dist-gzip std-options])

dnl On the fly charset transcoding
AM_ICONV

RDC_STRUCT_SOCKADDR_LEN

dnl Enable/Disable slp
AS_IF([test "x${enable_slp}" == "x"],
	[enable_slp="no"])
AC_ARG_ENABLE(slp,
	[AC_HELP_STRING([--enable-slp],
			[enable SLP announcing (default disabled)])],
	[AS_IF([test "${enable_slp}" != "no"],
		[AC_CHECK_HEADERS(slp.h,,
			[AC_MSG_ERROR(["Unable to find SLP headers"])])
		 AC_DEFINE(CONFIG_SLP,,[SLP announcing support])
        ])
	])
AM_CONDITIONAL(CONFIG_SLP, [test "${enable_slp}" != "no"])

dnl Debug Mode
AC_ARG_ENABLE(debug,
	AC_HELP_STRING([--enable-debug],
			[enable debug mode (default enabled)]))

AS_IF([test "${enable_debug}" != "no"],[CFLAGS="${CFLAGS} -g"])


dnl ****************************
dnl       Output Files
dnl ****************************

AC_SUBST(CONFIG_SLP)
AC_SUBST(CFLAGS)

AC_OUTPUT(Makefile)
