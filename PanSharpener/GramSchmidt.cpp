#include "GramSchmidt.h"

GramSchmidt::GramSchmidt() { };
GramSchmidt::~GramSchmidt() { };

void proj(float* b, float* a, float* out, int size)
{
    int tempTop = 0;
    int tempBot = 0;
    for (int i = 0; i < size; i++)
    {
        tempTop += a[i] * b[i];
        tempBot += b[i] * b[i];
    }

    int temp = 0;
    if (tempBot != 0) temp = tempTop / tempBot;
    for (int i = 0; i < size; i++)
    {
        out[i] = temp * b[i];
    }
}

void combine(float* a, float* b, float* result, int size, bool sign)
{
    if (sign) for (int i = 0; i < size; i++) result[i] = a[i] + b[i];
    else for (int i = 0; i < size; i++) result[i] = a[i] - b[i];

}
void combine(float* a, float* b, float* c, float* result, int size, bool sign)
{
    if (sign) for (int i = 0; i < size; i++) result[i] = a[i] + b[i] + c[i];
    else  for (int i = 0; i < size; i++) result[i] = a[i] - b[i] - c[i];
}

void combine(float* a, float* b, float* c, float* d, float* result, int size, bool sign)
{
    if (sign) for (int i = 0; i < size; i++) result[i] = a[i] + b[i] + c[i] + d[i];
    else for (int i = 0; i < size; i++) result[i] = a[i] - b[i] - c[i] - d[i];
}

void GramSchmidt::compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData)
{
    if (sourceImages.size() != 4) throw std::invalid_argument("Error: source images count is different from 3.");
    //if (addData.size() != 3) throw std::invalid_argument("Incorrect amount of additional data");

    // Каналы: красны\зеленый\синий
    GDALRasterBand* fBand = sourceImages[0]->GetRasterBand(1);
    GDALRasterBand* sBand = sourceImages[1]->GetRasterBand(1);
    GDALRasterBand* tBand = sourceImages[2]->GetRasterBand(1);
    // Панхроматический канал
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
    for (int i = 0; i < 3; i++) source[i] = (float*)CPLMalloc(sizeof(float) * xSize);
    float** middle = new float* [3];
    for (int i = 0; i < 3; i++) middle[i] = (float*)CPLMalloc(sizeof(float) * xSize);
    float** projection = new float* [6];
    for (int i = 0; i < 6; i++) projection[i] = (float*)CPLMalloc(sizeof(float) * xSize);
    float** result = new float* [3];
    for (int i = 0; i < 3; i++) result[i] = (float*)CPLMalloc(sizeof(float) * xSize);

    float* panLine = (float*)CPLMalloc(sizeof(float) * xSize);
    float* panLowLine = (float*)CPLMalloc(sizeof(float) * xSize);

    for (int i = 0; i < ySize; i++)
    {
        fBand->RasterIO(GF_Read, 0, i, xSize, 1, source[0], xSize, 1, GDT_Float32, 0, 0);
        sBand->RasterIO(GF_Read, 0, i, xSize, 1, source[1], xSize, 1, GDT_Float32, 0, 0);
        tBand->RasterIO(GF_Read, 0, i, xSize, 1, source[2], xSize, 1, GDT_Float32, 0, 0);
        panBand->RasterIO(GF_Read, 0, i, xSize, 1, panLine, xSize, 1, GDT_Float32, 0, 0);

        for (int j = 0; j < xSize; j++)
        {
            float div = (result[0][j] + result[1][j] + result[2][j]);
            float change = 1;
            if (div != 0) change = panLine[j] / div; // No end of the universe today
            panLowLine[j] = panLine[j] * change;
        }

        proj(panLowLine, source[0], projection[0], xSize);
        combine(source[0], projection[0], middle[0], xSize, false);

        proj(panLowLine, source[1], projection[1], xSize);
        proj(middle[0], source[1], projection[2], xSize);
        combine(source[1], projection[1], projection[2], middle[1], xSize, false);

        proj(panLowLine, source[2], projection[3], xSize);
        proj(middle[0], source[2], projection[4], xSize);
        proj(middle[1], source[2], projection[5], xSize);
        combine(source[2], projection[3], projection[4], projection[5], middle[2], xSize, false);


        proj(panLowLine, source[0], projection[0], xSize);
        //proj(panLine, source[0], projection[0], xSize);
        combine(middle[0], projection[0], result[0], xSize, true);

        proj(panLowLine, source[1], projection[1], xSize);
        //proj(panLine, source[1], projection[1], xSize);
        proj(middle[0], source[1], projection[2], xSize);
        combine(middle[1], projection[1], projection[2], result[1], xSize, true);
        
        proj(panLowLine, source[2], projection[3], xSize);
        //proj(panLine, source[2], projection[3], xSize);
        proj(middle[0], source[2], projection[4], xSize);
        proj(middle[1], source[2], projection[5], xSize);
        combine(middle[2], projection[3], projection[4], projection[5], result[2], xSize, true);


        newfBand->RasterIO(GF_Write, 0, i, xSize, 1, result[0], xSize, 1, GDT_Float32, 0, 0);
        newsBand->RasterIO(GF_Write, 0, i, xSize, 1, result[1], xSize, 1, GDT_Float32, 0, 0);
        newtBand->RasterIO(GF_Write, 0, i, xSize, 1, result[2], xSize, 1, GDT_Float32, 0, 0);
    }

    double GeoTransform[6];
    sourceImages.front()->GetGeoTransform(GeoTransform);

    resultImage->SetDescription((char*)"Sharped dataset");
    resultImage->SetGeoTransform(GeoTransform);
    resultImage->SetProjection(sourceImages.front()->GetProjectionRef());

    for (int i = 0; i < 3; i++) free(source[i]);
    for (int i = 0; i < 3; i++) free(projection[i]);
    for (int i = 0; i < 3; i++) free(result[i]);
    for (int i = 0; i < 3; i++) free(middle[i]);

    delete[] source;
    delete[] projection;
    delete[] result;
    delete[] middle;

    free(panLine);
    free(panLowLine);
}
