#pragma once


#include "../Utils/WolfCommon.h"


class WolfRenderer;


class WolfFader final
{
public:
	WolfFader() {}
	~WolfFader() {}

	void SetFadeTime(f32 fadeTime);
	void FadeIn();
	void FadeOut();
	void Tick(f32 dt);

	BV_INLINE auto IsInFadeInMode() const { return m_FadeMul > 0.0f; }
	BV_INLINE auto IsFadingIn() const { return IsInFadeInMode() && m_CurrFadeTime < m_TotalFadeTime; }
	BV_INLINE auto HasFadedIn() const { return IsInFadeInMode() && m_CurrFadeTime == m_TotalFadeTime; }

	BV_INLINE auto IsInFadeOutMode() const { return m_FadeMul < 0.0f; }
	BV_INLINE auto IsFadingOut() const { return IsInFadeOutMode() && m_CurrFadeTime < m_TotalFadeTime; }
	BV_INLINE auto HasFadedOut() const { return IsInFadeOutMode() && m_CurrFadeTime == 0.0f; }

	BV_INLINE auto GetFadeValue() const { return m_CurrFadeTime / m_TotalFadeTime; }

private:
	f32 m_TotalFadeTime = 1.0f;
	f32 m_CurrFadeTime = 0.0f;
	f32 m_FadeMul = 1.0f;
};


class WolfPhase
{
public:
	WolfPhase() {};
	virtual ~WolfPhase() {}

	virtual void Activate() = 0;
	virtual void Update(f32 dt) = 0;
	virtual void Render() = 0;
};


class WolfTitleScreenPhase final : public WolfPhase
{
public:
	WolfTitleScreenPhase() {}
	~WolfTitleScreenPhase() {}

	void Activate() override;
	void Update(f32 dt) override;
	void Render() override;

private:
	WolfRenderer* m_pRenderer = nullptr;
	WolfFader m_Fader;
	f32 m_TotalScreenTime = 3.0f; // How long to stay active
	f32 m_CurrScreenTime = 0.0f; // Current active time
	u32 m_TitlePic = 0; // Texture id
	bool m_Done = false; // When to stop this phase
};