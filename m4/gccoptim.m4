AC_DEFUN([ADL_GCC_OPTIM],
[if test -n "$GCC"; then
  AC_CACHE_CHECK([for gcc optimization options], ac_cv_prog_gcc_opt_flags,
  [changequote(,)dnl
  cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[]) { return argv[argc-1] == 0; }
EOF
  changequote([,])dnl
  cf_save_CFLAGS="$CFLAGS"
  ac_cv_prog_gcc_opt_flags="-O3"
  for cf_opt in \
    ffast-math \
    fstrict-aliasing \
    fomit-frame-pointer
  do
    CFLAGS="$cf_save_CFLAGS $ac_cv_prog_gcc_opt_flags -$cf_opt"
    if AC_TRY_EVAL([ac_compile]); then
      ac_cv_prog_gcc_opt_flags="$ac_cv_prog_gcc_opt_flags -$cf_opt"
    fi
  done
  rm -f conftest*
  CFLAGS="$cf_save_CFLAGS $ac_cv_prog_gcc_opt_flags"])
else
  CFLAGS="$cf_save_CFLAGS -O"
fi
])dnl
