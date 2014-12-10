3dreconstruct
=============
This project is created just because of my personal interesting in 3D images displaydesigned. It is a QT project. That means it can be opened, modified, built by Qt Creator with needed libs.

Currently I'm working with Qt 5.3 and Qt Creator 3.1 on Windows and Linux. The boost, glm, glew are needed for compilation. For Windows, the boost and the glew should be built by mingw.

Dependencies: OpenGL4.0 and newer versions, [glew](http://glew.sourceforge.net/), [glm](http://glm.g-truc.net/) and [boost](http://www.boost.org/). Click [here]
(http://blog.sina.com.cn/s/blog_54145a610101dfby.html) to find out how to compile boost with mingw on Windows.

Hardware requirements: It is alway good to use best hardware to compile and run this project. At least your computer may have at least 4GB memory and a NVIDIA GT620 with 1GB graphic memory. The alogrithm of surface rendering(marching cubes) will generate lots of intermedia geometry primitives. The display of surface or volume will use graphic memory.

Information: Use my [weibo](http://www.weibo.com/u/1410620001) and [blog](http://blog.sina.com.cn/davyqin) to contact me.

v1.0 is tagged. The surface rendering of marching cubes is finished.

v1.1 is tagged. The volume rendering using glsl with gray scale transfer function is finished.

v1.2 is tagged. The CUDA can be used for surface rendering and edge detection.
