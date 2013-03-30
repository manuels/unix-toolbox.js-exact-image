# ExactImage Perl Example
# Copyright (C) 2006 - 2010 Rene Rebe, ExactCODE GmbH

use strict;

# the ExactImage module
use lib './objdir/api/perl';

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

if (ExactImage::encodeImageFile ($image, "test.jpg", 80, ""))
{
        print "image written all fine.\n";
} else {
        print "something went wrong writing the image ...\n";
        exit;
}

# advanced use, use in memory locations
my $image_bits=`cat testsuite/tif/5.1.13.tif`;

if (ExactImage::decodeImage ($image, $image_bits))
{
        print "image read from RAM.\n";
} else {
        print "something went wrong decoding the RAM\n";
        exit;
}

# image properties

print "Width: " . ExactImage::imageWidth ($image) . "\n";
print "Height: " . ExactImage::imageHeight ($image) . "\n";
print "Xres: " . ExactImage::imageXres ($image) . "\n";
print "Yres: " . ExactImage::imageYres ($image) . "\n";

print "Channels: " . ExactImage::imageChannels ($image) . "\n";
print "Channel depth: " . ExactImage::imageChannelDepth ($image). "\n";

# setable as well

ExactImage::imageSetXres ($image, 144);
ExactImage::imageSetYres ($image, 144);

print "Xres: " . ExactImage::imageXres ($image) . "\n";
print "Yres: " . ExactImage::imageYres ($image) . "\n";

# image data manipulation
ExactImage::imageRotate ($image, 90);
ExactImage::imageScale ($image, 4);
ExactImage::imageBoxScale ($image, .5);

$image_bits = ExactImage::encodeImage ($image, "jpeg", 80, "");
print "size: " . length($image_bits) . "\n";

if (length($image_bits) > 0)
{
        print "image encoded all fine.\n";
} else {
        print "something went wrong encoding the image into RAM\n";
        exit;
}

# write the file to disc using Perl
open (IMG, ">perl.jpg");
print IMG $image_bits;
close IMG;

# complex all-in-one function
if (ExactImage::decodeImageFile ($image, "testsuite/tif/4.2.04.tif"))
  {
    my $image_copy = ExactImage::copyImage ($image);

    ExactImage::imageOptimize2BW ($image, 0, 0, 170, 3, 2.1);
    ExactImage::encodeImageFile ($image, "optimize.tif", 0, "");

    my $is_empty = ExactImage::imageIsEmpty ($image, 0.02, 16);
    if ($is_empty) {
      print "Image is empty\n";
    } else {
      print "Image is not empty, too many pixels ...\n";
    }

    # the image is bw, now - but we still have a copy
    ExactImage::encodeImageFile ($image_copy, "copy.tif", 0, "");
    # and do not forget the free the copy, otherwise it is leaked
    ExactImage::deleteImage ($image_copy);
  }
else
  {
    printf "Error loading testsuite/deskew/01.tif\n";
  }

if (ExactImage::decodeImageFile ($image, "testsuite/empty-page/empty.tif"))
  {
    my $is_empty = ExactImage::imageIsEmpty ($image, 0.02, 16);
    if ($is_empty) {
      print "Image is empty\n";
    } else {
      print "Image is not empty, too many pixels ...\n";
    }
  }
else
  {
    printf "Error loading testsuite/empty-page/empty.tif\n";
  }

# barcode decoding

while (<testsuite/barcodes/Scan-001-4.tif>)
  {
    printf "looking for barcodes in $_\n";
    
    if (ExactImage::decodeImageFile ($image, "$_"))
      {
	my $barcodes =
          ExactImage::imageDecodeBarcodes ($image,
	  				   "code39|CODE128|CODE25|EAN13|EAN8|UPCA|UPCE",
					   3, # min length
					   10); # max length
          for (my $i;$i< scalar(@$barcodes);$i+=2) {
            print "@$barcodes[$i] @$barcodes[$i+1]\n";
          }
      }
    else
      {
	printf "Error loading $_\n";
      }
  }

# we do not want to leak memory, always delete the image
# when you are done with it!
ExactImage::deleteImage ($image);

print "ok, here the example ends (for now) ...\n";
