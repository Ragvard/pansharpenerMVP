#include "Combine.h"

Combine::Combine() { };
Combine::~Combine() { };

    void Combine::compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
    {
        if (sourceImages.size() != 3) throw std::invalid_argument("Error: source images count is different from 3."); 
        //if (addData.size() != 3) throw std::invalid_argument("Incorrect amount of additional data");

        GDALRasterBand* fBand = sourceImages[0]->GetRasterBand(1);
        GDALRasterBand* sBand = sourceImages[1]->GetRasterBand(1);
        GDALRasterBand* tBand = sourceImages[2]->GetRasterBand(1);

        int xSize = fBand->GetXSize();
        int ySize = fBand->GetYSize();
        double nodata = fBand->GetNoDataValue();

        GDALRasterBand* newfBand = resultImage->GetRasterBand(1);
        GDALRasterBand* newsBand = resultImage->GetRasterBand(2);
        GDALRasterBand* newtBand = resultImage->GetRasterBand(3);

        GDALSetRasterNoDataValue(newfBand, nodata);
        GDALSetRasterNoDataValue(newsBand, nodata);
        GDALSetRasterNoDataValue(newtBand, nodata);

        float* fLine = new float[xSize];
        float* sLine = new float[xSize];
        float* tLine = new float[xSize];
        float* newfLine = new float[xSize];
        float* newsLine = new float[xSize];
        float* newtLine = new float[xSize];

        for (int i = 0; i < ySize; i++)
        {
            fBand->RasterIO(GF_Read, 0, i, xSize, 1, fLine, xSize, 1, GDT_Float32, 0, 0);
            sBand->RasterIO(GF_Read, 0, i, xSize, 1, sLine, xSize, 1, GDT_Float32, 0, 0);
            tBand->RasterIO(GF_Read, 0, i, xSize, 1, tLine, xSize, 1, GDT_Float32, 0, 0);
            for (int j = 0; j < xSize; j++)
            {
                // Black magic
                newfLine[j] = fLine[j] * addData[0];
                newsLine[j] = sLine[j] * addData[1];
                newtLine[j] = tLine[j] * addData[2];

            }
            newfBand->RasterIO(GF_Write, 0, i, xSize, 1, fLine, xSize, 1, GDT_Float32, 0, 0);
            newsBand->RasterIO(GF_Write, 0, i, xSize, 1, sLine, xSize, 1, GDT_Float32, 0, 0);
            newtBand->RasterIO(GF_Write, 0, i, xSize, 1, tLine, xSize, 1, GDT_Float32, 0, 0);
        }

        double GeoTransform[6];
        sourceImages.front()->GetGeoTransform(GeoTransform);

        resultImage->SetDescription((char*)"Sharped dataset");
        resultImage->SetGeoTransform(GeoTransform);
        resultImage->SetProjection(sourceImages.front()->GetProjectionRef());
        delete[] fLine;
        delete[] sLine;
        delete[] tLine;
        
        delete[]newsLine;
        delete[]newfLine;
        delete[]newtLine;
    }
