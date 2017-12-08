/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef H_LIST_TEXTURE
#define H_LIST_TEXTURE

struct SegmentNode
{
    int         next;
    float       depth[2];
    float       trans;
};

struct VisibilityNode
{
    int         next;
    float       depth;
    float       trans; 
};

struct FragmentNode
{
    int         next;
    float       depth;
    int         color;
};


#endif // H_LIST_TEXTURE