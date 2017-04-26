#把编译出来的可执行文件输出到项目的bin目录下  
#例如运行“cmake ..”的目录为build，则在build/bin目录下生成  
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)  

#定义可执行文件名称,同时学习到如何在cmake中定义变量
SET(TARGET_BIN net-Broadcast) 

#添加头文件搜索路径  
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/net 
					${PROJECT_SOURCE_DIR}/src/common 
					${PROJECT_SOURCE_DIR}/src/audio 
					${PROJECT_SOURCE_DIR}/src/protocol
					/usr/local/arm-alsa/include)  

ADD_SUBDIRECTORY(net)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(audio)
ADD_SUBDIRECTORY(protocol)

#添加alsa 与 mad 库
LINK_DIRECTORIES(/usr/local/arm-alsa/lib)

#用于将当前目录下的所有源文件的名字保存在变量 APP_SRCS 中  
#AUX_SOURCE_DIRECTORY(. APP_SRC)  

#该目录下及所有子文件夹内的所有后缀为.c的文件的路径，全部放入SRC_LIST这个变量中
FILE(GLOB_RECURSE SRC_LIST "*.c")  

ADD_EXECUTABLE(${TARGET_BIN} ${SRC_LIST})  

find_package(Threads REQUIRED)
	if(THREADS_HAVE_PTHREAD_ARG)  
		set_property(TARGET ${TARGET_BIN} PROPERTY COMPILE_OPTIONS "-pthread") 
		set_property(TARGET ${TARGET_BIN} PROPERTY INTERFACE_COMPILE_OPTIONS "-pthread")
	endif()
if(CMAKE_THREAD_LIBS_INIT)  
	target_link_libraries(${TARGET_BIN} "${CMAKE_THREAD_LIBS_INIT}")
endif()

#加入alsa也libmad库的支持
TARGET_LINK_LIBRARIES(${TARGET_BIN} mad asound)  

#用于指定从一组源文件 source1 source2 … sourceN(在变量APP_SRCS中定义)   
#编译出一个可执行文件且命名为TestHttpClient  

