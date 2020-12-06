#include <dirent.h>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <functional>
#include <omp.h>
using namespace std;
using namespace std::chrono;
using clk = chrono::high_resolution_clock;
namespace fs = std::filesystem;

void applyFilter(string inFile, string outFile, string operation){
    auto t1_Load = clk::now();
    FILE* f;

    if(fopen(inFile.c_str(), "rb") == nullptr){
        cout << "File could not be opened" << endl;
    }
    f = fopen(inFile.c_str(),"rb");

    unsigned char header[54];
    int read;
    read = fread(header, sizeof(unsigned char), 54, f);

    uint16_t nplano = *(int*)&header[26];        // uint16 -> 2 Bytes
    uint16_t spunto = *(int*)&header[28];
    uint32_t vcompresion = *(int*)&header[30];   // uint32 -> 4 Bytes

    if(nplano !=1){
        cerr << "Wrong format of BMP image. Number of color planes should be 1." << endl;
    }
    if(spunto!=24){
        cerr << "Wrong format of BMP image. Point size should be 24." << endl;
    }
    if(vcompresion !=0){
        cerr << "Wrong format of BMP image. Compression type should be 0." << endl;
    }

    // Extract image height and width from header
    int width = *(int*)&header[18];
    int height = *(int*)&header[22];
    int startInfo = *(int*)&header[10];
    int imageSize = *(uint32_t*)&header[34];

    int padding = (imageSize - width*height*3)/height;

    unsigned char *infoBeforePixels = new unsigned char[startInfo-54];
    read = fread(infoBeforePixels, sizeof(unsigned char), startInfo-54, f);

    unsigned char *pixelsData = new unsigned char[imageSize];
    read = fread(pixelsData, sizeof(unsigned char), imageSize, f);
    if (read != imageSize){
        cout << "Read error" << endl;
    }

    fclose(f);

    unsigned char **blue_array = new unsigned char *[height+4];
    unsigned char **green_array = new unsigned char *[height+4];
    unsigned char **red_array = new unsigned char *[height+4];
    for(int i=0; i<(height+4);i++){
        blue_array[i] = new unsigned char[width+4];
        green_array[i] = new unsigned char[width+4];
        red_array[i] = new unsigned char[width+4];
    }

    int indice = 0;
    for(int i= 2; i < height+2;i++){
        if (i>2){
            indice = indice + padding;
        }
        for(int j=2; j<width+2; j++){
            *(*(blue_array+i)+j) = pixelsData[indice];
            *(*(green_array+i)+j) = pixelsData[indice+1];
            *(*(red_array+i)+j) = pixelsData[indice+2];
            indice = indice+3;
        }
    }

    auto t2_Load = clk::now();
    auto diff_Load = duration_cast<microseconds>(t2_Load - t1_Load);
    double secsLoad = (double) (diff_Load.count());
    double totalTime = secsLoad;

    /*-------LOAD FINISHED------*/
    
    // Do the filters (if any):

    double secsGauss, secsSobel;
    if(operation.compare("gauss")==0 || operation.compare("sobel")==0){ //----Gauss filter----
        auto t1_Gauss = clk::now();
        
        int w = 273;
        int masc[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
        
        int new_blue = 0, new_green=0, new_red=0;

        for(int i=1; i<height+1; i++){
            for(int j=1; j<width+1; j++){
                for(int s=0; s<=4; s++){
                    for(int t=0; t<=4; t++){
                        new_blue = new_blue + (masc[s][t]) * (*(*(blue_array + i + s - 1) + (j+t-1)));
                        new_green = new_green + (masc[s][t]) * (*(*(green_array + i + s - 1) + (j+t-1)));
                        new_red = new_red + (masc[s][t]) * (*(*(red_array + i + s - 1) + (j+t-1)));
                    }
                }
                new_blue = new_blue / w;
                new_green = new_green / w;
                new_red = new_red/ w;
                *(*(blue_array+i)+j) = (unsigned char) (new_blue);
                *(*(green_array+i)+j) = (unsigned char) (new_green);
                *(*(red_array+i)+j) = (unsigned char) (new_red);
                new_blue = new_green = new_red = 0;
            }
        }

        auto t2_Gauss = clk::now();
        auto diff_Gauss = duration_cast<microseconds>(t2_Gauss - t1_Gauss);
        secsGauss = (double) (diff_Gauss.count());
        totalTime = totalTime + secsGauss;

        if (operation.compare("sobel")==0){
            auto t1_Sobel = clk::now();

            w = 8;
            
            int m_x[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
            int m_y[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};

            int new_blueX = 0, new_greenX=0, new_redX=0;
            int new_blueY = 0, new_greenY=0, new_redY=0;

            for(int i=1; i<height+1; i++){
                for(int j=1; j<width+1; j++){
                    for(int s=0; s<=2; s++){
                        for(int t=0; t<=2; t++){
                            new_blueX = new_blueX + (m_x[s][t]) * (*(*(blue_array + i + s - 1) + (j+t-1)));
                            new_greenX = new_greenX + (m_x[s][t]) * (*(*(green_array + i + s - 1) + (j+t-1)));
                            new_redX = new_redX + (m_x[s][t]) * (*(*(red_array + i + s - 1) + (j+t-1)));

                            new_blueY = new_blueY + (m_y[s][t]) * (*(*(blue_array + i + s - 1) + (j+t-1)));
                            new_greenY = new_greenY + (m_y[s][t]) * (*(*(green_array + i + s - 1) + (j+t-1)));
                            new_redY = new_redY + (m_y[s][t]) * (*(*(red_array + i + s - 1) + (j+t-1)));
                        
                        }
                    }
                    new_blueX = new_blueX / w;
                    new_greenX = new_greenX / w;
                    new_redX = new_redX/ w;

                    new_blueY = new_blueY / w;
                    new_greenY = new_greenY / w;
                    new_redY = new_redY / w;

                    int sumBlue = (abs(new_blueX)+abs(new_blueY)) / w;
                    int sumGreen = (abs(new_greenX)+abs(new_greenY)) / w;
                    int sumRed = (abs(new_redX)+abs(new_redY)) / w;

                    *(*(blue_array+i)+j) = (unsigned char) (sumBlue);
                    *(*(green_array+i)+j) = (unsigned char) (sumGreen);
                    *(*(red_array+i)+j) = (unsigned char) (sumRed);
                    new_blueX = new_greenX = new_redX = 0;
                    new_blueY = new_greenY = new_redY = 0;
                }
            }

            auto t2_Sobel = clk::now();
            auto diff_Sobel = duration_cast<microseconds>(t2_Sobel - t1_Sobel);
            secsSobel = (double) (diff_Sobel.count());
            totalTime = totalTime + secsSobel;
        }
    }

    /*------OPERATIONS AND FILTERS FINISHED------*/

    //Starting to write the new file:
    
    auto t1_Store=clk::now();

    indice = 0;
    for(int i= 0; i < height;i++){ // Write the new colors of the pixels
        if (i>0){
            indice = indice + padding;
        }
        for(int j=0; j<width; j++){
            pixelsData[indice] = *(*(blue_array+i+1)+(j+1));
            pixelsData[indice + 1] = *(*(green_array+i+1)+(j+1));
            pixelsData[indice + 2] = *(*(red_array+i+1)+(j+1));

            indice = indice+3;
        }
    }

    //New header:

    int fileSize = 54 + imageSize;
    header[2] = fileSize%256;
    header[3] = (fileSize/256)%256;
    header[4] = (fileSize/65536)%256;
    header[5] = fileSize/16777216;

    header[6] = (uint32_t) 0;

    //Inicio de datos de la imagen
    header [10] = 54;

    header[14] = 40;
    header[15] = header[16] = header[17] = 0;
    header[11] = header[12] = header[13] = 0;

    //Valores 2835

    header[38] =  19;
    header[39] =  11;
    header[42] = 19;
    header[43] = 11;

    header[40] = 0;
    header[41] = 0;
    header[42] = 0;
    header[43] = 0;
    header[44] = 0;
    header[45] = 0;

    FILE* output = fopen(outFile.c_str(), "wb");
    fseek(output, 0, SEEK_SET);

    fwrite(header,sizeof(unsigned char), 54, output);
    fwrite(pixelsData, sizeof(unsigned char), imageSize, output);
    
    fclose(output);

    auto t2_Store = clk::now();
    auto diff_Store = duration_cast<microseconds>(t2_Store - t1_Store);
    double secsStore = (double) (diff_Store.count());
    totalTime = totalTime + secsStore;

    /*------------WRITE FINISHED-----------*/

    //------------Printing the time----------
    cout << " (time: " << totalTime << ")" << endl;
    cout << " Load time: " << secsLoad << endl;
    if(strcmp(operation.c_str(), "gauss")==0){
        cout << " Gauss: " << secsGauss << endl;
    }else if(strcmp(operation.c_str(), "sobel")==0){
        cout << " Gauss: " << secsGauss << endl;
        cout << " Sobel: " << secsSobel << endl;
    }
    cout << " Store time: " << secsStore << endl;

}


int main(int nargs, char *args[]){
    if(nargs != 4){ //Son necesarios 4 argumentos. P.ej.---> ./image-seq gauss indir outdir
        cerr<<"Wrong format:"<<endl;
        cerr<<"image-seq operation in_path out_path;" <<endl;
        cerr<<"operation: copy, gauss, sobel"<<endl;
        return -1;
    }

    //Direcciones
    DIR *dir_in; dir_in = opendir(args[2]);   // pointer to input directory
    DIR *dir_out; dir_out = opendir(args[3]); //pointer to output directory

    if(strcmp(args[1], "gauss")!=0 && strcmp(args[1], "copy")!=0 && strcmp(args[1], "sobel")!=0){ //Operación inválida
        cerr<<"Unexpected operation: " << args[1] <<endl;
        cerr<<"image-seq operation in_path out_path;" <<endl;
        cerr<<"operation: copy, gauss, sobel"<<endl;
        return -1;
    }

    if(!dir_in){ //No existe directorio de entrada
        cerr<<"Input path: " << args[2] <<endl;
        cerr<<"Output path: " << args[3] <<endl;
        cerr<<"Cannot open directory " <<args[2]<<endl;
        cerr<<"image-seq operation in_path out_path;" <<endl;
        cerr<<"operation: copy, gauss, sobel"<<endl;
        return -1;
    }

    if(!dir_out){ //No existe directorio de salida
        cerr<<"Input path: " << args[2] <<endl;
        cerr<<"Output path: " << args[3] <<endl;
        cerr<<"Output directory " <<args[3] << " does not exist "<<endl;
        cerr<<"image-seq operation in_path out_path;" <<endl;
        cerr<<"operation: copy, gauss, sobel"<<endl;
        return -1;
    }

    cout << "Input path: " << args[2] << endl;
    cout << "Output path: " << args[3] << endl;

    string operation = args[1];
    string dir_in_str = args[2];
    string dir_out_str = args[3];
    string filePathOrigin = dir_in_str + "/";
    for (const auto &entry : fs::directory_iterator(filePathOrigin)){
        filePathOrigin = entry.path().string();
        cout << "File: " << filePathOrigin; 
        int pos = filePathOrigin.find("/");
        string filePathDestiny = dir_out_str + "/" + filePathOrigin.substr(pos+1);

        applyFilter(filePathOrigin, filePathDestiny, operation);
        
    }

    return 0;
}