/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <stdexcept>
#include <string_view>
#include <Windows.h>
#include <OleAuto.h>
#include "../str.h"

namespace wl::com {

/// Manages a COM BSTR string.
/// @see https://docs.microsoft.com/en-us/previous-versions/windows/desktop/automat/bstr
class bstr final {
private:
	BSTR _bstr = nullptr;

public:
	/// Destructor. Calls free().
	~bstr() { this->free(); }

	/// Default constructor.
	bstr() = default;

	/// Move constructor.
	bstr(bstr&& other) noexcept { this->operator=(std::move(other)); }

	/// Constructor.
	/// Creates a copy of the string with SysAllocString().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/oleauto/nf-oleauto-sysallocstring
	bstr(std::wstring_view s) { this->operator=(s); }

	/// Automatic conversion to BSTR& type.
	[[nodiscard]] operator const BSTR&() const noexcept { return this->_bstr; }

	/// Returns the underlying BSTR pointer.
	[[nodiscard]] const BSTR* operator&() const noexcept { return &this->_bstr; }
	/// Returns the underlying BSTR pointer.
	[[nodiscard]] BSTR*       operator&() noexcept       { return &this->_bstr; }

	/// Move assignment operator.
	bstr& operator=(bstr&& other) noexcept
	{
		this->free();
		std::swap(this->_bstr, other._bstr);
		return *this;
	}

	/// Assignment operator.
	/// Creates a copy of the string with SysAllocString().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/oleauto/nf-oleauto-sysallocstring
	bstr& operator=(std::wstring_view s)
	{
		this->free();

		if (!s.empty()) {
			this->_bstr = SysAllocString(s.data());

			if (!this->_bstr) {
				throw std::runtime_error(
					str::unicode_to_ansi(
						str::format(L"SysAllocString failed \"%s\".", s) ));
			}
		}
		
		return *this;
	}

	/// Converts the BSTR into const wchar_t*.
	[[nodiscard]] const wchar_t* c_str() const noexcept { return static_cast<const wchar_t*>(this->_bstr); }

	/// Calls SysFreeString().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/oleauto/nf-oleauto-sysfreestring
	void free() noexcept
	{
		if (this->_bstr) {
			SysFreeString(this->_bstr);
			this->_bstr = nullptr;
		}
	}
};

}//namespace wl::com