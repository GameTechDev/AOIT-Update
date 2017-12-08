/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
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