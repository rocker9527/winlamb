/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <string_view>
#include <system_error>
#include <Windows.h>
#include <ShObjIdl.h>
#include "internals/base_native_control.h"
#include "internals/control_visuals.h"
#include "internals/interfaces.h"
#include "com.h"

namespace wl {

/// Native progress bar control.
/// Optionally reflects the progress in window taskbar.
/// @see https://docs.microsoft.com/en-us/windows/win32/controls/progress-bar-control
/// @see https://docs.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-itaskbarlist3
class progress_bar final : public i_control {
private:
	_wli::base_native_control _base;

	com::lib _comLib{com::lib::init::LATER};
	com::ptr<ITaskbarList3> _taskbar;

public:
	/// Defines if the window taskbar button will reflect the progress bar state.
	enum class taskbar {
		/// Progress operations will be reflected in the window taskbar button.
		REFLECT,
		/// Ordinary progress bar, don't reflect progress status in window taskbar button.
		DONT_USE
	};

	/// Possible progress states.
	enum class state {
		/// No progress is being shown; taskbar button shows nothing.
		NONE,
		/// Normal progress operation, green color.
		NORMAL,
		/// Paused progress state, yellow color.
		PAUSED,
		/// Error progress state, red color.
		ERR,
		/// Side-to-side marquee, waiting state.
		INDETERMINATE
	};

	/// Constructor.
	explicit progress_bar(taskbar reflectOnTaskbar = taskbar::DONT_USE)
	{
		if (reflectOnTaskbar == taskbar::REFLECT) {
			this->_comLib.initialize();
			this->_taskbar = com::co_create_instance<ITaskbarList3>(CLSID_TaskbarList);
		}
	}

	/// Move constructor.
	progress_bar(progress_bar&&) = default;

	/// Move assignment operator.
	progress_bar& operator=(progress_bar&&) = default;

	/// Calls CreateWindowEx(), range is 0-100 unless changed with set_max_val().
	/// Should be called during parent's WM_CREATE processing.
	/// Position and size will be adjusted to match current system DPI.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-createwindowexw
	/// @see https://docs.microsoft.com/en-us/windows/win32/winmsg/wm-create
	progress_bar& create(
		const i_window* parent, int id, POINT pos, SIZE size = {140, 21})
	{
		pos = _wli::multiply_dpi(pos);
		size = _wli::multiply_dpi(size);

		HWND h = this->_base.create_window(parent, id, PROGRESS_CLASSW, {},
			pos, size, WS_CHILD | WS_VISIBLE, 0);
		return *this;
	}

	/// Assigns this object to an existing control in a dialog window.
	progress_bar& assign(const i_window* parent, int ctrlId) { this->_base.assign(parent, ctrlId); return *this; }

	/// Installs a window subclass and exposes methods to add message handlers.
	[[nodiscard]] _wli::msg_proxy& on_subclass_msg() { return this->_base.on_subclass_msg(); }

	/// Returns the underlying HWND handle.
	[[nodiscard]] HWND hwnd() const noexcept override { return this->_base.hwnd(); }

	/// Retrieves the control ID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getdlgctrlid
	[[nodiscard]] int id() const noexcept override { return this->_base.id(); }

	/// Defines a new maximum position for the progress bar.
	/// Minimum position is always zero.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/pbm-setrange
	const progress_bar& set_max_pos(size_t maxPos) const noexcept
	{
		SendMessageW(this->hwnd(), PBM_SETRANGE, 0, MAKELPARAM(0, maxPos));
		return *this;
	}

	/// Retrieves the current maximum progress position.
	/// Min value is always zero.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/pbm-getrange
	[[nodiscard]] size_t max_pos() const noexcept { return SendMessageW(this->hwnd(), PBM_GETRANGE, FALSE, 0); }

	/// Sets the state to state::NORMAL, and updates current progress position.
	const progress_bar& set_pos(size_t posVal) const
	{
		this->set_state(state::NORMAL);

		SendMessageW(this->hwnd(), PBM_SETPOS, static_cast<WPARAM>(posVal), 0);

		if (!this->_taskbar.empty()) { // if reflecting on the taskbar
			HRESULT hr = const_cast<progress_bar*>(this)->_taskbar->
				SetProgressValue(GetAncestor(this->hwnd(), GA_ROOTOWNER),
					posVal, this->max_pos());
			if (FAILED(hr)) {
				throw std::system_error(GetLastError(), std::system_category(),
					"ITaskbarList3::SetProgressValue failed.");
			}
		}
		return *this;
	}

	/// Retrieves current progress position.
	/// @see https://docs.microsoft.com/en-us/windows/win32/controls/pbm-getpos
	[[nodiscard]] size_t pos() const noexcept { return SendMessageW(this->hwnd(), PBM_GETPOS, 0, 0); }

	/// Sets the current state of the progress bar.
	const progress_bar& set_state(state newState) const
	{
		this->_set_pb_marquee_style(newState == state::INDETERMINATE);

		switch (newState) {
		case state::NONE:
			SendMessageW(this->hwnd(), PBM_SETSTATE, PBST_NORMAL, 0);
			SendMessageW(this->hwnd(), PBM_SETPOS, 0, 0); // reset progress bar position
			this->_set_tb_state(TBPF_NOPROGRESS);
			break;

		case state::NORMAL:
			SendMessageW(this->hwnd(), PBM_SETSTATE, PBST_NORMAL, 0);
			this->_set_tb_state(TBPF_NORMAL);
			break;

		case state::PAUSED:
			SendMessageW(this->hwnd(), PBM_SETSTATE, PBST_PAUSED, 0);
			this->_set_tb_state(TBPF_PAUSED);
			break;

		case state::ERR:
			SendMessageW(this->hwnd(), PBM_SETSTATE, PBST_ERROR, 0);
			this->_set_tb_state(TBPF_ERROR);
			break;

		case state::INDETERMINATE:
			this->_set_tb_state(TBPF_INDETERMINATE);
		}
		return *this;
	}

private:
	void _set_pb_marquee_style(bool hasMarquee) const noexcept
	{
		// http://stackoverflow.com/a/23689663
		LONG_PTR curStyle = GetWindowLongPtrW(this->hwnd(), GWL_STYLE);

		if (hasMarquee && !(curStyle & PBS_MARQUEE)) {
			SetWindowLongPtrW(this->hwnd(), GWL_STYLE, curStyle | PBS_MARQUEE);
			SendMessageW(this->hwnd(), PBM_SETMARQUEE, TRUE, 0);
		}

		if (!hasMarquee && (curStyle & PBS_MARQUEE)) {
			SendMessageW(this->hwnd(), PBM_SETMARQUEE, FALSE, 0);
			SetWindowLongPtrW(this->hwnd(), GWL_STYLE, curStyle & ~PBS_MARQUEE);
		}
	}

	void _set_tb_state(TBPFLAG tbpfFlag) const
	{
		if (!this->_taskbar.empty()) { // if reflecting on the taskbar
			HRESULT hr = const_cast<progress_bar*>(this)->_taskbar->
				SetProgressState(GetAncestor(this->hwnd(), GA_ROOTOWNER), tbpfFlag);
			if (FAILED(hr)) {
				throw std::system_error(GetLastError(), std::system_category(),
					"ITaskbarList3::SetProgressState failed.");
			}
		}
	}
};

}//namespace wl