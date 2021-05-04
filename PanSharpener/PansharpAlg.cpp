#include "PansharpAlg.h"


PansharpAlg::PansharpAlg() { };
PansharpAlg::~PansharpAlg() { };

void PansharpAlg::rescale(GDALDataset* lowRes, GDALDataset* highRes, GDALResampleAlg rescalingAlg, string path)
{
    GDALDriver* gtiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataType type = GDALGetRasterDataType(lowRes->GetRasterBand(1));
    int xSize = highRes->GetRasterXSize();
    int ySize = highRes->GetRasterYSize();
    GDALDataset* newImage = gtiffDriver->Create(path.c_str(), xSize, ySize, 1, type, NULL);

    double geoTransform[6];
    highRes->GetGeoTransform(geoTransform);
    newImage->SetGeoTransform(geoTransform);
    newImage->SetProjection(highRes->GetProjectionRef());

    GDALReprojectImage(lowRes, NULL, newImage, NULL, rescalingAlg, 0.0, 0.0, NULL, NULL, NULL);
    GDALClose(newImage);
}

void PansharpAlg::compute(vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) {};

void PansharpAlg::applySharpening(std::vector<GDALDataset*> sourceImages, string resultPath, vector<float> addData)
{
    // Remove NULL images
    for (int i = 0; i < sourceImages.size(); i++)
    {
        if (sourceImages[i] == NULL)
        {
            sourceImages.erase(sourceImages.begin() + i);
        }
    }
    while (sourceImages.back() == NULL) sourceImages.pop_back();
    int xSize = sourceImages.back()->GetRasterXSize();
    int ySize = sourceImages.back()->GetRasterYSize();
    for (GDALDataset* it : sourceImages)
    {
        if (xSize != it->GetRasterXSize() || ySize != it->GetRasterYSize())
        {
            throw std::invalid_argument("Error: source images are not of the same size.");
        }
    }

    GDALDriver* gtiffDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    GDALDataType type = GDALGetRasterDataType(sourceImages.front()->GetRasterBand(1));

    GDALDataset* resultImage = gtiffDriver->Create(resultPath.c_str(), xSize, ySize, 3, type, NULL);

    
    compute(sourceImages, resultImage, addData);

    GDALClose(resultImage);
}