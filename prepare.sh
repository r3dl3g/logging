echo ======= New Prepare at $(date) ======= >> prepare.log
defines="-DCMAKE_INSTALL_PREFIX=$HOME/install -DCMAKE_PREFIX_PATH=$HOME/install"
cmake -S. -B $HOME/build/${PWD##*/}/Debug -DCMAKE_BUILD_TYPE=Debug $defines $* 2>&1 | tee -a prepare.log
cmake -S. -B $HOME/build/${PWD##*/}/Release -DCMAKE_BUILD_TYPE=Release $defines $* 2>&1 | tee -a prepare.log
