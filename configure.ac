#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h strings.h sys/socket.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([bzero socket strtol select])
#AC_CHECK_LIB([libconfig++], [config_init], [], [
PKG_CHECK_MODULES([LIBCONFIGXX], [libconfig++ >= 1.5],,
   AC_MSG_ERROR([libconfig++ 1.7 or newer not found. It is required for managing config files.])
   )

AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_FILES([Makefile
		 src/Makefile])

AC_OUTPUT
