build for debug without connection module<br />
cmake -S ../ -B . -DCMAKE_BUILD_TYPE=Debug -DREMOTE_CLIENT=OFF, DNO_REMOTE_CLIENT can be omitted <br />
build for debug with connection module<br />
cmake -S ../ -B . -DCMAKE_BUILD_TYPE=Debug -DREMOTE_CLIENT=ON <br />