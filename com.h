/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <system_error>
#include <Windows.h>
#include <objbase.h>
#include "internals/com_bstr.h"
#include "internals/com_ptr.h"
#include "internals/com_variant.h"

/// Utility classes to COM library and objects.
namespace wl::com {

/// Automates CoInitialize() and CoUninitialize() calls with RAII.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/objbase/nf-objbase-coinitialize
/// @see https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize
class lib final {
private:
	bool _running = false;

public:
	/// When the COM library must be initialized.
	enum class init {
		/// CoInitialize() will be called in the constructor, right away.
		NOW,
		/// CoInitialize() won't be automatically called.
		/// You must manually call initialize() later.
		LATER
	};

	/// Destructor. Calls un_initialize().
	~lib() { this->un_initialize(); } // https://stackoverflow.com/q/47123650/6923555

	/// Move constructor.
	lib(lib&& other) noexcept { this->operator=(std::move(other)); }

	/// Constructor.
	/// @param when Defines if CoInitialize() will be called automatically in constructor.
	explicit lib(init when)
	{
		if (when == init::NOW) { // otherwise you must manually call initialize()
			this->initialize();
		}
	}

	/// Move assignment operator.
	lib& operator=(lib&& other) noexcept
	{
		this->un_initialize();
		std::swap(this->_running, other._running);
		return *this;
	}

	/// Calls CoInitialize(), can be carelessly called multiple times.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/objbase/nf-objbase-coinitialize
	void initialize()
	{
		if (!this->_running) {
			HRESULT hr = CoInitialize(nullptr);
			if (hr != S_OK && hr != S_FALSE) {
				throw std::system_error(GetLastError(), std::system_category(),
					"CoInitialize failed.");
			}
			this->_running = true;
		}
	}

	/// Calls CoUninitialize(), can be carelessly called multiple times.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-couninitialize
	void un_initialize() noexcept
	{
		if (this->_running) {
			CoUninitialize();
			this->_running = false;
		}
	}
};

}//namespace wl::com