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

ExitReason::Enum engineMain(int argc, char* argv[]);