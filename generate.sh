
builddir=build/$(c++ --version | { read first rest ; echo $first ; })$(c++ -dumpversion)
prjdir=$PWD

build () {
  if [ "$2" == "On" ]; then
    target=$1-static
  else
    target=$1-dynamic
  fi

  mkdir -p $builddir/$target
  pushd $builddir/$target
  cmake "$prjdir" -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="$1" -DLOGGING_BUILD_STATIC_MODULE_LIB="$2"
  make -j6
  popd
}

build Debug On
build Debug Off
build Release On
build Release Off
