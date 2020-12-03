#include <dirent.h>
#include <iostream>
#include <cstring>
#include <filesystem>
#include <functional>
#include <omp.h>
using namespace std;
using namespace std::chrono;
using clk = chrono::high_resolution_clock;

unsigned char * gauss(unsigned char * data, int width, int height){
    int w = 273; 
    if (height==0 || width==0){
        cout << "Error, heigth or width of the image is 0";
    }
    int mascara[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}};
    #pragma omp parallel
    {
    #pragma omp for
    for (int i=0; i<height; i++){
        for (int j=0; j<width; j++){
            
            int valueBlue = 0, valueGreen = 0, valueRed=0; // Variables que contendran el nuevo color de cada posición
            
            for (int s=0; s<=4; s++){
                for (int t=0; t<=4; t++){
                    // Cogemos el dato de cada color en cada pixel. Si esta fuera de la imagen ese color sera 0
                    int pixelDataBlue, pixelDataGreen, pixelDataRed;
                    if ((i-2+s)>=height || (j-2+t)>=width || (i-2+s)<0 || (j-2+t)<0){
                        pixelDataBlue = 0;
                        pixelDataGreen = 0;
                        pixelDataRed = 0;
                    }else{
                        pixelDataBlue = (int)data[3 * ((i-2+s) * width + (j-2+t))]; 
                        pixelDataGreen = (int)data[3 * ((i-2+s) * width + (j-2+t)) + 1];
                        pixelDataRed = (int)data[3 * ((i-2+s) * width + (j-2+t)) + 2];   
                    }
                    // Cogemos la posición correspondiente a la mascara, para posteriormente aplicar la formula
                    int mascData = mascara[s][t];
                    valueBlue = mascData*pixelDataBlue + valueBlue;    
                    valueGreen = mascData*pixelDataGreen + valueGreen;   
                    valueRed = mascData*pixelDataRed + valueRed;            
                }
            }
            valueBlue = valueBlue / w;
            valueGreen = valueGreen / w;
            valueRed = valueRed / w;
            // Asignamos los nuevos colores a la posición de la imagen en la que nos encontramos
            data[3 * (i * width + j)] = (unsigned char) valueBlue;
            data[3 * (i * width + j) + 1] = (unsigned char) valueGreen;
            data[3 * (i * width + j) + 2] = (unsigned char) valueRed;
        }    
    }}
    return data;
}

unsigned char * sobel(unsigned char * data, int width, int height){
    int w = 8;
    if (height==0 || width==0){
        cout << "Error, heigth or width of the image is 0";
    }
    int m_x[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
    int m_y[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
    
    #pragma omp parallel
    {
    #pragma omp for
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            
            // Variables que contendran el nuevo color de cada posición:
            int valueBlueX = 0, valueGreenX = 0, valueRedX =0;
            int valueBlueY = 0, valueGreenY = 0, valueRedY =0;
            
            for (int s=0; s<=2 ; s++) {
                for (int t=0; t<=2 ; t++) {
                    // Cogemos el dato de cada color en cada pixel. Si esta fuera de la imagen ese color sera 0
                    int pixelDataBlue, pixelDataGreen, pixelDataRed;
                    if ((i-1+s)>=height || (j-1+t)>=width || (i-1+s)<0 || (j-1+t)<0){
                        pixelDataBlue = 0;
                        pixelDataGreen = 0;
                        pixelDataRed = 0;
                    }else{
                        pixelDataBlue = (int)data[3 * ((i-1+s) * width + (j-1+t))]; 
                        pixelDataGreen = (int)data[3 * ((i-1+s) * width + (j-1+t)) + 1];
                        pixelDataRed = (int)data[3 * ((i-1+s) * width + (j-1+t)) + 2];  
                    }
                    // Cogemos la posición correspondiente a la mascara, para posteriormente aplicar la formula
                    int mascData_x = m_x[s][t];
                    int mascData_y = m_y[s][t];
                    
                    valueBlueX = mascData_x*pixelDataBlue + valueBlueX;
                    valueGreenX = mascData_x*pixelDataGreen + valueGreenX;
                    valueRedX = mascData_x*pixelDataRed + valueRedX;

                    valueBlueY = mascData_y*pixelDataBlue + valueBlueY;
                    valueGreenY = mascData_y*pixelDataGreen + valueGreenY;
                    valueRedY = mascData_y*pixelDataRed + valueRedY;
                }
            }

            int valueBlue = (abs(valueBlueX)+abs(valueBlueY)) / w;
            int valueGreen = (abs(valueGreenX)+abs(valueGreenY)) / w;
            int valueRed = (abs(valueRedX)+abs(valueRedY)) / w;
            // Asignamos los nuevos colores a la posición de la imagen en la que nos encontramos
            data[3 * (i * width + j)] = (unsigned char) valueBlue;
            data[3 * (i * width + j) + 1] = (unsigned char) valueGreen;
            data[3 * (i * width + j) + 2] = (unsigned char) valueRed;
        }
    }}
    return data;
}

