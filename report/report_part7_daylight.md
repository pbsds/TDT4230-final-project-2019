# Day-night cycle

```{.shebang im_out="stdout"}
#!/usr/bin/env bash
echo At this point I\'m only playing around, but it\'s all for that sick-ass demo, am I right? | cowsay -f cheese | sed -e "s/^/          /"
```

Now that all the ground-work is in place, i can start adding timestamped events to happen in the scene:
Camera movement, change of lightning, etc. I started by adding a day-night cycle. I then expanded these timestamps to control the car headlights:

```cpp
struct seq_t { double t; vec3 light_c; vec3 bg_c; bool has_headlights; };
static const vector<seq_t> sequence = {
	{ 0, vec3(0.2 , 0.2 , 0.7), vec3(0.05, 0.1 , 0.15), 1}, // night
	{ 9, vec3(0.4 , 0.4 , 0.8), vec3(0.15, 0.15, 0.35), 1}, // dusk
	{10, vec3(1.0 , 0.6 , 0.4), vec3(0.8 , 0.4 , 0.2 ), 1}, // sunrise
	{11, vec3(0.9 , 0.7 , 0.5), vec3(0.8 , 0.6 , 0.2 ), 1}, // sunrise2
	{12, vec3(0.85, 0.85, 0.9), vec3(0.3 , 0.5 , 0.8 ), 0}, // morning
	{18, vec3(1.0 , 1.0 , 1.0), vec3(0.35, 0.6 , 0.9 ), 0}, // noon
	{24, vec3(0.7 , 0.9 , 1.0), vec3(0.3 , 0.5 , 0.8 ), 0}, // evening
	{25, vec3(0.9 , 0.7 , 0.5), vec3(0.8 , 0.6 , 0.2 ), 0}, // sundown
	{26, vec3(1.0 , 0.6 , 0.4), vec3(0.8 , 0.4 , 0.2 ), 1}, // sunset
	{27, vec3(0.5 , 0.5 , 0.8), vec3(0.35, 0.15, 0.35), 1}, // dusk
	{36, vec3(0.2 , 0.2 , 0.7), vec3(0.05, 0.1 , 0.15), 1}, // night
};
```

Interpolating between these points per frame made it look quite nice:

![](images/26-day.png)

Now where there once again shines light, we can see the post-processing effects that much better!
