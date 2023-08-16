/**
 * Copyright (c) 2019-2021 Paul-Louis Ageneau
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef RTC_UTILS_H
#define RTC_UTILS_H

#include <functional>
#include <memory>
#include <mutex>

#if defined(HAVE_CXX17_OPTIONAL)
#include <optional>
#else
#include "tl/optional.hpp"
#endif

#include <tuple>
#include <utility>

namespace rtc {

#if defined(HAVE_CXX17_OPTIONAL)
using std::optional;
#else
using tl::optional;
#endif

// https://stackoverflow.com/a/32476942
template <class... Fs>
struct overloaded;

template <class F0, class... Frest>
struct overloaded<F0, Frest...> : F0, overloaded<Frest...>
{
    overloaded(F0 f0, Frest... rest) : F0(f0), overloaded<Frest...>(rest...) {}

    using F0::operator();
    using overloaded<Frest...>::operator();
};

template <class F0>
struct overloaded<F0> : F0
{
    overloaded(F0 f0) : F0(f0) {}

    using F0::operator();
};

template <class... Fs>
auto make_overloaded(Fs... fs)
{
    return overloaded<Fs...>(fs...);
}

template<int ...>
struct seq { };

template<int N, int ...S>
struct gens : gens<N-1, N-1, S...> { };

template<int ...S>
struct gens<0, S...> {
  typedef seq<S...> type;
};

#if defined(HAVE_CXX17_WEAK_FROM_THIS)
template <class C> std::weak_ptr<C> weak_from_this(C *c) { return c->weak_from_this(); }
#else
template <class C> std::weak_ptr<C> weak_from_this(C *c) {
	try {
		return c->shared_from_this();
	} catch (std::bad_weak_ptr const&) {
		return std::weak_ptr<C>();
	}
}
#endif

// weak_ptr bind helper
template <typename F, typename T, typename... Args> auto weak_bind(F &&f, T *t, Args &&..._args) {
	return [bound = std::bind(f, t, _args...), weak_this = rtc::weak_from_this(t)](auto &&...args) {
		if (auto shared_this = weak_this.lock())
			return bound(args...);
		else
			return static_cast<decltype(bound(args...))>(false);
	};
}


// scope_guard helper
class scope_guard final {
public:
	scope_guard(std::function<void()> func) : function(std::move(func)) {}
	scope_guard(scope_guard &&other) = delete;
	scope_guard(const scope_guard &) = delete;
	void operator=(const scope_guard &) = delete;

	~scope_guard() {
		if (function)
			function();
	}

private:
	std::function<void()> function;
};

// callback with built-in synchronization
template <typename... Args> class synchronized_callback {
public:
	synchronized_callback() = default;
	synchronized_callback(synchronized_callback &&cb) { *this = std::move(cb); }
	synchronized_callback(const synchronized_callback &cb) { *this = cb; }
	synchronized_callback(std::function<void(Args...)> func) { *this = std::move(func); }
	virtual ~synchronized_callback() { *this = nullptr; }

	synchronized_callback &operator=(synchronized_callback &&cb) {
		std::lock(mutex, cb.mutex);
        std::lock_guard<std::recursive_mutex> lk1(mutex, std::adopt_lock);
        std::lock_guard<std::recursive_mutex> lk2(cb.mutex, std::adopt_lock);
		//std::scoped_lock lock(mutex, cb.mutex);
		set(std::exchange(cb.callback, nullptr));
		return *this;
	}

	synchronized_callback &operator=(const synchronized_callback &cb) {
		std::lock(mutex, cb.mutex);
        std::lock_guard<std::recursive_mutex> lk1(mutex, std::adopt_lock);
        std::lock_guard<std::recursive_mutex> lk2(cb.mutex, std::adopt_lock);
		//std::scoped_lock lock(mutex, cb.mutex);
		set(cb.callback);
		return *this;
	}

	synchronized_callback &operator=(std::function<void(Args...)> func) {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		set(std::move(func));
		return *this;
	}

	bool operator()(Args... args) const {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return call(std::move(args)...);
	}

	operator bool() const {
		std::lock_guard<std::recursive_mutex> lock(mutex);
		return callback ? true : false;
	}

protected:
	virtual void set(std::function<void(Args...)> func) { callback = std::move(func); }
	virtual bool call(Args... args) const {
		if (!callback)
			return false;

		callback(std::move(args)...);
		return true;
	}

	std::function<void(Args...)> callback;
	mutable std::recursive_mutex mutex;
};

// callback with built-in synchronization and replay of the last missed call
template <typename... Args>
class synchronized_stored_callback final : public synchronized_callback<Args...> {
public:
	template <typename... CArgs>
	synchronized_stored_callback(CArgs &&...cargs)
	    : synchronized_callback<Args...>(std::forward<CArgs>(cargs)...) {}
	~synchronized_stored_callback() {}

private:
	void set(std::function<void(Args...)> func) {
		synchronized_callback<Args...>::set(func);
		if (func && stored) {
			func_dispatch(func);
			stored.reset();
		}
	}

	void func_dispatch(std::function<void(Args...)> func) {
    	call_func(typename gens<sizeof...(Args)>::type(), func);
  	}

	template<int ...S>
  	void call_func(seq<S...>, std::function<void(Args...)> func) {
    	func(std::get<S>(*stored) ...);
  	}

	bool call(Args... args) const {
		if (!synchronized_callback<Args...>::call(args...))
			stored.emplace(std::move(args)...);

		return true;
	}

	mutable optional<std::tuple<Args...>> stored;
};

// pimpl base class
template <typename T> using impl_ptr = std::shared_ptr<T>;
template <typename T> class CheshireCat {
public:
	CheshireCat(impl_ptr<T> impl) : mImpl(std::move(impl)) {}
	template <typename... Args>
	CheshireCat(Args... args) : mImpl(std::make_shared<T>(std::forward<Args>(args)...)) {}
	CheshireCat(CheshireCat<T> &&cc) { *this = std::move(cc); }
	CheshireCat(const CheshireCat<T> &) = delete;

	virtual ~CheshireCat() = default;

	CheshireCat &operator=(CheshireCat<T> &&cc) {
		mImpl = std::move(cc.mImpl);
		return *this;
	};
	CheshireCat &operator=(const CheshireCat<T> &) = delete;

protected:
	impl_ptr<T> impl() { return mImpl; }
	impl_ptr<const T> impl() const { return mImpl; }

private:
	impl_ptr<T> mImpl;
};

} // namespace rtc

#endif
