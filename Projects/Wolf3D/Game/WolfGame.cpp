#include "WolfGame.h"


void WolfGame::Run()
{
	m_App.Initialize();
	m_App.RegisterRawInput(true, true);

	auto pMonitor = BvMonitor::Primary();
	WindowDesc windowDesc;
	{
		auto& area = pMonitor->GetFullscreenArea();
		windowDesc.m_X = ((area.m_Right - area.m_Left) - windowDesc.m_Width) >> 1;
		windowDesc.m_Y = ((area.m_Bottom - area.m_Top) - windowDesc.m_Height) >> 1;
	}
	m_pWindow = m_App.CreateWindow(windowDesc);

	BvPath path("D:\\Games\\Wolf3D\\wolf3d"); //= BvPath::FromCurrentDirectory();
	m_Archive.Load(path);

	m_World.Initialize();

	m_Renderer.Initialize(m_pWindow);
	m_Renderer.ToggleOverlay(false);

	for (auto i = STARTPICS; i < STARTPICM; i++)
	{
		m_Renderer.CreateTexture(m_Archive.GetImage(i));
	}
	auto font = m_Renderer.CreateTexture(m_Archive.GetFontImage(STARTFONT + 1));


	auto w = m_pWindow->GetWidth();
	m_ZBuffer.Resize(w, kF32Max);

	BvKeyboard kb;
	auto prevTs = BvTime::GetCurrentTimestampInMs();
	f32 accum = 0.0f;
	m_CurrSprite = m_Renderer.CreateTexture(m_Archive.GetSpriteData(330), 64 * 64 * sizeof(u32));
	while (!m_pWindow->IsClosed())
	{
		m_App.ProcessOSEvents();

		w = m_pWindow->GetWidth();
		if (w != m_ZBuffer.Size())
		{
			m_ZBuffer.Resize(w, kF32Max);
		}

		auto currTs = BvTime::GetCurrentTimestampInMs();
		f32 dt = (currTs - prevTs) * 0.005f;
		prevTs = currTs;

		constexpr f32 kTickRate = 70.0f;
		constexpr f32 kTickTime = 1.0f / kTickRate;

		accum += dt;
		while (accum >= kTickTime)
		{
			m_World.Update(dt);
			accum -= kTickTime;
		}

		m_Renderer.Update(dt);

		DrawDDA();
		if (kb.KeyWentDown(BvKey::kD))
		{
			m_DrawTopDownView = !m_DrawTopDownView;
		}

		if (m_DrawTopDownView)
		{
			DrawTopdownView();
		}

		m_Renderer.Render();
	}
	m_Renderer.Shutdown();

	m_World.Shutdown();

	m_App.DestroyWindow(m_pWindow);
	m_App.Shutdown();
}


void WolfGame::DrawTopdownView()
{
	m_Renderer.NewColorBatch();
	m_Renderer.NewTextureBatch();

	const BvVec2 squareSize = BvVec2(18.0f, 18.0f);
	const BvVec2 squareDrawSize = BvVec2(20.0f, 20.0f);
	const BvVec2 drawPos = squareDrawSize + ((squareDrawSize - squareSize) * 0.5f);

	auto& map = m_World.GetMap();
	for (auto h = 0; h < map.m_Height; h++)
	{
		for (auto w = 0; w < map.m_Width; w++)
		{
			if (map.m_ppMap2D[h][w])
			{
				BvVec2 pos(w, h);
				m_Renderer.DrawSquare(pos * drawPos, squareSize, BvVec2(0.0f, 0.0f), BvVec2(1.0f, 1.0f), map.m_ppMap2D[h][w] - 1, BvVec4(1.0f, 1.0f, 1.0f));
			}
		}
	}

	auto& player = m_World.GetPlayer();
	m_Renderer.DrawColoredSquare(player.m_Pos * drawPos, squareSize, Float4(1.0f, 0.0f, 0.0f, 1.0f));

	const f32 fov = std::tanf(XMConvertToRadians(30.0f));
	BvVec2 camPlane = BvVec2(player.m_Dir.GetY(), -player.m_Dir.GetX());
	camPlane *= fov;

	f32 numRays = 30.0f;
	f32 step = 2.0f / numRays;
	f32 rayLen = 100.0f;
	BvVec2 camPos = (player.m_Pos * drawPos) + (squareSize * 0.5f);

	f32 posX = player.m_Pos.GetX();
	f32 posY = player.m_Pos.GetY();
	//if (posX >= 0 && posX < f32(map.m_Width) && posY >= 0 && posY < f32(map.m_Height))
	//{
	//	rayLen = std::hypotf(posX, posY) * std::max(squareDrawSize.GetX(), squareDrawSize.GetY());
	//}

	for (f32 i = -1.0f; i <= 1.0f; i += step)
	{
		BvVec2 camRay = player.m_Dir + (camPlane * i);
		BvVec2 camTo = camPos + (camRay * rayLen);
		m_Renderer.DrawLine(camPos, camTo, BvVec4(0.0f, 1.0f, 0.0f, 1.0f));
	}
}

