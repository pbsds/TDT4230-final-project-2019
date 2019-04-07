# What i learned about the methods in terms of advantages, limitations, and how to use it effectively {#sec:learn}

Post-processing is a great tool, but it adds complexity to the rendering pipeline. Debugging issues with the framebuffer isn't easy. It does have the advantage allowing me to skew the window along a sinus curve should i want to.

Post-processing also a cost-saving measure in terms of performance. It can allow me to only compute some value only once per pixel instead of once per fragment which are privy to cover one another. The grain and vignette effect are both possible to implement in the scene shader, doing it in the post processing step spares computation.

The method i used to render transparent objects works *okay*, as described in @sec:trans, but it does have consequences for the post-processing step later in the pipeline. I now have an incomplete depth buffer to work with, as seen in @fig:img-depth-map, where no grass or leaves show up. This makes adding a fog effect in post create weird artifacts. Fog can however be done in the fragment shader for the scene anyway, with only a slight performance penalty due to overlapping fragments.

One other weakness with the way i render transparent objects is that transparent meshes which cut into each other will be render incorrectly. The whole mesh is sorted and rendered, not each face. If i had two transparent ice cubes inside one another *(kinda like a Venn diagram)* then one cube would be rendered on top of the other one. This doesn't matter for grass, but more complex and central objects in the scene may suffer from this.
