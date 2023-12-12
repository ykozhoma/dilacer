CXX=c++
CXXFLAGS= -std=c++17
INCLUDES= -I./jpeg-9e/out/include
JPEG_LIB= -L./jpeg-9e/out/lib

all : dilace

dilace : src/dilacer.cpp src/main.cpp  
	$(CXX) $(CXXFLAGS) $^ -o $@ $(INCLUDES) -ljpeg $(JPEG_LIB)

.PHONY: clean
clean:
	rm -f dilace