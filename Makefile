# Makefile for Fluid Dynamics Model 
# Final Project in Computer Graphics I, CS488
# at University of Illinois at Chicago, Fall 2014


BASE_DIR = ../../../aluminum
RUN_PATH = $(BASE_DIR)/linux
INCLUDE_DIR = /usr/include
LIB_DIR = /usr/lib
APP_PATH = .
SRC_DIR = $(BASE_DIR)/src
BUILD_DIR = $(BASE_DIR)/build

COMPILER = g++

FLAGS = -std=c++11 -Wreturn-type -Wformat -Wmissing-braces -Wparentheses -Wswitch -Wunused-variable -Wsign-compare -Wno-unknown-pragmas -Woverloaded-virtual # -O3
LINUXGLLIBS = -lglut -lGL -lm -lfreeimage -lSDL -lSDL_image
INCLUDE = -I./ -I$(BUILD_DIR)/include -I$(INCLUDE_DIR) -I$(APP_PATH)
SRC = $(SRC_DIR)/*.cpp $(RUN_PATH)/*.cpp $(APP_SRC)


all: compile

# Precompiling headers
# Should only be needed if Include.hpp changed
pre:
	$(RUN_PATH)/scripts/precompileHeaders.sh $(INCLUDE_DIR)

# Build aluminum static lib
# Should only be needed if aluminum files changed
# Assumes that Include.hpp has been pre-compiled (make pre)
build: 
	$(RUN_PATH)/scripts/makeStaticLibrary.sh $(INCLUDE_DIR)

# Installing Aluminum
install: 	
	$(RUN_PATH)/scripts/installAluminumLinux.sh $(BASE_DIR)


# COMPILING
# Compile with aluminum lib
compile: simulation
	@echo "Compiling..."

simulation: sphModel.o Simulation.o 
	$(COMPILER) $(FLAGS) $(LINUXGLLIBS) $(INCLUDE) *.o -o simulation $(BASE_DIR)/build/lib/libaluminum.a

sphModel.o: sphModel.cpp sphModel.hpp
	$(COMPILER) -c $(FLAGS) sphModel.cpp

Simulation.o: Simulation.cpp
	$(COMPILER) -c $(FLAGS) $(LINUXGLLIBS) $(INCLUDE) Simulation.cpp $(BASE_DIR)/build/lib/libaluminum.a


run: all
	./simulation

new: clean pre build all

clean:
	rm -rf *o simulation \
	rm $(BASE_DIR)/build/lib/libaluminum.a; \
	rm $(BASE_DIR)/build/include/Aluminum/*.hpp; \
	rm $(BASE_DIR)/build/include/Aluminum/*.h; \
	rm $(BASE_DIR)/build/include/Includes.hpp.gch; \
	rm simulation; \
	touch flag;
