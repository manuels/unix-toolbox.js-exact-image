# ExactImage Python Example
# Copyright (C) 2008 - 2010 Rene Rebe, ExactCODE GmbH

import sys
sys.path.append('./objdir/api/python')

import ExactImage
image = ExactImage.newImage()

if ExactImage.decodeImageFile (image, "testsuite/tif/4.2.04.tif"):
    print "image decoded all fine."
else:
    print "something went wrong ..."
    exit

if ExactImage.encodeImageFile (image, "test.jpg", 80, ""):
    print "image written all fine."
else:
    print "something went wrong writing the image ..."
    exit

# advanced use, use in memory locations
f = open("testsuite/tif/5.1.13.tif")
try:
    image_bits = f.read()

finally:
    f.close()

if ExactImage.decodeImage (image, image_bits):
    print "image read from RAM."
else:
    print "something went wrong decoding the RAM\n";
    exit


# image properties

print "Width: ", ExactImage.imageWidth (image)
print "Height: ", ExactImage.imageHeight (image)
print "Xres: ", ExactImage.imageXres (image)
print "Yres: ", ExactImage.imageYres (image)

print "Channels: ", ExactImage.imageChannels (image)
print "Channel depth: ", ExactImage.imageChannelDepth (image)

# setable as well

ExactImage.imageSetXres (image, 144);
ExactImage.imageSetYres (image, 144);

print "Xres: ", ExactImage.imageXres (image)
print "Yres: ", ExactImage.imageYres (image)

# image data manipulation
ExactImage.imageRotate (image, 90);
ExactImage.imageScale (image, 4);
ExactImage.imageBoxScale (image, .5);

image_bits = ExactImage.encodeImage (image, "jpeg", 80, "");
print "size: ", len(image_bits)

f = open("python.jpg", "w")
try:
    image_bits = f.write(image_bits)

finally:
    f.close()


ExactImage.deleteImage(image)
