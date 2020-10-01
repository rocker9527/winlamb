/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <Windows.h>

namespace wl {

/// Abstract class, base to all windows.
class i_window {
public:
	virtual ~i_window() { }

	/// Returns the underlying HWND handle.
	[[nodiscard]] virtual HWND hwnd() const noexcept = 0;
};

/// Abstract class, base to all controls.
class i_control : public i_window {
public:
	virtual ~i_control() { }

	/// Retrieves the control ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdlgctrlid
	[[nodiscard]] virtual int id() const noexcept = 0;
};

}//namespace wl