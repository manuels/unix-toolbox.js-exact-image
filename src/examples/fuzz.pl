# ExactImage Perl Example
# Copyright (C) 2008 Rene Rebe, ExactCODE GmbH

use strict;

# the ExactImage module
use lib './objdir/api/perl';

use Math::Trig;
use ExactImage;

# create an ExactImage
my $image = ExactImage::newImage ();

# easy use, use on-disc files:

if (ExactImage::decodeImageFile ($image, "testsuite/tif/4.2.04.tif"))
{
	print "image decoded all fine.\n";
} else {
	print "something went wrong ...\n";
	exit;
}

my $width = ExactImage::imageWidth($image);
my $height = ExactImage::imageHeight($image);

my $i = 0;

# TODO: a loop over all common color spaces
# ExactImage::imageConvertColorspace($image, "gray1");

for (my $n = 100; $n > 0; $n--, $i++)
{
  my $x = int rand(2 * $width) - $width;
  my $y = int rand(2 * $height) - $height;
  my $w = int rand($width);
  my $h = int rand($height);
  my $a = rand(2*pi) - pi;
  
  print ("crop: $x $y $w $h $a\n");
  
  my $newimage = ExactImage::copyImageCropRotate($image, $x, $y, $w, $h, $a);
  
  if (ExactImage::encodeImageFile ($newimage, "fuzz-out-$i.pnm"))
    {
      print "image written all fine.\n";
    } else {
      print "something went wrong writing the image ...\n";
      exit;
    }
  ExactImage::deleteImage ($newimage);
}


# we do not want to leak memory, always delete the image
# when you are done with it!
ExactImage::deleteImage ($image);

print "ok, here the example ends (for now) ...\n";
