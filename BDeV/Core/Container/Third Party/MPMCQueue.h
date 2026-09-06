/*
Copyright (c) 2020 Erik Rigtorp <erik@rigtorp.se>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */


// Some modifications made by myself (Bruno)

#pragma once


#include "BDeV/Core/System/Memory/BvMemory.h"
#include <atomic>


namespace rigtorp {
	namespace mpmc {
		template <typename T> struct Slot {
			~Slot() noexcept {
				if (turn & 1) {
					destroy();
				}
			}

			template <typename... Args> void construct(Args &&...args) noexcept {
				static_assert(std::is_nothrow_constructible<T, Args &&...>::value,
					"T must be nothrow constructible with Args&&...");
				new (storage) T(std::forward<Args>(args)...);
			}

			void destroy() noexcept {
				static_assert(std::is_nothrow_destructible<T>::value,
					"T must be nothrow destructible");
				reinterpret_cast<T*>(storage)->~T();
			}

			T&& move() noexcept {
				return std::move(*reinterpret_cast<T*>(storage));
			}

			// Align to avoid false sharing between adjacent slots
			alignas(kCacheLineSize) std::atomic<size_t> turn = { 0 };
			alignas(T) u8 storage[sizeof(T)];
		};

		template <typename T>
		class Queue {
		private:
			static_assert(std::is_nothrow_copy_assignable<T>::value ||
				std::is_nothrow_move_assignable<T>::value,
				"T must be nothrow copy or move assignable");

			static_assert(std::is_nothrow_destructible<T>::value,
				"T must be nothrow destructible");

		public:
			Queue()
				: head_(0), tail_(0), pArena_(BV_DEFAULT_MEMORY_ARENA), slots_(nullptr)
			{
			}

			explicit Queue(const size_t capacity,
				IBvMemoryArena* pArena = BV_DEFAULT_MEMORY_ARENA)
				: capacity_(capacity), pArena_(pArena), head_(0), tail_(0) {
				init(capacity);
			}

			~Queue() noexcept {
				if (slots_)
				{
					for (size_t i = 0; i < capacity_; ++i) {
						slots_[i].~Slot();
					}
					pArena_->Free(slots_);
					slots_ = nullptr;
				}
			}

			void init(const size_t capacity)
			{
				capacity_ = capacity;

				if (capacity_ < 1) {
					throw std::invalid_argument("capacity < 1");
				}
				// Allocate one extra slot to prevent false sharing on the last slot
				slots_ = reinterpret_cast<Slot<T>*>(pArena_->Allocate(sizeof(Slot<T>) * (capacity_ + 1), alignof(Slot<T>)));
				// Allocators are not required to honor alignment for over-aligned types
				// (see http://eel.is/c++draft/allocator.requirements#10) so we verify
				// alignment here
				if (reinterpret_cast<size_t>(slots_) % alignof(Slot<T>) != 0) {
					pArena_->Free(slots_);
					throw std::bad_alloc();
				}
				for (size_t i = 0; i < capacity_; ++i) {
					new (&slots_[i]) Slot<T>();
				}
				static_assert(
					alignof(Slot<T>) == kCacheLineSize,
					"Slot must be aligned to cache line boundary to prevent false sharing");
				static_assert(sizeof(Slot<T>) % kCacheLineSize == 0,
					"Slot size must be a multiple of cache line size to prevent "
					"false sharing between adjacent slots");
				static_assert(sizeof(Queue) % kCacheLineSize == 0,
					"Queue size must be a multiple of cache line size to "
					"prevent false sharing between adjacent queues");
				static_assert(
					offsetof(Queue, tail_) - offsetof(Queue, head_) ==
					static_cast<std::ptrdiff_t>(kCacheLineSize),
					"head and tail must be a cache line apart to prevent false sharing");
			}

			// non-copyable and non-movable
			Queue(const Queue&) = delete;
			Queue& operator=(const Queue&) = delete;

			template <typename... Args> void emplace(Args &&...args) noexcept {
				static_assert(std::is_nothrow_constructible<T, Args &&...>::value,
					"T must be nothrow constructible with Args&&...");
				auto const head = head_.fetch_add(1);
				auto& slot = slots_[idx(head)];
				while (turn(head) * 2 != slot.turn.load(std::memory_order_acquire))
					;
				slot.construct(std::forward<Args>(args)...);
				slot.turn.store(turn(head) * 2 + 1, std::memory_order_release);
			}

