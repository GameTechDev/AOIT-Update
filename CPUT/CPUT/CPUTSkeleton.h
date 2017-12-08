/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __CPUTSKELETON_H__
#define __CPUTSKELETON_H__

#include "CPUT.h"
#include "CPUTAssetLibrary.h"
#include <vector>


class CPUTJoint
{
public:
    cString  mName;         //Joint name
    UINT     mParentIndex;  //Array Index of Parent Joint

    float4x4 mInverseBindPoseMatrix;  //This represents the inverse of the joint transforms at time of binding
    float4x4 mRTMatrix;              //Rotation and Translation matrix; propagated to children
    float4x4 mPreRotationMatrix;     //Orientation of joint, prior to binding
    float4x4 mScaleMatrix;           //Scale matrix; not propagated to children

    CPUTJoint();
    bool LoadJoint(std::ifstream& file);
};

struct CPUTSkeleton
{
    void LoadSkeleton(std::ifstream& file);
public:
    cString mName;         //Name of Mesh associated with Skeleton
    UINT mNumberOfJoints;  //Total number of joints contained in skeleton
    std::vector<CPUTJoint> mJointsList; //List of Joints in Skeleton

    CPUTSkeleton();
    
    bool LoadSkeleton(cString path);    //Uses fullpath
};

#endif  //__CPUTSKELETON_H__