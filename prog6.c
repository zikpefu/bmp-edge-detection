/*
   Zachary Ikpefua
   Project 6
   ECE 2220
   Purpose: To read in an image to which we will change that particular image
            using an edge detector and a filter.
   Expectations: This project took 2 days where I thought it would take only
                a few hours
   Known Bugs: NONE
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <string.h>

#define MAX_FILE 200
#define MIN_FILE -200

struct fheader {
        unsigned short int Type; /* Magic identifier */
        unsigned int Size; /* File size in bytes */
        unsigned short int Reserved1, Reserved2;
        unsigned int Offset; /* Offset to data (in B) */
} __attribute__((__packed__)) Fileheader;   /* -- 14 Bytes -- */

struct iheader {
        unsigned int Size; /* Header size in bytes */
        int Width, Height; /* Width / Height of image */
        unsigned short int Planes; /* Number of colour planes */
        unsigned short int Bits; /* Bits per pixel */
        unsigned int Compression; /* Compression type */
        unsigned int ImageSize; /* Image size in bytes */
        int xResolution, yResolution; /* Pixels per meter */
        unsigned int Colors; /* Number of colors */
        unsigned int ImportantColors; /* Important colors */
} __attribute__((__packed__)) Infoheader; /* -- 40 Bytes -- */

struct pixel {
        unsigned char Red, Green, Blue;
};

void createEdge(struct iheader Infoheader,struct pixel **Original,struct pixel **Edge);
void createFilter(struct iheader Infoheader,struct pixel **Original, struct pixel **Filter,int,int,int);

int main(int argc, char* argv[]) {
        FILE *image;
        FILE *outImage;
        struct pixel **Original;
        struct pixel **Edge;
        struct pixel **Filter;
        int i = 0;
        int j = 0;
        int number1 = atoi(argv[5]);
        int number2 = atoi(argv[4]);
        int number3 = atoi(argv[3]);
        char edgeName[50];
        char filterName[50];
        int len = 0;

        strcpy(edgeName,argv[2]);
        strcpy(filterName,argv[2]);
        //User doesnt enter 5 arguments (excluding executable)
        if(argc != 6) {
                fprintf(stderr,"Please enter one input file, one output file and 3 numbers");
                exit(1);
        }
        //User enters number above 200 or lower than -200
        if(atoi(argv[3]) > MAX_FILE || atoi(argv[3]) < MIN_FILE || atoi(argv[4]) > MAX_FILE || atoi(argv[4]) < MIN_FILE || atoi(argv[5]) > MAX_FILE || atoi(argv[5]) <MIN_FILE) {
                fprintf(stderr,"Please enter the numbers between -200 and 200");
                exit(1);
        }

        image = fopen(*(argv + 1),"rb");
        if(image == NULL) {
                fprintf(stderr,"Please input an bmp image:\n");
                exit(1);
        }
        //Read in File header
        fread(&Fileheader,sizeof(struct fheader),1,image);
        if(Fileheader.Type != 0x4D42) {
                fclose(image);
                fprintf(stderr,"Please input an bmp image:\n");
                exit(1);
        }
        //Appedn respective names to file
        len = strlen(edgeName);
        edgeName[len - 4] = '\0';
        strcat(edgeName,"(edge).bmp");
        filterName[len - 4] = '\0';
        strcat(filterName,"(shade).bmp");
        //Read in Infoheader
        fread(&Infoheader,sizeof(struct iheader),1,image);
        fseek(image,Fileheader.Offset,SEEK_SET);
        //Mallocing all 3 different pixel arrays
        Original = (struct pixel**)malloc(sizeof(struct pixel*) * Infoheader.Height);
        for(i = 0; i < Infoheader.Height; i++) {
                Original[i] = (struct pixel*)malloc(sizeof(struct pixel) * Infoheader.Width);
        }

        for(i = 0; i < Infoheader.Height; i++) {
                for(j = 0; j < Infoheader.Width; j++) {
                        fread(&Original[i][j].Red,sizeof(unsigned char),1,image);
                        fread(&Original[i][j].Green,sizeof(unsigned char),1,image);
                        fread(&Original[i][j].Blue,sizeof(unsigned char),1,image);
                }
        }

        Edge = (struct pixel**)malloc(sizeof(struct pixel*) * Infoheader.Height);
        for(i = 0; i < Infoheader.Height; i++) {
                Edge[i] = (struct pixel*)malloc(sizeof(struct pixel) * Infoheader.Width);
        }

        Filter = (struct pixel**)malloc(sizeof(struct pixel*) * Infoheader.Height);
        for(i = 0; i < Infoheader.Height; i++) {
                Filter[i] = (struct pixel*)malloc(sizeof(struct pixel) * Infoheader.Width);
        }

        createEdge(Infoheader,Original,Edge);
        //Write to appended output file
        outImage = fopen(edgeName,"wb");
        fwrite(&Fileheader,sizeof(struct fheader),1,outImage);
        fwrite(&Infoheader,sizeof(struct iheader),1,outImage);
        fseek(outImage,Fileheader.Offset,SEEK_SET);

        for(i = 0; i < Infoheader.Height; i++) {
                for(j = 0; j < Infoheader.Width; j++) {
                        fwrite(&Edge[i][j].Red,sizeof(unsigned char),1,outImage);
                        fwrite(&Edge[i][j].Green,sizeof(unsigned char),1,outImage);
                        fwrite(&Edge[i][j].Blue,sizeof(unsigned char),1,outImage);
                }
        }
        createFilter(Infoheader,Original,Filter,number1,number2,number3);
        outImage = fopen(filterName,"wb");
        fwrite(&Fileheader,sizeof(struct fheader),1,outImage);
        fwrite(&Infoheader,sizeof(struct iheader),1,outImage);
        fseek(outImage,Fileheader.Offset,SEEK_SET);

        for(i = 0; i < Infoheader.Height; i++) {
                for(j = 0; j < Infoheader.Width; j++) {
                        fwrite(&Filter[i][j].Red,sizeof(unsigned char),1,outImage);
                        fwrite(&Filter[i][j].Green,sizeof(unsigned char),1,outImage);
                        fwrite(&Filter[i][j].Blue,sizeof(unsigned char),1,outImage);
                }
        }

//Closing and freeing memory
        free(Original);
        free(Edge);
        free(Filter);
        fclose(image);
        fclose(outImage);

        //Inform User completion
        printf("Images Done! Check the output files!\n");
        return 0;
}
/*
   Function: createEdge
   Inputs:  InfoHeader, Original, Edge
   Outputs: None
   Purpose: This function will increment each pixel by the edge
            detection formula
 */
