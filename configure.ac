dnl Process this file with autoconf to produce a configure script.


AC_INIT(c_icap_allow2,00000x)

AM_MAINTAINER_MODE
AM_CONFIG_HEADER(autoconf.h)

AM_INIT_AUTOMAKE([subdir-objects])

AC_PROG_CC
AM_PROG_CC_C_O

AC_DISABLE_STATIC
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

dnl AC_CANONICAL_HOST

CFLAGS="$CFLAGS -Wall"

dnl check for c-icap
AC_ARG_WITH(c-icap,
[  --with-c-icap	Where to find c-icap ],
[
    cicapdir="$withval"
    if test -f $withval/bin/c-icap-config; then
       cicapflags=`$withval/bin/c-icap-config --cflags`
       cicaplibs=`$withval/bin/c-icap-config --libs`
       cicapetc=`$withval/bin/c-icap-config --configdir`
       cicapprefix=`$withval/bin/c-icap-config --prefix`
       cicapmods=`$withval/bin/c-icap-config --modulesdir`
       cicapdatadir=`$withval/bin/c-icap-config --datarootdir`
    else
       cicapflags=`c-icap-config --cflags`
       cicaplibs=`c-icap-config --libs`
       cicapetc=`c-icap-config --configdir`
       cicapprefix=`c-icap-config --prefix`
       cicapmods=`c-icap-config --modulesdir`
       cicapdatadir=`c-icap-config --datarootdir`
    fi
],
)

if test -z "$cicapdir"; then
    if c-icap-config >& /dev/null; then
        cicapflags=`c-icap-config --cflags`
        cicaplibs=`c-icap-config --libs`
        cicapetc=`c-icap-config --configdir`
        cicapprefix=`c-icap-config --prefix`
        cicapmods=`c-icap-config --modulesdir`
        cicapdatadir=`c-icap-config --datarootdir`
    else
        echo "ERROR: c-icap-config not found!"
	exit -1;
    fi
fi

AM_CONDITIONAL(CICAPMODULESDIR, [test -n $cicapmods])

CFLAGS="$CFLAGS $cicapflags"
MODULES_LIBADD="$cicaplibs"
CONFDIR="$cicapetc"
MODSDIR="$cicapmods"
DATADIR="$cicapdatadir"
AC_SUBST(MODULES_LIBADD)
AC_SUBST(CONFDIR)
AC_SUBST(MODSDIR)
AC_SUBST(DATADIR)

dnl Checks for libraries

dnl Determine whether or not we are big or little endian
AC_MSG_CHECKING([for endianness])
AC_TRY_RUN(
[
int main()
{
short one = 1;
char *cp = (char*)&one;
if ( *cp == 0 )
return(0);
else
return(1);
}
], [echo big-endian; CFLAGS="$CFLAGS -DBIG_ENDIAN"; AC_DEFINE(BIG_ENDIAN, 1, [Define BIG_ENDIAN])],[echo little-endian; CFLAGS="$CFLAGS -DLITTLE_ENDIAN"; AC_DEFINE(LITTLE_ENDIAN, 1, [Define LITTLE_ENDIAN])],[echo cross-compiling])

dnl Determine LARGEFILE support
AC_SYS_LARGEFILE

dnl Check for OpenCV
AC_ARG_WITH(opencv,
[  --with-opencv       Where to find opencv ],
[ case "$withval" in
  yes)
#    AC_MSG_WARN([--with-opencv called without argument])
    opencv="yes"
  ;;
  no )
    opencv="no"
  ;;
  * )
    opencv="yes"
    opencvinc="-I$withval/include/opencv"
    opencvlib="-L$withval/lib"
  ;;
  esac
],
)

