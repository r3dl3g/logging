. ./.project_defines

BUILD_PREFIX=~/.cache/build/emc
INSTALL_PREFIX=~/.local/emc
CMAKE_CMD="emcmake cmake"
DEFINES=-DLOGGING_BUILD_DEPENDENT_LIBS=OFF

prepare Debug -DLOGGING_TESTS=ON $@
prepare Release $@
