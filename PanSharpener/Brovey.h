#pragma once
#include "PansharpAlg.h"

// Class for Brovey algorithm
class Brovey : public  PansharpAlg
{
public:
    Brovey();

    ~Brovey();

private:
    void compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) override;
    void computeRGB(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData);
    void computeRGBNIR(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData);
};