#include "IHSSharpening.h"
#include <cmath>
#include <algorithm>

// Class for HSVSharpening algorithm

IHSSharpening::IHSSharpening() { };
IHSSharpening::~IHSSharpening() { };

void IHSSharpening::computeRGBNIR(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
{
    GDALRasterBand* fBand = sourceImages[0]->GetRasterBand(1);
    GDALRasterBand* sBand = sourceImages[1]->GetRasterBand(1);
    GDALRasterBand* tBand = sourceImages[2]->GetRasterBand(1);
    GDALRasterBand* iBand = sourceImages[3]->GetRasterBand(1);
    GDALRasterBand* panBand = sourceImages.back()->GetRasterBand(1);

    int xSize = panBand->GetXSize();
    int ySize = panBand->GetYSize();
    double nodata = panBand->GetNoDataValue();

    GDALRasterBand* newfBand = resultImage->GetRasterBand(1);
    GDALRasterBand* newsBand = resultImage->GetRasterBand(2);
    GDALRasterBand* newtBand = resultImage->GetRasterBand(3);

    GDALSetRasterNoDataValue(newfBand, nodata);
    GDALSetRasterNoDataValue(newsBand, nodata);
    GDALSetRasterNoDataValue(newtBand, nodata);

    float* fLine = new float[xSize];
    float* sLine = new float[xSize];
    float* tLine = new float[xSize];
    float* iLine = new float[xSize];
    float* panLine = new float[xSize];
    float* newfLine = new float[xSize];
    float* newsLine = new float[xSize];
    float* newtLine = new float[xSize];
    for (int i = 0; i < ySize; i++)
    {
        fBand->RasterIO(GF_Read, 0, i, xSize, 1, fLine, xSize, 1, GDT_Float32, 0, 0);
        sBand->RasterIO(GF_Read, 0, i, xSize, 1, sLine, xSize, 1, GDT_Float32, 0, 0);
        tBand->RasterIO(GF_Read, 0, i, xSize, 1, tLine, xSize, 1, GDT_Float32, 0, 0);
        iBand->RasterIO(GF_Read, 0, i, xSize, 1, iLine, xSize, 1, GDT_Float32, 0, 0);
        panBand->RasterIO(GF_Read, 0, i, xSize, 1, panLine, xSize, 1, GDT_Float32, 0, 0);

        for (int j = 0; j < xSize; j++)
        {

            if (panLine[j] == nodata)
            {
                newfLine[j] = nodata;
                newsLine[j] = nodata;
                newtLine[j] = nodata;
            }
            else
            {
                float R = fLine[j];
                float G = sLine[j];
                float B = tLine[j];
                float P = panLine[j];
                float I = iLine[j];
                R = R / 65536;
                B = B / 65536;
                G = G / 65536;
                P = P / 65536;
                I = I / 65536;

                float ma = max(max(R, G), B);
                float mi = min(min(R, G), B);
                float delta = ma - mi;
                float H, S, V;
                if (delta > 0)
                {
                    if (ma == R)
                    {
                        H = 60 * (fmod(((G - B) / delta), 6));
                    }
                    else if (ma == G)
                    {
                        H = 60 * (((B - R) / delta) + 2);
                    }
                    else if (ma == B)
                    {
                        H = 60 * (((R - G) / delta) + 4);
                    }

                    if (ma > 0)
                    {
                        S = delta / ma;
                    }
                    else
                    {
                        S = 0;
                    }

                    V = ma;
                }
                else
                {
                    H = 0;
                    S = 0;
                    V = ma;
                }

                if (H < 0)
                {
                    H = 360 + H;
                }
                //////////////////////////

                V = P - addData[0] * I;

                ////////////////////////
                float C = V * S; // Chroma
                float fHPrime = fmod(H / 60.0, 6);
                float X = C * (1 - fabs(fmod(fHPrime, 2) - 1));
                float M = V - C;

                if (0 <= fHPrime && fHPrime < 1) {
                    R = C;
                    G = X;
                    B = 0;
                }
                else if (1 <= fHPrime && fHPrime < 2) {
                    R = X;
                    G = C;
                    B = 0;
                }
                else if (2 <= fHPrime && fHPrime < 3) {
                    R = 0;
                    G = C;
                    B = X;
                }
                else if (3 <= fHPrime && fHPrime < 4) {
                    R = 0;
                    G = X;
                    B = C;
                }
                else if (4 <= fHPrime && fHPrime < 5) {
                    R = X;
                    G = 0;
                    B = C;
                }
                else if (5 <= fHPrime && fHPrime < 6) {
                    R = C;
                    G = 0;
                    B = X;
                }
                else {
                    R = 0;
                    G = 0;
                    B = 0;
                }

                R += M;
                G += M;
                B += M;



                R = R * 65536;
                B = B * 65536;
                G = G * 65536;
                P = P * 65536;

                newfLine[j] = R;
                newsLine[j] = G;
                newtLine[j] = B;
            }

        }

        newfBand->RasterIO(GF_Write, 0, i, xSize, 1, newfLine, xSize, 1, GDT_Float32, 0, 0);
        newsBand->RasterIO(GF_Write, 0, i, xSize, 1, newsLine, xSize, 1, GDT_Float32, 0, 0);
        newtBand->RasterIO(GF_Write, 0, i, xSize, 1, newtLine, xSize, 1, GDT_Float32, 0, 0);
    }

    double GeoTransform[6];
    sourceImages.back()->GetGeoTransform(GeoTransform);

    resultImage->SetDescription((char*)"Sharped dataset");
    resultImage->SetGeoTransform(GeoTransform);
    resultImage->SetProjection(sourceImages.back()->GetProjectionRef());

    delete[] fLine;
    delete[] sLine;
    delete[] tLine;
    delete[] iLine;
    delete[] panLine;
    delete[]newsLine;
    delete[]newfLine;
    delete[]newtLine;
}

void IHSSharpening::computeRGB(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
{
    GDALRasterBand* fBand = sourceImages[0]->GetRasterBand(1);
    GDALRasterBand* sBand = sourceImages[1]->GetRasterBand(1);
    GDALRasterBand* tBand = sourceImages[2]->GetRasterBand(1);
    GDALRasterBand* panBand = sourceImages.back()->GetRasterBand(1);

    int xSize = panBand->GetXSize();
    int ySize = panBand->GetYSize();
    double nodata = panBand->GetNoDataValue();

    GDALRasterBand* newfBand = resultImage->GetRasterBand(1);
    GDALRasterBand* newsBand = resultImage->GetRasterBand(2);
    GDALRasterBand* newtBand = resultImage->GetRasterBand(3);

    GDALSetRasterNoDataValue(newfBand, nodata);
    GDALSetRasterNoDataValue(newsBand, nodata);
    GDALSetRasterNoDataValue(newtBand, nodata);

    float* fLine = new float[xSize];
    float* sLine = new float[xSize];
    float* tLine = new float[xSize];
    float* panLine = new float[xSize];
    float* newfLine = new float[xSize];
    float* newsLine = new float[xSize];
    float* newtLine = new float[xSize];
    for (int i = 0; i < ySize; i++)
    {
        fBand->RasterIO(GF_Read, 0, i, xSize, 1, fLine, xSize, 1, GDT_Float32, 0, 0);
        sBand->RasterIO(GF_Read, 0, i, xSize, 1, sLine, xSize, 1, GDT_Float32, 0, 0);
        tBand->RasterIO(GF_Read, 0, i, xSize, 1, tLine, xSize, 1, GDT_Float32, 0, 0);
        panBand->RasterIO(GF_Read, 0, i, xSize, 1, panLine, xSize, 1, GDT_Float32, 0, 0);

        for (int j = 0; j < xSize; j++)
        {

            if (panLine[j] == nodata)
            {
                newfLine[j] = nodata;
                newsLine[j] = nodata;
                newtLine[j] = nodata;
            }
            else
            {
                float R = fLine[j];
                float G = sLine[j];
                float B = tLine[j];
                float P = panLine[j];
                R = R / 65536;
                B = B / 65536;
                G = G / 65536;
                P = P / 65536;

                float ma = max(max(R, G), B);
                float mi = min(min(R, G), B);
                float delta = ma - mi;
                float H, S, V;
                if (delta > 0)
                {
                    if (ma == R)
                    {
                        H = 60 * (fmod(((G - B) / delta), 6));
                    }
                    else if (ma == G)
                    {
                        H = 60 * (((B - R) / delta) + 2);
                    }
                    else if (ma == B)
                    {
                        H = 60 * (((R - G) / delta) + 4);
                    }

                    if (ma > 0)
                    {
                        S = delta / ma;
                    }
                    else
                    {
                        S = 0;
                    }

                    V = ma;
                }
                else
                {
                    H = 0;
                    S = 0;
                    V = ma;
                }

                if (H < 0)
                {
                    H = 360 + H;
                }
                //////////////////////////

                V = P;

                ////////////////////////
                float C = V * S; // Chroma
                float fHPrime = fmod(H / 60.0, 6);
                float X = C * (1 - fabs(fmod(fHPrime, 2) - 1));
                float M = V - C;

                if (0 <= fHPrime && fHPrime < 1) {
                    R = C;
                    G = X;
                    B = 0;
                }
                else if (1 <= fHPrime && fHPrime < 2) {
                    R = X;
                    G = C;
                    B = 0;
                }
                else if (2 <= fHPrime && fHPrime < 3) {
                    R = 0;
                    G = C;
                    B = X;
                }
                else if (3 <= fHPrime && fHPrime < 4) {
                    R = 0;
                    G = X;
                    B = C;
                }
                else if (4 <= fHPrime && fHPrime < 5) {
                    R = X;
                    G = 0;
                    B = C;
                }
                else if (5 <= fHPrime && fHPrime < 6) {
                    R = C;
                    G = 0;
                    B = X;
                }
                else {
                    R = 0;
                    G = 0;
                    B = 0;
                }

                R += M;
                G += M;
                B += M;



                R = R * 65536;
                B = B * 65536;
                G = G * 65536;
                P = P * 65536;

                newfLine[j] = R;
                newsLine[j] = G;
                newtLine[j] = B;
            }

        }

        newfBand->RasterIO(GF_Write, 0, i, xSize, 1, newfLine, xSize, 1, GDT_Float32, 0, 0);
        newsBand->RasterIO(GF_Write, 0, i, xSize, 1, newsLine, xSize, 1, GDT_Float32, 0, 0);
        newtBand->RasterIO(GF_Write, 0, i, xSize, 1, newtLine, xSize, 1, GDT_Float32, 0, 0);
    }

    double GeoTransform[6];
    sourceImages.back()->GetGeoTransform(GeoTransform);

    resultImage->SetDescription((char*)"Sharped dataset");
    resultImage->SetGeoTransform(GeoTransform);
    resultImage->SetProjection(sourceImages.back()->GetProjectionRef());

    delete[] fLine;
    delete[] sLine;
    delete[] tLine;
    delete[] panLine;
    delete[]newsLine;
    delete[]newfLine;
    delete[]newtLine;
}

void IHSSharpening::compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
{
    if (sourceImages.size() == 4) computeRGB(sourceImages, resultImage, addData);
    else if (sourceImages.size() == 5) computeRGBNIR(sourceImages, resultImage, addData);
    else throw std::invalid_argument("Incorrect number of images.");
}