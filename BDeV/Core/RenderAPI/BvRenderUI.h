#pragma once


#include "BDeV/Core/Utils/BvObject.h"
#include "BDeV/Core/Container/BvVector.h"
#include "BDeV/Core/Container/BvString.h"


class BvCommandContext;


BV_OBJECT_DEFINE_ID(BvRenderUI, "8a9c658d-963e-49a2-9d67-7e5316f1a8fc");
class BvRenderUI : public BvObjectBase
{
	BV_NOCOPYMOVE(BvRenderUI);

public:
	virtual bool Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual bool Header(const char* pText) = 0;
	virtual bool CheckBox(const char* pText, bool value) = 0;
	virtual bool CheckBox(const char* pText, i32 value) = 0;
	virtual bool RadioButton(const char* pText, bool value) = 0;
	virtual bool InputFloat(const char* pText, f32 value, f32 step, u32 precision) = 0;
	virtual bool SliderFloat(const char* pText, f32 value, f32 min, f32 max) = 0;
	virtual bool SliderInt(const char* pText, i32 value, i32 min, i32 max) = 0;
	virtual bool ComboBox(const char* pText, i32 itemindex, const BvVector<BvString>& items) = 0;
	virtual bool Button(const char* pText) = 0;
	virtual bool ColorPicker(const char* pText, float* pColor) = 0;
	virtual void Text(const char* pText, ...) = 0;

protected:
	BvRenderUI() {}
	~BvRenderUI() {}
};
BV_OBJECT_ENABLE_ID_OPERATOR(BvRenderUI);