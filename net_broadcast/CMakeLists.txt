PROJECT (NET_BROADCAST) 
cmake_minimum_required(VERSION 2.8)
SET(CMAKE_SYSTEM_NAME Linux) #设置系统名称
SET(CMAKE_C_COMPILER "arm-none-linux-gnueabi-gcc") #设置编译器名称或绝对路径，如果不设置默认为gcc

ADD_SUBDIRECTORY(src) #src 指定源程序路径 ，在src目录下有CMakeLists.txt文件，bin目录是指将生成的可执行程序存放于构建目录下的bin目录下
