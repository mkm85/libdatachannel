/**
 * Copyright (c) 2019 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RTC_COMMON_H
#define RTC_COMMON_H

#ifdef RTC_STATIC
#define RTC_CPP_EXPORT
#else // dynamic library
#ifdef _WIN32
#ifdef RTC_EXPORTS
#define RTC_CPP_EXPORT __declspec(dllexport) // building the library
#else
#define RTC_CPP_EXPORT __declspec(dllimport) // using the library
#endif
#else // not WIN32
#define RTC_CPP_EXPORT
#endif
#endif

#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0602 // Windows 8
#endif
#ifdef _MSC_VER
#pragma warning(disable : 4251) // disable "X needs to have dll-interface..."
#endif
#endif

#ifndef RTC_ENABLE_WEBSOCKET
#define RTC_ENABLE_WEBSOCKET 1
#endif

#ifndef RTC_ENABLE_MEDIA
#define RTC_ENABLE_MEDIA 1
#endif

#include "rtc.h" // for C API defines

#include "utils.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>

#if defined(HAVE_CXX17_OPTIONAL)
#include <optional>
#else
#include "tl/optional.hpp"
#endif

#include <string>

#if defined(HAVE_CXX17_STRING_VIEW)
#include <string_view>
#else
#include "nonstd/string_view.hpp"
#endif

#if defined(HAVE_CXX17_VARIANT)
#include <variant>
#else
#include "nonstd/variant.hpp"
#endif

#if defined(HAVE_CXX17_BYTE)
#include <cstddef>
#else
#include "nonstd/byte.hpp"
#endif


#include <vector>

namespace rtc {

#if defined(HAVE_CXX17_BYTE)
using std::byte;
#else
using nonstd::byte;
#endif

#if defined(HAVE_CXX17_OPTIONAL)
using std::make_optional;
using std::nullopt;
using std::optional;
#else
using tl::nullopt;
using tl::optional;
using tl::make_optional;
#endif

using std::shared_ptr;
using std::string;

#if defined(HAVE_CXX17_STRING_VIEW)
using std::string_view;
#else
using nonstd::string_view;
#endif

using std::unique_ptr;

#if defined(HAVE_CXX17_VARIANT)
using std::variant;
using std::visit;
#else
using nonstd::variant;
using nonstd::visit;
#endif

#if defined(HAVE_CXX17_BYTE)
//namespace lib {
using std::to_integer;
//}
#else
//namespace lib {
using nonstd::to_integer;
//}
#endif

using std::weak_ptr;

using binary = std::vector<byte>;
using binary_ptr = shared_ptr<binary>;
using message_variant = variant<binary, string>;

using std::int16_t;
using std::int32_t;
using std::int64_t;
using std::int8_t;
using std::ptrdiff_t;
using std::size_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;
using std::uint8_t;

} // namespace rtc

#endif
