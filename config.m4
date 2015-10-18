dnl $Id$
dnl config.m4 for extension phpast

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(phpast, for phpast support,
dnl Make sure that the comment is aligned:
dnl [  --with-phpast             Include phpast support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(phpast, whether to enable phpast support,
[  --enable-phpast           Enable phpast support])

if test "$PHP_PHPAST" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-phpast -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/phpast.h"  # you most likely want to change this
  dnl if test -r $PHP_PHPAST/$SEARCH_FOR; then # path given as parameter
  dnl   PHPAST_DIR=$PHP_PHPAST
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for phpast files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PHPAST_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PHPAST_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the phpast distribution])
  dnl fi

  dnl # --with-phpast -> add include path
  dnl PHP_ADD_INCLUDE($PHPAST_DIR/include)

  dnl # --with-phpast -> check for lib and symbol presence
  dnl LIBNAME=phpast # you may want to change this
  dnl LIBSYMBOL=phpast # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PHPAST_DIR/$PHP_LIBDIR, PHPAST_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PHPASTLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong phpast lib version or lib not found])
  dnl ],[
  dnl   -L$PHPAST_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PHPAST_SHARED_LIBADD)

  PHP_NEW_EXTENSION(phpast, phpast.c phpast_kind.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
