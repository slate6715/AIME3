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
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h limits.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/socket.h termios.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_STRERROR_R
AC_FUNC_STRTOD
AC_CHECK_FUNCS([bzero floor memmove memset select socket strchr strstr strtol])

#AC_CHECK_LIB([libconfig++], [config_init], [], [
PKG_CHECK_MODULES([LIBCONFIGXX], [libconfig++ >= 1.5],,
   AC_MSG_ERROR([libconfig++ 1.7 or newer not found. It is required for managing config files.])
   )

AC_CHECK_LIB([argon2], [argon2i_hash_raw], [], [
   echo "You are missing libargon2. It is required for password authentication."
   exit -1;
   ])

AX_BOOST_BASE([1.67.0],,
	AC_MSG_ERROR([Program requires Boost but it was not found on your system])
	)

AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_FILES([Makefile
		 src/Makefile])

AC_OUTPUT
