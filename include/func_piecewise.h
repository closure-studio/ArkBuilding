#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
#pragma once
#include "primitive_types.h"
#include "util.h"

namespace albc
{
	struct __attribute__((aligned(32))) PiecewiseDef
	{ // this struct indicates a segment of the piecewise function
	  // the segment is defined by the following equation:
	  // f(t) = mul * (base + acc * t) + extra
	  // base, acc, mul, and extra of a segment is aggregated by all the segments that are defined before it
	  // base = base_n + base_n-1 + ... + base_1 + base_0
	  // acc = acc_n + acc_n-1 + ... + acc_1 + acc_0
	  // mul = mul_n * mul_n-1 * ... * mul_1 * mul_0
	  // extra = extra_n + extra_n-1 + ... + extra_1 + extra_0

		double base_delta = 0;	// base_n - base_n-1
		double acc_delta = 0;	// acc_n - acc_n-1
		double mul = 1;			// mul_n / mul_n-1
		double extra_delta = 0; // extra_n - extra_n-1

		PiecewiseDef(const double base, const double acc, const double mul, const double extra) : base_delta(base),
																								  acc_delta(acc),
																								  mul(mul),
																								  extra_delta(extra)
		{
		}

		[[nodiscard]] constexpr auto IsEmpty() const -> bool
		{
			return fp_eq(base_delta, 0.) && fp_eq(acc_delta, 0.) && fp_eq(mul, 1.) && fp_eq(extra_delta, 0.);
		}
	};

	struct __attribute__((aligned(64))) PiecewiseIndex
	{
		double ts = 0.;
		PiecewiseDef def;
	};

	template <UInt32 N>
	struct PiecewiseMap
	{ // indicates all the segments of the piecewise function
	  // using a forward-linked list
		struct __attribute__((aligned(64))) Forward
		{
			PiecewiseIndex data{INFINITY, {0., 0., 1., 0.}};
			Forward *next = nullptr;
		};

		struct Iterator
		{
			explicit Iterator() : ptr()
			{
			}

			explicit Iterator(Forward *ptr) : ptr(ptr)
			{
			}

			Forward *ptr;

			[[nodiscard]] auto operator*() const noexcept -> PiecewiseIndex &
			{
				return ptr->data;
			}

			auto operator++() noexcept -> Iterator &
			{
				ptr = ptr->next;
				return *this;
			}

			Iterator operator++(int) noexcept
			{
				Iterator tmp = *this;
				ptr = ptr->next;
				return tmp;
			}

			auto operator==(const Iterator &rhs) const noexcept -> bool
			{
				return ptr == rhs.ptr;
			}

			auto operator!=(const Iterator &rhs) const noexcept -> bool
			{
				return !(*this == rhs);
			}

			[[nodiscard]] constexpr auto HasNext() const noexcept -> bool
			{
				return ptr->next != nullptr;
			}

			[[nodiscard]] constexpr auto IsEnd() const noexcept -> bool
			{
				return ptr == nullptr;
			}
		};

		Forward f_list[N + 1U];	   // nodes of the forward-linked list
		Forward *before_begin_ptr; // before-begin node
		UInt32 n = 0;			   // number of segments, excluding the before-begin node

		PiecewiseMap()
		{
			for (auto &fwd : f_list)
			{ // initialize the forward-linked list
				fwd.next = nullptr;
			}
			before_begin_ptr = std::addressof(f_list[0]); // before-begin node
		}

		[[nodiscard]] constexpr auto empty() const -> bool
		{
			return n <= 0;
		}

//		constexpr void clear()
//		{
//			f_list[0].next = nullptr;
//			n = 0;
//		}

		[[nodiscard]] constexpr auto begin() const -> Iterator
		{
			return Iterator{f_list[0].next};
		}

		[[nodiscard]] constexpr auto before_begin() const -> Iterator
		{
			return Iterator{before_begin_ptr};
		}

		[[nodiscard]] constexpr auto end() const -> Iterator
		{
			return Iterator{nullptr};
		}

		constexpr void Insert(const double ts, const double base, const double acc, const double mul,
							  const double extra)
			__attribute__((always_inline))
		{								  // insert a new segment
            bool do_append = true;
			Iterator it = before_begin(); // before-begin node
            Forward* cur_seg = it.ptr;
			for (it; it.HasNext(); ++it)
			{
                cur_seg = it.ptr;
                if (fp_eq(cur_seg->data.ts, ts))
				{ // if the segment with the same ts exists, merge the existing segment with the new one
                    do_append = false;
                    break;
				}else if (ts > cur_seg->data.ts)
				{ // found insertion point
					break;
				}
			}

            if(do_append) {
                assert(n < N + 1); // prevent overflow
                cur_seg = &f_list[++n];
                cur_seg->next = it.ptr->next;
                it.ptr->next = cur_seg;
            }

            cur_seg->data.ts = ts;
            // PiecewiseDef为一次性用品，不考虑清除或重新赋值问题
//            cur_seg->data.def.base_delta   = do_append ? base : cur_seg->data.def.base_delta + base;
//            cur_seg->data.def.acc_delta    = do_append ? acc : cur_seg->data.def.acc_delta + acc;
//            cur_seg->data.def.mul          = do_append ? mul : cur_seg->data.def.mul * mul;
//            cur_seg->data.def.extra_delta  = do_append ? extra : cur_seg->data.def.extra_delta + extra;
            cur_seg->data.def.base_delta += base;
            cur_seg->data.def.acc_delta += acc;
            cur_seg->data.def.mul *= mul;
            cur_seg->data.def.extra_delta += extra;
		}
	};
}

#pragma clang diagnostic pop