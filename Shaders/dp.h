// ########## begin noise.frag #########################
#pragma optionNV(fastmath off)
#pragma optionNV(fastprecision off)

vec2 ds_add (vec2 dsa, vec2 dsb)
{
vec2 dsc;
	float t1, t2, e;
 
	t1 = dsa.x + dsb.x;
	e = t1 - dsa.x;
	t2 = ((dsb.x - e) + (dsa.x - (t1 - e))) + dsa.y + dsb.y;
 
	dsc.x = t1 + t2;
	dsc.y = t2 - (dsc.x - t1);
	return dsc;
}

vec2 ds_mul (vec2 dsa, vec2 dsb)
{
	vec2 dsc;
	float c11, c21, c2, e, t1, t2;
	float a1, a2, b1, b2, cona, conb, split = 8193.0;
 
	cona = dsa.x * split;
	conb = dsb.x * split;
	a1 = cona - (cona - dsa.x);
	b1 = conb - (conb - dsb.x);
	a2 = dsa.x - a1;
	b2 = dsb.x - b1;
 
	c11 = dsa.x * dsb.x;
	c21 = a2 * b2 + (a2 * b1 + (a1 * b2 + (a1 * b1 - c11)));
 
	c2 = dsa.x * dsb.y + dsa.y * dsb.x;
 
	t1 = c11 + c2;
	e = t1 - c11;
	t2 = dsa.y * dsb.y + ((c2 - e) + (c11 - (t1 - e))) + c21;
 
	dsc.x = t1 + t2;
	dsc.y = t2 - (dsc.x - t1);
 
	return dsc;
}

vec2 ds_mul (vec2 dsa, float b)
{
	vec2 dsc;
	float c11, c21, c2, e, t1, t2;
	float a1, a2, b1, b2, cona, conb, split = 8193.0;
 
	cona = dsa.x * split;
	conb = b * split;
	a1 = cona - (cona - dsa.x);
	b1 = conb - (conb - b);
	a2 = dsa.x - a1;
	b2 = b - b1;
 
	c11 = dsa.x * b;
	c21 = a2 * b2 + (a2 * b1 + (a1 * b2 + (a1 * b1 - c11)));
 
	c2 = dsa.y * b;
 
	t1 = c11 + c2;
	e = t1 - c11;
	t2 = ((c2 - e) + (c11 - (t1 - e))) + c21;
 
	dsc.x = t1 + t2;
	dsc.y = t2 - (dsc.x - t1);
 
	return dsc;
}
