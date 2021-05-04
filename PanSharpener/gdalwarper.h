/******************************************************************************
  * $Id$
  *
  * Project:  GDAL High Performance Warper
  * Purpose:  Prototypes, and definitions for warping related work.
  * Author:   Frank Warmerdam, warmerdam@pobox.com
  *
  ******************************************************************************
  * Copyright (c) 2003, Frank Warmerdam
  * Copyright (c) 2009-2012, Even Rouault <even dot rouault at spatialys.com>
  *
  * Permission is hereby granted, free of charge, to any person obtaining a
  * copy of this software and associated documentation files (the "Software"),
  * to deal in the Software without restriction, including without limitation
  * the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the
  * Software is furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included
  * in all copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
  * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  * DEALINGS IN THE SOFTWARE.
  ****************************************************************************/

#ifndef GDALWARPER_H_INCLUDED
#define GDALWARPER_H_INCLUDED

#include "gdal_alg.h"
#include "cpl_minixml.h"
#include "cpl_multiproc.h"

CPL_C_START

/* Note: values are selected to be consistent with GDALRIOResampleAlg of gcore/gdal.h */
typedef enum {
    GRA_NearestNeighbour = 0, GRA_Bilinear = 1, GRA_Cubic = 2, GRA_CubicSpline = 3, GRA_Lanczos = 4, GRA_Average = 5, GRA_Mode = 6,
    /*  GRA_Gauss=7 reserved. */ GRA_Max = 8, GRA_Min = 9, GRA_Med = 10, GRA_Q1 = 11, GRA_Q3 = 12, GRA_Sum = 13, GRA_RMS = 14,
    GRA_LAST_VALUE = GRA_RMS
} GDALResampleAlg;

typedef enum {
    GWKAOM_Average = 1, GWKAOM_Fmode = 2, GWKAOM_Imode = 3, GWKAOM_Max = 4, GWKAOM_Min = 5, GWKAOM_Quant = 6, GWKAOM_Sum = 7, GWKAOM_RMS = 8
} GWKAverageOrModeAlg;

typedef int
(*GDALMaskFunc)(void* pMaskFuncArg,
    int nBandCount, GDALDataType eType,
    int nXOff, int nYOff,
    int nXSize, int nYSize,
    GByte** papabyImageData,
    int bMaskIsFloat, void* pMask);

CPLErr CPL_DLL
GDALWarpNoDataMasker(void* pMaskFuncArg, int nBandCount, GDALDataType eType,
    int nXOff, int nYOff, int nXSize, int nYSize,
    GByte** papabyImageData, int bMaskIsFloat,
    void* pValidityMask, int* pbOutAllValid);

CPLErr CPL_DLL
GDALWarpDstAlphaMasker(void* pMaskFuncArg, int nBandCount, GDALDataType eType,
    int nXOff, int nYOff, int nXSize, int nYSize,
    GByte** /*ppImageData */,
    int bMaskIsFloat, void* pValidityMask);
CPLErr CPL_DLL
GDALWarpSrcAlphaMasker(void* pMaskFuncArg, int nBandCount, GDALDataType eType,
    int nXOff, int nYOff, int nXSize, int nYSize,
    GByte** /*ppImageData */,
    int bMaskIsFloat, void* pValidityMask, int* pbOutAllOpaque);

CPLErr CPL_DLL
GDALWarpSrcMaskMasker(void* pMaskFuncArg, int nBandCount, GDALDataType eType,
    int nXOff, int nYOff, int nXSize, int nYSize,
    GByte** /*ppImageData */,
    int bMaskIsFloat, void* pValidityMask);

CPLErr CPL_DLL
GDALWarpCutlineMasker(void* pMaskFuncArg, int nBandCount, GDALDataType eType,
    int nXOff, int nYOff, int nXSize, int nYSize,
    GByte** /* ppImageData */,
    int bMaskIsFloat, void* pValidityMask);
/************************************************************************/
/*                           GDALWarpOptions                            */
/************************************************************************/

