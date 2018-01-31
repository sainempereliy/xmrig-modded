mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=gcc-7 -DCMAKE_CXX_COMPILER=g++-7 -DWITH_LIBCPUID=ON -DWITH_AEON=ON -DWITH_HTTPD=OFF
make

sysctl -w vm.nr_hugepages=500
ulimit -n 65000
echo "* hard nofile 1048576" >> /etc/security/limits.conf
echo "* soft nofile 1048576" >> /etc/security/limits.conf
echo 800000 > /proc/sys/fs/file-max
echo "vm.nr_hugepages=500" >> /etc/sysctl.conf

./xmrig --help
