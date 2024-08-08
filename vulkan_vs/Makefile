vulkan_sdk  = "%VK_SDK_PATH%"
glfw_sdk    = "C:/Program Files/OpenGL"

include_path= /Isrc /I $(vulkan_sdk)/Include /I $(glfw_sdk)/Include /I .
link_path	= /link /LIBPATH:$(vulkan_sdk)/Lib /LIBPATH:$(glfw_sdk)/lib

defines		= /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE 

cc_option 	= 	/nologo /W3 /WX- /diagnostics:column \
				/sdl /O2 /Oi /GL /Gm- /EHsc /MD /GS /Gy \
				/fp:precise /Zc:wchar_t /Zc:forScope /Zc:inline \
				/utf-8 /std:c++20 /wd4996 #/wd4834

link_option = 	/NODEFAULTLIB:msvcrtd.lib

shader_path = "src/shader"

				
cc			= cl #clang-cl


all: src/main.cpp
	build.bat > log 

build: src/main.cpp #<-はbuild.bat経由で実行される
	$(cc) $(cc_option) $(include_path) $(defines) src/main.cpp \
	$(link_path) $(link_option)

r: 
	./main.exe 
	
cs:
	glslc 	$(shader_path)/shader.vert  -o $(shader_path)/shader.vert.spv
	glslc  	$(shader_path)/shader.frag 	-o $(shader_path)/shader.frag.spv

	glslc  	$(shader_path)/rt0/phoneShading.rchit 	-o $(shader_path)/rt0/phoneShading.rchit.spv 	--target-env=vulkan1.2
	glslc  	$(shader_path)/rt0/refract.rchit 	-o $(shader_path)/rt0/refract.rchit.spv 	--target-env=vulkan1.2
	glslc  	$(shader_path)/rt0/miss.rmiss 		-o $(shader_path)/rt0/miss.rmiss.spv		--target-env=vulkan1.2
	glslc  	$(shader_path)/rt0/raygen.rgen 		-o $(shader_path)/rt0/raygen.rgen.spv		--target-env=vulkan1.2