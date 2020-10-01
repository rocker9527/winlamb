/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <functional>
#include <type_traits>
#include <Windows.h>

namespace _wli {

const UINT WM_UI_WORK_THREAD = WM_APP + 0x3fff;

}//namespace _wli

namespace wl {

/// Exposes a method that allows code to run in UI thread.
/// This class is used as the argument for background_work() lambdas.
class ui_work final {
private:
	HWND _hWnd;

public:
	explicit ui_work(HWND hWnd) : _hWnd{hWnd} { }

	/// Executes a function in the UI thread, allowing UI updates.
	/// This method is blocking: it returns only after the function is finished.
	/// @tparam F std::function<void()>
	/// @param func []() {}
	template<typename F>
	auto work(F&& func) noexcept -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<void()>
		>>
	{
		std::function<void()>* pFunc = new std::function<void()>{std::move(func)};
		SendMessageW(this->_hWnd, _wli::WM_UI_WORK_THREAD,
			0xc0def00d, reinterpret_cast<LPARAM>(pFunc));
	}
};

}//namespace wl