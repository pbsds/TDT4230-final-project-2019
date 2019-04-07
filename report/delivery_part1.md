% TDT4230Final assignment report
% Peder Bergebakken Sundt
% 7'th of April 2019

\small
```{.shebang im_out="stdout"}
#!/usr/bin/env bash
printf "time for some intricate graphics surgery!\n" | cowsay -f surgery | head -n -5 | sed -e "s/^/                                    /"
```
\normalsize

\newpage

# The project

For this project, we're supposed to investigate a more advanced or complex visualisation method in detail by implementing it ourselves using C++ and OpenGl 4.3+. I'll be doing it by myself.

The idea i have in mind for the scene i want to create, is a field of grass with trees spread about in it, where a car is driving along the ups and downs of the hills. I then plan to throw all the effect i can at it to make it look good.

I want to look more into effects one can apply to a scene of different materials. In detail, i plan to implement:
    Phong lighting,
    texturing,
    normal mapping,
    displacement mapping,
    importing model meshes with transformations and materials from external files,
    reflections,
    fog and
    rim backlights.

I also want to implement some post-processing effects:
    Chromatic aberration,
    Depth of field,
    Vignette and
    Noise / Grain
