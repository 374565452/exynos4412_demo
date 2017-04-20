MESSAGE(STATUS "This is TestHttpClient_SOURCE_DIR="${PROJECT_SOURCE_DIR})  
MESSAGE(STATUS "This is CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR})  


#该目录下及所有子文件夹内的所有后缀为.c的文件的路径，全部放入SRC_LIST这个变量中
FILE(GLOB_RECURSE LIBHELLO_SRC "*.c") 
#将生成的库放置在编译目录下的lib目录下 此工程中的build/lib下
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
#创建一个共享库---动态库libhello.so
ADD_LIBRARY(hello SHARED ${LIBHELLO_SRC}) 
#创建一个共享库---静态库，但此时不能生成一个libhello.a
#ADD_LIBRARY(hello STATIC ${LIBHELLO_SRC}) 
#创建一个共享库---静态库libhello_static.a
ADD_LIBRARY(hello_static STATIC ${LIBHELLO_SRC})

#为了可以实现同时生成libhello.so与libhello.a
SET_TARGET_PROPERTIES(hello_static PROPERTIES OUTPUT_NAME "hello") #将libhello_static.a重命名为libhello.a
#防止名称相同的库删除掉
SET_TARGET_PROPERTIES(hello PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(hello_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#向库文件名添加上版本号信息
SET_TARGET_PROPERTIES(hello PROPERTIES VERSION 1.2 SOVERSION 1)

#安装库与头文件
INSTALL(TARGETS hello hello_static 
	LIBRARY DESTINATION lib
  	ARCHIVE DESTINATION lib)

INSTALL(FILES hello.h DESTINATION include) 
