#include "WolfPhase.h"
#include "../Renderer/WolfRenderer.h"


void WolfFader::SetFadeTime(f32 fadeTime)
{
	BV_ASSERT(fadeTime > 0.0f, "Invalid fade time");
	m_TotalFadeTime = fadeTime;
}

void WolfFader::FadeIn()
{
	m_FadeMul = 1.0f;
}

void WolfFader::FadeOut()
{
	m_FadeMul = -1.0f;
}

void WolfFader::Tick(f32 dt)
{
	m_CurrFadeTime += dt * m_FadeMul;
	if (m_CurrFadeTime > m_TotalFadeTime)
	{
		m_CurrFadeTime = m_TotalFadeTime;
	}
	else if (m_CurrFadeTime < 0.0f)
	{
		m_CurrFadeTime = 0.0f;
	}
}


void WolfTitleScreenPhase::Activate()
{
	m_Fader.FadeIn();
}

void WolfTitleScreenPhase::Update(f32 dt)
{
	if (m_Done)
	{
		return;
	}

	auto currFade = m_Fader.GetFadeValue();

	if (m_Fader.HasFadedIn())
	{
		m_CurrScreenTime += dt;
	}
	else if (m_Fader.HasFadedOut())
	{
		m_Done = true;
		return;
	}

	m_Fader.Tick(dt);
	if (!m_Fader.IsInFadeOutMode() && (m_CurrScreenTime >= m_TotalScreenTime || BvKeyboard().GetKeyStateChanges()))
	{
		m_Fader.FadeOut();
	}
}

void WolfTitleScreenPhase::Render()
{
	auto [w, h] = m_pRenderer->GetWindow()->GetSizeF();

	BvVec2 pos(0.0f), size(w, h);

	m_pRenderer->DrawSquare(pos, size, TITLEPIC, BvVec4(m_Fader.GetFadeValue()));
}