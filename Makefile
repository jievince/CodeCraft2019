# 可执行文件
TARGET=main
# CPP文件
SRCS = main.cpp
# 目标文件
OBJS = $(SRCS:.cpp=.o)
 
# 指令编译器和选项
CPP=g++
 
$(TARGET):$(OBJS)
#	@echo TARGET:$@
#	@echo OBJECTS:$^
$(CPP) -o $@ $^
 
clean:
rm -rf $(TARGET) $(OBJS)
 
%.o:%.cpp
$(CPP) -o $@ -c $<
