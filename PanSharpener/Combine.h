#pragma once
#include "PansharpAlg.h"

// Class for Combine function
class Combine : public  PansharpAlg
{
public:
    Combine();

    ~Combine();

private:
    void compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) override;
};