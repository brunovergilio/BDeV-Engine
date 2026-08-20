#pragma once


#include "../Renderer/WolfRenderer.h"
#include "WolfWorld.h"
#include "FPSCounter.h"
#include "WolfResource.h"


class WolfGame
{
public:
	WolfGame() = default;
	~WolfGame() = default;

	void Run();

private:
	void DrawTopdownView();
	void DrawDDA();

private:
	BvApplication m_App;
	BvWindow* m_pWindow = nullptr;
	WolfRenderer m_Renderer;
	WolfWorld m_World;
	WolfArchiveManager m_Archive;
	FPSCounter m_FPS;
	BvVector<f32> m_ZBuffer;
	BvVector<u32> m_Sprites;
	u32 m_CurrSprite;
	bool m_DrawTopDownView = false;
};