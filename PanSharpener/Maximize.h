#pragma once
#include "PansharpAlg.h"

// Class for Simple-Mean algorithm
class Maximize : public  PansharpAlg
{
public:
    Maximize();

    ~Maximize();

private:
    void compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) override;
};