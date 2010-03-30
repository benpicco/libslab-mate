AC_INIT([libslab], [2.30.0],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=FIXME])
AC_CONFIG_SRCDIR([libslab])

AM_INIT_AUTOMAKE([1.9 foreign no-dist-gzip dist-bzip2])
AM_MAINTAINER_MODE

AC_CONFIG_HEADERS([config.h])

# Enable silent build when available (Automake 1.11)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Before making a release, the LT_VERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A

LT_VERSION=2:0:2
AC_SUBST(LT_VERSION)

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "\${ACLOCAL_FLAGS}")

GNOME_COMPILE_WARNINGS

AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AM_PROG_LIBTOOL

dnl ==============================================
dnl translation support
dnl ==============================================

GETTEXT_PACKAGE=libslab
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

IT_PROG_INTLTOOL([0.35.0])
AM_GLIB_GNU_GETTEXT

dnl ==============================================
dnl Check that we meet the dependencies
dnl ==============================================

GLIB_REQUIRED=2.18.0
GTK_REQUIRED=2.14.0

AM_PATH_GLIB_2_0($GLIB_REQUIRED,,,gobject)

MODULES="glib-2.0 >= $GLIB_REQUIRED	\
	 gio-2.0 >= $GLIB_REQUIRED	\
	 gobject-2.0			\
	 gtk+-2.0 >= $GTK_REQUIRED	\
	 gdk-2.0     			\
	 gnome-desktop-2.0		\
	 librsvg-2.0			\
	 gconf-2.0			\
	 libgnome-menu"

PKG_CHECK_MODULES(LIBSLAB, [ $MODULES ])

AC_SUBST(LIBSLAB_CFLAGS)
AC_SUBST(LIBSLAB_LIBS)

dnl ==============================================
dnl Check strftime for %l and %k support
dnl ==============================================

AC_MSG_CHECKING(for %l and %k support in strftime)
AC_TRY_RUN([
#include <string.h>
#include <time.h>

int main(int argc, char **argv)
{
	char buf[10];
	time_t rawtime;
	struct tm *timeinfo;

	time(&rawtime);
	timeinfo=localtime(&rawtime);
	buf[0] = '\0';
	strftime(buf, 10, "%lx%k", timeinfo);

	if (buf[0] == '\0' || buf[0] == 'x' || strstr(buf, "l") || strstr(buf, "k"))
		exit(1);
	else
		exit(0);
}],[
AC_DEFINE(HAVE_LKSTRFTIME, 1, [strftime supports use of l and k])
ac_cv_lkstrftime=yes
],ac_cv_lkstrftime=no,ac_cv_lkstrftime=no,[
AC_DEFINE(HAVE_LKSTRFTIME, 1, [strftime supports use of l and k])
ac_cv_lkstrftime=yes
])
AC_MSG_RESULT($ac_cv_lkstrftime)

dnl ===============================================
dnl Enable internal copies of libslab, but not here
dnl ===============================================

AM_CONDITIONAL(LIBSLAB_FOR_INTERNAL_USE, test "yes" = "no")

AC_CONFIG_FILES([
Makefile
libslab.pc
libslab/Makefile
po/Makefile.in
])

AC_OUTPUT
