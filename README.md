# Image-filters 🏙️🏙️
Script to apply some filters to BMP images. Written entirely in c ++.

## Run it🚀
Just clone the project using <code>git clone</code> in the directory where you want to use the project. Once the project is cloned, the most important files you will have will be the "makefile" file and the code itself. In addition, certain sample images are also provided in an additional folder.

To run it you must have an input directory (the one we give as an example is valid) and an output directory (it can be the same as the input directory). Once this is cleared, simply type <code>make</code> in the terminal, in the root directory of the project. If there are no errors, continue entering the following command:
  > ./img-par filter input_directory output_directory
  
**Warning:** _This project is developed and thought to be compiled in a Linux environment. You must have the necessary environment to compile c ++ files (using the g ++ compiler). Other libraries may need installation, especially in case we want to work on windows_

## Posible filters📷
At the moment, only one of the following filters can be used:
- Copy
- Gauss
- Sobel

To use one of these filters, just replace the word "filter" of the command from the previous section with the word of the corresponding filter.
