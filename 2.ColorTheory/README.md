# Image and Color Segmentation

Project Description
- This assignment requires the implementation of an image segmentation application. The program will take an RGB image and two hue threshold values (h1 and h2) as input. It will then convert the image from RGB to HSV color space. Pixels whose hue values fall within the specified range (h1 to h2) will retain their original color in the output image, while all other pixels (those outside the threshold) will be converted to grayscale.

Input Parameters
- The program will accept three command-line parameters:
  1. Image Name: The path to an 8-bit per channel RGB image (24 bits per pixel). All images are assumed to be 512×512 pixels.
  2. Hue Threshold 1 (h1): An integer between 0 and 360, representing the first hue threshold for segmentation.
  3. Hue Threshold 2 (h2): An integer between 0 and 360, representing the second hue threshold. This value will always be greater than h1.
 
Example
<image src = "https://github.com/user-attachments/assets/c70d361c-88f8-4802-9ca9-30e3799dd37a" alt = "colorTheory"></image>
