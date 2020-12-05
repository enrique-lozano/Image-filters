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

void filtroGauss(string ficheroBmp, string outFile, string mode){
    FILE* f;
    cout << "filtrando: " << ficheroBmp.c_str() << endl;

    if(fopen(ficheroBmp.c_str(), "rb") == nullptr){
        cout << "File could not be opened" << endl;
    }
    f = fopen(ficheroBmp.c_str(),"rb");

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

    int cerosFila = (imageSize - width*height*3)/height;

    unsigned char *demas = new unsigned char[startInfo-54];
    read = fread(demas, sizeof(unsigned char), startInfo-54, f);
    delete(demas);

    unsigned char *img = new unsigned char[imageSize];
    read = fread(img, sizeof(unsigned char), imageSize, f);
    if (read != imageSize){
        cout << "Read error" << endl;
    }

    fclose(f);

    //NUEVA CABECERA
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
    header[40] = 0;
    header[41] = 0;
    header[44] = 0;
    header[45] = 0;

    unsigned char **byte1 = new unsigned char *[height+4];
    unsigned char **byte2 = new unsigned char *[height+4];
    unsigned char **byte3 = new unsigned char *[height+4];
    for(int i=0; i<(height+4);i++){
        byte1[i] = new unsigned char[width+4];
        byte2[i] = new unsigned char[width+4];
        byte3[i] = new unsigned char[width+4];
    }

    int indice = 0;
    for(int i= 2; i < height+2;i++){
        if (i>2){
            indice = indice + cerosFila;
        }
        for(int j=2; j<width+2; j++){
            *(*(byte1+i)+j) = img[indice];
            *(*(byte2+i)+j) = img[indice+1];
            *(*(byte3+i)+j) = img[indice+2];
            indice = indice+3;
        }
    }

    unsigned char **g1 = new unsigned char *[height+4];
    unsigned char **g2 = new unsigned char *[height+4];
    unsigned char **g3 = new unsigned char *[height+4];
    for(int i=0; i<(height+4);i++){
        g1[i] = new unsigned char[width+4];
        g2[i] = new unsigned char[width+4];
        g3[i] = new unsigned char[width+4];
    }

    if(mode.compare("gauss")==0 || mode.compare("sobel")==0){
        
        //GAUSS
        int masc[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
        int w = 273;

        float a = 0, b=0, c=0;

        for(int i=1; i<height+1; i++){
            for(int j=1; j<width+1; j++){
                for(int s=0; s<5; s++){
                    for(int t=0; t<5; t++){
                        a+=(masc[s][t]) * (*(*(byte1 + i + s - 1) + (j+t-1)));
                        b+=(masc[s][t]) * (*(*(byte2 + i + s - 1) + (j+t-1)));
                        c+=(masc[s][t]) * (*(*(byte3 + i + s - 1) + (j+t-1)));
                    }
                }
                a = a / w;
                b = b / w;
                c = c / w;
                *(*(g1+i)+j) = (unsigned char) (a);
                *(*(g2+i)+j) = (unsigned char) (b);
                *(*(g3+i)+j) = (unsigned char) (c);
                a = b = c = 0;
            }
        }

        if (mode.compare("sobel")==0){
            //SOBEL
            cout << "doing sobel" << endl;
        }
    }

    indice = 0;
    for(int i= 0; i < height;i++){
        if (i>0){
            indice = indice + cerosFila;
        }
        for(int j=0; j<width; j++){
            img[indice] = *(*(g1+i+1)+(j+1));
            img[indice + 1] = *(*(g2+i+1)+(j+1));
            img[indice + 2] = *(*(g3+i+1)+(j+1));

            indice = indice+3;
        }
    }

    FILE* output = fopen(outFile.c_str(), "wb");
    fseek(output, 0, SEEK_SET);

    fwrite(header,sizeof(unsigned char), 54, output);
    fwrite(img, sizeof(unsigned char), imageSize, output);

    delete[] img;
    fclose(output);

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

    string dir_in_str = args[2];
    string dir_out_str = args[3];
    string filePathOrigin = dir_in_str + "/";
    for (const auto &entry : fs::directory_iterator(filePathOrigin)){
        filePathOrigin = entry.path().string();
        cout << "File: " << filePathOrigin << endl;
        int pos = filePathOrigin.find("/");
        string filePathDestiny = dir_out_str + "/" + filePathOrigin.substr(pos+1);
        cout << "File Destiny: " << filePathDestiny << endl;

        filtroGauss(filePathOrigin, filePathDestiny, "gauss");
    }
    

    return 0;
}