AC_SUBST(opencvinc)
AC_SUBST(opencvlib)
OLD_CFLAGS=$CFLAGS
OLD_LIBS=$LIBS
LIBS=$opencvlib
CFLAGS=$opencvinc
opencv="no"
opencv_badversion="no"
dnl -------------------------------------------------------------
AH_TEMPLATE([HAVE_OPENCV],[Define if OpenCV version < 2.2])
AH_TEMPLATE([HAVE_OPENCV_22X],[Define if OpenCV version is 2.2X])
AH_TEMPLATE([HAVE_OPENCV_23X],[Define if OpenCV version is 2.3X])
AH_TEMPLATE([HAVE_OPENCV_30X],[Define if OpenCV version is 3.0X or newer])
AC_MSG_CHECKING([for OpenCV older than 2.2.x])
AC_CHECK_LIB(cxcore, cvCreateImage, [OPENCV_ADD_LDFLAG="-lcv -lcxcore -lhighgui";  opencv="yes"; opencv_version="HAVE_OPENCV"])
AC_MSG_CHECKING([for OpenCV 2.2.x or newer])
AC_CHECK_LIB(opencv_core, cvCreateImage, [OPENCV_ADD_LDFLAG="-lopencv_core -lopencv_highgui -lopencv_objdetect -lopencv_features2d"; opencv="yes"; opencv_version="HAVE_OPENCV_22X"])
AC_MSG_CHECKING([for OpenCV 2.3.x or newer])
AC_CHECK_LIB(opencv_imgproc, cvResize, [OPENCV_ADD_LDFLAG="-lopencv_core -lopencv_highgui -lopencv_objdetect -lopencv_features2d -lopencv_imgproc";  opencv="yes"; opencv_version="HAVE_OPENCV_23X"])
if test "$opencv" = "yes" ; then
	AC_MSG_CHECKING([for OpenCV 3.0 or newer])
	AC_CHECK_LIB(opencv_core, cvRound, [], [echo "Disabling support for OpenCV >= 3.0"; OPENCV_ADD_CFLAG=""; OPENCV_ADD_LDFLAG=""; opencv="no"; opencv_badversion="yes"; opencv_version="HAVE_OPENCV_30X"])
fi
if test "$opencv" = "yes" ; then
	AC_DEFINE_UNQUOTED([$opencv_version], 1, [Define correct HAVE_OPENCV_VERSION if OpenCV installed])
	AC_DEFINE(HAVE_OPENCV, 1, [Define HAVE_OPENCV if OpenCV installed])
fi
AC_SUBST(OPENCV_ADD_LDFLAG)
AC_SUBST(OPENCV_ADD_CFLAG)
CFLAGS=$OLD_CFLAGS
LIBS=$OLD_LIBS

if test "$opencv" = "no" && test "$opencv_badversion" = "no"; then
	AC_SUBST(opencvinc)
	AC_SUBST(opencvlib)
        unset ac_cv_lib_cxcore_cvCreateImage
        unset ac_cv_lib_opencv_core_cvCreateImage
        unset ac_cv_lib_opencv_imgproc_cvResize
	OLD_CFLAGS=$CFLAGS
	OLD_LIBS=$LIBS
	LIBS=$opencvlib
	CFLAGS="$opencvinc -fPIC"
	opencv="no";
	dnl -------------------------------------------------------------
	AC_MSG_CHECKING([for OpenCV older than 2.2.x])
	AC_CHECK_LIB(cxcore, cvCreateImage, [OPENCV_ADD_LDFLAG="-lcv -lcxcore -lhighgui";  opencv="yes";])
	AC_MSG_CHECKING([for OpenCV 2.2.x or newer])
	AC_CHECK_LIB(opencv_core, cvCreateImage, [OPENCV_ADD_LDFLAG="-lopencv_core -lopencv_highgui -lopencv_objdetect -lopencv_features2d"; opencv="yes"; opencv_version="HAVE_OPENCV_22X"])
	AC_MSG_CHECKING([for OpenCV 2.3.x or newer])
	AC_CHECK_LIB(opencv_imgproc, cvResize, [OPENCV_ADD_LDFLAG="-lopencv_core -lopencv_highgui -lopencv_objdetect -lopencv_features2d -lopencv_imgproc";  opencv="yes"; opencv_version="HAVE_OPENCV_23X"])
	if test "$opencv" = "yes" ; then
		AC_MSG_CHECKING([for OpenCV 3.0 or newer])
		AC_CHECK_LIB(opencv_core, cvRound, [], [echo "Disabling support for OpenCV >= 3.0"; OPENCV_ADD_CFLAG=""; OPENCV_ADD_LDFLAG=""; opencv="no"; opencv_badversion="yes"; opencv_version="HAVE_OPENCV_30X"])
	fi
	if test "$opencv" = "yes" ; then
		AC_DEFINE_UNQUOTED([$opencv_version], 1, [Define $opencv_version if OpenCV installed])
		AC_DEFINE(HAVE_OPENCV, 1, [Define HAVE_OPENCV if OpenCV installed])
	fi
	AC_SUBST(OPENCV_ADD_LDFLAG)
	AC_SUBST(OPENCV_ADD_CFLAG)
	CFLAGS=$OLD_CFLAGS
	LIBS=$OLD_LIBS
