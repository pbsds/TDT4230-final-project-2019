#include <glm/glm.hpp>
#include "material.hpp"
#include "imageLoader.hpp"

Material Material::apply(const Material& other) const {
	Material out(*this);
	
	if (!other.ignore_diffuse)  out.diffuse_color   = other.diffuse_color;
	if (!other.ignore_emissive) out.emissive_color  = other.emissive_color;
	if (!other.ignore_specular) out.specular_color  = other.specular_color;
	if (!other.ignore_specular) out.shininess       = other.shininess;
	if (!other.ignore_diffuse)  out.ignore_diffuse  = false;
	if (!other.ignore_emissive) out.ignore_emissive = false;
	if (!other.ignore_specular) out.ignore_specular = false;
	if (!other.ignore_backlight)out.ignore_backlight= false;
	if (!other.texture_reset)   out.texture_reset   = true;
	
	if (other.texture_reset || other.diffuse_texture)      out.diffuse_texture      = other.diffuse_texture;
	if (other.texture_reset || other.normal_texture)       out.normal_texture       = other.normal_texture;
	if (other.texture_reset || other.displacement_texture) out.displacement_texture = other.displacement_texture;
	if (other.texture_reset || other.reflection_texture)   out.reflection_texture   = other.reflection_texture;
	if (other.texture_reset || other.reflection_texture)   out.reflexiveness        = other.reflexiveness;
	
	return out;
}


Material Material::diffuse(glm::vec3 color) const {
	Material out(*this);
	out.diffuse_color = color;
	return out;
}
Material Material::specular(glm::vec3 color, float shininess) const {
	Material out(*this);
	out.specular_color = color;
	out.shininess = shininess;
	return out;
}
Material Material::emissive(glm::vec3 color) const {
	Material out(*this);
	out.emissive_color = color;
	return out;
}
Material Material::backlight(glm::vec3 color, float strength) const {
	Material out(*this);
	out.backlight_color = color;
	out.backlight_strength = strength;
	return out;
}
Material Material::textured(PNGImage* diffuse) const {
	Material out(*this);
	out.diffuse_texture = diffuse;
	return out;
}
Material Material::normal_mapped(PNGImage* normal) const {
	Material out(*this);
	out.normal_texture = normal;
	return out;
}
Material Material::diffuse_mapped(PNGImage* diffuse) const {
	Material out(*this);
	out.diffuse_texture = diffuse;
	return out;
}
Material Material::displacement_mapped(PNGImage* displacement) const {
	Material out(*this);
	out.displacement_texture = displacement;
	return out;
}
Material Material::reflection_mapped(PNGImage* reflection, float reflexiveness) const {
	Material out(*this);
	out.reflection_texture = reflection;
	out.reflexiveness = reflexiveness;
	return out;
}
Material Material::no_texture_reset() const {
	Material out(*this);
	out.texture_reset = false;
	return out;
}
Material Material::no_colors() const {
	Material out(*this);
	out.ignore_diffuse = true;
	out.ignore_emissive = true;
	out.ignore_specular = true;
	out.ignore_backlight = true;
	return out;
}
Material Material::no_diffuse() const {
	Material out(*this);
	out.ignore_diffuse = true;
	return out;
}
Material Material::no_emissive() const {
	Material out(*this);
	out.ignore_emissive = true;
	return out;
}
Material Material::no_specular() const {
	Material out(*this);
	out.ignore_specular = true;
	return out;
}
Material Material::no_backlight() const {
	Material out(*this);
	out.ignore_backlight = true;
	return out;
}
Material Material::diffuse_only() const {
	return this->no_emissive().no_specular().no_backlight();
}
Material Material::emissive_only() const {
	return this->no_diffuse().no_specular().no_backlight();
}
Material Material::specular_only() const {
	return this->no_diffuse().no_emissive().no_backlight();
}
Material Material::backlight_only() const {
	return this->no_diffuse().no_emissive().no_specular();
}
