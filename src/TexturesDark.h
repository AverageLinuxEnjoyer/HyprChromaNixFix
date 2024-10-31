#pragma once

#include <string>

#include <hyprland/src/render/shaders/Textures.hpp>


const std::string DARK_MODE_FUNC = R"glsl(
	// Original shader by ikz87

	// Apply opacity changes to pixels similar to one color
	// vec3 color_rgb = vec3(0,0,255); // Color to replace, in rgb format
	float similarity = 0.05; // How many similar colors should be affected.

	float amount = 1.4; // How much similar colors should be changed.
	float target_opacity = 0.73;
	// Change any of the above values to get the result you want

	vec3 chroma[5];
	chroma[0] = vec3(bkg[0]/255.0, bkg[1]/255.0, bkg[2]/255.0);
	chroma[1] = vec3(bkg1[0]/255.0, bkg1[1]/255.0, bkg1[2]/255.0);
	chroma[2] = vec3(bkg2[0]/255.0, bkg2[1]/255.0, bkg2[2]/255.0);
	chroma[3] = vec3(bkg3[0]/255.0, bkg3[1]/255.0, bkg3[2]/255.0);
	chroma[4] = vec3(bkg4[0]/255.0, bkg4[1]/255.0, bkg4[2]/255.0);

	for (int i = 0; i < 5; ++i) {  // Adjust loop count based on max colors
      if (pixColor.x >= chroma[i].x - similarity && pixColor.x <= chroma[i].x + similarity &&
          pixColor.y >= chroma[i].y - similarity && pixColor.y <= chroma[i].y + similarity &&
          pixColor.z >= chroma[i].z - similarity && pixColor.z <= chroma[i].z + similarity &&
          pixColor.w >= 0.99)
      {
          vec3 error = vec3(abs(chroma[i].x - pixColor.x), abs(chroma[i].y - pixColor.y), abs(chroma[i].z - pixColor.z));
          float avg_error = (error.x + error.y + error.z) / 3.0;
          pixColor.w = target_opacity + (1.0 - target_opacity) * avg_error * amount / similarity;
      }
  }
    )glsl";


inline const std::string TEXFRAGSRCRGBA_DARK = R"glsl(
precision mediump float;
varying vec2 v_texcoord; // is in 0-1
uniform sampler2D tex;
uniform float alpha;

uniform vec2 topLeft;
uniform vec2 fullSize;
uniform float radius;

uniform int discardOpaque;
uniform int discardAlpha;
uniform float discardAlphaValue;

uniform int applyTint;
uniform vec3 tint;

uniform vec3 bkg;

void main() {

    vec4 pixColor = texture2D(tex, v_texcoord);

    if (discardOpaque == 1 && pixColor[3] * alpha == 1.0)
	    discard;

    if (discardAlpha == 1 && pixColor[3] <= discardAlphaValue)
        discard;

    if (applyTint == 2) {
	    pixColor[0] = pixColor[0] * tint[0];
	    pixColor[1] = pixColor[1] * tint[1];
	    pixColor[2] = pixColor[2] * tint[2];
    }

	)glsl" + DARK_MODE_FUNC +  R"glsl(

    if (radius > 0.0) {
    )glsl" +
    ROUNDED_SHADER_FUNC("pixColor") + R"glsl(
    }

    gl_FragColor = pixColor * alpha;
})glsl";

inline const std::string TEXFRAGSRCRGBX_DARK = R"glsl(
precision mediump float;
varying vec2 v_texcoord;
uniform sampler2D tex;
uniform float alpha;

uniform vec2 topLeft;
uniform vec2 fullSize;
uniform float radius;

uniform int discardOpaque;
uniform int discardAlpha;
uniform int discardAlphaValue;

uniform int applyTint;
uniform vec3 tint;

uniform vec3 bkg;

void main() {

    if (discardOpaque == 1 && alpha == 1.0)
	discard;

    vec4 pixColor = vec4(texture2D(tex, v_texcoord).rgb, 1.0);

    if (applyTint == 2) {
	pixColor[0] = pixColor[0] * tint[0];
	pixColor[1] = pixColor[1] * tint[1];
	pixColor[2] = pixColor[2] * tint[2];
    }

	)glsl" + DARK_MODE_FUNC +  R"glsl(

    if (radius > 0.0) {
    )glsl" +
    ROUNDED_SHADER_FUNC("pixColor") + R"glsl(
    }

    gl_FragColor = pixColor * alpha;
})glsl";

inline const std::string TEXFRAGSRCEXT_DARK = R"glsl(
#extension GL_OES_EGL_image_external : require

precision mediump float;
varying vec2 v_texcoord;
uniform samplerExternalOES texture0;
uniform float alpha;

uniform vec2 topLeft;
uniform vec2 fullSize;
uniform float radius;

uniform int discardOpaque;
uniform int discardAlpha;
uniform int discardAlphaValue;

uniform int applyTint;
uniform vec3 tint;

uniform vec3 bkg;

void main() {

    vec4 pixColor = texture2D(texture0, v_texcoord);

    if (discardOpaque == 1 && pixColor[3] * alpha == 1.0)
        discard;

    if (applyTint == 2) {
	pixColor[0] = pixColor[0] * tint[0];
	pixColor[1] = pixColor[1] * tint[1];
	pixColor[2] = pixColor[2] * tint[2];
    }

	)glsl" + DARK_MODE_FUNC +  R"glsl(

    if (radius > 0.0) {
    )glsl" +
    ROUNDED_SHADER_FUNC("pixColor") + R"glsl(
    }

    gl_FragColor = pixColor * alpha;
}
)glsl";
