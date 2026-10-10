#ifndef _SUNLIGHT_H
#define _SUNLIGHT_H
// ########## begin sunlight.h #########################
// Atmospheric reddening of direct sunlight (sunrise/sunset color).
//
// Sunlight reaching a point low on the horizon has crossed far more air
// than at noon, and Rayleigh scattering removes blue from that path much
// faster than red (beta ~ 1/lambda^4), so the light that's left goes
// white -> yellow -> orange -> red as the sun drops.
//
// sunTint() returns only the HUE change, relative to the sun overhead:
//  - exactly white when the sun is at the zenith, so midday scenes look
//    the same as before this was added
//  - normalized so its brightest channel is 1; the existing twilight
//    band (twilite_min/max/dph) still handles the dimming
//
//   sinElev   sine of the sun's elevation at the lit point (LdotR in the
//             lighting shaders). <=0 is held at the horizon value.
//   tau       zenith optical depth at green (Sky::sunTau(): ~0.1 for an
//             Earth-like 1 ATM atmosphere, scaled by pressure)
//   strength  Sky::redden. 0 = off (white), 1 = physical (at least
//             Earth-like for thin skies), >1 exaggerates
//             (deepens the color and shifts it to higher sun angles)
//
// Shaders that don't set sun_tau/sun_redden get 0 -> white (no change).
//
// REDDEN COLOR AND SHARPEN (Sky Lighting tab, "Redden" row):
//   redden_color  Sky::redden_color - the color the sunlight turns at the
//                 horizon (strictly: at a path depth of REDDEN_REF, which
//                 is the base of the horizon band for an Earth-like sky at
//                 redden=1). Each channel is extinguished at a rate
//                 -log(c/max(c)), so the light still goes white -> pale ->
//                 full color -> deeper, through the in-between hues, and
//                 any color works (e.g. bluish for a dusty-Mars look). The
//                 default REDDEN_DEF reproduces the 1/lambda^4 Rayleigh
//                 result exactly (SUN_BETA below), so scenes that never set
//                 it look the same as before.
//   redden_grad   Sky::redden_grad ("Sharpen"). 1 = physical. Higher keeps
//                 the effect closer to the horizon and to a lower sun;
//                 lower spreads it higher up the sky and later into the day.
// Both are declared here so every shader that includes this file gets
// them; one that doesn't set them reads 0 and falls back to the defaults.
uniform vec3  redden_color;
uniform float redden_grad;

// Same 1/lambda^4 ratios as scattering.h (R=680nm, G=550nm, B=440nm)
const vec3 SUN_BETA = vec3(0.428, 1.0, 2.441);
// largest zenith depth used for the sunlight COLOR (see sunTint)
const float SUN_TAU_MAX = 0.25;
// path depth (tau*(airmass-1)) at which the light reaches redden_color
const float REDDEN_REF = 2.0;
// exp(-REDDEN_REF*(SUN_BETA-SUN_BETA.r)): the Rayleigh sunset color
const vec3 REDDEN_DEF = vec3(1.0, 0.3185, 0.01785);

// per-channel extinction rate from redden_color (0 for its brightest channel)
vec3 reddenRate(){
	vec3 c=redden_color;
	float m=max(c.r,max(c.g,c.b));
	if(m<=0.0)
		c=REDDEN_DEF;
	else
		c/=m;
	return -log(clamp(c,1.0/255.0,1.0));
}
float reddenGrad(){
	return redden_grad>0.0?redden_grad:1.0;
}

vec3 sunTint(float sinElev, float tau, float strength){
	if(strength<=0.0 || tau<=0.0)
		return vec3(1.0);
	// thin atmospheres (e.g. pressure 0.005 -> tau ~0.0006) give almost no
	// reddening physically, so redden had no visible effect there. Give the
	// path an Earth-like minimum depth scaled by redden, so redden behaves
	// the same on any planet; thicker skies still use their own tau.
	tau=max(tau,0.1*min(strength,1.0));
	// ...and cap it for thick ones. sunTint() is normalized (brightest
	// channel = 1), so it only carries the HUE of the filtered light, not
	// its dimming. On Venus (pressure 92 -> tau ~12) the uncapped hue was
	// pure red until the sun was ~50 deg up, and redden 1 vs 2 flipped the
	// whole sky between olive and solid red. In a real thick atmosphere
	// multiple scattering washes the light out rather than saturating it.
	// 0.25 is ~2.5x Earth, so Earth-like and thin skies are unchanged.
	tau=min(tau,SUN_TAU_MAX);
	// Sharpen: >1 makes the sun act higher than it is, so the color is
	// confined to a lower sun; <1 spreads it to higher sun angles
	float s=clamp(sinElev*reddenGrad(),0.0,1.0);
	float elev=degrees(asin(s));
	// Kasten-Young (1989) relative air mass: 1 at zenith, ~38 at horizon
	float am=1.0/(s+0.50572*pow(elev+6.07995,-1.6364));
	// relative to zenith; with the default color this is exactly
	// exp(-tau*strength*SUN_BETA*(am-1)) normalized to its red channel
	vec3 t=exp(-reddenRate()*(tau*strength*(am-1.0)/REDDEN_REF));
	t/=max(max(t.r,t.g),max(t.b,1e-6));
	return mix(vec3(1.0),t,min(strength,1.0));
}

