﻿#include "Device.h"
#include "Resource.h"

Resource::Resource(const ComPtr<Device>& d3dDevice, const ComPtr<ID3D12Resource>& d3dResource)
    : d3dDevice(d3dDevice), d3dResource(d3dResource)
{
}

Resource::~Resource() = default;

ID3D12Resource* Resource::getD3DResource() const
{
    return d3dResource.Get();
}

FUNCTION_STUB(HRESULT, Resource::GetDevice, Device** ppDevice)

FUNCTION_STUB(HRESULT, Resource::SetPrivateData, REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)

FUNCTION_STUB(HRESULT, Resource::GetPrivateData, REFGUID refguid, void* pData, DWORD* pSizeOfData)

FUNCTION_STUB(HRESULT, Resource::FreePrivateData, REFGUID refguid)

FUNCTION_STUB(DWORD, Resource::SetPriority, DWORD PriorityNew)

FUNCTION_STUB(DWORD, Resource::GetPriority)

FUNCTION_STUB(void, Resource::PreLoad)

FUNCTION_STUB(D3DRESOURCETYPE, Resource::GetType)