#include "Maximize.h"
#include <algorithm>

Maximize::Maximize() { };
Maximize::~Maximize() { };

void Maximize::compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
{
    if (sourceImages.size() != 4) throw std::invalid_argument("Error: source images count is different from 4.");
    if (addData.size() != 3) throw std::invalid_argument("Incorrect amount of additional data");

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

    float** source = new float* [3];
    for (int i = 0; i < 3; i++) source[i] = new float[xSize];
    float** result = new float* [3];
    for (int i = 0; i < 3; i++) result[i] = new float[xSize];

    float* panLine = new float[xSize];

    for (int i = 0; i < ySize; i++)
    {
        fBand->RasterIO(GF_Read, 0, i, xSize, 1, source[0], xSize, 1, GDT_Float32, 0, 0);
        sBand->RasterIO(GF_Read, 0, i, xSize, 1, source[1], xSize, 1, GDT_Float32, 0, 0);
        tBand->RasterIO(GF_Read, 0, i, xSize, 1, source[2], xSize, 1, GDT_Float32, 0, 0);
        panBand->RasterIO(GF_Read, 0, i, xSize, 1, panLine, xSize, 1, GDT_Float32, 0, 0);

        for (int j = 0; j < xSize; j++)
        {

            if (panLine[j] == nodata)
            {
                result[0][j] = nodata;
                result[1][j] = nodata;
                result[2][j] = nodata;
            }
            else
            {
                result[0][j] = max(source[0][j], panLine[j]) * addData[0];
                result[1][j] = max(source[1][j], panLine[j]) * addData[1];
                result[2][j] = max(source[2][j], panLine[j]) * addData[2];
            }

        }

        newfBand->RasterIO(GF_Write, 0, i, xSize, 1, result[0], xSize, 1, GDT_Float32, 0, 0);
        newsBand->RasterIO(GF_Write, 0, i, xSize, 1, result[1], xSize, 1, GDT_Float32, 0, 0);
        newtBand->RasterIO(GF_Write, 0, i, xSize, 1, result[2], xSize, 1, GDT_Float32, 0, 0);
    }

    double GeoTransform[6];
    sourceImages.back()->GetGeoTransform(GeoTransform);

    resultImage->SetDescription((char*)"Sharped dataset");
    resultImage->SetGeoTransform(GeoTransform);
    resultImage->SetProjection(sourceImages.back()->GetProjectionRef());

    for (int i = 0; i < 3; i++) delete[] source[i];
    for (int i = 0; i < 3; i++) delete[] result[i];

    delete[] panLine;

    delete[] source;
    delete[] result;
}