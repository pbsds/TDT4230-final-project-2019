# Appendix

![
    The seqmented pane with the cobble texture and normal map and lighting applied to it.
](images/0-base.png){#fig:img-base}

![
    The plane from @fig:img-base with a perlin noise displacement map applied to it
](images/1-perlin-displacement.png){#fig:img-perlin-displacement}

![
    First rendering of the downloaded grass texture and normal map
](images/2-wrong-handedness.png){#fig:img-wrong-handedness}

![
    Rendering of the downloaded grass texture with flipped normal map handedness
](images/3-flipped-handedness.png){#fig:img-flipped-handedness}

![
    The field with grass texture, normal map and displacement map
](images/4-fine-plane.png){#fig:img-fine-plane}

![
    How a mirrored-on-repeat texture behaves
](images/5-gl-mirror.jpg){#fig:img-gl-mirror}


```{.cpp #lst:new-tbn caption="Modified TBN matrix to account for the slope of the displacement"}
if (isDisplacementMapped) {
    float o = texture(displaceTex, UV).r * 2 - 1;
    float u = (texture(displaceTex, UV + vec2(0.0001, 0)).r*2-1 - o) / 0.0004; // magic numbers!
    float v = (texture(displaceTex, UV + vec2(0, 0.0001)).r*2-1 - o) / 0.0004; // magic numbers!
    TBN = mat3(
        normalize(tangent   + normal*u),
        normalize(bitangent + normal*v),
        normalize(cross(tangent + normal*u, bitangent + normal*v))
    );
}
```

![
    The displaced field with the TBN matrix rotated along the slope of the displacement.   
](images/6-displacement-normals.png){#fig:img-displacement-normals}

![
    Car mesh loaded from car model, without transformations
](images/7-car-meshes.png){#fig:img-car-meshes}

![
    Car mesh loaded from car model with transformations applied.
](images/8-car-transformations.png){#fig:img-car-transformations}

![
    Car mesh loaded from car model with transformations applied, rotated to make z point skyward.
](images/9-car-coordinate-system.png){#fig:img-car-coordinate-system}

![
    Diffuse colors loaded from car model
](images/10-car-materials.png){#fig:img-car-materials}

![
    Diffuse, emissive and specular colors loaded from car model
](images/11-material-colors.png){#fig:img-material-colors}

![
    Car model with all colors, with reflection mapping applied.
](images/12-reflection.png){#fig:img-reflection}

![
    The reflection map texture applied to the car
](../res/textures/reflection_field.png){#fig:img-reflection-map}

![
    Tree model loaded from model file, no texture support yet.
](images/13-tree.png){#fig:img-tree}

![
    Tree model loaded from model file, textures applied.
](images/14-tree-alpha.png){#fig:img-tree-alpha}

![
    Tree model with textures, transparent meshes rendered last in sorted order.
](images/15-tree-sorted.png){#fig:img-tree-sorted}

![
    Tree model with textures, transparent meshes rendered last in sorted order, with depthbuffer in read-only mode.
](images/16-tree-depth-readonly.png){#fig:img-tree-depth-readonly}

```{.dot include="images/rendering-pipeline.dot" caption="The scene rendering pipline, handling transparent nodes" #fig:render-pipeline}
```

![
    Grass model loaded, cloned and placed randomly throughout the scene
](images/17-low-fps.png){#fig:img-low-fps}

```{.cpp caption="The node uniforms being uploaded to the shader" #lst:uniform-upload}
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
 
```{.cpp caption="Function for caching the uniform locations per shader. The commented line is the old implementation." #lst:location-cache}
GLint inline Shader::location(std::string const& name) {
    //return  glGetUniformLocation(mProgram, name.c_str());
    auto it = this->cache.find(name);
    if (it == this->cache.end()) 
        return this->cache[name] = glGetUniformLocation(mProgram, name.c_str());
    return it->second;
}
```

```{.cpp caption="The uniform caching defines" #lst:uniform-cache}
bool shader_changed = s != prev_s;
#define cache(x) static decltype(node->x) cached_ ## x; \
    if (shader_changed || cached_ ## x != node->x) { cached_ ## x = node->x;
#define um4fv(x) cache(x) glUniformMatrix4fv(s->location(#x), 1, GL_FALSE, glm::value_ptr(node->x)); }
#define u2fv(x)  cache(x) glUniform2fv( s->location(#x), 1, glm::value_ptr(node->x)); }
#define u3fv(x)  cache(x) glUniform3fv( s->location(#x), 1, glm::value_ptr(node->x)); }
#define u1f(x)   cache(x) glUniform1f(  s->location(#x), node->x); }
#define u1ui(x)  cache(x) glUniform1ui( s->location(#x), node->x); }
```

![
    Car, trees and grass combined into a night driving scene. Two yellow spot lights attached to the head lights, two yelllow point lights attached to the head lights, two red point lights attached to the read lights.
](images/18-night-scene-lights.png){#fig:img-night-scene-lights}

![
    A pink rim backlight applied to the car with strength of 0.3.
](images/19-rim-lights.png){#fig:img-rim-lights}

![
    Visualisation of the transformed depth buffer, transformed into a 'point of focus' buffer. z=0 at the depth of the car, tends toward 1 otherwise.
](images/20-depth-map.png){#fig:img-depth-map}

![
    Depth of field applied to the scene
](images/21-depth-of-field.png){#fig:img-depth-of-field}

![
    The vignette effect, applied to a white frame buffer
](images/22-vingette.png){#fig:img-vingette}

![
    The chromatic aberration effect. F is the point of focus. The transformed depthbuffer is centered around the vertical line crossing F.
](images/23.5-what-is.jpg){#fig:img-what-is}

![
    Chromatic aberration applied to the scene, where the aberration coefficients have been multiplied by 3.
](images/23-chromatic-aberration.png){#fig:img-chromatic-aberration}

![
    Noise/grain applied to the frame buffer.
](images/24-noise.png){#fig:img-noise}

![
    Noise/grain, multiplied by the depthbuffer/point of focus, applied to the frame buffer.
](images/25-all-effects.png){#fig:img-all-effects}

![
    The same scene, during the day. Spotlights have been turned off.
](images/26-day.png){#fig:img-day}

![
    The early-morning scene with some strong fog applied. The code was later changed to have the fog affect the background color as well.
](images/27-fog.png){#fig:img-fog}

```{.dot include="images/effect-order.dot" caption="A high-level graph representing the fragment shader for the scene" #fig:effect-order}
```
