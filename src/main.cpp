#include <iostream>
#include <cstdio>
#include "sim.h"

using namespace std;

#ifdef RELEASE_BUILD
int main(int argc, char* argv[])
{
	freopen("stdout.log", "w", stdout);
	freopen("stderr.log", "w", stderr);
	Sim * sim = createSim(argc, argv);
	runSim(sim);
	destroySim(sim);
	return 0;
}
#else
int main(int argc, char* argv[])
{
	Sim* sim = createSim(argc, argv);

	bool running = true;
	while(running)
	{
		switch(runSim(sim))
		{
		case ExitReason::UserAction:
			running = false;
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
		}
	}

	destroySim(sim);
	return 0;
}
#endif