			template <typename... Args> bool try_emplace(Args &&...args) noexcept {
				static_assert(std::is_nothrow_constructible<T, Args &&...>::value,
					"T must be nothrow constructible with Args&&...");
				auto head = head_.load(std::memory_order_acquire);
				for (;;) {
					auto& slot = slots_[idx(head)];
					if (turn(head) * 2 == slot.turn.load(std::memory_order_acquire)) {
						if (head_.compare_exchange_strong(head, head + 1)) {
							slot.construct(std::forward<Args>(args)...);
							slot.turn.store(turn(head) * 2 + 1, std::memory_order_release);
							return true;
						}
					}
					else {
						auto const prevHead = head;
						head = head_.load(std::memory_order_acquire);
						if (head == prevHead) {
							return false;
						}
					}
				}
			}

			void push(const T& v) noexcept {
				static_assert(std::is_nothrow_copy_constructible<T>::value,
					"T must be nothrow copy constructible");
				emplace(v);
			}

			template <typename P,
				typename = typename std::enable_if<
				std::is_nothrow_constructible<T, P&&>::value>::type>
			void push(P&& v) noexcept {
				emplace(std::forward<P>(v));
			}

			bool try_push(const T& v) noexcept {
				static_assert(std::is_nothrow_copy_constructible<T>::value,
					"T must be nothrow copy constructible");
				return try_emplace(v);
			}

			template <typename P,
				typename = typename std::enable_if<
				std::is_nothrow_constructible<T, P&&>::value>::type>
			bool try_push(P&& v) noexcept {
				return try_emplace(std::forward<P>(v));
			}

			void pop(T& v) noexcept {
				auto const tail = tail_.fetch_add(1);
				auto& slot = slots_[idx(tail)];
				while (turn(tail) * 2 + 1 != slot.turn.load(std::memory_order_acquire))
					;
				v = slot.move();
				slot.destroy();
				slot.turn.store(turn(tail) * 2 + 2, std::memory_order_release);
			}

			bool try_pop(T& v) noexcept {
				auto tail = tail_.load(std::memory_order_acquire);
				for (;;) {
					auto& slot = slots_[idx(tail)];
					if (turn(tail) * 2 + 1 == slot.turn.load(std::memory_order_acquire)) {
						if (tail_.compare_exchange_strong(tail, tail + 1)) {
							v = slot.move();
							slot.destroy();
							slot.turn.store(turn(tail) * 2 + 2, std::memory_order_release);
							return true;
						}
					}
					else {
						auto const prevTail = tail;
						tail = tail_.load(std::memory_order_acquire);
						if (tail == prevTail) {
							return false;
						}
					}
				}
			}

			/// Returns the number of elements in the queue.
			/// The size can be negative when the queue is empty and there is at least one
			/// reader waiting. Since this is a concurrent queue the size is only a best
			/// effort guess until all reader and writer threads have been joined.
			ptrdiff_t size() const noexcept {
				// TODO: How can we deal with wrapped queue on 32bit?
				return static_cast<ptrdiff_t>(head_.load(std::memory_order_relaxed) -
					tail_.load(std::memory_order_relaxed));
			}

			/// Returns true if the queue is empty.
			/// Since this is a concurrent queue this is only a best effort guess
			/// until all reader and writer threads have been joined.
			bool empty() const noexcept { return size() <= 0; }

		private:
			constexpr size_t idx(size_t i) const noexcept { return i % capacity_; }

			constexpr size_t turn(size_t i) const noexcept { return i / capacity_; }

		private:
			size_t capacity_;
			Slot<T>* slots_;
			IBvMemoryArena* pArena_;

			// Align to avoid false sharing between head_ and tail_
			alignas(kCacheLineSize) std::atomic<size_t> head_;
			alignas(kCacheLineSize) std::atomic<size_t> tail_;
		};
	} // namespace mpmc

	template <typename T>
	using MPMCQueue = mpmc::Queue<T>;
} // namespace rigtorp