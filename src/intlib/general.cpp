#include <general.h>

float intlib::ntohf(const float f)
{
	float retVal;
	char *in = (char*)&f;
	char *out = (char*)&retVal;

	// swap the bytes into a temporary buffer
	out[0] = in[3];
	out[1] = in[2];
	out[2] = in[1];
	out[3] = in[0];

	return retVal;
}
