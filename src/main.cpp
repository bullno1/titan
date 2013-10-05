#include <iostream>
#include "sim.h"

using namespace std;

int main(int argc, char* argv[])
{
	Sim* sim = createSim(argc, argv);
	while(true)
	{
		switch(runSim(sim))
		{
		case ExitReason::UserAction:
			return 0;
		case ExitReason::Error:
			cout << "-------------------------------" << endl
			     << "Press enter to restart"          << endl
			     << "-------------------------------" << endl;
			cin.get();
			break;
		case ExitReason::Restart:
			cout << "-------------------------------" << endl
			     << "Restarting engine"               << endl
			     << "-------------------------------" << endl;
			break;
		}
	}
	destroySim(sim);
	return 0;
}