// sunCoreTint(): color for very bright, direct views of the sun (the disk
// itself, specular glints on water). In a real sunset photo the disk stays
// bright yellow-white even when everything around it is orange - it's so
// much brighter than the sky that it saturates the eye/sensor - so only a
// fraction of the reddening is applied, biased toward yellow.
// Each channel keeps 0..65% of its drop, more for the channels that
// fade fastest; with the default color this is exactly the original
// (1, 0.75+0.25*g, 0.35+0.65*b).
vec3 sunCoreTint(vec3 tint){
	vec3 k=reddenRate();
	float km=max(k.r,max(k.g,k.b));
	vec3 w=km>0.0?0.65*pow(k/km,vec3(0.75)):vec3(0.0);
	return vec3(1.0)-(vec3(1.0)-tint)*w;
}

// sunsetBand(): horizon glow around a low sun. Unlike the round halo, the
// real effect is a band hugging the horizon that spreads far to either
// side of the sun and grades from red-orange at the horizon through
// orange/yellow to the normal sky color above. Light reaching the eye from
// low view angles has crossed the most air, so it is the reddest.
//   viewSin  sine of the view ray elevation at the viewer
//   sunSin   sine of the sun elevation at the viewer
//   cosAz    cosine of the horizontal (azimuth) angle between view and sun
//   cosSun   cosine of the angle between the view ray and the sun
//   tau, strength  as for sunTint()
// returns rgb = glow color, a = mix weight (0..1)
vec4 sunsetBand(float viewSin, float sunSin, float cosAz, float cosSun, float tau, float strength){
	if(strength<=0.0 || tau<=0.0)
		return vec4(0.0);
	// how "sunset" it is: full when the sun is near the horizon, fading as
	// it climbs (~24 deg) and as it sinks well below (~-11 deg)
	// Sharpen (redden_grad) scales how high the sun can be for the band to
	// show, and (below) how high in the sky and how far around it reaches
	float sh=reddenGrad();
	float low=(1.0-smoothstep(0.0,0.4/sh,sunSin))*smoothstep(-0.2,-0.02,sunSin);
	if(low<=0.0)
		return vec4(0.0);
	// sky only: fade out just below the horizon so the band isn't painted
	// over terrain/sea pixels behind the sky shell
	float above=smoothstep(-0.02,0.0,viewSin);
	if(above<=0.0)
		return vec4(0.0);
	float v=max(viewSin,0.0);
	// azimuth spread: strongest toward the sun, still visible ~90 deg away
	float az=pow(0.5+0.5*cosAz,1.5*sh);
	// vertical profile: red-orange band at the horizon + broad warm glow above
	float w=exp(-v*sh/0.08)+0.6*exp(-v*sh/0.3);
	// extra warm glow immediately around the sun
	float glow=pow(max(cosSun,0.0),8.0);
	// color: redder for lower view angles and lower sun
	vec3 c=sunTint(max(sunSin,0.0)+0.03+0.6*v,tau,strength);
	// weight by how colored the band actually is (c is normalized so its
	// brightest channel is 1, so 1-min(c) measures saturation; with the
	// default color that's the blue channel). A thin atmosphere (small tau)
	// gives a near-white tint, and painting that at full weight bleached
	// the sky instead of reddening it. Earth-like tau saturates this to 1.
	float warm=smoothstep(0.0,0.5,1.0-min(c.r,min(c.g,c.b)));
	float a=clamp(min(strength,1.0)*warm*low*above*(az*w+0.6*glow),0.0,1.0);
	return vec4(c,a);
}
// ########## end sunlight.h #########################
#endif
