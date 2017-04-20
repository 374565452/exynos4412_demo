#此案例是一个综合案例，lib1目录下是一个生成库的工程项目（目标文件为libhello.so,libheelo.a)，lib2目录下是一个生成库的工程项目
#(目标文件为libfun.so,libfun.a)
#src目录是生成可执行程序工程（目标文件为pro)
PROJECT (PRO_DEMO) 
cmake_minimum_required(VERSION 2.8)

#要显示执行构建过程中详细的信息(比如为了得到更详细的出错信息)  
SET(CMAKE_VERBOSE_MAKEFILE ON )  

ADD_SUBDIRECTORY(lib1) #此命令会将lib目录下的CMakeLists.txt加进来
ADD_SUBDIRECTORY(lib2)
ADD_SUBDIRECTORY(src)

#在这个案例中，执行cmake -DCMAKE_INSTALL_PREFIX=/usr .. 则可以执行pro程序，调用libhello.so与libfun.so