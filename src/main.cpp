#include <iostream>
#include <cstdio>
#include "sim.h"

using namespace std;

int main(int argc, char* argv[])
{
	Sim* sim = createSim(argc, argv);

	bool running = true;
	int exitCode = 0;

	while(running)
	{
		switch(runSim(sim))
		{
		case ExitReason::Normal:
			running = false;
			exitCode = 0;
			break;
		case ExitReason::Error:
			{
				cout << "Program crashed. Restart? [Y/n]: ";
				char response = cin.get();
				if(response == 'n' || response == 'N')
				{
					running = false;
				}
			}
			break;
		case ExitReason::Restart:
			cout << "Restarting engine" << endl;
			break;
		case ExitReason::Abort:
			running = false;
			exitCode = 1;
			break;
		case ExitReason::Count:
			break;
		}
	}

	destroySim(sim);
	return exitCode;
}
