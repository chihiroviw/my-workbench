Radiosity Using Hemicubes: A Rasterization Approach
===================================================
#### Sabin Timalsena


This project applies global illumination to a scene using Radiosity. The form factor calcuation for the scene is done by hardware-based z-buffered hemi-cube approach.

Scene Geometry
--------------
All of the geometry was modeled in Blender and exported as an .obj file consisting entirely of triangles. This approach enabled me to experiment with different levels of subdivision and also gave me precise control over the triangulation of objects. The scene used in this project has 350 triangles.

Computing Form Factors
----------------------
The Hemicube method was used to calculate the form factors. For each face, all other faces were projected onto the sides of a unit hemicube, and the ratio of the projected area to the area of the hemicube was taken as the form factor. In order to compute the projected area, all faces were rasterized onto the sides of the hemicube by setting proper OpenGL projection and viewing parameters, and each face was rasterized with a  unique color. Then the contents of the framebuffer were read and pixels corresponding to each projected face were counted. By dividing this count by the total number of pixels on the five faces of the hemicube, form factors were obtained.

Optimal Hemicube Resolution
---------------------------
I first projected the scene onto the sides of hemicube of length 64 pixels (on the long edges). Upon inspecting the result, I found that if the hemicube is far from the windows, the image of the windows on the hemicube didn't look very good; they mapped to just a few pixels. The same was true for other facets of comparable sizes, although the result looked good for larger facets. 

Upon projecting onto a hemicube of length 128 pixels, the results looked pretty usable even for large distances between facets. I tried calculating radiosities using hemicubes of sides 128 and 256 pixels respectively, and didn't find a noticeable improvement in the rendered scene. Thus the optimal hemicube resolution was chosen to be 128 pixels (the length of the longest edge), since computation was significantly faster when compared to a resolution of 256 pixels.
There are three advantages to using this method:
* We make use of OpenGL's optimized pipeline for doing projections, clipping and rasterization
* This method handles occlusion among faces, since we utilize OpenGL's depth buffer
* All objects in the scene are projected onto a side of the hemicube in one draw call
The color buffer used was 24 bits, thus this method supports rasterizing upto 2^24 different faces in one batch, since we can have that many unique colors.

Radiosity Computation
---------------------
Once the form factors were computed, the radiosities for all faces were computed by applying the Gauss-Seidel relaxation on the radiosity equations. Surprisingly, for this scene, the process seemed to “converge” after just 3 iterations, since changes in illumination were very subtle after that, and practically no visible change after about 10 iterations. However, I let the program run upto 50 iterations in the background while still allowing the user to interact with the scene/quit.