typedef struct {

    char** papszWarpOptions;

    double              dfWarpMemoryLimit;

    GDALResampleAlg     eResampleAlg;

    GDALDataType        eWorkingDataType;

    GDALDatasetH        hSrcDS;

    GDALDatasetH        hDstDS;

    int                 nBandCount;

    int* panSrcBands;

    int* panDstBands;

    int                nSrcAlphaBand;

    int                nDstAlphaBand;

    double* padfSrcNoDataReal;
    double* padfSrcNoDataImag;

    double* padfDstNoDataReal;
    double* padfDstNoDataImag;

    GDALProgressFunc    pfnProgress;

    void* pProgressArg;

    GDALTransformerFunc pfnTransformer;

    void* pTransformerArg;

    GDALMaskFunc* papfnSrcPerBandValidityMaskFunc;
    void** papSrcPerBandValidityMaskFuncArg;

    GDALMaskFunc        pfnSrcValidityMaskFunc;
    void* pSrcValidityMaskFuncArg;

    GDALMaskFunc        pfnSrcDensityMaskFunc;
    void* pSrcDensityMaskFuncArg;

    GDALMaskFunc        pfnDstDensityMaskFunc;
    void* pDstDensityMaskFuncArg;

    GDALMaskFunc        pfnDstValidityMaskFunc;
    void* pDstValidityMaskFuncArg;

    CPLErr(*pfnPreWarpChunkProcessor)(void* pKern, void* pArg);
    void* pPreWarpProcessorArg;

    CPLErr(*pfnPostWarpChunkProcessor)(void* pKern, void* pArg);
    void* pPostWarpProcessorArg;

    void* hCutline;

    double              dfCutlineBlendDist;

} GDALWarpOptions;

GDALWarpOptions CPL_DLL* CPL_STDCALL GDALCreateWarpOptions(void);
void CPL_DLL CPL_STDCALL GDALDestroyWarpOptions(GDALWarpOptions*);
GDALWarpOptions CPL_DLL* CPL_STDCALL
GDALCloneWarpOptions(const GDALWarpOptions*);

void CPL_DLL CPL_STDCALL
GDALWarpInitDstNoDataReal(GDALWarpOptions*, double dNoDataReal);

void CPL_DLL CPL_STDCALL
GDALWarpInitSrcNoDataReal(GDALWarpOptions*, double dNoDataReal);

void CPL_DLL CPL_STDCALL
GDALWarpInitNoDataReal(GDALWarpOptions*, double dNoDataReal);

void CPL_DLL CPL_STDCALL
GDALWarpInitDstNoDataImag(GDALWarpOptions*, double dNoDataImag);

void CPL_DLL CPL_STDCALL
GDALWarpInitSrcNoDataImag(GDALWarpOptions*, double dNoDataImag);

void CPL_DLL CPL_STDCALL
GDALWarpResolveWorkingDataType(GDALWarpOptions*);

void CPL_DLL CPL_STDCALL
GDALWarpInitDefaultBandMapping(GDALWarpOptions*, int nBandCount);

CPLXMLNode CPL_DLL* CPL_STDCALL
GDALSerializeWarpOptions(const GDALWarpOptions*);
GDALWarpOptions CPL_DLL* CPL_STDCALL
GDALDeserializeWarpOptions(CPLXMLNode*);
/************************************************************************/
/*                         GDALReprojectImage()                         */
/************************************************************************/

CPLErr CPL_DLL CPL_STDCALL
GDALReprojectImage(GDALDatasetH hSrcDS, const char* pszSrcWKT,
    GDALDatasetH hDstDS, const char* pszDstWKT,
    GDALResampleAlg eResampleAlg, double dfWarpMemoryLimit,
    double dfMaxError,
    GDALProgressFunc pfnProgress, void* pProgressArg,
    GDALWarpOptions* psOptions);

CPLErr CPL_DLL CPL_STDCALL
GDALCreateAndReprojectImage(GDALDatasetH hSrcDS, const char* pszSrcWKT,
    const char* pszDstFilename, const char* pszDstWKT,
    GDALDriverH hDstDriver, char** papszCreateOptions,
    GDALResampleAlg eResampleAlg, double dfWarpMemoryLimit,
    double dfMaxError,
    GDALProgressFunc pfnProgress, void* pProgressArg,
    GDALWarpOptions* psOptions);

