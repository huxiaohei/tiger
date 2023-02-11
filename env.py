# -*- coding= utf-8 -*-

__time__ = '2023/02/09'
__author__ = '虎小黑'

import os

if not os.popen('g++ --version').readline():
    print('Please install g++. You can run this code')
    print('cd')
    print('sudo yum -y install gcc-c++')
else:
    print(os.popen('g++ --version').readline())

if not os.popen('ragel --version').readline():
    print('Please install ragel. You can run this code')
    print('cd')
    print('wget http://www.colm.net/files/ragel/ragel-6.10.tar.gz')
    print('tar -xzvf ragel-6.10.tar.gz')
    print('cd ./ragel-6.10')
    print('./configure')
    print('make')
    print('sudo make install')
else:
    print(os.popen('ragel --version').readline())

if not os.popen('cmake --version').readline():
    print('Please install cmake. You can run this code')
    print('cd')
    print('sudo yum -y install cmake')
else:
    print(os.popen('cmake --version').readline())


if not os.popen('locate libyaml-cpp.a').readline():
    print('Please install yaml-cpp. You can run this code')
    print('cd')
    print('wget https://github.com/jbeder/\
yaml-cpp/archive/refs/tags/yaml-cpp-0.6.2.tar.gz')
    print('tar -xzvf yaml-cpp-0.6.2.tar.gz')
    print('cd yaml-cpp-yaml-cpp-0.6.2/')
    print('mkdir build')
    print('cd build')
    print('cmake -DYAML_BUILD_SHARED_LIBS=on ..')
    print('make -j4')
    print('sudo make install')
else:
    print(os.popen('locate libyaml-cpp.a').readline())


if not os.popen('openssl version').readline():
    print('Please install openssl. You can run this code')
    print('cd')
    print('sudo yum -y install gcc libffi-devel zlib* openssl-devel')
    print('wget https://www.openssl.org/source/openssl-3.0.1.tar.gz')
    print('tar -xzvf openssl-3.0.1.tar.gz')
    print('cd openssl-3.0.1')
    print('./Configure --prefix=/usr/local/openssl')
    print('make')
    print('sudo make install')
else:
    print(os.popen('openssl version').readline())


if not os.popen('yum list installed | grep boost.x86_64').readline():
    print('Please install boost. You can run this code')
    print('cd')
    print('sudo yum -y install boost')
else:
    print(os.popen('yum list installed | grep boost.x86_64').readline())

if not os.popen('yum list installed | grep boost-devel.x86_64').readline():
    print('Please install boost. You can run this code')
    print('cd')
    print('sudo yum -y install boost-devel')
else:
    print(os.popen('yum list installed | grep boost-devel.x86_64').readline())
