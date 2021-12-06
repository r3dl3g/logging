
export RASPBIAN_BASE=/opt/raspberrypi
export RASPBIAN_ROOTFS=$RASPBIAN_BASE/rootfs
export PATH=/opt/raspberrypi/cross-pi-gcc/bin:/opt/raspberrypi/cross-pi-gcc/libexec/gcc/arm-linux-gnueabihf/8.3.0:$PATH
export RASPBERRY_VERSION=1
export CXX=arm-linux-gnueabihf-c++

builddir=~/Projekte/build/ibr/logging/$($CXX -dumpversion)-$($CXX -dumpmachine)
prjdir=$PWD

build () {
  if [ "$2" == "On" ]; then
    target=$1-static
  else
    target=$1-dynamic
  fi

  mkdir -p $builddir/$target
  pushd $builddir/$target
  cmake "$prjdir" -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="$1" -DCMAKE_TOOLCHAIN_FILE=$RASPBIAN_BASE/raspi-toolchain/Toolchain-rpi.cmake -DBUILD_FOR_ARM=ON -DLOGGING_BUILD_STATIC_MODULE_LIB="$2"
  make -j6
  popd
}

build Debug On
build Debug Off
build Release On
build Release Off
