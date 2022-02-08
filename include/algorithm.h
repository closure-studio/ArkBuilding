#pragma once
#include "operator_model.h"
#include "buff_model.h"

namespace albc {
    class Algorithm
	{
	public:
		virtual ~Algorithm() = default;
		Vector<RoomModel*> rooms;
		Vector<OperatorModel*> all_agents;

		Algorithm(const Vector<RoomModel*>& rooms, const Vector<OperatorModel*>& providers) :
			rooms(rooms),
			all_agents(providers)
		{
		}

		virtual void Run() = 0; // 实现算法
	};

	class BruteForce : public Algorithm // 暴力枚举，计算出所有可能的组合
	{
	public:
		BruteForce(const Vector<RoomModel*>& rooms,
		           const Vector<OperatorModel*>& operators,
				   const double max_allowed_duration = 3600 * 8);

		void Run() override;

	protected:
		Vector<OperatorModel*> solution_;
		Vector<OperatorModel*> current_;
		Vector<OperatorModel*> inbound_agents_;
		Vector<double> results_;
		double max_tot_delta_;
		int calc_cnt_;
		double max_allowed_duration_;

		void FilterAgents(const RoomModel* room);

		void MakeComb(const Vector<OperatorModel*>& agents, int max_n, RoomModel* room);

		void PrintSolution();
	};

    class PatternSearch : public Algorithm // 模式搜索，根据预设的模式，搜索出最优的组合
    {
    public:
        PatternSearch(const Vector<RoomModel*>& rooms,
                      const Vector<OperatorModel*>& operators,
                      const double max_allowed_duration = 3600 * 8);

        void Run() override;

    };
}