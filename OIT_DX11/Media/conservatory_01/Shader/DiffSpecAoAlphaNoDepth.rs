
[DepthStencilStateDX11]
DepthEnable = true
DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO

[RasterizerStateDX11]
CullMode = D3D11_CULL_NONE

[SamplerDX11_1]

[SamplerDX11_2]
ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL
Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT

[RenderTargetBlendStateDX11_1] 
BlendEnable = true 
SrcBlend = D3D11_BLEND_SRC_ALPHA 
DestBlend = D3D11_BLEND_INV_SRC_ALPHA 
BlendOp = D3D11_BLEND_OP_ADD
