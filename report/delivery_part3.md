# Notable problems encountered on the way, and how i solved them

## General difficulties

A lot of time was spent cleaning up and modifying the gloom base project. A lot of time was also spent working with `assimp` and getting the internal framebuffer to render correctly. `assimp` and `OpenGL` aren't the most verbose companion out there.

I learned that the handedness of face culling and normal maps aren't the same everywhere. Luckily `assimp` supports flipping faces. When reading the grass texture, i had to flip the R and G color components of the normal map to make it look right. See @fig:img-wrong-handedness and @fig:img-flipped-handedness.

## The slope of the displacement map

The scrolling field of grass is actually just a static plane mesh of 100x100 vertices, with a perlin noise displacement map applied to it (I use an UV offset uniform to make the field scroll). You can however see in @fig:img-fine-plane that the old normals doesn't mesh with the now displaced geometry. I therefore had to recalculate the normals using the slope of the displacement. I rotate the TBN matrix and normal vectors in the shader to make it behave nice with the lighting. Luckily i have both the tangent and bitangen vector pointing in the U and V direction. calculating the slope of the displacment in both of these directions allows me to add the normal vector times the slope to the tangent and the bitangent. after normalizing the tangens, i can compute the new normal vector using the cross product of the two. From these i construct the TBN matrix. See @lst:new-tbn for the code.

This did however give me a pretty coarse image, so I moved the computation of the TBN matrix from the vertex shader to the fragement shader. This will give me a slight performance penalty, but I can undo the change in a simplified shader should I need the performance boost. See @fig-img-displacement-normals for results.


## Transparent objects {#sec:trans}

When rendering transparent objects with depth testing enabled, we run into issues as seen in @fig:img-tree-alpha. The depth test is simply a comparison against the depth buffer, which determines if a fragment should be rendered or not. When a fragment is rendered, the depth buffer is updated with the depth of the rendered fragment. Only fragment which will appear behind already rendered fragments will be skipped. But non-opaque objects should allow objects behind to still be visible.

As a first step to try to fix this issue, i split the rendering of the scene into two stages: opaque nodes and transparent nodes. The first stage will traverse the scene graph and store all transparent nodes in a list. Afterwards the list is sorted by distance from camera, then rendered back to front. This will ensure that the transparent meshes furthest away are rendered before the ones in front, which won't trip up the depth test. The results of this can be viewed in @fig:img-tree-sorted.

We still have issues here however. Faces within the same mesh aren't sorted and could be rendered in the wrong order. This is visible near the top of the tree in @fig:img-tree-sorted. To fix one could sort all the faces, but this isn't feasible in real time rendering applications. I then had the idea to try to disable the depth test. This look *better* in this case, but it would mean that opaque objects would always be beneath transparent ones, since the transparent ones are rendered in a second pass afterwards.

I then arrived at the solution of setting `glDepthMask(GL_FALSE);`, which makes the depth buffer read only. All writes to the depth buffer is ignored. Using this, the depth buffer created by the opaque objects can be used while rendering the transparent ones, and since the transparent ones are rendered in sorted order, they *kinda* work out as well. See @fig:img-tree-depth-readonly for the result. The new rendering pipeline is visualized in @fig:render-pipeline. 


## Need for optimizations

At some point when i had over 5000 meshes in the scene i noticed a performance drop. I started to look into some optimizations. Resizing the window didn't affect the FPS, so I shouldn't be fragment bound. I assume i'm not vertex bound either, so i had to be bandwidth bound, which makes sense due to my single channel ram and  integrated graphics. Reducing the amount of data sent between the CPU and the GPU was my goal.

After some searching through the code I came over the part where I upload the uniforms for each draw call to gl. (See @lst:uniform-upload)

I first optimized the `s->location()` calls:
It is a lookup from a uniform name string to location `GLint`. Asking GL directly everytime is costly due to the limited bandwith, and the compiler being unable to inline dynamically linked function. Therefore I'll cache the results per shader. See @fig:location-cache for the fix.

Uploading all these uniforms per GL draw call is ineffective as well.
Most of the uniforms don't change between draw calls. I therefore added caching of the uniforms using a bunch of nasty prepossessing tricks and static variables. See @lst:uniform-cache.

The next step for optimization would be to combine meshes with same materials into a single mesh. Most of the grass could be optimized this way. Each bundle of grass consists of 64 nodes the same materials and textures applied. concatenating the meshes would decrease the scene traversal and other rendering overhead significantly.

I could also specialize the shader for each material. I thought of replacing many of the uniforms with defines, and compile a separate shader for each material, but time is a limited resource.
