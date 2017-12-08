/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CPUTCOMPUTESHADERDX11_H
#define _CPUTCOMPUTESHADERDX11_H

#include "CPUT.h"
#include "CPUTShaderDX11.h"

class CPUTComputeShaderDX11 : public CPUTShaderDX11
{
protected:
    ID3D11ComputeShader *mpComputeShader;

     // Destructor is not public.  Must release instead of delete.
    ~CPUTComputeShaderDX11(){ SAFE_RELEASE(mpComputeShader) }

public:
    static CPUTComputeShaderDX11 *CreateComputeShader(
        const cString      &name,
        const cString      &shaderMain,
        const cString      &shaderProfile,
        CPUT_SHADER_MACRO *pShaderMacros=NULL
    );

    static CPUTComputeShaderDX11 *CreateComputeShaderFromMemory(
        const cString     &name,
        const cString     &shaderMain,
        const cString     &shaderProfile,
        const char        *pShaderSource,
        CPUT_SHADER_MACRO *pShaderMacros=NULL
    );
    CPUTComputeShaderDX11() : mpComputeShader(NULL), CPUTShaderDX11(NULL) {}
    CPUTComputeShaderDX11(ID3D11ComputeShader *pD3D11ComputeShader, ID3DBlob *pBlob) : mpComputeShader(pD3D11ComputeShader), CPUTShaderDX11(pBlob) {}
    ID3DBlob *GetBlob() { return mpBlob; }
    ID3D11ComputeShader *GetNativeComputeShader() { return mpComputeShader; }
};

#endif //_CPUTCOMPUTESHADER_H