#pragma once

#include <glm/glm.hpp>
#include "imageLoader.hpp"

struct Material {
	float opacity = 1.0;
	float shininess = 1; // specular
	float reflexiveness = 0;
	float backlight_strength = 0;
	glm::vec3 diffuse_color  = glm::vec3(1.0);
	glm::vec3 emissive_color = glm::vec3(0.5);
	glm::vec3 specular_color = glm::vec3(0.2);
	glm::vec3 backlight_color= glm::vec3(0.0);
	PNGImage* diffuse_texture      = nullptr;
	PNGImage* normal_texture       = nullptr;
	PNGImage* displacement_texture = nullptr;
	PNGImage* reflection_texture   = nullptr;
	
	bool ignore_diffuse  = false;
	bool ignore_emissive = false;
	bool ignore_specular = false;
	bool ignore_backlight= false;
	bool texture_reset = true;
	
	Material apply(const Material& other) const;
	
	Material diffuse(glm::vec3 color) const;
	Material specular(glm::vec3 color, float shininess) const;
	Material emissive(glm::vec3 color) const;
	Material backlight(glm::vec3 color, float strength) const;
	Material textured(PNGImage* diffuse) const;
	Material normal_mapped(PNGImage* normal) const;
	Material diffuse_mapped(PNGImage* diffuse) const;
	Material displacement_mapped(PNGImage* displacement) const;
	Material reflection_mapped(PNGImage* reflection, float reflexiveness) const;
	
	// avoid touching these:
	Material no_texture_reset() const;
	Material no_colors() const; // leave all colors alone
	Material no_diffuse() const;
	Material no_emissive() const;
	Material no_specular() const;
	Material no_backlight() const;
	Material diffuse_only() const; // and not the other three
	Material emissive_only() const; // and not the other three
	Material specular_only() const; // and not the other three
	Material backlight_only() const; // and not the other three
};
