#include <iostream>
#include <SDL.h>
#include "engine.h"

using namespace std;

extern "C" int main(int argc, char* argv[])
{
	while(true)
	{
		ExitReason::Enum exitReason = engineMain(argc, argv);
		switch(exitReason)
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
	return 0;
}