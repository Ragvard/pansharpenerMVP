#pragma once
#include "PansharpAlg.h"

// Class for Simple-Mean algorithm
class SimpleMean : public  PansharpAlg
{
public:
    SimpleMean();
    ~SimpleMean();

private:
    void compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) override;
};