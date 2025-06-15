echo ======= New Prepare at $(date) ======= >> prepare.log
cmake -S. -Bbuild -DCMAKE_INSTALL_PREFIX=../libs -DCMAKE_PREFIX_PATH=../libs -DLOGGING_CONFIG_INSTALL=ON -DLOGGING_BUILD_STATIC_MODULE_LIB=OFF $* 2>&1 | tee -a prepare.log
