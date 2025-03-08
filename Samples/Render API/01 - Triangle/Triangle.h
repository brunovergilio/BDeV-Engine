#pragma once


#include "Samples/Render API/RenderAPISampleBase/SampleBase.h"


class Triangle : public SampleBase
{
public:
	void Initialize() override;
	void Update() override;
	void UpdateUI() override;
	void Draw() override;
	void Shutdown() override;
};