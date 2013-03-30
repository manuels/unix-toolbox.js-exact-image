#!/usr/bin/lua

-- ExactImage Lua Example
-- Copyright (C) 2008 - 2010 Rene Rebe, ExactCODE GmbH


package.cpath = "./objdir/api/lua/?.so;" .. package.cpath

require "ExactImage"

-- create an ExactImage
image = ExactImage.newImage ()

-- easy use, use on-disc files:

if ExactImage.decodeImageFile (image, "testsuite/tif/4.2.04.tif")
then
	print "image decoded all fine."
else
	print "something went wrong ..."
	os.exit(1)
end

if ExactImage.encodeImageFile (image, "test.jpg", 80, "")
then
        print "image written all fine."
else
        print "something went wrong writing the image ..."
        os.exit(1)
end

-- advanced use, use in memory locations
f = io.open ("testsuite/tif/4.2.04.tif")
image_bits = f:read("*all")
if ExactImage.decodeImage (image, image_bits)
then
        print "image read from RAM."
else
        print "something went wrong decoding the RAM ..."
        os.exit(1)
end
f:close()

-- image properties

print ("Width: " .. ExactImage.imageWidth (image))
print ("Height: " .. ExactImage.imageHeight (image))
print ("Xres: " .. ExactImage.imageXres (image))
print ("Yres: " .. ExactImage.imageYres (image))

print ("Channels: " .. ExactImage.imageChannels (image))
print ("Channel depth: " .. ExactImage.imageChannelDepth (image))

-- setable as well

ExactImage.imageSetXres (image, 144);
ExactImage.imageSetYres (image, 144);

print ("Xres: " .. ExactImage.imageXres (image))
print ("Yres: " .. ExactImage.imageYres (image))

-- image data manipulation
--[[
ExactImage.imageRotate (image, 90)
ExactImage.imageScale (image, 4)
ExactImage.imageBoxScale (image, .5) ]]--

for y = 0, ExactImage.imageHeight(image) - 1 do
   for x = 0, ExactImage.imageWidth(image) - 1 do
      local r, g, b, a = ExactImage.get(image, x, y)
      ExactImage.set(image, x, y, 1-r, 1-g, 1-b, 1-a) -- x / 256, y / 256, x*y / 256, 1)
   end
end

image_bits = ExactImage.encodeImage (image, "jpeg", 80, "")
print ("size: " .. string.len(image_bits))

if string.len(image_bits) > 0
then
        print "image encoded all fine."
else
        print "something went wrong encoding the image into RAM ..."
        os.exit(1)
end

-- write the file to disc natively
f = io.open ("lua.jpg", "w");
f:write (image_bits)
f:close ()

-- complex all-in-one function
if ExactImage.decodeImageFile (image, "testsuite/tif/4.2.04.tif")
then
    image_copy = ExactImage.copyImage (image);

    ExactImage.imageOptimize2BW (image, 0, 0, 170, 3, 2.1);
    ExactImage.encodeImageFile (image, "optimize.tif", 0, "");

    is_empty = ExactImage.imageIsEmpty (image, 0.02, 16);
    if is_empty then
      print "Image is empty"
    else
      print "Image is not empty, too many pixels ...\n";
    end

    -- the image is bw, now - but we still have a copy
    ExactImage.encodeImageFile (image_copy, "copy.tif", 0, "");
    -- and do not forget the free the copy, otherwise it is leaked
    ExactImage.deleteImage (image_copy);
else
    printf "Error loading testsuite/deskew/01.tif"
end

if ExactImage.decodeImageFile (image, "testsuite/empty-page/empty.tif")
then
    is_empty = ExactImage.imageIsEmpty (image, 0.02, 16);
    if is_empty then
      print "Image is empty";
    else
      print "Image is not empty, too many pixels ...\n";
    end
else
    print "Error loading testsuite/empty-page/empty.tif"
end

-- barcode decoding
--while (<Scan-001-4.tif>)
    f = "testsuite/barcodes/Scan-001-4.tif"
    print ("looking for barcodes in " .. f)
    
    if ExactImage.decodeImageFile (image, f) then
	barcodes =
          ExactImage.imageDecodeBarcodes (image,
	  				  "code39|CODE128|CODE25|EAN13|EAN8|UPCA|UPCE",
					  3, -- min length
					  10) -- max length
	print ("type: " .. type(barcodes))

          --for (my $i;$i< scalar(@$barcodes);$i+=2) {
          --  print "@$barcodes[$i] @$barcodes[$i+1]\n";
          --}
    else
	print ("Error loading " .. f)
    end
--end

-- we do not want to leak memory, always delete the image
-- when you are done with it!
ExactImage.deleteImage (image);

print "ok, here the example ends (for now) ..."
