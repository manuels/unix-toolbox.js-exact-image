
#include "Image.hh"

// quickly counts pixels, returns whether set pixels are below
// threshold and optionally can return the set pixels of all pixels

// if the image is not 1 bit per pixel it will be optimized to b/w
// (if you want more control over that call it yourself before)
// the margin are the border pixels skipped, it must be a multiple
// of 8 for speed reasons and will be rounded down to the next
// multiple of 8 if necessary.

bool detect_empty_page (Image& image, double percent = 0.05, int margin = 8,
			int* set_pixels = 0);