void createEdge(struct iheader info, struct pixel **pix, struct pixel **edgePtr){

        char redPix = 0;
        char greenPix = 0;
        char bluePix = 0;
        int i = 0;
        int j = 0;
        int r = 0;
        int c = 0;
        int a = 0;
        int b = 0;
        char Matrix[3][3] =
        { { 0, -1, 0 },
          { -1, 4, -1 },
          { 0, -1, 0 }};

        for(i = 1; i < info.Height -1; i++) {
                for(j = 1; j < info.Width - 1; j++) {
                        redPix = 0;
                        greenPix = 0;
                        bluePix = 0;
                  //For loop to calcuate location for original array
                  for(a = 0; a < 3; a++){
                    for(b = 0; b < 3;b++){
                      if(b == 0){
                         c = -1;
                       }
                       else if(b == 1){
                         c = 0;
                       }
                       else{
                         c = 1;
                       }
                        if(a == 0){
                           r = -1;
                        }
                        else if(a == 1){
                            r = 0;
                          }
                        else{
                            r = 1;
                          }
                      redPix += pix[i + r][j + c].Red * Matrix[a][b];
                      greenPix += pix[i + r][j + c].Green * Matrix[a][b];
                      bluePix += pix[i + r][j + c].Blue* Matrix[a][b];
                    }
                  }
                  //Decide and Check to see if the pixel calc is between 0 and 255
                        edgePtr[i][j].Red = redPix;
                        if(edgePtr[i][j].Red > 255) {
                                edgePtr[i][j].Red = 255;
                        }
                        else if(edgePtr[i][j].Red < 0) {
                                edgePtr[i][j].Red = 0;
                        }
                        edgePtr[i][j].Green = greenPix;
                        if(edgePtr[i][j].Green > 255) {
                                edgePtr[i][j].Green = 255;
                        }
                        else if(edgePtr[i][j].Green < 0) {
                                edgePtr[i][j].Green = 0;
                        }
                        edgePtr[i][j].Blue = bluePix;
                        if(edgePtr[i][j].Blue > 255) {
                                edgePtr[i][j].Blue = 255;
                        }
                        else if(edgePtr[i][j].Blue < 0) {
                                edgePtr[i][j].Blue = 0;
                        }
                }
        }
                return;
}
/*
   Function: createFilter
   Inputs:  InfoHeader, Original, Filter, nums: 1, 2, and 3
   Outputs: None
   Purpose: This function will incremtment all pixels and check to
            which pixels are in the range of 0 to 255
 */
void createFilter(struct iheader info,struct pixel **pix,struct pixel **filterPtr,int num1,int num2,int num3){
        int i = 0;
        int j = 0;
        for(i = 0; i < info.Height; i++) {
                for(j = 0; j < info.Width; j++) {
                      //typecasting as int b/c nums are integers
                        if(num1 + (int)pix[i][j].Red >= 255) {
                                filterPtr[i][j].Red = (unsigned char)  255;
                        }
                        else if(num1 + (int)pix[i][j].Red <= 0) {
                                filterPtr[i][j].Red = (unsigned char) 0;
                        }
                        else{
                                filterPtr[i][j].Red = (unsigned char)(num1 + (int)pix[i][j].Red);
                        }

                        if(num2 + (int)pix[i][j].Green >= 255) {
                                filterPtr[i][j].Green = (unsigned char)  255;
                        }
                        else if(num2 + (int)pix[i][j].Green <= 0) {
                                filterPtr[i][j].Green = (unsigned char) 0;
                        }
                        else{
                                filterPtr[i][j].Green = (unsigned char)(num2 + (int)pix[i][j].Green);
                        }

                        if(num3 + (int)pix[i][j].Blue >= 255) {
                                filterPtr[i][j].Blue = (unsigned char)  255;
                        }
                        else if(num3 + (int)pix[i][j].Blue <= 0) {
                                filterPtr[i][j].Blue = (unsigned char) 0;
                        }
                        else{
                                filterPtr[i][j].Blue = (unsigned char)(num3 + (int)pix[i][j].Blue);
                        }
                }
        }
        return;
}
