SET(SRC_LIST main.c)
ADD_EXECUTABLE(hello ${SRC_LIST})
INSTALL(TARGETS hello RUNTIME DESTINATION bin) #将可执行程序hello安装在bin下 ，注意这里的bin路径指的是${CMAKE_INSTALL_PREFIX}/bin目录