



 # xsteg - Image Steganography Tool

### What is xsteg?
It's an image-based steganography tool.
https://en.wikipedia.org/wiki/Steganography

### What can I do with it?

The main utility of this tool is to encode/hide arbitrary data into the color and alpha channels of an image.
    
You can also establish specific rules for how much data is encoded per color channel, depending on the color data of each individual pixel.
    
For example, you could encode your data using only the blue-ish pixels of an image, leaving the rest untouched.
    
### Why would I want to hide data in an image?

The most obvious use case is to establish a secure communication channel hidden in 'plain sight'.
    
Someone can send or post an image with data encoded on it and, to the unknowing, it looks just like a normal image. But the receiver, who knows the rules used while encoding the data into the image, can decipher the hidden message.
    
This, combined with an external encription algorithm such as AES, can not only prevent 3rd parties from intercepting communications, but also prevent the perception of existence of such communications.

### Won't the resulting image look weird?

As long as you don't go overboard with the encoding rules, the resulting image will be visually identical to the original (especially when using lossy formats such as jpeg).
    
For example, using the last 2 bits of every pixel's color channels (without alpha) (-t AVERAGE_RGB UP 0.0 2220), the color difference of every pixel might vary around ±0.75%, which is virtually indiscernible for the human eye.

### What are these 'encoding rules'?

The encoding rules are defined as 'thresholds'. Every threshold specifies a data-type (color/saturation/luminance/etc.), a threshold value from 0 to 1, a direction (higher or lower than the value) and an rgba bits override mask.
    
The rgba bits override mask is a sequence of 4 characters. Each character represents how many bits (from 0 to 8) of a channel is to be used for every pixel that is over the threshold. The bits used are always the least significant ones, for obvious reasons.
    
For example, a mask to use 1 bit for the red channel, 3 for the green channel, 2 for the blue channel and none for the alpha channel would be represented by the sequence '1320'. 
    
The input image data is truncated to the bits not used to store the data, whenever extracting visual information.

To avoid resetting pixel overrides, any character of the mask can be replaced by an underscore ('_'), which will prevent overriding previous threshold values for pixels that go over multiple thresholds.

### What image formats are supported?
For input image files (i.e. images onto which data will be encoded), supported formats are:
- **jpeg** _(baseline & progressive)_
- **png** _(1/2/4/8/16-bit-per-channel)_
- **tga**
- **bmp**
- **psd** _(composited view only, no extra channels, 8/16 bit-per-channel)_
- **gif**

**For output images, the only supported format is 4-channel png**. Technically, any lossless 3~4 channel image format could be used, as long as the alpha is ignored for 3-channel formats.

## Building

#### Requirements:

- C++17 compatible compiler
- A relatively recent version of CMake
#### How to build:
- **Windows/Linux/MacOS**: 
Run any of the supplied *.bat(Windows) or *.sh(Linux/MacOS) release build scripts.

- **Other platforms**:
Just do a standard CMake build. E.g. _(assuming a POSIX style CLI)_:
	```
	mkdir build
	cd build
	cmake .. -DCMAKE_BUILD_TYPE=Release
	cmake --build . --config Release
	```
- **Tested compilers**:

  - _MSVC 14.1X (Visual Studio 2017)_
  - _GNU-GCC 8.3.0_

## Command-Line Tool Usage

### Encoding modes (pick one):
    '-e':  Encode
    '-d':  Decode
    '-m':  Diff-map
    '-vd': Generate visual-data maps
    '-gk': Generate thresholds key

### Resizing modes (pick one):
    '-ra': Generate resized image (absolute pixel dimensions)
    '-rp': Generate resized image (proportional percentages)
    Both commands require two aditional arguments for specifiying width and height.

### Threshold '-t' specification 

`(-t [0](visual_data)[1](direction)[2](orb_mask)[3](value))`
```
    [0]: Visual data type
        - Available types:
         > COLOR_RED
         > COLOR_GREEN
         > COLOR_BLUE
         > ALPHA
         > AVERAGE_RGB
         > AVERAGE_RGBA
         > SATURATION
         > LUMINANCE

    [1]: Threshold direction
        - UP: Higher values
        - DOWN: Lower values

    [2]: Sequence of channel bits (rgba) available per
    available pixel for encoding
        - e.g. '1120' means:
            > 1 bit on red channel
            > 1 bit on green channel
            > 2 bits on blue channel
            > 0 bits on alpha channel

    [3]: Threshold value from 0.00 to 1.00

### Arguments:

```
`-ii`: Input image path

`-oi`: Output image path (encoding, exclusively png format)

`-if`: Input file path (key restore)

`-of`: Output path (decoding)

`-x` : Direct text-data input (encoding, not-recommended)

`-df`: Input data file (encoding)

`-rk`: Restore thresholds from key-string

`-v` : Verbose mode

`-nomt`: Disable multithreading

### Command examples:

_Encode a text file, using pixels with color saturation higher than 50% (0.5), using 1 bit per color channel, leaving the alpha channel untouched._
```
xsteg -e -t SATURATION UP 1110 0.5 -ii image.jpg -oi image.encoded.png -df text.txt
```

_Decode contents of an image with encoded data within, using the last example's threshold specification:_
```
xsteg -d -t SATURATION UP 1110 0.5 -ii image.encoded.png -of text.decoded.txt
```

_Generate visual data maps for an image:_
```
xsteg -vd -ii image.jpg
```

_Generate 50% luminance diff-map for an image:_
```
xsteg -m -ii image.jpg -t LUMINANCE UP 0000 0.5 -oi image.luminance.50.png
```

_Generate a encoding/deconding key from a list of thresholds (-gk):_
```
xsteg.exe -gk 
    -t SATURATION UP 1110 0.344
    -t COLOR_RED UP 1000 0.5
    -t COLOR_GREEN UP 0100 0.5
    -t COLOR_BLUE UP 0010 0.5
```
```
output: &S>A*1110+0.344&0>A*1000+0.5&1>A*0100+0.5&2>A*0010+0.5
```

_Generate a encoding/deconding key from a list of thresholds and save it to a file (-gk):_
```
xsteg.exe -gk
    -t SATURATION UP 1110 0.344
    -t COLOR_RED UP 1000 0.5
    -t COLOR_GREEN UP 0100 0.5
    -t COLOR_BLUE UP 0010 0.5
    -of 'encodign_key.txt'
```

_Encode a data file into an image, using the previously generated encoding key (-rk):_
```
xsteg -e -ii image.jpg -oi image_encoded.png -df data.txt 
    -rk '&S>A*1110+0.344&0>A*1000+0.5&1>A*0100+0.5&2>A*0010+0.5'
```

_Generate resized image, in absolute pixel dimensions:_
```
xsteg -ra 800 600 -ii image.jpg -oi image_resized.png
```

_Generate resized image, in proportional percentage dimensions:_
```
xsteg -rp 65.5 35.25 -ii image.jpg -oi image_resized.png
```