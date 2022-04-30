#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify"
#pragma once
#include "albc_types.h"
#include "util.h"

namespace albc::model::buff
{
	struct alignas(64) PiecewiseDef
	{ 
		// 该结构体表示一个分段函数的段
		// 段的定义是：
		// f(t) = mul * (base + acc * t) + extra
		// 某段中的base, acc, mul, 和extra 是所有之前定义的段的中响应值的累加：
		// base = base_n + base_n-1 + ... + base_1 + base_0
		// acc = acc_n + acc_n-1 + ... + acc_1 + acc_0
		// mul = mul_n * mul_n-1 * ... * mul_1 * mul_0
		// extra = extra_n + extra_n-1 + ... + extra_1 + extra_0
		double base_delta = 0;	// base_n - base_n-1
		double acc_delta = 0;	// acc_n - acc_n-1
		double mul = 1;			// mul_n / mul_n-1
		double extra_delta = 0; // extra_n - extra_n-1

        PiecewiseDef(const double base, const double acc, const double mul, const double extra)
            : base_delta(base),
              acc_delta(acc),
              mul(mul),
              extra_delta(extra)
        {
        }

		[[nodiscard]] constexpr auto IsEmpty() const -> bool
		{
			return util::fp_eq(base_delta, 0.)
                   && util::fp_eq(acc_delta, 0.)
                   && util::fp_eq(mul, 1.)
                   && util::fp_eq(extra_delta, 0.);
		}
	};

	struct alignas(64) PiecewiseIndex
	{
		double ts = 0.;
		PiecewiseDef def;
	};

	template <UInt32 N>
	struct alignas(64) PiecewiseMap
	{
		// 土制前向链表，表示函数中的所有段
		struct alignas(64) Forward
		{
			PiecewiseIndex data{INFINITY, {0., 0., 1., 0.}};
			Forward *next = nullptr;
		};

		struct alignas(64) Iterator
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

			const Iterator operator++(int) noexcept
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

		Forward f_list[N + 1U];
		Forward *before_begin_ptr; // 指向第一个段的前一个指针
		UInt32 n = 0;			   // 段的数量，不包含第一段之前的段

		PiecewiseMap()
		{
			for (auto &fwd : f_list)
			{
				fwd.next = nullptr;
			}
			before_begin_ptr = std::addressof(f_list[0]);
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
		{
            bool do_append = true;
			Iterator it = before_begin();
            Forward* cur_seg = it.ptr;
			for (; it.HasNext(); ++it)
			{
                cur_seg = it.ptr;
                if (util::fp_eq(cur_seg->data.ts, ts))
				{
                    do_append = false;
                    break;
				}else if (ts > cur_seg->data.ts)
				{
					break;
				}
			}

            if(do_append) {
                assert(n < N + 1);
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