unsigned char* doOperation(string filename, string exitFile, string operation)
{
    auto t1_Load = clk::now();
    FILE* f = fopen(filename.c_str(), "rb");
    unsigned char info[54];

    // read the 54-byte header
    size_t result = fread(info, sizeof(unsigned char), 54, f);
    if (result<=0){ 
        cerr<<"Error in function fread"<<endl;
    }

    uint16_t nplano = *(int*)&info[26];        // uint16 -> 2 Bytes
    uint16_t spunto = *(int*)&info[28];
    uint32_t vcompresion = *(int*)&info[30];   // uint32 -> 4 Bytes

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
    int width = *(int*)&info[18];
    int height = *(int*)&info[22];
    int startInfo = *(int*)&info[10] - 54;
    int pad = 4 - width%4;

    cout << "Bits sobrantes: " << startInfo << endl;

    // allocate 3 bytes per pixel
    int size = 3 * width * height;
    unsigned char* data = new unsigned char[size];
    unsigned char* dataUntilStart = new unsigned char[size]; 

    // Leemos la cadena situada continuación del header
    result = fread(dataUntilStart, sizeof(unsigned char), startInfo, f); // Bytes extra antes de llegar a la info de los colores
    result = fread(data, sizeof(unsigned char), size, f);   // Data contiene los colores
    if (result<=0){ 
        cerr<<"Error in function fread"<<endl;
    }
/*
    for (int i = 0; i < 200; i++)
    {
        cout << "i=" << i << ": "<<(int)data[i] << ",  ";
    }
    cout << "" << endl;
*/
    // Pixel(i,j) = data[3*(i*width+j)]
    /* //Para imprimir los colores de cada pixel:
    for (int i=0; i<height; i++){
        for (int j=0; j<width; j++){
            cout <<"i->"<< i<<"j--->"<< j<<"\t Blue: "<< (int)data[3 * (i * width + j)] << " Green: " << (int)data[3 * (i * width + j)+1]<< " Red: " << (int)data[3 * (i * width + j) +2]<< endl;
        }
    }
    */

    fclose(f);

    auto t2_Load = clk::now();
    auto diff_Load = duration_cast<microseconds>(t2_Load - t1_Load);
    double secsLoad = (double) (diff_Load.count());
    double totalTime = secsLoad;

    //-----------LOAD ENDED--------------
    //-----------------------------------
    //-------OPERATIONS STARTING---------

    double secsGauss, secsSobel;
    if(strcmp(operation.c_str(), "gauss")==0){
        auto t1_Gauss = clk::now();
        
        data = gauss(data, width, height);
        
        auto t2_Gauss = clk::now();
        auto diff_Gauss = duration_cast<microseconds>(t2_Gauss - t1_Gauss);
        secsGauss = (double) (diff_Gauss.count());
        totalTime = totalTime + secsGauss;
    }
    if(strcmp(operation.c_str(), "sobel")==0){
        auto t1_Gauss = clk::now();
        
        data = gauss(data, width, height);
        
        auto t2_Gauss = clk::now();
        auto diff_Gauss = duration_cast<microseconds>(t2_Gauss - t1_Gauss);
        secsGauss = (double) (diff_Gauss.count());
        
        auto t1_Sobel = clk::now();

        data = sobel(data, width, height);
        
        auto t2_Sobel = clk::now();
        auto diff_Sobel = duration_cast<microseconds>(t2_Sobel - t1_Sobel);
        secsSobel = (double) (diff_Sobel.count());
        totalTime = totalTime + secsGauss + secsSobel;
    }

    auto t1_Store=clk::now();
    
    FILE* newf = fopen(exitFile.c_str(), "w");
    
    result = fwrite(info, sizeof(unsigned char), 54, newf);  //Write header info
    result = fwrite(dataUntilStart, sizeof(unsigned char), startInfo, f);
    result = fwrite(data, sizeof(unsigned char), size, newf);//Write pixels
    if (result<=0){ 
        cerr<<"Error in function fwrite"<<endl;
    }

    auto t2_Store = clk::now();
    auto diff_Store = duration_cast<microseconds>(t2_Store - t1_Store);
    double secsStore = (double) (diff_Store.count());
    totalTime = totalTime + secsStore;

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

    return data;
}

// Función que crea los nombres de los nuevos ficheros a crear.
string exitFileToString(string entryfile){
    string delim = "/";

    auto start = 0U;
    auto end = entryfile.find(delim);
    while (end != string::npos)
    {
        //cout << s.substr(start, end - start) << endl;
        start = end + delim.length();
        end = entryfile.find(delim, start);
    }

    return entryfile.substr(start, end);
}


int main(int nargs, char *args[]){
    omp_set_num_threads(4);

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

    const auto root = filesystem::current_path(); // Directorio actual
    const auto target = root / args[3];   // Directorio salida = "Actual/dir_out"

    string path = args[2]; //Directorio de entrada

    //REALIZAMOS UNA DE LAS TRES OPERACIONES:
    
    if(strcmp(args[1], "copy")==0) { //La operación es copy
        for (const auto &entry : filesystem::directory_iterator(path)) {

            cout << "File: " << entry.path();

            string entryfile = entry.path().string();

            string filename = exitFileToString(entryfile);  // Filename of new file. It will have the same name as entryfile
            string exitFolderName(args[3]);                 // Folder of new file
            const auto exitFile = exitFolderName + "/" + filename;

            doOperation(entryfile, exitFile, "copy");  // Path to string
        }
    }

    else if(strcmp(args[1], "gauss")==0){ //La operación es gauss
        for (const auto &entry : filesystem::directory_iterator(path)) {
            cout << "File: " << entry.path();

            string entryfile = entry.path().string();

            string filename = exitFileToString(entryfile); // Filename of new file. It will have the same name as entryfile
            string exitFolderName(args[3]);                 // Folder of new file
            const auto exitFile = exitFolderName + "/" + filename;

            doOperation(entryfile, exitFile, "gauss");  // Path to string
        }

    }

    else if(strcmp(args[1], "sobel")==0){ //La operación es gauss
        for (const auto &entry : filesystem::directory_iterator(path)) {
            cout << "File: " << entry.path();

            string entryfile = entry.path().string();

            string filename = exitFileToString(entryfile);; // Filename of new file. It will have the same name as entryfile
            string exitFolderName(args[3]);                 // Folder of new file
            const auto exitFile = exitFolderName + "/" + filename;

            doOperation(entryfile, exitFile, "sobel");  // Path to string
        }

    }


    return 0;

}