/************************************************************************/
/*                           VRTWarpedDataset                           */
/************************************************************************/

GDALDatasetH CPL_DLL CPL_STDCALL
GDALAutoCreateWarpedVRT(GDALDatasetH hSrcDS,
    const char* pszSrcWKT, const char* pszDstWKT,
    GDALResampleAlg eResampleAlg,
    double dfMaxError, const GDALWarpOptions* psOptions);

GDALDatasetH CPL_DLL CPL_STDCALL
GDALAutoCreateWarpedVRTEx(GDALDatasetH hSrcDS,
    const char* pszSrcWKT, const char* pszDstWKT,
    GDALResampleAlg eResampleAlg,
    double dfMaxError, const GDALWarpOptions* psOptions,
    CSLConstList papszTransformerOptions);

GDALDatasetH CPL_DLL CPL_STDCALL
GDALCreateWarpedVRT(GDALDatasetH hSrcDS,
    int nPixels, int nLines, double* padfGeoTransform,
    GDALWarpOptions* psOptions);

CPLErr CPL_DLL CPL_STDCALL
GDALInitializeWarpedVRT(GDALDatasetH hDS,
    GDALWarpOptions* psWO);

CPL_C_END

#if defined(__cplusplus) && !defined(CPL_SUPRESS_CPLUSPLUS)

/************************************************************************/
/*                            GDALWarpKernel                            */
/*                                                                      */

#define WARP_EXTRA_ELTS    1

class CPL_DLL GDALWarpKernel
{
    CPL_DISALLOW_COPY_ASSIGN(GDALWarpKernel)

public:
    char** papszWarpOptions;

    GDALResampleAlg     eResample;
    GDALDataType        eWorkingDataType;
    int                 nBands;

    int                 nSrcXSize;
    int                 nSrcYSize;
    double              dfSrcXExtraSize;
    double              dfSrcYExtraSize;
    GByte** papabySrcImage;

    GUInt32** papanBandSrcValid;
    GUInt32* panUnifiedSrcValid;
    float* pafUnifiedSrcDensity;

    int                 nDstXSize;
    int                 nDstYSize;
    GByte** papabyDstImage;
    GUInt32* panDstValid;
    float* pafDstDensity;

    double              dfXScale;
    double              dfYScale;
    double              dfXFilter;
    double              dfYFilter;
    int                 nXRadius;
    int                 nYRadius;
    int                 nFiltInitX;
    int                 nFiltInitY;

    int                 nSrcXOff;
    int                 nSrcYOff;

    int                 nDstXOff;
    int                 nDstYOff;

    GDALTransformerFunc pfnTransformer;
    void* pTransformerArg;

    GDALProgressFunc    pfnProgress;
    void* pProgress;

    double              dfProgressBase;
    double              dfProgressScale;

    double* padfDstNoDataReal;

    void* psThreadData;
    GDALWarpKernel();
    virtual           ~GDALWarpKernel();

    CPLErr              Validate();
    CPLErr              PerformWarp();
};

void* GWKThreadsCreate(char** papszWarpOptions,
    GDALTransformerFunc pfnTransformer,
    void* pTransformerArg);
void GWKThreadsEnd(void* psThreadDataIn);
/************************************************************************/
/*                         GDALWarpOperation()                          */
/*                                                                      */
/*      This object is application created, or created by a higher      */
/*      level convenience function.  It is responsible for              */
/*      subdividing the operation into chunks, loading and saving       */
/*      imagery, and establishing the varios validity and density       */
/*      masks.  Actual resampling is done by the GDALWarpKernel.        */
/************************************************************************/

typedef struct _GDALWarpChunk GDALWarpChunk;
class CPL_DLL GDALWarpOperation {

    CPL_DISALLOW_COPY_ASSIGN(GDALWarpOperation)

private:
    GDALWarpOptions* psOptions;

    void            WipeOptions();
    int             ValidateOptions();

    CPLErr          ComputeSourceWindow(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize,
        int* pnSrcXOff, int* pnSrcYOff,
        int* pnSrcXSize, int* pnSrcYSize,
        double* pdfSrcXExtraSize, double* pdfSrcYExtraSize,
        double* pdfSrcFillRatio);

