#此案例是测试多个目录，生成一个可执行程序


PROJECT (ONE_EXE) 
cmake_minimum_required(VERSION 2.8)

#要显示执行构建过程中详细的信息(比如为了得到更详细的出错信息)  
SET(CMAKE_VERBOSE_MAKEFILE ON )  


ADD_SUBDIRECTORY(src)

#在这个案例中，执行cmake -DCMAKE_INSTALL_PREFIX=/usr .. 则可以执行pro程序，调用libhello.so与libfun.so