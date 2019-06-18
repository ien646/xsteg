

 ## xsteg - Image Steganography Tool

### What is xsteg?
It's an image-based steganography tool.

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


## Command-Line Tool Usage

### Encoding modes:
    '-e':  Encode
    '-d':  Decode
    '-m':  Diff-map
    '-vd': Generate visual-data maps
    '-gk': Generate thresholds key

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
```
`-ii`: Input image file-path

`-oi`: Output image file-path (encoding, exclusively png format)

`-of`: Output file-path (decoding)

`-x` : Direct text-data input (encoding, not-recommended)

`-df`: Input data file (encoding)

`-rk`: Restore thresholds from key-string

### Command examples:

_Encode a text file, using pixels with color saturation higher than 50% (0.5), 1 bit per color channel._
```
xsteg -e -t SATURATION UP 1110 0.5 -ii image.jpg -oi image.encoded.png -df text.txt
```

_Decode contents of an image with encoded data within:_
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

