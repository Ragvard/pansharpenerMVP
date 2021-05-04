#include "Brovey.h"

// Class for Brovey algorithm


    Brovey::Brovey() { };
    Brovey::~Brovey() { };

    void Brovey::computeRGB(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
    {
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
                    float div = (fLine[j] + sLine[j] + tLine[j]);
                    float change = 0;
                    if (div != 0) change = panLine[j] / div; // No end of the universe today
                    

                    newfLine[j] = fLine[j] * change * addData[0];
                    newsLine[j] = sLine[j] * change * addData[1];
                    newtLine[j] = tLine[j] * change * addData[2];
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

    void Brovey::computeRGBNIR(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
    {
        if (addData.size() != 4) throw std::invalid_argument("Incorrect amount of additional data");

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
                    float div = addData[0] * fLine[j] + addData[1] * sLine[j] + addData[2] * tLine[j];
                    float change = 0;
                    if (div != 0) change = (panLine[j] - addData[3] * iLine[j]) / div;

                    newfLine[j] = fLine[j] * change;
                    newsLine[j] = sLine[j] * change;
                    newtLine[j] = tLine[j] * change;
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

    void Brovey::compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) 
    {
        if (sourceImages.size() == 4) computeRGB(sourceImages, resultImage, addData);
        else if (sourceImages.size() == 5) computeRGBNIR(sourceImages, resultImage, addData);
        else throw std::invalid_argument("Incorrect number of images.");
    }