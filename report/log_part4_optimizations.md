# Optimizations

So at this point I started to look into some optimizations. I tried resizing my window to see if I was fragment bound or not. This didn't make a significant difference. I therefore had to be either vertex bound or bandwidth bound. Being vertexbound but not fragment bound with this little geometry makes little sense, so I started to look into reducing the amount of bandwidth between the cpu and the gpu, and the amount of data the shader uses (since I'm running on an integrated graphics processor using the same ram as the cpu).

After some searching through the code I came over the part where I upload the uniforms for each draw call to gl:

\small
```c++
glUniformMatrix4fv(s->location("MVP")     , 1, GL_FALSE, glm::value_ptr(node->MVP));
glUniformMatrix4fv(s->location("MV")      , 1, GL_FALSE, glm::value_ptr(node->MV));
glUniformMatrix4fv(s->location("MVnormal"), 1, GL_FALSE, glm::value_ptr(node->MVnormal));
glUniform2fv(s->location("uvOffset")      , 1,           glm::value_ptr(node->uvOffset));
glUniform3fv(s->location("diffuse_color") , 1,           glm::value_ptr(node->diffuse_color));
glUniform3fv(s->location("emissive_color"), 1,           glm::value_ptr(node->emissive_color));
glUniform3fv(s->location("specular_color"), 1,           glm::value_ptr(node->specular_color));
glUniform1f( s->location("opacity"),                 node->opacity);
glUniform1f( s->location("shininess"),               node->shininess);
glUniform1f( s->location("reflexiveness"),           node->reflexiveness);
glUniform1f( s->location("displacementCoefficient"), node->displacementCoefficient);
glUniform1ui(s->location("isTextured"),              node->isTextured);
glUniform1ui(s->location("isVertexColored"),         node->isVertexColored);
glUniform1ui(s->location("isNormalMapped"),          node->isNormalMapped);
glUniform1ui(s->location("isDisplacementMapped"),    node->isDisplacementMapped);
glUniform1ui(s->location("isReflectionMapped"),      node->isReflectionMapped);
glUniform1ui(s->location("isIlluminated"),           node->isIlluminated);
glUniform1ui(s->location("isInverted"),              node->isInverted);
```
\normalsize

*Yeah...* I think I could optimize this. The `s->location` function is a uniform name string to location GLint ID lookup. I believe calling GL for this lookup is costly, so I'll cache the results per shader and make the compiler inline the caching lookup function (not possible with the gl functions, since code is dynamically linked, my code on the other hand is subject to compile-time optimizations). The cached function is shown below. The commented line is the old implementation:

```c++
GLint inline Shader::location(std::string const& name) {
	//return  glGetUniformLocation(mProgram, name.c_str());
	auto it = cache.find(name);
	if (it == cache.end()) 
		return cache[name] = glGetUniformLocation(mProgram, name.c_str());
	return it->second;
}
```

This boosted me from 11 FPS up to around 14 FPS, pretty neat!

Next up: avoiding reuploading unchanged information to the shader time and time again for every object in the scene.
Most of the time, objects are pretty similar, only differing in tranformation/position. Most of the uniforms remain unchanged between draw calls.
I replaced all the `glUniformX` methods with these defines shown below, which performs caching for me using static cache variables:

\small
```c++
bool shader_changed = s != prev_s;
#define cache(x) static decltype(node->x) cached_ ## x; \
	if (shader_changed || cached_ ## x != node->x) \
	{ cached_ ## x = node->x;
#define um4fv(x) cache(x) glUniformMatrix4fv(s->location(#x), 1, GL_FALSE, glm::value_ptr(node->x)); }
#define u2fv(x)  cache(x) glUniform2fv( s->location(#x), 1, glm::value_ptr(node->x)); }
#define u3fv(x)  cache(x) glUniform3fv( s->location(#x), 1, glm::value_ptr(node->x)); }
#define u1f(x)   cache(x) glUniform1f(  s->location(#x), node->x); }
#define u1ui(x)  cache(x) glUniform1ui( s->location(#x), node->x); }
```
\normalsize

This is a bigger CPU workload, but it optimizes communication with the graphics processor, which is well suited for high bandwidth streaming but not for smaller back and forth communication.

This caching bumped me up from 14 FPS to 21 on my less-than-ideal integrated graphics chip. The precomputed `shader_changed` bool value alone was responsible for 2 FPS. `s` and `prev_s` are static, making the compiler unable to optimize it with reuse like I do here.

Caching the `glBindTextureUnit` calls done when selecting textures further sped up the rendering by an another 4.3 FPS.

Perhaps restructuring the `renderNode` function to use iteration instead of recursion would be the next logical improvement. I don't need that kind of performance as of now. I believe I'm currently bandwidth bound anyway, since disabling the tangents and bitangents give a 8 FPS improvement. That's integrated graphics for you.
