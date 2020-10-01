/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <vector>
#include <Windows.h>
#include "internals/base_native_control.h"
#include "internals/interfaces.h"
#include "internals/param_wm.h"
#include "str.h"

namespace wl {

/// Native status bar control.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/status-bars
class status_bar final : i_window {
private:
	struct part final {
		UINT sizePixels = 0;
		UINT resizeWeight = 0;
	};

	_wli::base_native_control _base;
	std::vector<part> _parts;
	std::vector<int> _rightEdges; // buffer to speed up adjust() calls

public:
	/// Default constructor.
	status_bar() = default;

	/// Move constructor.
	status_bar(status_bar&&) = default;

	/// Move assignment operator.
	status_bar& operator=(status_bar&&) = default;

	/// Calls CreateWindowEx().
	/// Should be called during parent's WM_CREATE processing (or if dialog, WM_INITDIALOG).
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	/// @see https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-initdialog
	status_bar& create(const i_window* parent)
	{
		LONG_PTR parentStyle = GetWindowLongPtrW(parent->hwnd(), GWL_STYLE);
		bool canStretch = (parentStyle & WS_MAXIMIZEBOX) != 0
			|| (parentStyle & WS_SIZEBOX) != 0;

		HWND h = this->_base.create_window(parent, 0, STATUSCLASSNAMEW, {}, {0, 0}, {0, 0},
			WS_CHILD | WS_VISIBLE | (canStretch ? SBARS_SIZEGRIP : 0), 0);

		return *this;
	}

	/// Installs a window subclass and exposes methods to add message handlers.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/subclassing-overview
	[[nodiscard]] _wli::msg_proxy& on_subclass_msg() { return this->_base.on_subclass_msg(); }

	/// Returns the underlying HWND handle.
	[[nodiscard]] HWND hwnd() const noexcept override { return this->_base.hwnd(); }

	/// Adds a new part with fixed width.
	status_bar& add_fixed_part(UINT sizePixels)
	{
		this->_parts.push_back({sizePixels, 0});
		this->_rightEdges.emplace_back(0);

		this->resize_to_fit(msg::wm{
			SIZE_RESTORED,
			MAKELPARAM(this->_parent_cx(), 0)
		});
		return *this;
	}

	/// Adds a new resizable part.
	/// 
	/// How resizeWeight works:
	/// - Suppose you have 3 parts, respectively with weights of 1, 1 and 2.
	/// - If available client area is 400px, respective part widths will be 100, 100 and 200px.
	status_bar& add_resizable_part(UINT resizeWeight)
	{
		if (!resizeWeight) {
			throw std::invalid_argument("Resize weight of status bar part can't be zero.");
		}

		// Zero weight means a fixed-width part, which internally should have sizePixels set.
		this->_parts.push_back({0, resizeWeight});
		this->_rightEdges.emplace_back(0);

		this->resize_to_fit(msg::wm{
			SIZE_RESTORED,
			MAKELPARAM(this->_parent_cx(), 0)
		});
		return *this;
	}

	/// Sets the text of a part.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/sb-settext
	const status_bar& set_part_text(size_t partIndex, std::wstring_view text) const
	{
		if (!SendMessageW(this->hwnd(), SB_SETTEXT,
			MAKEWPARAM(MAKEWORD(partIndex, 0), 0),
			reinterpret_cast<LPARAM>(text.data()) ))
		{
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"SB_SETTEXT failed for \"%s\" at %d.", text, partIndex) ));
		}
		return *this;
	}

	/// Retrieves the text of a part.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/sb-gettext
	[[nodiscard]] std::wstring part_text(size_t partIndex) const noexcept
	{
		WORD len = LOWORD(SendMessageW(this->hwnd(), SB_GETTEXTLENGTHW, partIndex, 0));

		std::wstring buf;
		if (len) {
			buf.resize(static_cast<size_t>(len) + 1, L'\0');
			SendMessageW(this->hwnd(), SB_GETTEXT, partIndex,
				reinterpret_cast<LPARAM>(&buf[0]) );
			buf.resize(len);
		}
		return buf;
	}

	// Sets the icon; pass nullptr to clear.
	// You must destroy the icon manually after use.
	const status_bar& set_part_icon(size_t partIndex, HICON hIcon) const
	{
		if (!SendMessageW(this->hwnd(), SB_SETICON,
			partIndex, reinterpret_cast<LPARAM>(hIcon) ))
		{
			throw std::runtime_error(
				str::unicode_to_ansi(
					str::format(L"SB_SETICON failed at %d.", partIndex) ));
		}
		return *this;
	}

	/// Resizes the status bar to fill the available width on parent window.
	/// Intended to be called with parent's WM_SIZE processing.
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-size
	status_bar& resize_to_fit(msg::wm_size p) noexcept
	{
		if (!p.is_minimized() && this->hwnd()) {
			int cx = p.client_area_size().cx; // available width
			SendMessageW(this->hwnd(), WM_SIZE, 0, 0); // tell statusbar to fit parent

			// Find the space to be divided among variable-width parts,
			// and total weight of variable-width parts.
			UINT totalWeight = 0;
			int  cxVariable = cx;
			for (const part& onePart : this->_parts) {
				if (!onePart.resizeWeight) { // fixed-width?
					cxVariable -= onePart.sizePixels;
				} else {
					totalWeight += onePart.resizeWeight;
				}
			}

			// Fill right edges array with the right edge of each part.
			int cxTotal = cx;
			for (size_t i = this->_parts.size(); i-- > 0; ) {
				this->_rightEdges[i] = cxTotal;
				cxTotal -= (!this->_parts[i].resizeWeight) ? // fixed-width?
					this->_parts[i].sizePixels :
					static_cast<int>( (cxVariable / totalWeight) * this->_parts[i].resizeWeight );
			}
			SendMessageW(this->hwnd(), SB_SETPARTS,
				this->_rightEdges.size(),
				reinterpret_cast<LPARAM>(&this->_rightEdges[0]) );
		}

		return *this;
	}

private:
	// Returns the width of parent client area.
	// Cached, since parts are intended to be added during window creation only,
	// and you can have only 1 status bar per window.
	[[nodiscard]] int _parent_cx() noexcept
	{
		static int cx = 0;
		if (!cx && this->hwnd()) {
			RECT rc{};
			GetClientRect(GetParent(this->hwnd()), &rc);
			cx = rc.right;
		}
		return cx;
	}
};

}//namespace wl