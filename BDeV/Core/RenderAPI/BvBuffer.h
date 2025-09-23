#pragma once


#include "BvRenderCommon.h"


BV_OBJECT_DEFINE_ID(IBvBuffer, "6184cda7-615d-4b60-a31c-92f679f62d03");
class IBvBuffer : public BvRCObj
{
	BV_NOCOPYMOVE(IBvBuffer);

public:
	virtual void* const Map(u64 size = 0, u64 offset = 0) = 0;
	virtual void Unmap() = 0;
	virtual void Flush(u64 size = 0, u64 offset = 0) const = 0;
	virtual void Invalidate(u64 size = 0, u64 offset = 0) const = 0;
	virtual const BufferDesc& GetDesc() const = 0;
	virtual void* GetMappedData() const = 0;
	virtual u64 GetDeviceAddress() const = 0;

	template<typename Type>
	BV_INLINE Type* GetMappedDataAsT() const { return reinterpret_cast<Type*>(GetMappedData()); }

protected:
	IBvBuffer() {}
	~IBvBuffer() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(IBvBuffer);