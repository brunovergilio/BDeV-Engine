#pragma once


#include <BDeV/BvCore.h>
#include <BDeV/System/File/BvFile.h>
#include "BDeV/Container/BvString.h"


struct PropertyData
{
	BvString name;
	BvString type;
	bool isPointer;
	bool isFunctionPointer;
};

struct MethodData
{
	BvString name;
	BvString type;
};


struct ObjectData
{
	BvString name;
	BvVector<PropertyData> properties;
	BvVector<MethodData> methods;
	BvVector<BvString> baseObjects;
};

struct EnumData
{
	BvString name;
	BvVector<BvString> constants;
};


class BvReflector
{
public:
	BvReflector();
	void Process(const wchar_t* projectDir);

private:
	void ParseHeaderFile(BvFile& file);

	void ParseObject(const BvVector<BvString>& symbols, u32& symbolIndex);
	void ParseVariable(const BvVector<BvString>& symbols, u32& symbolIndex, u32 lastSymbolIndex);
	void ParseMethod(const BvVector<BvString>& symbols, u32& symbolIndex, u32 lastSymbolIndex);
	void ParseEnum(const BvVector<BvString>& symbols, u32& symbolIndex);
	void GenerateMetadata();

private:
	BvVector<ObjectData> m_Objects;
	BvVector<EnumData> m_Enums;
};