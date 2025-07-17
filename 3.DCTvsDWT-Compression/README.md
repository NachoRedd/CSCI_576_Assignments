# Image Compression Comparison

Project Description
- This assignment focuses on understanding image compression by comparing two frequency space representations: the Discrete Cosine Transform (DCT) and the Discrete Wavelet Transform (DWT). The program will read an RGB image (512x512 pixels) and process each color channel independently. It will then generate two output images displayed side-by-side: one reconstructed using DCT coefficients and the other using DWT coefficients.

Input Parameters
- The program will accept two command-line parameters:
1. Input Image File Name: The path to an RGB image file with dimensions 512x512 pixels, similar to previous assignments.
2. Number of Coefficients (n): An integer that defines the number of coefficients to use for decoding.
  - n will be a power of 4, ranging from 4096 to 262144.
  - n = -1 or -2 for progressive analysis

Program Invocation
MyExe Image.rgb 262144
- Uses all coefficients, so the output for both DCT and DWT should be identical to the original image (no loss).
MyExe Image.rgb -1
-  Triggers part 1 of progressive analysis.
MyExe Image.rgb -2
-  Triggers part 2 of progressive analysis.

Implementation Details
Encoding and Decoding
- DCT Conversion: The image data for each channel is broken into 8x8 contiguous blocks (64 pixels each). A DCT is then performed for each block. For a 512x512 image, there will be 4096 (64x64) such blocks.
- <image width = "25%" src = "https://upload.wikimedia.org/wikipedia/commons/2/24/DCT-8x8.png"></image>
- DWT Conversion: For each channel, performed a DWT by converting each row into low-pass and high-pass coefficients pairwise. Subsequently, apply the same process to each column based on the output of the row processing. This process should is recursive, operating on the low-pass section at each iteration.
- <image width = "25%" src = "https://upload.wikimedia.org/wikipedia/commons/thumb/e/e0/Jpeg2000_2-level_wavelet_transform-lichtenstein.png/500px-Jpeg2000_2-level_wavelet_transform-lichtenstein.png"> </image>

Progressive Analysis (for n = -1 and n = -2)
This part involves creating an animation to study the output quality of DCT vs DWT with progressive decoding steps.
Part 1 (n = -1)
- DCT will be decoded with 64 iterations, each iteration increments the coefficient of each block by one starting from 1.
- DWT will be decoded with 10 iterations, each iteration increases the coefficient of each block by a power of 4. 
Part 2 (n = -2)
- Both compression techniques wil increment using the same number of coefficients each iteration.

Example of Progressive Analysis 1
<video src="https://github.com/user-attachments/assets/90633baa-afc9-4444-b74d-cebcd8a2dc2c"></video>


