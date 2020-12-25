# Image-filters 🏙️🏙️
Script to apply some filters to BMP images. Written entirely in c ++.

## Run it🚀
Just clone the project using <code>git clone</code> in the directory where you want to use the project. Once the project is cloned, the most important files you will have will be the "makefile" file and the code itself. In addition, certain sample images are also provided in an additional folder.

To run it you must have an input directory (the one we give as an example is valid) and an output directory (it can be the same as the input directory). Once this is cleared, simply type <code>make</code> in the terminal, in the root directory of the project. If there are no errors, continue entering the following command:
  > ./img-par filter input_directory output_directory
  
If no errors appear, you should see the images from the input directory in the output directory, but with the specified filter applied.

**Warning:** _This project is developed and thought to be compiled in a Linux environment. You must have the necessary environment to compile c ++ files (using the g ++ compiler). Other libraries may need installation, especially in case we want to work on windows_

## Posible filters📷
At the moment, only one of the following filters can be used:
- **Copy**: The equivalent of not using any filter. The files will simply be copied from the input directory to the output directory. For example, when copying the following image of the tiger, we will obtain:
<img src="/Examples_images/tiger.bmp" alt="Tiger image" width="50%" height="50%" />

- **Gauss**: In image processing, a Gaussian blur (also known as Gaussian smoothing) is the result of blurring an image by a Gaussian function. [Duck Duck Go](https://duckduckgo.com)
<img src="/Examples_images/Output_examples/tiger_gauss.bmp" alt="Tiger image" width="50%" height="50%" />

- **Sobel**: The Sobel operator, sometimes called the Sobel–Feldman operator or Sobel filter, is used in image processing and computer vision, particularly within edge detection algorithms where it creates an image emphasising edges.
<img src="/Examples_images/Output_examples/tiger_sobel.bmp" alt="Tiger image" width="50%" height="50%" />

To use one of these filters, just replace the word "filter" of the command from the previous section with the word of the corresponding filter.

## Want to collaborate?🙋🏻
Feel free to try adding new filters, or improving and optimizing existing code. The sobel filter works a bit bad on certain edges of some images.
