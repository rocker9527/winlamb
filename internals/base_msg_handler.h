/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <functional>
#include <optional>
#include <process.h>
#include <type_traits>
#include <Windows.h>
#include "catch_all_excps.h"
#include "msg_proxy_all.h"
#include "param_notif.h"
#include "store.h"
#include "ui_work.h"

namespace _wli {

// Owns the message handler store.
// Handles the UI thread message.
class base_msg_handler final {
private:
	store<UINT> _msgs; // user lambdas for messages, WM_COMMAND and WM_NOTIFY
	store<WORD> _cmds;
	store<std::pair<UINT_PTR, int>> _nfys; // NMHDR is UINT_PTR/UINT, but UINT notif codes are mostly negative, triggering overflow warnings
	msg_proxy_all _msgProxy;

public:
	base_msg_handler(bool isDialog) : _msgProxy{_msgs, _cmds, _nfys, isDialog}
	{
		this->_default_msg_handlers();
	}

	[[nodiscard]] msg_proxy_all& on_msg() noexcept { return this->_msgProxy; }

	// Searches for a stored handler for the given message and executes it, if any.
	std::optional<LRESULT> exec(UINT msg, WPARAM wp, LPARAM lp) noexcept
	{
		// Search a stored user handler.
		std::function<LRESULT(wl::msg::wm)>* userFunc = nullptr;

		if (msg == WM_COMMAND) {
			userFunc = this->_cmds.find(LOWORD(wp)); // search an user lambda for this WM_COMMAND
		} else if (msg == WM_NOTIFY) {
			const NMHDR* nmhdr = reinterpret_cast<const NMHDR*>(lp);
			userFunc = this->_nfys.find({nmhdr->idFrom, static_cast<int>(nmhdr->code)}); // search an user lambda for this WM_NOTIFY
		} else {
			userFunc = this->_msgs.find(msg); // search an user lambda for this message
		}

		if (userFunc) {
			LRESULT retVal = 0;
			catch_all_excps([&]() {
				retVal = (*userFunc)({wp, lp}); // execute user lambda
			}, post_quit_on_catch::YES);
			return retVal;
		}

		return {}; // message handler not found
	}

	// Executes a function asynchronously, in a new detached background thread.
	// func: [](ui_work ui) {}
	template<typename F>
	auto background_work(HWND hWnd, F&& func) -> std::enable_if_t<
		std::is_same_v<
			decltype(std::function{std::forward<F>(func)}),
			std::function<void(wl::ui_work)>
		>>
	{
		struct pack final {
			HWND hWnd;
			std::function<void(wl::ui_work)> func;
		};

		pack* pPack = new pack{hWnd, std::move(func)};

		uintptr_t hThread = _beginthreadex(nullptr, 0,
			[](void* ptr) noexcept -> unsigned int {
				pack* pPack = reinterpret_cast<pack*>(ptr);
				_wli::catch_all_excps([&]() {
					pPack->func(wl::ui_work{pPack->hWnd}); // invoke user func
				}, _wli::post_quit_on_catch::NO);
				delete pPack;
				_endthreadex(0); // http://www.codeproject.com/Articles/7732/A-class-to-synchronise-thread-completions/
				return 0;
			},
			pPack, 0, nullptr);

		if (hThread) {
			CloseHandle(reinterpret_cast<HANDLE>(hThread));
		}
	}

private:
	void _default_msg_handlers()
	{
		this->on_msg().wm(WM_UI_WORK_THREAD, [](wl::msg::wm p) noexcept -> LRESULT
		{
			// Handles our special message, sent by ui_work::work().
			if (p.wparam == 0xc0def00d && p.lparam) {
				std::function<void()>* pFunc = reinterpret_cast<std::function<void()>*>(p.lparam); // retrieve back
				catch_all_excps([&]() {
					(*pFunc)(); // invoke user func
				}, post_quit_on_catch::YES);
				delete pFunc;
			}
			return 0;
		});
	}
};

}//namespace _wli