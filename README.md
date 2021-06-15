# Odin Engine
## Installation guide 

### Install the project and its dependencies
```
$ git clone https://github.com/warriormaster12/OdinEngine
$ cd OdinEngine
$ git submodule init
$ git submodule update
```
```
Download latest Vulkan-SDK https://vulkan.lunarg.com/
```


#### additional dependencies for Windows
```
Download and install cmake-gui https://cmake.org/download/
Download and install python for compiling glslang https://www.python.org/downloads/
```

####  Vulkan-SDK doc provides installation guide for installing SDK succefully
![Screenshot_20200509_113228](https://user-images.githubusercontent.com/33091666/81468532-cd3d2c80-91e8-11ea-94d6-cf9ce4713e68.png)
### Build the project
#### Linux
```
$ mkdir build
$ cd build
$ cmake ..
```
##### if you want to compile the engine for Wayland display server instead of X11
```
$ cmake .. -DGLFW_USE_WAYLAND=ON
```
```
(without multithreading)
$ make

(with multithreading)
$ make -j4 
```
#### Windows 
```
create "build" folder
Generate compiler instruction set with cmake-gui and build https://youtu.be/LxHV-KNEG3k?t=155
```
### Shader compilation is automated

### Vulkan version used 
```
1.1
```
