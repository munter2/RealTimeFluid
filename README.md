=========================================
REAL TIME INTERACTIVE FLUID SIMULATION
=========================================

Computer Graphics I final project, 12/12/2014

Authors: Michael Berg & Matthias Untergassmair

Contact: untergam@student.ethz.ch



-----------------------------------------
Quick Intro
-----------------------------------------
This is a very simple real time SPH implementation.
If you are in an up-to-date 'aluminum/linux/SOME_SUBFOLDER' folder
and just want to run the program, just type

			make run
			
on the command line

For more info, keep reading...


-----------------------------------------
Running the Program
-----------------------------------------
This simulation program relies on the OpenGL Aluminum framework 
that we used in class (https://github.com/CreativeCodingLab/aluminum.git).
The Working directory with the source files is expected to be 
located in 'Aluminum/linux/SOME_SUBFOLDER/'.
As of now, it only runs on linux machines (tested on Fedora 20),
but with some minor modifications it should also run on MacOS 
and Windows.
In order to have more control over the compilation process and 
not to compile the entire Aluminum framework everytime, we made 
a Makefile that only compiles the necessary files in each run.
It widely builds on the 'run.command' file from the original
Aluminum project and relies on the scripts in
'aluminum/linux/scripts'

The Makefile can be used as follows:

	make install: installs Aluminum

	make clean: cleans all executables, object files and the
		Aluminum framework.

	make pre: precompiles headers of Aluminum library

	make build: builds the Aluminum static library

	make (all): compiles the program Simulation.cpp with all
		dependencies and creates an executable 'simulation'
		ready to run

	make run: is a handy shortcut to compile and run the 
		program with one single command.


-----------------------------------------
Interacting with the application
-----------------------------------------
The following keyboard commands can be used to interact with
the simulation / visualization

		1		go fullscreen

	5-0:		Switch between available visualization modes:

		0		Metaball visualization (not implemented yet)
		9		No colorcoding
		8		Colorcoding by particle speed
		7		Colorcoding by particle velocity
		6		Colorcoding by preassure
		5		Colorcoding by density

	SPACE		Switch between displaying all particles,
				displaying fluid only, and displaying fluid
				with object but without walls

	q,w			Move the object in x-direction
	a,s			Move the object in y-direction
	z,x			Move the object in z-direction

	e,r			Move the camera in x-direction
	d,f			Move the camera in y-direction
	c,v			Move the camera in z-direction
	ARROWS		rotating the camera

	g			toggle gravity
