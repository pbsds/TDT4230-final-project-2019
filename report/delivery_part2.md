# How does the implementation achieve its goal?

The final implementation has four types defined:
`SceneNode`, `Mesh`, `PNGImage`,  and `Material`. The `Material` structs references several `PNGImage`s and stores colors and rules on how to be applied to a `SceneNode`. `SceneNode`s reference a `Mesh`, stores all material properties applied to the node, which shader it should rendered with, and a list of child `SceneNode`s.

Each mesh can be UV mapped. Each vertex has a UV coordinate assigned to it, which is passed along with the vertex position into the shaders. Texturing meshes is done by looking up the pixel color from a diffuse texture, using the interpolated UV coordinates. This diffuse color is used as the 'basecolor' in further calculations.

## Normal mapping

Normals are defined in two places: One normal vector per vertex in the mesh, and an optional tangental normal map texture. The normal vector is combined with it's tangent and bitangent vectors (tangents in the U and V directions respectively) into a TBN transformation matrix, which the tangential normal vector fetched from the normal map can be transformed with. This allows us to define the normal vector along the surfaces of the mesh.

## Displacement mapping

Displacement mapping is done in the vertex shader. A displacement texture is mapped into using the UV coordinates. The texture describes how much to offset the vertex along the normal vector. This is further controlled with a displacement coefficient uniform passed to the vertex shader. See @fig:img-fine-plane and @fig:img-displacement-normals.

## Phong lighting

The Phong lighting model is implemented in the fragment shader. The model describes four light components: The diffuse component, the emissive component, the specular component and the ambient component. Each of these components have a color/intensity assigned with them, which is stored in the `SceneNode`/`Material`.
The colors are computed using the normal vector computed as described above. The basecolor is multiplied with sum of the diffuse and the emissive colors, and the specular color is added on top. I chose to combine the ambient and emissive into one single component, since i don't need the distinction in my case. I did however make the small change of multiplying the emissive color with the color of the first light in the scene. This allows me to 'tint' the emissive components.
I have two type of nodes in the scene for lights: point lights and spot lights. Each light has a color associated with them as well as a position and three attenuation factors. The final attenuation is computed as $\frac{1}{x + y\cdot |L| + z\cdot |L|^2}$ from these three factors.

## Loading models

Importing of models is done using the library called `assimp`. It is a huge and bulky library which takes decades to compile, but it gets the job done. Each model file is actually a whole 'scene'. I first traverse the materials defined in this scene and store them into my own material structs. I then traverse the textures in the scene and load them into `PNGImage` structs. I then traverse all the meshes stored in the scene and store the. At last i traverse the nodes in the scene, creating my own nodes. I apply the transformations, materials, textures and meshes referenced. Finally i transform the root node to account for me using a coordinate system where z points skyward.

## Reflections

Reflections are implemented in the fragment shader, using the vector pointing from the camera to the fragment (F), and the normal vector. I reflect the F vector along the normal vector and normalize the result. Computing the dot product between the normalized reflection and any other unit vector gives my the cosine of the angle between the two. Computing this cosine northward and skyward allows me to map the reflection into a sphere and retrieve the UV coordinates used to fetch a reflection color value from a reflection map texture (see fig:img-reflection and fig:img-reflection-map). 


## Fog

TODO

## Rim backlights

To make objects pop a bit more, one can apply a rim backlight color. The effect tries to create a edge/rim/silhouette light around an object: The more the surface normal points away from the camera, the more it lights up. Maximum brightness at 90 degrees away from the camera, and decreases the more it faces the camera. I compute the dot product between the normalized vector from camera to the fragment, and the normal vector, which gives me the cosine value between the two: A value of 1 when pointing away from the camera, 0 when at 90 degrees, and a value of -1 when facing the camera. Adding a "strength" value to this will skew it more towards the camera. Divide it by the same strength value and clamping it will yield the rim light strength (see @fig:img-rim-lights).

## Post processing

Post processing is achieved by rendering the whole scene, not to the window, but to an internal framebuffer instead. This framebuffer is then used as a texture covering a single quad which is then rendered to the window. This in-between step allows me to apply different kinds of effects using the fragment shader, which rely on being able to access neighboring pixel's depth and color values.

### Depth of Field / Blur

Using this post processing shader, I could apply blur to the scene. Depth of field is a selective blur, keeping a certain distance in focus. I first transform the depthbuffer (see @fig:img-depth-map) to be 0 around the point of focus and tend towards 1 otherwise. I then use this focus value as the range of my blur. The blur is simply the average of a selection of neighboring pixels. See @fig:img-depth-of-field for results.

### Chromatic aberration

Light refracts differently depending on wavelength. (see @fig:img-what-is). By scaling the tree color components by different amounts, i can recreate this effect. This scaling is further multiplied by the focus value, to avoid aberration near the vertical line in @fig:img-what-is.

### Vignette

The vignette effect is simply a darkening of the image the further away from the center one is. One can simply use the euclidean distance to compute this. See @fig:img-vingette.

### Noise / Grain

GLSL doesn't have a random number generator built in, but I found one online. I modified it to use the UV vector and a time uniform as its seed. This generator is used to add noise to the image. The nose is multiplied with the focus value for a dramatic effect.
