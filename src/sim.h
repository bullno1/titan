#pragma once

namespace ExitReason
{
	enum Enum
	{
		Normal,
		Error,
		Restart,
		Abort,

		Count
	};
}

class Sim;

Sim* createSim(int argc, char* argv[]);
ExitReason::Enum runSim(Sim* sim);
void destroySim(Sim* sim);
