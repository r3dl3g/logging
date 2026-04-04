. ./.project_defines

if [ "$PLATFORM" == "emc" ]; then
    INSTALL_PREFIX=~/.local/emc
    CMAKE_CMD="emcmake cmake"
    DEFINES="-DLOGGING_BUILD_DEPENDENT_LIBS=OFF -DLOGGING_NO_THREAD=ON"
elif [ "$PLATFORM" == "win32" ]; then
    INSTALL_PREFIX=~/.local/win32
    DEFINES="-DWIN32=ON -DLOGGING_BUILD_STATIC_MODULE_LIB=ON"
fi

prepare Debug $@
prepare Release $@
