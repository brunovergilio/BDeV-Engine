#include "WolfWorld.h"
#include "BDeV/Core/System/HID/BvKeyboard.h"
#include "BDeV/Core/Math/BvMath.h"
#include <algorithm>


constexpr auto kWidth = 21;
constexpr auto kHeight = 21;
static u8 g_Map[kHeight][kWidth] =
{
	{ 3, 2, 2, 2, 2, 2, 2, 2,   2, 2, 2, 2, 2, 2, 2,   3, 2, 2, 2, 2, 3 },
	{ 3, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 3, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 3, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 1, 1, 1, 1, 1, 1, 1, 'D', 1, 1, 1, 1, 1, 1,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 1, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 1, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 1, 0, 0, 0,   0, 0, 0, 0, 0, 1, 0, 'D', 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 1, 0, 0, 0, 'S',   0, 0, 0, 0, 1, 0, 0,   3, 0, 0, 0, 0, 3},
	{ 1, 0, 0, 1, 0, 0, 0, 0,   0, 0, 0, 0, 1, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 4, 0, 0, 0,   4, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 1, 0, 0,   1, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 2, 0, 0, 0, 0, 0, 'S', 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0,   3, 0, 0, 0, 0, 3 },
	{ 1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1 },
};


void WolfWorld::Initialize()
{
	m_Map.m_Width = kWidth;
	m_Map.m_Height = kHeight;
	auto pointerSize = sizeof(u8*) * kHeight;
	auto pMem = reinterpret_cast<u8*>(BV_ALLOC(kWidth * kHeight + pointerSize, 1));
	m_Map.m_ppMap2D = reinterpret_cast<u8**>(pMem);
	auto pStart = pMem + pointerSize;
	for (auto h = 0; h < kHeight; h++)
	{
		m_Map.m_ppMap2D[h] = pStart + h * kWidth;
	}

	for (auto h = 0; h < kHeight; h++)
	{
		for (auto w = 0; w < kWidth; w++)
		{
			auto val = g_Map[h][w];
			m_Map.m_ppMap2D[h][w] = val;
			if (val == Map::kHorzDoorTile)
			{
				m_Doors.PushBack({ {w, h} });
			}
			else if (val == Map::kVertDoorTile)
			{
				m_Doors.PushBack({ {w, h}, 0.0f, true });
			}
			else if (val == Map::kPushWallTile)
			{
				m_PushWalls.PushBack({ {w, h} });
			}
		}
	}

	for (auto& door : m_Doors)
	{
		m_DoorTable.Emplace(door.m_TilePos, &door);
	}

	for (auto& pushWall : m_PushWalls)
	{
		m_PushWallTable.Emplace(pushWall.m_TilePos, &pushWall);
	}

	m_Player.m_Dir.Set(0.0f, 1.0f);
	m_Player.m_Pos.Set(8.0f, 8.0f);

	m_Sprites.Resize(1);
	//m_Sprites[0] = { BvVec2(10.0f, 13.0f), 0 };
	//m_Sprites[1] = { BvVec2(4.0f, 8.0f), 500 };
	m_Sprites[0] = { BvVec2(8.0f, 10.0f), 500 };

	BvVec2 pos = m_Player.m_Pos;
	std::sort(m_Sprites.begin(), m_Sprites.end(), [pos](const Sprite& lhs, const Sprite& rhs) -> bool
		{
			return bool((pos - lhs.m_Pos).LengthSqr() > (pos - rhs.m_Pos).LengthSqr());
		});
}


void WolfWorld::Shutdown()
{
	BV_FREE(m_Map.m_ppMap2D);
}


void WolfWorld::Update(f32 dt)
{
	BvVec2 nextPos;

	BvKeyboard kb;
	if (kb.KeyIsDown(BvKey::kUp))
	{
		nextPos = m_Player.m_Pos + m_Player.m_Dir * dt;
	}
	else if (kb.KeyIsDown(BvKey::kDown))
	{
		nextPos = m_Player.m_Pos - m_Player.m_Dir * dt;
	}

	// TODO: Handle collision
	i32 x = i32(nextPos.GetX());
	i32 y = i32(nextPos.GetY());
	if (x >= 0 && x < m_Map.m_Width && y >= 0 && y < m_Map.m_Height)
	{
		m_Player.m_Pos = nextPos;
	}

	if (kb.KeyIsDown(BvKey::kRight))
	{
		m_Player.m_Dir = m_Player.m_Dir.Rotate(-dt * 0.5f);
	}
	else if (kb.KeyIsDown(BvKey::kLeft))
	{
		m_Player.m_Dir = m_Player.m_Dir.Rotate(dt * 0.5f);
	}

	for (auto& door : m_Doors)
	{
		door.m_Visible = false;
		door.Update(dt);
	}

	for (auto& pushWall : m_PushWalls)
	{
		pushWall.m_Visible = false;
		Int2 prevPos = pushWall.m_TilePos;
		if (pushWall.Update(dt))
		{
			m_PushWallTable.Erase(prevPos);
			m_PushWallTable.Emplace(pushWall.m_TilePos, &pushWall);
			Int2 tilePos = pushWall.m_TilePos;
			std::swap(m_Map.m_ppMap2D[prevPos.y][prevPos.x], m_Map.m_ppMap2D[tilePos.y][tilePos.x]);

			if (pushWall.m_State == PushWall::State::kDone)
			{
				m_Map.m_ppMap2D[tilePos.y][tilePos.x] = 2;
			}
		}
	}
}