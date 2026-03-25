. ./.project_defines

BUILD_PREFIX=~/.cache/build/emc
INSTALL_PREFIX=~/.local/emc
CMAKE_CMD="emcmake cmake"
DEFINES="-DLOGGING_BUILD_DEPENDENT_LIBS=OFF -DLOGGING_NO_THREAD=ON -DVERBOSE=ON -DLOGGING_TESTS=ON"

prepare Debug $@
prepare Release $@
