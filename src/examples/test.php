#!/usr/bin/php -d extension_dir=./objdir/api/php
<?php

# ExactImage PHP Example
# Copyright (C) 2008 - 2010 Rene Rebe, ExactCODE GmbH

ini_set("include_path", "./objdir/api/php/");
//ini_set("extension_dir", "./objdir/api/php/"); // does not work here

// load the module
include("ExactImage.php");

$image = newImage();

if (decodeImageFile ($image, "testsuite/tif/4.2.04.tif"))
{
    print "image decoded all fine.\n";
}
else {
    print "something went wrong ...\n";
    exit;
}

if (encodeImageFile ($image, "test.jpg", 80, ""))
{
    print "image written all fine.\n";
} else {
    print "something went wrong writing the image ...\n";
    exit;
}

# advanced use, use in memory locations
$image_bits=`cat testsuite/tif/5.1.13.tif`;

if (decodeImage ($image, $image_bits))
{
        print "image read from RAM.\n";
} else {
        print "something went wrong decoding the RAM\n";
        exit;
}

# image properties

print "Width: " . imageWidth ($image) . "\n";
print "Height: " . imageHeight ($image) . "\n";
print "Xres: " . imageXres ($image) . "\n";
print "Yres: " . imageYres ($image) . "\n";

print "Channels: " . imageChannels ($image) . "\n";
print "Channel depth: " . imageChannelDepth ($image). "\n";

# setable as well

imageSetXres ($image, 144);
imageSetYres ($image, 144);

print "Xres: " . imageXres ($image) . "\n";
print "Yres: " . imageYres ($image) . "\n";


deleteImage($image);

?>
