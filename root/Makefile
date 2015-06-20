
build_dir := build
src_dirs := 
srcs := cinit_main.cpp test.cpp preload_main.cpp

targets := cinit mtest preload cinit_s preload_s

#-finput-charset=ascii

common_cpp := --std=c++11 -Wall
common_ld := -lpthread -lrt
common_cpp += -Wall -Wextra -pedantic -Wmissing-declarations -Wpointer-arith -Wwrite-strings -Wformat=2 -Wlogical-op -Wcast-align -Wcast-qual -Wundef -Wmissing-include-dirs -Wfloat-equal -Wconversion
common_cpp += -I../../common -I. 

cinit_src := cinit_main.cpp 
mtest_src := test.cpp
preload_src := preload_main.cpp

# static linked version
cinit_s_src := cinit_main.cpp
preload_s_src := preload_main.cpp

cinit_s_ld := -static
preload_s_ld := -static

#serializer_mtest_src_dir := mtest
#serializer_mtest_src := cpp-lib/posix/File.cpp cpp-lib/posix/thread.cpp
#serializer_mtest_cpp := -I../cpputest/include
#serializer_mtest_cpp += -include ../cpputest/include/CppUTest/MemoryLeakDetectorNewMacros.h
#serializer_mtest_cpp += -include ../cpputest/include/CppUTest/MemoryLeakDetectorMallocMacros.h
#serializer_mtest_ld := -Wl,-Bstatic -lCppUTest -Wl,-Bstatic -lCppUTestExt -Wl,-Bdynamic -lgcc_s  -L../cpputest/lib

include Makefile.common

install: $(build_dir)/cinit
	cp $^ ../