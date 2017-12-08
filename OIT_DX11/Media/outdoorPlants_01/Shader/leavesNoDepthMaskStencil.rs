
[DepthStencilStateDX11]
DepthEnable = true
DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO
StencilEnable               = true
StencilReadMask             = 255
StencilWriteMask            = 255
FrontFaceStencilFailOp      = D3D11_STENCIL_OP_KEEP
FrontFaceStencilDepthFailOp = D3D11_STENCIL_OP_KEEP
FrontFaceStencilPassOp      = D3D11_STENCIL_OP_REPLACE
FrontFaceStencilFunc        = D3D11_COMPARISON_ALWAYS
BackFaceStencilFailOp       = D3D11_STENCIL_OP_KEEP
BackFaceStencilDepthFailOp  = D3D11_STENCIL_OP_KEEP
BackFaceStencilPassOp       = D3D11_STENCIL_OP_REPLACE
BackFaceStencilFunc         = D3D11_COMPARISON_ALWAYS
StencilRef                  = 1

[RasterizerStateDX11]
CullMode = D3D11_CULL_NONE

[SamplerDX11_1]

[SamplerDX11_2]
Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT

[RenderTargetBlendStateDX11_1] 
BlendEnable = true 
SrcBlend = D3D11_BLEND_SRC_ALPHA 
DestBlend = D3D11_BLEND_INV_SRC_ALPHA 
BlendOp = D3D11_BLEND_OP_ADD
