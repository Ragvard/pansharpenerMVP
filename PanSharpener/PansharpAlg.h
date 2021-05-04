#pragma once

#include "gdal_priv.h"
#include <vector>
#include <stdexcept>
#include "gdalwarper.h" 
#include <iostream>
using namespace std;

// Base class for all pan-sharpening algorithms
class PansharpAlg
{
public:

    // Constructor
    PansharpAlg();

    ~PansharpAlg();

    // Static method for resclaing first image to the resolution of the second image
    static void rescale(GDALDataset* lowRes, GDALDataset* highRes, GDALResampleAlg alg, string path);
 
    // Method for starting pansharpening algorithm
    void applySharpening(vector<GDALDataset*> sourceImages, string resultPath, vector<float> addData);

protected:

    virtual void compute(vector<GDALDataset*> sourceImages, GDALDataset* resultImage, vector<float> addData);
};