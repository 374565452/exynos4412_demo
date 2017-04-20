PROJECT (HELLO) 
cmake_minimum_required(VERSION 2.8)
#SET(CMAKE_SYSTEM_NAME Linux) #设置系统名称
#SET(CMAKE_C_COMPILER "arm-none-linux-gnueabi-gcc") #设置编译器名称或绝对路径，如果不设置默认为gcc
MESSAGE(STATUS "This is BINARY dir " ${HELLO_BINARY_DIR})
MESSAGE(STATUS "This is SOURCE dir "${HELLO_SOURCE_DIR})
ADD_SUBDIRECTORY(src bin) #src 指定源程序路径 ，在src目录下有CMakeLists.txt文件，bin目录是指将生成的可执行程序存放于构建目录下的bin目录下
ADD_SUBDIRECTORY(doc) #将doc目录下的内容安装在share/doc/cmake/t2下
INSTALL(FILES COPYRIGHT README DESTINATION share/doc/cmake/t2) #将COPYRIGHT README安装在share/doc/cmake/t2下
INSTALL(PROGRAMS runhello.sh DESTINATION bin) #将runhello.sh安装在bin下

#INSTALL(DIRECTORY doc/ DESTINATION share/doc/cmake/t2) #将doc目录下的hello.txt安装在share/doc/cmake/t2下，这里是将doc目录下的内容安装在share/doc/cmake/t2下，并不是将doc整个目录安装在share/doc/cmake/t2下,这可以利用在doc下面的CMakeLists.txt来完成此功能