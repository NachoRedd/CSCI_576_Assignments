# Resampling and Filtering Images

Project Description
- This assignment focuses on practical understanding of image resampling and filtering techniques, specifically how they impact visual media like images and videos. The core task involves creating a program that can display images in RGB format and perform both down-sampling and up-sampling operations to convert input images of varying resolutions to specified standard output formats

Program Functionality
- The program will take a 4:3 aspect ratio image as input, which can be either high-resolution (4000×3000) or low-resolution (400×300). It will then generate an output image in one of the following standard formats:
  - 1920x1080 
  - 1280x720 
  - 640×480 
- Depending on the input size and desired output format, the program will perform either down-sampling or up-sampling.

Resampling Methods
- The program will implement different methods for down-sampling and up-sampling:
  - Down-sampling (when output resolution is lower than input): 
    - Average (or Gaussian) smoothing algorithm.
  - Up-sampling (when output resolution is higher than input): 
    - Cubic interpolation algorithm.
   
Input Parameters
- The program will accept four command-line arguments: 
  1. Filename (string): Path to the input image file (rgb format).
  2. Width (int): Width of the input image in pixels (e.g., 4000 or 400).
  3. Height (int): Height of the input image in pixels (e.g., 3000 or 300).
  4. Output format (string): "01", "02", or "03" corresponding to 1920x1080, 1280x720, or 640x480 respectively.

 Example Invocation:
- MyImageApplication.exe ../hw1_data_rgb/hw1_1_high_res.rgb 4000 3000 O3

Example Outputs:
Downsampling Example from 4000x3000 -> 640x480
<image src = "https://github.com/user-attachments/assets/44f287bd-8c11-468e-8a59-9f773e829365"></image>

Upsampling Example from 400x300 -> 1920x1080
<image src = "https://github.com/user-attachments/assets/c2a88f9a-a4aa-41a2-b921-533621429680"></image>
