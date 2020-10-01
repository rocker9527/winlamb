/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <Windows.h>
#include "internals/base_dialog.h"
#include "internals/control_visuals.h"
#include "internals/interfaces.h"

namespace wl {

/// Custom dialog-based user control.
/// Allows message and notification handling.
///
/// You must set these styles in the resource editor:
/// - Border: none
/// - Control: true
/// - Style: child
/// - Visible: true (otherwise will start invisible)
/// - Client Edge: true (only if you want a border)
///
/// Default handled messages:
/// - WM_NCPAINT
class dialog_control : public i_control {
public:
	/// Setup options for dialog_control.
	struct setup_opts final {
		/// Resource dialog ID, must be set.
		/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createdialogparamw
		int dialog_id = 0;
	};

private:
	setup_opts _setup;
	_wli::base_dialog _base;

public:
	/// Default constructor.
	dialog_control()
	{
		this->_default_msg_handlers();
	}

	/// Move constructor.
	dialog_control(dialog_control&&) = default;

	/// Move assignment operator.
	dialog_control& operator=(dialog_control&&) = default;

	/// Exposes variables that will be used during dialog creation.
	[[nodiscard]] setup_opts& setup()
	{
		if (this->hwnd()) {
			throw std::logic_error("Cannot call setup() after dialog_control is created.");
		}
		return this->_setup;
	}

	/// Creates the dialog control.
	/// Should be called during parent's WM_CREATE processing (or if dialog, WM_INITDIALOG).
	/// A size of {0, 0} will use the size defined in the dialog resource.
	/// Position and size will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createdialogparamw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	/// @see https://docs.microsoft.com/en-us/windows/win32/dlgbox/wm-initdialog
	void create(const i_window* parent, int id, POINT pos, SIZE size = {0, 0})
	{
		if (!parent) {
			throw std::invalid_argument("No parent passed to dialog_control::create().");
		}

		HINSTANCE hInst = reinterpret_cast<HINSTANCE>(
			GetWindowLongPtrW(parent->hwnd(), GWLP_HINSTANCE));
		this->_base.create_dialog_param(hInst, parent, this->_setup.dialog_id);

		SetWindowLongPtrW(this->hwnd(), GWLP_ID, id); // so the control has an ID

		bool useResourceSize = !size.cx && !size.cy;
		SetWindowPos(this->hwnd(), nullptr, pos.x, pos.y, size.cx, size.cy,
			SWP_NOZORDER | (useResourceSize ? SWP_NOSIZE : 0));
	}

	/// Exposes methods to add message handlers.
	[[nodiscard]] _wli::msg_proxy_all& on_msg() { return this->_base.on_msg(); }

	/// Executes a function asynchronously, in a new detached background thread.
	/// @tparam F std::function<void()>
	/// @param func [](ui_work ui) {}
	template<typename F>
	void background_work(F&& func) { this->_base.background_work(std::move(func)); }

	/// Returns the underlying HWND handle.
	[[nodiscard]] HWND hwnd() const noexcept override { return this->_base.hwnd(); }

	/// Retrieves the control ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdlgctrlid
	[[nodiscard]] int id() const noexcept override { return GetDlgCtrlID(this->hwnd()); }

private:
	void _default_msg_handlers()
	{
		this->on_msg().wm_nc_paint([this](msg::wm_nc_paint p) noexcept
		{
			_wli::paint_control_borders(this->hwnd(), p.wparam, p.lparam);
		});
	}
};

}//namespace wl