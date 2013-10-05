#pragma once

namespace ExitReason
{
	enum Enum
	{
		UserAction,
		Error,
		Restart,

		Count
	};
}

class Sim;

Sim* createSim(int argc, char* argv[]);
ExitReason::Enum runSim(Sim* sim);
void destroySim(Sim* sim);