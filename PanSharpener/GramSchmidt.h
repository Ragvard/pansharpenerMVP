#pragma once
#include "PansharpAlg.h"

// Class for GramSchmidt algorithm
class GramSchmidt : public  PansharpAlg
{
public:
    GramSchmidt();

    ~GramSchmidt();

private:
    void compute(std::vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData) override;
};