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
//   strength  Sky::redden. 0 = off (white), 1 = physical, >1 exaggerates
//             (deepens the color and shifts it to higher sun angles)
//
// Shaders that don't set sun_tau/sun_redden get 0 -> white (no change).

// Same 1/lambda^4 ratios as scattering.h (R=680nm, G=550nm, B=440nm)
const vec3 SUN_BETA = vec3(0.428, 1.0, 2.441);

vec3 sunTint(float sinElev, float tau, float strength){
	if(strength<=0.0 || tau<=0.0)
		return vec3(1.0);
	float s=clamp(sinElev,0.0,1.0);
	float elev=degrees(asin(s));
	// Kasten-Young (1989) relative air mass: 1 at zenith, ~38 at horizon
	float am=1.0/(s+0.50572*pow(elev+6.07995,-1.6364));
	vec3 t=exp(-tau*strength*SUN_BETA*(am-1.0)); // relative to zenith
	t/=max(max(t.r,t.g),max(t.b,1e-6));
	return mix(vec3(1.0),t,min(strength,1.0));
}

// sunCoreTint(): color for very bright, direct views of the sun (the disk
// itself, specular glints on water). In a real sunset photo the disk stays
// bright yellow-white even when everything around it is orange - it's so
// much brighter than the sky that it saturates the eye/sensor - so only a
// fraction of the reddening is applied, biased toward yellow.
vec3 sunCoreTint(vec3 tint){
	return vec3(1.0, 0.75+0.25*tint.g, 0.35+0.65*tint.b);
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
	float low=(1.0-smoothstep(0.0,0.4,sunSin))*smoothstep(-0.2,-0.02,sunSin);
	if(low<=0.0)
		return vec4(0.0);
	// sky only: fade out just below the horizon so the band isn't painted
	// over terrain/sea pixels behind the sky shell
	float above=smoothstep(-0.02,0.0,viewSin);
	if(above<=0.0)
		return vec4(0.0);
	float v=max(viewSin,0.0);
	// azimuth spread: strongest toward the sun, still visible ~90 deg away
	float az=pow(0.5+0.5*cosAz,1.5);
	// vertical profile: red-orange band at the horizon + broad warm glow above
	float w=exp(-v/0.08)+0.6*exp(-v/0.3);
	// extra warm glow immediately around the sun
	float glow=pow(max(cosSun,0.0),8.0);
	// color: redder for lower view angles and lower sun
	vec3 c=sunTint(max(sunSin,0.0)+0.03+0.6*v,tau,strength);
	float a=clamp(min(strength,1.0)*low*above*(az*w+0.6*glow),0.0,1.0);
	return vec4(c,a);
}
// ########## end sunlight.h #########################
#endif
