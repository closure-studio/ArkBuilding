#include "algorithm.h"
#include "flag_util.h"
#include "simulator.h"

namespace albc
{
	BruteForce::BruteForce(const Vector<RoomModel *> &rooms,
						   const Vector<OperatorModel *> &operators,
						   const double max_allowed_duration)
		: Algorithm(rooms, operators),
		  solution_(kRoomMaxBuffSlots),
		  current_(kRoomMaxBuffSlots),
		  max_tot_delta_(0),
		  calc_cnt_(0),
		  max_allowed_duration_(max_allowed_duration)
	{
	}

	void BruteForce::Run()
	{
		auto *const room = rooms[0];
		calc_cnt_ = 0;

		// filter operators
		FilterAgents(room);

		// measures the time of MakeComb()
		const double elapsedSec = MeasureTime(&BruteForce::MakeComb, this, inbound_agents_, room->max_slot_count, room).count();

		// prints the number of calculations
		LOG_D << "calc cnt: " << calc_cnt_ << std::endl;

		// print elapsed time
		LOG_D << "elapsed time: " << elapsedSec << std::endl;

		// prints average calculations per second
		LOG_D << "calc/sec: " << calc_cnt_ / elapsedSec << std::endl;

		// prints the best solution
		if (!solution_.empty())
		{
			PrintSolution();
		}
		else
		{
			LOG_W << "No solution!" << std::endl;
		}
	}

	void BruteForce::FilterAgents(const RoomModel *room)
	{
		inbound_agents_.clear();

		for (auto *const agent : all_agents)
		{
			if (agent->buffs.empty())
			{
				continue;
			}

			if (!check_flag(agent->room_type_mask, room->type))
			{
				continue;
			}

			if (!std::all_of(agent->buffs.begin(), agent->buffs.end(), [room](RoomBuff *buff) -> bool
							 { return buff->ValidateTarget(room); }))
			{
				continue;
			}

			inbound_agents_.push_back(agent);
		}
		LOG_D << "Filtered " << inbound_agents_.size() << " operators for room: " << room->id
			  << " : [P]" << to_string(room->room_attributes.prod_type)
			  << " [O]" << to_string(room->room_attributes.order_type) << std::endl;
	}

	// this function is a transform from recursive DFS to iterative DFS, using stack
	void BruteForce::MakeComb(const Vector<OperatorModel *> &operators, int max_n, RoomModel *room)
	{
		int calc_cnt = 0;
		const int size = static_cast<int>(operators.size()); // size of operators
		max_n = std::min(max_n, size);						 // max_n is the max number of operators to be used in a combination
		int pos[kRoomMaxBuffSlots]{};						 // pos[i] is the index of i-th recursion
															 // a position indicates the index of the operator in the operators vector

		int buff_cnt[kRoomMaxBuffSlots]{};			  // buff_cnt[i] is the number of buffs in i-th recursion
		bool status[kRoomMaxBuffSlots]{};			  // status[i] is the status of i-th recursion
		//UInt32 buff_slot_stack[kRoomMaxBuffSlots]{};  // buff stack
		OperatorModel *current[kRoomMaxBuffSlots]{};  // stores the current solution, including operators in a unique combination
		OperatorModel *solution[kRoomMaxBuffSlots]{}; // stores the best solution
		double max_tot_delta = 0.;					  // max delta of the total production of the solution
        double max_duration = max_allowed_duration_;

		//int b_stk_top = -1; // top of the buff_slot_stack
		int dep = 0;		// depth of recursion, when dep + 1 == max_n, we have a unique combination
		while (dep >= 0)
		{							 // when dep >= 0, the root of the recursion is not finished
			int &cur_pos = pos[dep]; // current position
			bool &cur_status = status[dep];
			if (!cur_status)
			{
				for (auto *const buff : (current[dep] = operators[cur_pos])->buffs)
				{
					if (buff == nullptr || !check_flag(buff->room_type, room->type))
					{
						continue;
					}

					//buff_slot_stack[++b_stk_top] =
                    room->PushBuff(buff);
					++buff_cnt[dep];
				}

				cur_status = true;
				if (dep >= max_n - 1)
				{
					++calc_cnt;
					const double result = Simulator::DoCalc(room, max_duration);
					// DoCalc() is inlined, without overhead of function call and variable allocation
					if (result > max_tot_delta)
					{
						std::copy_n(current, max_n, solution);
						max_tot_delta = result;
					}
				}
				else
				{
					pos[dep + 1] = cur_pos + 1; // in the next recursion, the position of the operator will be increased by 1
					++dep;						// move to the next recursion
					continue;
				}
			}

			if (cur_status)
			{
//                for (int i = 0; i < buff_cnt[dep]; ++i)
//                {
//                    room->PopBuff();
//                }
                room->n_buff -= buff_cnt[dep];
                buff_cnt[dep] = 0;

				cur_status = false;
				if (cur_pos < size - max_n + dep)
				{
					++cur_pos; // move to the next operator
				}
				else
				{
					--dep; // the list of operators is exhausted, move to the previous recursion
				}
			}
		}
		calc_cnt_ = calc_cnt;
		max_tot_delta_ = max_tot_delta;
		std::copy_n(solution, max_n, solution_.begin());
	}

	void BruteForce::PrintSolution()
	{
		for (auto *const agent : solution_)
		{
			if (agent == nullptr)
			{
				break;
			}

			LOG_I << agent->char_id << std::endl;

			for (const auto &buff : agent->buffs)
			{
				LOG_I << buff->buff_id << std::endl;
			}
		}
		LOG_I << max_tot_delta_ << std::endl;
	}
}