void WolfGame::DrawDDA()
{
	//m_Renderer.NewColorBatch();
	m_Renderer.NewTextureBatch();

	auto& map = m_World.GetMap();
	auto& player = m_World.GetPlayer();
	auto& sprites = m_World.GetSprites();

	auto screenWidth = i32(m_pWindow->GetWidth());
	auto screenHeight = i32(m_pWindow->GetHeight());

	f32 playerPosX = player.m_Pos.GetX();
	f32 playerPosY = player.m_Pos.GetY();

	const f32 fov = std::tanf(XMConvertToRadians(30.0f));
	BvVec2 camPlane = BvVec2(player.m_Dir.GetY(), -player.m_Dir.GetX());
	camPlane *= fov;

	f32 playerDirX = player.m_Dir.GetX();
	f32 playerDirY = player.m_Dir.GetY();
	constexpr i32 kTextureWidth = 64;

	bool spaceDown = BvKeyboard().KeyWentDown(BvKey::kSpace);
	for (auto w = 0; w < screenWidth; w++)
	{
		auto currX = i32(playerPosX);
		auto currY = i32(playerPosY);

		f32 camX = 2.0f * w / f32(screenWidth) - 1.0f;
		f32 rayDirX = playerDirX + f32(camPlane.GetX()) * camX;
		f32 rayDirY = playerDirY + f32(camPlane.GetY()) * camX;

		i32 stepX = (1 - 2 * (rayDirX < 0.0f));
		i32 stepY = (1 - 2 * (rayDirY < 0.0f));
		f32 deltaDistX = std::fabsf(1.0f / rayDirX);
		f32 deltaDistY = std::fabsf(1.0f / rayDirY);

		bool hit = false;

		f32 sideDistX = (stepX > 0 ? f32(currX + 1) - playerPosX : playerPosX - f32(currX)) * deltaDistX;
		f32 sideDistY = (stepY > 0 ? f32(currY + 1) - playerPosY : playerPosY - f32(currY)) * deltaDistY;
		f32 perpWallDist = 0.0f;
		i32 side = 0;
		f32 doorCoord = 0.0f;
		f32 wallCoord = 0.0f;
		bool hitDoor = false;
		bool hitDoorNeighbors = false;
		bool hitPushWall = false;
		f32 pushWallAmount = 0.0f;
		u32 tileTexture = kU32Max;
		while (!hit)
		{
			if (sideDistX < sideDistY)
			{
				sideDistX += deltaDistX;
				currX += stepX;

				side = 0;
				if (map.IsXOutOfBounds(currX))
				{
					currX = std::clamp(currX, 0, map.m_Width);
					hit = true;
					break;
				}
			}
			else
			{
				sideDistY += deltaDistY;
				currY += stepY;

				side = 1;
				if (map.IsYOutOfBounds(currY))
				{
					currY = std::clamp(currY, 0, map.m_Height);
					hit = true;
					break;
				}
			}

			if (map.IsWall(currX, currY))
			{
				hit = true;
				tileTexture = map.m_ppMap2D[currY][currX] - 1;

				if (side == 0)
				{
					if (map.IsDoor(currX - 1, currY) || map.IsDoor(currX + 1, currY))
					{
						hitDoorNeighbors = true;
						tileTexture = 1;
					}
				}
				else
				{
					if (map.IsDoor(currX, currY - 1) || map.IsDoor(currX, currY + 1))
					{
						hitDoorNeighbors = true;
						tileTexture = 1;
					}
				}
			}
			else if (map.IsDoor(currX, currY))
			{
				auto pDoor = m_World.GetDoor(currX, currY);
				if (!pDoor)
				{
					continue;
				}

				if (!std::exchange(pDoor->m_Visible, true) && spaceDown)
				{
					pDoor->Activate();
				}

				i32 planeTile = 0;
				i32 hitPlaneTile = 0.0f;
				f32 posAxis = 0.0f;
				f32 dirAxis = 0.0f;
				f32 hitPosAxis = 0.0f;
				f32 hitDirAxis = 0.0f;
				if (side == 0)
				{
					planeTile = currX;
					posAxis = playerPosX;
					dirAxis = rayDirX;
					hitPosAxis = playerPosY;
					hitDirAxis = rayDirY;
					hitPlaneTile = currY;
				}
				else
				{
					planeTile = currY;
					posAxis = playerPosY;
					dirAxis = rayDirY;
					hitPosAxis = playerPosX;
					hitDirAxis = rayDirX;
					hitPlaneTile = currX;
				}

				f32 doorPlane = planeTile + 0.375f;
				f32 t = (doorPlane - posAxis) / dirAxis;
				f32 hitVal = hitPosAxis + t * hitDirAxis;
				doorCoord = hitVal - hitPlaneTile;

				if ((side == 0 && doorCoord >= pDoor->m_OpenAmount && doorCoord <= 1.0f) 
					|| (side == 1 && doorCoord <= 1.0f - pDoor->m_OpenAmount && doorCoord >= 0.0f))
				{
					doorCoord -= pDoor->m_OpenAmount;
					hit = true;
					hitDoor = true;
					perpWallDist = t;
					tileTexture = 1;
				}
			}
			else if (map.IsPushWall(currX, currY))
			{
				auto pPushWall = m_World.GetPushWall(currX, currY);
				if (!pPushWall)
				{
					continue;
				}

				i32 sX = 0;
				i32 sY = 0;
				if (!std::exchange(pPushWall->m_Visible, true) && spaceDown)
				{
					if (side == 0)
					{
						sX = 1 - 2 * (currX < playerPosX);
						sY = 0;
					}
					else
					{
						sY = 1 - 2 * (currY < playerPosY);
						sX = 0;
					}

					pPushWall->Activate(sX, sY);
				}

				if (pPushWall->m_State == WolfWorld::PushWall::State::kActive)
				{
					sX = pPushWall->m_Step.GetX();
					sY = pPushWall->m_Step.GetY();
				}

				i32 planeTile = 0;
				i32 hitPlaneTile = 0.0f;
				f32 posAxis = 0.0f;
				f32 dirAxis = 0.0f;
				f32 hitPosAxis = 0.0f;
				f32 hitDirAxis = 0.0f;
				f32 stepAxis = 0.0f;
				if (sX != 0)
				{
					stepAxis = sX;
					planeTile = currX;
					posAxis = playerPosX;
					dirAxis = rayDirX;
					hitPosAxis = playerPosY;
					hitDirAxis = rayDirY;
					hitPlaneTile = currY;
				}
				else
				{
					stepAxis = stepY;
					planeTile = currY;
					posAxis = playerPosY;
					dirAxis = rayDirY;
					hitPosAxis = playerPosX;
					hitDirAxis = rayDirX;
					hitPlaneTile = currX;
				}

				if (pPushWall->m_State == WolfWorld::PushWall::State::kInactive)
				{
					hit = true;
					tileTexture = 1;
					continue;
				}

				f32 pwPlane = planeTile + (stepAxis < 0) + pPushWall->m_MoveAmount * stepAxis;
				f32 t = (pwPlane - posAxis) / dirAxis;
				f32 hitVal = hitPosAxis + t * hitDirAxis;
				wallCoord = hitVal - hitPlaneTile;

				if (wallCoord >= 0.0f && wallCoord <= 1.0f)
				{
					hit = true;
					hitPushWall = true;
					perpWallDist = t;
					tileTexture = 1;
				}
			}
		}

		f32 wallX = 0.0f;
		if (!(hitDoor || hitPushWall))
		{
			perpWallDist = side == 0 ? sideDistX - deltaDistX : sideDistY - deltaDistY;
			wallX = side == 0 ? playerPosY + perpWallDist * rayDirY : playerPosX + perpWallDist * rayDirX;
			wallX -= std::floorf(wallX);
		}
		else if (hitPushWall)
		{
			wallX = wallCoord;
		}
		else
		{
			wallX = doorCoord;
		}

		m_ZBuffer[w] = perpWallDist;


		i32 texX = i32(wallX * kTextureWidth);

		if (!hitDoorNeighbors)
		{
			if (side == 0 && rayDirX > 0)
			{
				texX = kTextureWidth - texX - 1;
			}

			if (side == 1 && rayDirY < 0)
			{
				texX = kTextureWidth - texX - 1;
			}
		}

		f32 u = texX / f32(kTextureWidth);

		i32 lineHeight = i32(screenHeight / perpWallDist);
		f32 y1 = -lineHeight + screenHeight * 0.5f;
		if (y1 < 0.0f)
		{
			y1 = 0.0f;
		}
		f32 y2 = lineHeight + screenHeight * 0.5f;
		if (y2 >= screenHeight)
		{
			y2 = screenHeight - 1;
		}

		BvVec2 lineStart(w, y1);
		BvVec2 lineEnd(w, y2);

		//m_Renderer.DrawSquare({ f32(w), y1 }, { 1, f32(lineHeight) }, { u0, 0.0f }, { u1, 1.0f }, map.m_ppMap2D[currX][currY] - 1);

		//m_Renderer.DrawLine(lineStart, lineEnd);

		BvVec4 colorMultipliers[] =
		{
			BvVec4(1.0f, 1.0f, 1.0f),
			BvVec4(0.3f, 0.3f, 0.3f),
		};

		m_Renderer.DrawLine(lineStart, lineEnd, BvVec2(u, 0.0f), BvVec2(u, 1.0f), tileTexture, colorMultipliers[side]);
		//m_Renderer.DrawLine(lineStart, lineEnd, colors[map.m_ppMap2D[currX][currY] - 1]);
	}

	f32 planeX = camPlane.GetX();
	f32 planeY = camPlane.GetY();

	f32 invDet = 1.0f / (planeX * playerDirY - playerDirX * planeY);
	for (auto& sprite : sprites)
	{
		f32 relSpritePosX = sprite.m_Pos.GetX() - playerPosX;
		f32 relSpritePosY = sprite.m_Pos.GetY() - playerPosY;

		f32 transformX = invDet * (playerDirY * relSpritePosX - playerDirX * relSpritePosY);
		f32 transformY = invDet * (-planeY * relSpritePosX + planeX * relSpritePosY) - 0.25f;

		constexpr f32 kSpriteNearClip = 0.34375f;
		if (transformY < kSpriteNearClip)
		{
			continue;
		}

		i32 spriteScreenX = i32(screenWidth * 0.5f) * (1 + transformX / transformY);

		i32 spriteHeight = std::abs(i32(screenHeight / transformY));
		i32 spriteWidth = spriteHeight; // Works as long as sprites' sizes are squared

		f32 y1 = (screenHeight - spriteHeight) * 0.5f;
		if (y1 < 0.0f)
		{
			y1 = 0.0f;
		}
		f32 y2 = (screenHeight + spriteHeight) * 0.5f;
		if (y2 >= screenHeight)
		{
			y2 = screenHeight - 1;
		}

		f32 x1 = spriteScreenX - spriteWidth * 0.5f;
		//if (x1 < 0.0f)
		//{
		//	x1 = 0.0f;
		//}
		f32 x2 = spriteScreenX + spriteWidth * 0.5f;
		//if (x2 >= screenWidth)
		//{
		//	x2 = screenWidth - 1;
		//}

		i32 drawX1 = std::max(0, (i32)std::ceil(x1));
		i32 drawX2 = std::min(screenWidth, (i32)std::ceil(x2));
		for (auto w = drawX1; w < drawX2; w++)
		{
			if (transformY <= 0.0f || transformY >= m_ZBuffer[w])
			{
				continue;
			}

			f32 u = (w - x1) / f32(spriteWidth);
			BvVec2 lineStart(w, y1);
			BvVec2 lineEnd(w, y2);

			m_Renderer.DrawLine(lineStart, lineEnd, BvVec2(u, 0.0f), BvVec2(u, 1.0f), sprite.m_TextureIndex);
		}
	}
}