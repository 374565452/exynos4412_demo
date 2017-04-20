#把编译出来的可执行文件输出到项目的bin目录下  
#例如运行“cmake ..”的目录为build，则在build/bin目录下生成  
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)  
  
#添加头文件搜索路径  
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib1 ${PROJECT_SOURCE_DIR}/lib2)  

#添加头文件搜索路径  
#INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib2)  
  
#添加库文件搜索路径  ,因为生成的库文件在 build/lib下，所以只需要加入一个目录就可以
LINK_DIRECTORIES(${PROJECT_BINARY_DIR}/lib)  
  
#用于将当前目录下的所有源文件的名字保存在变量 APP_SRCS 中  
AUX_SOURCE_DIRECTORY(. APP_SRC)  
  
#如果调用"CMake -D DEBUG_MODE=ON .."  
#则为源文件设置_DEBUG宏  
IF(DEBUG_MODE)  
  ADD_DEFINITIONS(-D_DEBUG)  
ENDIF()  
  
#用于指定从一组源文件 source1 source2 … sourceN(在变量APP_SRCS中定义)   
#编译出一个可执行文件且命名为TestHttpClient  
ADD_EXECUTABLE(pro ${APP_SRC})  
  
#添加编译可执行程序所需要的链接库、如果有多个中间用空格隔开  
#第一个参数是可执行程序名称，第二个开始是依赖库  
#在这里根据名字XXX去寻找libXXX.a文件  
TARGET_LINK_LIBRARIES(pro libhello.so libfun.so)  

#将可执行程序pro安装在bin下 ，注意这里的bin路径指的是${CMAKE_INSTALL_PREFIX}/bin目录
INSTALL(TARGETS pro RUNTIME DESTINATION bin) 