fi

dnl Check for Allow2 Library
dnl -------------------------------------------------------------
dnl AC_MSG_CHECKING([for TRE regex header (tre/tre.h)])
dnl AC_CHECK_HEADER(tre/tre.h, [AC_DEFINE(HAVE_TRE_HEADER, 1, [Define HAVE_TRE_HEADER if TRE header found])], [AC_MSG_ERROR([Sorry, tre/tre.h not found, this is required])])
dnl AC_MSG_CHECKING([for TRE regex library])
dnl AC_CHECK_LIB(tre, tre_regwncomp, [AC_DEFINE(HAVE_TRE_LIB, 1, [Define HAVE_TRE_LIB if TRE library found])], [AC_MSG_ERROR([Sorry, TRE regex library not found, this is required])])

dnl Check for scandir
dnl -------------------------------------------------------------
AC_CHECK_FUNCS(scandir, [AC_DEFINE(_DEFAULT_SOURCE, 1, [Define _DEFAULT_SOURCE if scandir function is found]),AC_DEFINE(_SVID_SOURCE, 1, [Define _SVID_SOURCE if scandir function is found])])

dnl Check for libicu
dnl -------------------------------------------------------------
dnl AC_DEFUN([AC_CHECK_ICU], [
dnl   succeeded=no
dnl
dnl   if test -z "$ICU_CONFIG"; then
dnl     AC_PATH_PROG(ICU_CONFIG, icu-config, no)
dnl   fi
dnl
dnl   if test "$ICU_CONFIG" = "no" ; then
dnl     echo "*** The icu-config script could not be found. Make sure it is"
dnl     echo "*** in your path, and that taglib is properly installed."
dnl     echo "*** Or see http://ibm.com/software/globalization/icu/"
dnl   else
dnl     ICU_VERSION=`$ICU_CONFIG --version`
dnl     AC_MSG_CHECKING(for ICU >= $1)
dnl         VERSION_CHECK=`expr $ICU_VERSION \>\= $1`
dnl         if test "$VERSION_CHECK" = "1" ; then
dnl             AC_MSG_RESULT(yes)
dnl             succeeded=yes
dnl
dnl             AC_MSG_CHECKING(ICU_CFLAGS)
dnl             ICU_CFLAGS=`$ICU_CONFIG --cflags`
dnl             AC_MSG_RESULT($ICU_CFLAGS)
dnl
dnl             AC_MSG_CHECKING(ICU_CXXFLAGS)
dnl             ICU_CXXFLAGS=`$ICU_CONFIG --cxxflags`
dnl             AC_MSG_RESULT($ICU_CXXFLAGS)
dnl
dnl             AC_MSG_CHECKING(ICU_LIBS)
dnl             ICU_LIBS=`$ICU_CONFIG --ldflags`
dnl             AC_MSG_RESULT($ICU_LIBS)
dnl
dnl             AC_DEFINE(HAVE_LIBICU, 1, [Define HAVE_LIBICU if libicu is installed])
dnl         else
dnl             ICU_CFLAGS=""
dnl             ICU_CXXFLAGS=""
dnl             ICU_LIBS=""
dnl             ## If we have a custom action on failure, don't print errors, but
dnl             ## do set a variable so people can do so.
dnl             ifelse([$3], ,echo "can't find ICU &gt;= $1",)
dnl         fi
dnl
dnl         AC_SUBST(ICU_CFLAGS)
dnl         AC_SUBST(ICU_CXXFLAGS)
dnl         AC_SUBST(ICU_LIBS)
dnl   fi
dnl
dnl   if test $succeeded = yes; then
dnl      ifelse([$2], , :, [$2])
dnl   else
dnl      ifelse([$3], , AC_MSG_ERROR([Library requirements (ICU) not met.]), [$3])
dnl   fi
dnl ])
dnl
dnl AC_MSG_CHECKING([for libicu])
dnl AC_CHECK_ICU(50.1.2)

dnl Check for programs
AC_PROG_LN_S
AC_PATH_PROG(LN, ln, cp)
AC_PATH_PROG(RM, rm, $FALSE)
dnl Libtool 2.2.6 requires: rm -f
RM="$RM -f"

dnl Now determine which modules will going to build .....

AM_CONDITIONAL(USEOPENCV,[test a"$opencv" != ano])

AC_OUTPUT([Makefile \
services/Makefile \
services/allow2/Makefile \
])