/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
/////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __CPUTRENDERPARAMS_H__
#define __CPUTRENDERPARAMS_H__

// TODO:  Change name to CPUTRenderContext?
class CPUTCamera;
class CPUTBuffer;
class CPUTRenderParameters
{
public:
    bool         mShowBoundingBoxes;
    bool         mDrawModels;
    bool         mRenderOnlyVisibleModels;
    int          mWidth;
    int          mHeight;
    CPUTCamera  *mpCamera;
    CPUTCamera  *mpShadowCamera;
    CPUTBuffer  *mpPerModelConstants;
    CPUTBuffer  *mpPerFrameConstants;

    CPUTRenderParameters() :
        mShowBoundingBoxes(false),
        mDrawModels(true),
        mRenderOnlyVisibleModels(true),
        mpCamera(0),
        mpShadowCamera(0),
        mpPerModelConstants(0),
        mpPerFrameConstants(0)
    {}
    ~CPUTRenderParameters(){}
private:
};

#endif // __CPUTRENDERPARAMS_H__