    void            ComputeSourceWindowStartingFromSource(
        int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize,
        double* padfSrcMinX, double* padfSrcMinY,
        double* padfSrcMaxX, double* padfSrcMaxY);

    static CPLErr          CreateKernelMask(GDALWarpKernel*, int iBand,
        const char* pszType);

    CPLMutex* hIOMutex;
    CPLMutex* hWarpMutex;

    int             nChunkListCount;
    int             nChunkListMax;
    GDALWarpChunk* pasChunkList;

    int             bReportTimings;
    unsigned long   nLastTimeReported;

    void* psThreadData;

    void            WipeChunkList();
    CPLErr          CollectChunkListInternal(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize);
    void            CollectChunkList(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize);
    void            ReportTiming(const char*);

public:
    GDALWarpOperation();
    virtual        ~GDALWarpOperation();

    CPLErr          Initialize(const GDALWarpOptions* psNewOptions);
    void* CreateDestinationBuffer(int nDstXSize, int nDstYSize,
        int* pbWasInitialized = nullptr);
    static void     DestroyDestinationBuffer(void* pDstBuffer);

    const GDALWarpOptions* GetOptions();

    CPLErr          ChunkAndWarpImage(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize);
    CPLErr          ChunkAndWarpMulti(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize);
    CPLErr          WarpRegion(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize,
        int nSrcXOff = 0, int nSrcYOff = 0,
        int nSrcXSize = 0, int nSrcYSize = 0,
        double dfProgressBase = 0.0, double dfProgressScale = 1.0);
    CPLErr          WarpRegion(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize,
        int nSrcXOff, int nSrcYOff,
        int nSrcXSize, int nSrcYSize,
        double dfSrcXExtraSize, double dfSrcYExtraSize,
        double dfProgressBase, double dfProgressScale);
    CPLErr          WarpRegionToBuffer(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize,
        void* pDataBuf,
        GDALDataType eBufDataType,
        int nSrcXOff = 0, int nSrcYOff = 0,
        int nSrcXSize = 0, int nSrcYSize = 0,
        double dfProgressBase = 0.0, double dfProgressScale = 1.0);
    CPLErr          WarpRegionToBuffer(int nDstXOff, int nDstYOff,
        int nDstXSize, int nDstYSize,
        void* pDataBuf,
        GDALDataType eBufDataType,
        int nSrcXOff, int nSrcYOff,
        int nSrcXSize, int nSrcYSize,
        double dfSrcXExtraSize, double dfSrcYExtraSize,
        double dfProgressBase, double dfProgressScale);
};

#endif /* def __cplusplus */

CPL_C_START

typedef void* GDALWarpOperationH;

GDALWarpOperationH CPL_DLL GDALCreateWarpOperation(const GDALWarpOptions*);
void CPL_DLL GDALDestroyWarpOperation(GDALWarpOperationH);
CPLErr CPL_DLL GDALChunkAndWarpImage(GDALWarpOperationH, int, int, int, int);
CPLErr CPL_DLL GDALChunkAndWarpMulti(GDALWarpOperationH, int, int, int, int);
CPLErr CPL_DLL GDALWarpRegion(GDALWarpOperationH,
    int, int, int, int, int, int, int, int);
CPLErr CPL_DLL GDALWarpRegionToBuffer(GDALWarpOperationH, int, int, int, int,
    void*, GDALDataType,
    int, int, int, int);

/************************************************************************/
/*      Warping kernel functions                                        */
/************************************************************************/

int GWKGetFilterRadius(GDALResampleAlg eResampleAlg);

typedef double (*FilterFuncType)(double dfX);
FilterFuncType GWKGetFilterFunc(GDALResampleAlg eResampleAlg);

// TODO(schwehr): Can padfVals be a const pointer?
typedef double (*FilterFunc4ValuesType)(double* padfVals);
FilterFunc4ValuesType GWKGetFilterFunc4Values(GDALResampleAlg eResampleAlg);
CPL_C_END

#endif /* ndef GDAL_ALG_H_INCLUDED */
