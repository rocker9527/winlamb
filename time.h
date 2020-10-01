/*
 * Part of WinLamb - Win32 API Lambda Library
 * https://github.com/rodrigocfd/winlamb
 * This library is released under the MIT License.
 */

#pragma once
#include <chrono>
#include <system_error>
#include <Windows.h>

/// Utilities for std::chrono.
namespace wl::time {

/// Converts chrono::system_clock::time_point to FILETIME.
[[nodiscard]] inline FILETIME
	timepoint_to_filetime(std::chrono::system_clock::time_point tp) noexcept
{
	// 100-nanoseconds since Jan 1, 1601 UTC.
	// https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
	LONGLONG nanos =
		std::chrono::time_point_cast<std::chrono::seconds>(tp)
			.time_since_epoch().count()
			* 10'000'000 + 116'444'736'000'000'000;

	FILETIME ft{};
	ft.dwLowDateTime = static_cast<DWORD>(nanos);
	ft.dwHighDateTime = nanos >> 32;
	return ft;
}

/// Converts chrono::system_clock::time_point to SYSTEMTIME.
[[nodiscard]] inline SYSTEMTIME
	timepoint_to_systemtime(std::chrono::system_clock::time_point tp)
{
	FILETIME ft = timepoint_to_filetime(tp);

	SYSTEMTIME st{};
	if (!FileTimeToSystemTime(&ft, &st)) {
		throw std::system_error(GetLastError(), std::system_category(),
			"FileTimeToSystemTime failed.");
	}
	return st;
}

/// Converts FILETIME to chrono::system_clock::time_point.
[[nodiscard]] inline std::chrono::system_clock::time_point
	filetime_to_timepoint(const FILETIME& ft) noexcept
{
	// 100-nanoseconds since Jan 1, 1601 UTC.
	// https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
	LONGLONG nanos = ft.dwLowDateTime | static_cast<LONGLONG>(ft.dwHighDateTime) << 32;

	std::chrono::time_point<std::chrono::system_clock> tp{
		std::chrono::seconds{
			(nanos - 116'444'736'000'000'000) / 10'000'000
		}
	};
	return tp;
}

/// Converts SYSTEMTIME to chrono::system_clock::time_point.
[[nodiscard]] inline std::chrono::system_clock::time_point
	systemtime_to_timepoint(const SYSTEMTIME& st)
{
	FILETIME ft{};
	if (!SystemTimeToFileTime(&st, &ft)) {
		throw std::system_error(GetLastError(), std::system_category(),
			"SystemTimeToFileTime failed.");
	}
	return filetime_to_timepoint(ft);
}

/// Converts the timezone of a chrono::system_clock::timepoint, from UTC to local time.
/// @see https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-gettimezoneinformation
/// @see https://docs.microsoft.com/en-us/windows/win32/api/timezoneapi/nf-timezoneapi-systemtimetotzspecificlocaltime
[[nodiscard]] inline std::chrono::system_clock::time_point
	utc_to_local(std::chrono::system_clock::time_point tp)
{
	SYSTEMTIME st = timepoint_to_systemtime(tp);

	TIME_ZONE_INFORMATION tzi{};
	if (GetTimeZoneInformation(&tzi) == TIME_ZONE_ID_INVALID) {
		throw std::system_error(GetLastError(), std::system_category(),
			"GetTimeZoneInformation failed.");
	}

	if (!SystemTimeToTzSpecificLocalTime(&tzi, &st, &st)) {
		throw std::system_error(GetLastError(), std::system_category(),
			"SystemTimeToTzSpecificLocalTime.");
	}

	return systemtime_to_timepoint(st);
}

}//namespace wl::time