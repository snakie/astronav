# ASTRONAV

A framework for simulating the solar system, designed as a test harness for solar system navigation software, specifically a startracker. 

# Prerequisites

## OpenGL

sudo apt install freeglut3-dev libglu1-mesa-dev

## CSPICE

Download cspice toolkit:
  http://naif.jpl.nasa.gov/pub/naif/toolkit//C/PC_Linux_GCC_64bit/packages/cspice.tar.Z

Download cspice kernel de430.bsp:
  https://naif.jpl.nasa.gov/pub/naif/generic_kernels/spk/planets/de430.bsp

  make cspice/lib/cspice.a

## HYGDATA
Download hygdata base:
http://www.astronexus.com/files/downloads/hygdata_v3.csv.gz

  make hygdata_v3.csv

# Building

  make

# Running

  (it runs better then the gifs)

  ./solar
  
  ![Outer Animation](../assets/photos/astronav-outer.gif?raw=true)
  
  ./solar --inner
  
  ![Inner Animation](../assets/photos/astronav-inner.gif?raw=true)
  
  ./solar --sat
  
  ![Sat Animation](../assets/photos/astronav-sat.gif?raw=true)
  
  ./solar --auto
  
  ![Auto Animation](../assets/photos/astronav-auto.gif?raw=true)
