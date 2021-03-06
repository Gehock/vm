#pragma once
#include <vector>
#include <memory>
#include <string>

#include "callstack.h"
#include "string_op.h"

namespace sqf
{
	class value;
	class instruction;
	class virtualmachine;
	class vmstack
	{
	private:
		std::vector<std::shared_ptr<sqf::callstack>> mstacks;
		std::vector<std::shared_ptr<sqf::value>> mvalstack;
		bool misscheduled;
		long long mwakeupstamp;
		bool misasleep;
	public:
		vmstack(void) : misscheduled(false), misasleep(false) {}
		vmstack(bool isscheduled) : misscheduled(isscheduled), misasleep(false) {}
		void pushinst(sqf::virtualmachine* vm, std::shared_ptr<instruction> inst);
		inline std::shared_ptr<instruction> popinst(sqf::virtualmachine* vm)
		{
			if (mstacks.empty())
				return std::shared_ptr<sqf::instruction>();
			auto ret = mstacks.back()->popinst(vm);
			if (!ret.get())
			{
				dropcallstack();
				return popinst(vm);
			}
			return ret;
		}
		inline std::shared_ptr<instruction> peekinst(void) { if (mstacks.empty()) return std::shared_ptr<sqf::instruction>(); return mstacks.back()->peekinst(); }
		inline void pushcallstack(std::shared_ptr<sqf::callstack> cs) { mstacks.push_back(cs); }
		inline void dropcallstack() { if(!mstacks.empty()) mstacks.pop_back(); }
		inline void dropcallstack(std::string name, bool include = true)
		{
			int i;
			for (i = (int)mstacks.size() - 1; i >= 0; i--)
			{
				auto stack = mstacks[i];
				if (str_cmpi(stack->getscopename().c_str(), -1, name.c_str(), -1) == 0)
				{
					i = (int)mstacks.size() - i;
					if (include)
					{
						i++;
					}
					for (; i > 0; i--)
					{
						mstacks.pop_back();
					}
					break;
				}
			}
		}


		inline std::vector<std::shared_ptr<sqf::callstack>>::reverse_iterator stacks_begin(void) { return mstacks.rbegin(); }
		inline std::vector<std::shared_ptr<sqf::callstack>>::reverse_iterator stacks_end(void) { return mstacks.rend(); }
		inline std::shared_ptr<sqf::callstack> stacks_top(void) { return mstacks.back(); }

		inline void pushval(std::shared_ptr<value> val) { mvalstack.push_back(val); }
		inline std::shared_ptr<value> popval(bool &success) { if (mvalstack.empty()) { success = false; return std::shared_ptr<value>(); } success = true; auto val = mvalstack.back(); mvalstack.pop_back(); return val; }
		inline std::shared_ptr<value> peekval(void) { if (mvalstack.empty()) return std::shared_ptr<value>(); return mvalstack.back(); }
		inline void dropvals(void) { mvalstack.clear(); }
		std::shared_ptr<value> getlocalvar(std::string varname);
		inline bool isempty(void) { return mstacks.size() == 0; }
		inline bool isscheduled(void) { return misscheduled; }
		inline bool isasleep(void) { return misasleep; }
		inline void wakeup(void) { misasleep = false; }
		inline long long get_wakeupstamp(void) { return mwakeupstamp; }
		void sleep(long long ms);
	};
}