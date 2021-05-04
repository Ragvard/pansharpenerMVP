#pragma once
#include "PansharpAlg.h"

// Class for HSVSharpening algorithm
class IHSSharpening : public  PansharpAlg
{
public:
    IHSSharpening();

    ~IHSSharpening();

private:
    void compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) override;
    void computeRGB(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData);
    void computeRGBNIR(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData);
};