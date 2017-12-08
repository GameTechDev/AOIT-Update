/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////

#ifndef H_IOT_RESOLVE
#define H_IOT_RESOLVE


#include "DXAOIT.hlsl"
#include "FragmentList.hlsl"



float4 AOITResolvePS(float4 pos: SV_POSITION, float2 tex : TEX_COORD0) : SV_Target
{
    uint i;
    uint nodeOffset;  
    int2 screenAddress = int2(pos.xy);  

    // Get offset to the first node
    uint firstNodeOffset = FL_GetFirstNodeOffset(screenAddress); 

    AOITData data;
    // Initialize AVSM data    
    [unroll]for (i = 0; i < OIT_RT_COUNT; ++i) {
        data.depth[i] = IOT_EMPTY_NODE_DEPTH.xxxx;
        data.trans[i] = OIT_FIRT_NODE_TRANS.xxxx;
    }
   
    // Fetch all nodes and add them to our visibiity function
    nodeOffset = firstNodeOffset;
    [loop] while (nodeOffset != FL_NODE_LIST_NULL) 
    {
        // Get node..
        FragmentListNode node = FL_GetNode(nodeOffset);

        float depth;
        uint  coverageMask;
        FL_UnpackDepthAndCoverage(node.depth, depth, coverageMask);
    
            // Unpack color
            float4 nodeColor = FL_UnpackColor(node.color);        
            AOITInsertFragment(depth,  saturate(1.0 - nodeColor.w), data);      

        // Move to next node
        nodeOffset = node.next;                    
    }
    float3 color = float3(0, 0, 0);
    // Fetch all nodes again and composite them
    nodeOffset = firstNodeOffset;
    [loop]  while (nodeOffset != FL_NODE_LIST_NULL) {
        // Get node..
        FragmentListNode node = FL_GetNode(nodeOffset);
        float depth;
        uint  coverageMask;
        FL_UnpackDepthAndCoverage(node.depth, depth, coverageMask);
            
            // Unpack color
            float4 nodeColor = FL_UnpackColor(node.color);
            AOITFragment frag = AOITFindFragment(data, depth);
            float vis = frag.index == 0 ? 1.0f : frag.transA;
            color += nodeColor.xyz * nodeColor.www * vis.xxx;

        // Move to next node
        nodeOffset = node.next;                    
    }

    float4 blendColor = float4(color, data.trans[OIT_RT_COUNT - 1][3]);
    return blendColor;
}



#endif // H_AIOT_RESOLVE
