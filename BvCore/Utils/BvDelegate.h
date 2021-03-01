#pragma once


#include <tuple>


class BvDelegateBase
{
public:
	virtual ~BvDelegateBase() {}
	virtual void Invoke() const = 0;
};


template<class Fn, class... Args>
class BvDelegate : public BvDelegateBase
{
public:
	template<typename = typename std::enable_if_t<std::is_same_v<std::invoke_result_t<Fn, Args...>, void>&& std::is_invocable_v<Fn, Args...>>>
	BvDelegate(Fn&& fn, Args &&... args)
		: m_Function(std::forward<Fn>(fn)), m_Args(std::forward<Args>(args)...) {}

	~BvDelegate() {}

	void Invoke() const override
	{
		std::apply(m_Function, m_Args);
	}

private:
	Fn m_Function;
	std::tuple<Args...> m_Args;
};