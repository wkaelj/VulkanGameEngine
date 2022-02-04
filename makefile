
CFLAGS = -std=c18 
CC = gcc
DCC = gdb
LFLAGS = -lglfw -lvulkan #-I~/vulkan/1.2.198.1/x86_64/include/ -L$(VK_LAYER_PATH)

OUT = build
OBJSOUT = $(OUT)/objects

OBJECTS = main.c vulkantools/initvk.c debug/debug.c render/*.c utilities/*.c
HEADERS = dependencies.h vulkantools/initvk.h debug/debug.h render/*.h utilities/*.h

SHADERS = shaders #shaders folder
VERTSHADERS = simple_shader.vert
FRAGSHADERS = simple_shader.frag
SHADERCC = $(VULKAN_SDK)/bin/glslc

default: master.out

#compile master build
master.out: $(OBJECTS) $(HEADERS)
	$(CC) -O0 $(CFLAGS) $(OBJECTS) -o $(OUT)/master.out $(LFLAGS)

#compile debug build
debug.out: $(OBJECTS) $(HEADERS)
	$(CC) -g $(CFLAGS) $(OBJECTS) -o $(OUT)/debug.out $(LFLAGS)

#run master build
run: master.out
	./$(OUT)/master.out

#run debug build
rundbg: debug.out
	gdb $(OUT)/debug.out

#compile vertex shaders
shaders: shaders/*.vert shaders/*.frag
	./compile_shaders.sh
	