#把编译出来的可执行文件输出到项目的bin目录下  
#例如运行“cmake ..”的目录为build，则在build/bin目录下生成  
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)  
  
#添加头文件搜索路径  
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src/d1 ${PROJECT_SOURCE_DIR}/src/d2)  

ADD_SUBDIRECTORY(d2)
  
#用于将当前目录下的所有源文件的名字保存在变量 APP_SRCS 中  
#AUX_SOURCE_DIRECTORY(. APP_SRC)  

#该目录下及所有子文件夹内的所有后缀为.c的文件的路径，全部放入SRC_LIST这个变量中
FILE(GLOB_RECURSE SRC_LIST "*.c")  


#用于指定从一组源文件 source1 source2 … sourceN(在变量APP_SRCS中定义)   
#编译出一个可执行文件且命名为TestHttpClient  
ADD_EXECUTABLE(pro ${SRC_LIST})  
