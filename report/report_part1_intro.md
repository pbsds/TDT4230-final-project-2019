% TDT4230 - Final assignment
% Peder Berbebakken Sundt
% insert date here

\newpage

```{.shebang im_out="stdout"}
#!/usr/bin/env bash
printf "time for some intricate graphics surgery!\n" | cowsay -f surgery | head -n -1 | sed -e "s/^/              /"
```

# the task

todo

# Getting started

First I had to clean out all the cruft from the glowbox scene in the program. while at it, I removed a lot of the unneccesary code and made a lot of helpers which keeps track of VAO IDs buffer IDs. Now, these are generated and cached automatically causing me a lot less of hassle. 

I then went though and added the ability to use multiple shaders. The each sceneNode can specify a shader.
If the shader is a `nullptr`, then it is inherited from the parent.

When changing shader I had to make sure all the uniforms are passed properly. This was done through the use of a few helper structs and great miss-use of the c++ preprocessor to make all the uniforms named. Now the program simply uses the uniform name instead of hardcoding all the location numbers.

When support for multiple shaders was in place, I revamped the camera transforms to make sure I'm able to move the camera intuitively should I so choose to do so.
This was done with the help of `glm::lookAt()`. While at it I changed the coordinate system to make Z point skyward, the only sensible way.
