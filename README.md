# Refloid

This repository contains a GPU-based raytracer based on the NVIDIA optics framework [NVIDIA-Optix-6.0](https://developer.nvidia.com/optix). It was developed to simulate geometrical optical measurement techniques with the focus on predicting interfering reflections during measurements.

Through object-oriented programming, great care has been taken to ensure that the raytracer is kept general and can therefore be expanded for any application in geometrical-optical metrology. The code was commented using the official Doxygen syntax in order to be able to render the documentation properly.

Additionally, for the sake of simplicity, the framework was designed to allow raytracing to be run on a remote machine. The scene is configured by [ZMQ](https://zeromq.org) using Python over a socket connection, the rendering is started and finally the results are sent back to the local computer.

## Build

To build this project, NVIDIA-Optix-6.0 is needed with a corresponding working version of CUDA. Simply adjust the environment variable `OptiX_INSTALL_DIR` in the CMakeLists.txt file to your install path of the optix framework.  
It is prefered to use shadow build:

```bash
mkdir BUILD_DIR && cd BUILD_DIR  
git clone https://github.com/nilsmelchert/refloid.git
mkdir build && cd build
cmake ..
make
```

## Classes

The structure of the project can be seen either by looking directly at the documented code or by rendering the documentation using [Doxygen](http://www.doxygen.nl).

## Dependencies

    - Optix 6.0
    - Qt5 (From apt tree)
    - libtiff-dev (From apt tree)