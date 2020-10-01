/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <system_error>
#include <type_traits>
#include <utility>
#include <Windows.h>
#include <objbase.h>

namespace wl::com {

/// Manages a COM pointer.
/// Created with com::co_create_instance().
template<typename T>
class ptr final {
private:
	T* _ptr = nullptr;

public:
	/// Destructor. Calls release().
	~ptr() { this->release(); }

	/// Default constructor.
	ptr() = default;

	/// Move constructor.
	ptr(ptr&& other) noexcept { this->operator=(std::move(other)); }

	/// Allows calling methods directly from the underlying pointer.
	[[nodiscard]] const T* operator->() const noexcept { return this->_ptr; }
	/// Allows calling methods directly from the underlying pointer.
	[[nodiscard]] T*       operator->() noexcept       { return this->_ptr; }

	/// Tells if the underlying pointer is null.
	[[nodiscard]] bool empty() const noexcept { return this->_ptr == nullptr; }

	/// Returns the raw pointer to the COM interface pointer.
	[[nodiscard]] T** raw_pptr() noexcept { return &this->_ptr; }

	/// Move assignment operator.
	ptr& operator=(ptr&& other) noexcept
	{
		this->release();
		std::swap(this->_ptr, other._ptr);
		return *this;
	}

	/// Returns a safe clone of the COM pointer with AddRef().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-addref
	[[nodiscard]] ptr clone() const noexcept
	{
		this->_ptr->AddRef();

		ptr clonedObj{};
		clonedObj._ptr = this->_ptr;
		return clonedObj;
	}

	/// Calls IUnknown::QueryInterface() with the given REFIID.
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-queryinterface(refiid_void)
	template<typename Q,
		typename = std::enable_if_t<
			std::is_base_of_v<IUnknown, Q>
		>>
	[[nodiscard]] ptr<Q> query_interface(REFIID iid_any)
	{
		ptr<Q> ptrBuf;
		HRESULT hr = this->_ptr->QueryInterface(iid_any,
			reinterpret_cast<void**>(ptrBuf.raw_pptr()));
		if (FAILED(hr)) {
			throw std::system_error(hr, std::system_category(),
				"QueryInterface failed.");
		}
		return ptrBuf;
	}

	/// Calls IUnknown::QueryInterface() with IID_PPV_ARGS().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-queryinterface(q)
	template<typename Q,
		typename = std::enable_if_t<
			std::is_base_of_v<IUnknown, Q>
		>>
	[[nodiscard]] ptr<Q> query_interface()
	{
		ptr<Q> ptrBuf;
		HRESULT hr = this->_ptr->QueryInterface(IID_PPV_ARGS(ptrBuf.raw_pptr()));
		if (FAILED(hr)) {
			throw std::system_error(hr, std::system_category(),
				"QueryInterface failed.");
		}
		return ptrBuf;
	}

	/// Calls IUnknown::Release().
	/// @see https://docs.microsoft.com/en-us/windows/win32/api/unknwn/nf-unknwn-iunknown-release
	void release() noexcept
	{
		if (this->_ptr) {
			this->_ptr->Release();
			this->_ptr = nullptr;
		}
	}
};

/// Creates a COM object by calling CoCreateInstance() with the given REFIID.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
template<typename T,
	typename = std::enable_if_t<
		std::is_base_of_v<IUnknown, T>
	>>
[[nodiscard]] inline ptr<T> co_create_instance(
	REFCLSID clsid_any, REFIID iid_any, DWORD clsctxContext = CLSCTX_INPROC_SERVER)
{
	ptr<T> ptrBuf;
	HRESULT hr = CoCreateInstance(clsid_any, nullptr,
		clsctxContext, iid_any, reinterpret_cast<LPVOID*>(ptrBuf.raw_pptr()));
	if (FAILED(hr)) {
		throw std::system_error(hr, std::system_category(),
			"CoCreateInstance failed.");
	}
	return ptrBuf;
}

/// Creates a COM object by calling CoCreateInstance() with IID_PPV_ARGS().
/// @see https://docs.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cocreateinstance
template<typename T,
	typename = std::enable_if_t<
		std::is_base_of_v<IUnknown, T>
	>>
[[nodiscard]] inline ptr<T> co_create_instance(
	REFCLSID clsid_any, DWORD clsctxContext = CLSCTX_INPROC_SERVER)
{
	ptr<T> ptrBuf;
	HRESULT hr = CoCreateInstance(clsid_any, nullptr,
		clsctxContext, IID_PPV_ARGS(ptrBuf.raw_pptr()));
	if (FAILED(hr)) {
		throw std::system_error(hr, std::system_category(),
			"CoCreateInstance failed.");
	}
	return ptrBuf;
}

}//namespace wl::com