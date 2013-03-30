
inline void ycbcr_to_rgb( int ycbcr[3], int* const ret )
{
	int dummy[3] = {
		(int)((1/1.772) * (ycbcr[0] + 1.402 * ycbcr[2])),
		(int)((1/1.772) * (ycbcr[0] - 0.34413 * ycbcr[1] - 0.71414 * ycbcr[2])),
		(int)((1/1.772) * (ycbcr[0] + 1.772 * ycbcr[1])) };
	for( int k = 0; k < 3; ++k )
		ret[k] = dummy[k];
}
