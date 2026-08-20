#pragma once


#include "BDeV/Core/BvCore.h"
#include <BDeV/Core/Container/BvVector.h>
#include <BDeV/Core/Container/BvRobinMap.h>


template<>
struct std::equal_to<Int2>
{
	constexpr bool operator()(const Int2& lhs, const Int2& rhs) const
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}
};


class WolfWorld
{
public:
	struct Map
	{
		u8** m_ppMap2D;
		i32 m_Width;
		i32 m_Height;

		static constexpr u8 kHorzDoorTile = 'D';
		static constexpr u8 kVertDoorTile = 'V';
		static constexpr u8 kPushWallTile = 'S';

		bool IsWall(i32 x, i32 y) const
		{
			if (IsOutOfBounds(x, y))
			{
				return false;
			}

			auto tile = m_ppMap2D[y][x];
			if (!tile)
			{
				return false;
			}

			return !(tile == kHorzDoorTile || tile == kVertDoorTile || tile == kPushWallTile);
		}

		bool IsDoor(i32 x, i32 y) const
		{
			return !IsOutOfBounds(x, y) && (m_ppMap2D[y][x] == kHorzDoorTile || m_ppMap2D[y][x] == kVertDoorTile);
		}

		bool IsPushWall(i32 x, i32 y) const
		{
			return !IsOutOfBounds(x, y) && m_ppMap2D[y][x] == kPushWallTile;
		}

		bool IsOutOfBounds(i32 x, i32 y) const
		{
			return IsXOutOfBounds(x) || IsYOutOfBounds(y);
		}

		bool IsXOutOfBounds(i32 x) const
		{
			return x < 0 || x >= m_Width;
		}

		bool IsYOutOfBounds(i32 y) const
		{
			return y < 0 || y >= m_Height;
		}
	};

	struct Player
	{
		BvVec2 m_Pos;
		BvVec2 m_Dir;
	};

	struct Sprite
	{
		BvVec2 m_Pos;
		u32 m_TextureIndex;
	};

	struct Door
	{
		enum class State : u8
		{
			kClosed,
			kOpening,
			kOpen,
			kClosing
		};

		BvIVec2 m_TilePos{};
		f32 m_OpenAmount{};
		bool m_Vertical{};
		State m_State{};
		bool m_Visible{};

		void Activate()
		{
			switch (m_State)
			{
			case WolfWorld::Door::State::kClosed:
			case WolfWorld::Door::State::kClosing:
				m_State = State::kOpening;
				break;
			case WolfWorld::Door::State::kOpening:
			case WolfWorld::Door::State::kOpen:
				m_State = State::kClosing;
				break;
			}
		}

		void Update(f32 amount)
		{
			if (m_State == State::kOpening)
			{
				m_OpenAmount += amount;
				if (m_OpenAmount >= 1.0f)
				{
					m_OpenAmount = 1.0f;
					m_State = State::kOpen;
				}
			}
			else if (m_State == State::kClosing)
			{
				m_OpenAmount -= amount;
				if (m_OpenAmount <= 0.0f)
				{
					m_OpenAmount = 0.0f;
					m_State = State::kClosed;
				}
			}
		}
	};

	struct PushWall
	{
		enum class State : u8
		{
			kInactive,
			kActive,
			kDone
		};

		BvIVec2 m_TilePos{};
		f32 m_MoveAmount{};
		State m_State{};
		bool m_Visible{};
		BvIVec2 m_Step;
		i32 m_TilesMoved{};

		void Activate(i32 stepX, i32 stepY)
		{
			if (m_State == State::kInactive)
			{
				m_State = State::kActive;
				m_Step.Set(stepX, stepY);
			}
		}

		bool Update(f32 amount)
		{
			if (m_State == State::kActive)
			{
				m_MoveAmount += amount * 0.2f;
				if (m_MoveAmount >= 1.0f)
				{
					m_MoveAmount -= 1.0f;

					if (++m_TilesMoved == 2)
					{
						m_State = State::kDone;
						m_MoveAmount = 0;
					}

					m_TilePos += m_Step;
					m_TilePos += m_Step;

					return true;
				}
			}

			return false;
		}
	};

	struct StaticObject
	{
		BvIVec2 m_TilePos;
		u32 m_Sprite;
		u32 m_Flags;
	};

	struct Actor
	{
		enum class Direction : u8
		{
			North,
			NorthEast,
			East,
			SouthEast,
			South,
			SouthWest,
			West,
			NorthWest,
			NoDirection
		};
		Actor* m_pNext;
		Actor* m_pPrev;
		BvVec2 m_Pos;
		BvIVec2 m_TilePos;
		Direction m_Direction;
		f32 m_Speed;
		i32 m_Health;
		i32 m_TickCount;
		u32 m_Flags;
		//u32 m_State;
		//u32 m_Target;
	};

	struct ActorState
	{
		i32 m_Duration;
		void(*m_pThinkFn)(Actor&);
		void(*m_pAction)(Actor&);
		ActorState* m_pNext;
	};

	WolfWorld() = default;
	~WolfWorld() = default;

	void Initialize();
	void Shutdown();
	void Update(f32 dt);

	BV_INLINE const Map& GetMap() { return m_Map; }
	BV_INLINE const Player& GetPlayer() { return m_Player; }
	BV_INLINE const auto& GetSprites() { return m_Sprites; }
	BV_INLINE Door* GetDoor(i32 x, i32 y)
	{
		auto it = m_DoorTable.FindKey(Int2(x, y));
		if (it != m_DoorTable.cend())
		{
			return it->second;
		}

		return nullptr;
	}

	BV_INLINE PushWall* GetPushWall(i32 x, i32 y)
	{
		auto it = m_PushWallTable.FindKey(Int2(x, y));
		if (it != m_PushWallTable.cend())
		{
			return it->second;
		}

		return nullptr;
	}

private:
	Map m_Map;
	Player m_Player;
	BvVector<Sprite> m_Sprites;
	BvRobinMap<Int2, Door*> m_DoorTable;
	BvRobinMap<Int2, PushWall*> m_PushWallTable;
	BvVector<Door> m_Doors;
	BvVector<PushWall> m_PushWalls;
	BvVector<Actor> m_Actors;
};