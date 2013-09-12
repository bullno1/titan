#include "Titan.h"

using namespace FW;

Titan::Titan()
	:mRunning(true)
{
	RTTI_SINGLE(MOAILuaObject);
}

Titan::~Titan()
{
	for(WatchIDMap::iterator itr = mWatchFunctions.begin(); itr != mWatchFunctions.end(); ++itr)
	{
		delete itr->second;
	}
	mWatchFunctions.clear();
}

void Titan::RegisterLuaClass(MOAILuaState& state)
{
	state.SetField(-1, "FILE_ACTION_ADD", Actions::Add);
	state.SetField(-1, "FILE_ACTION_DELETE", Actions::Delete);
	state.SetField(-1, "FILE_ACTION_MODIFY", Actions::Modified);

	luaL_Reg regTable[] = {
		{"restart", _restart},
		{"addWatch", _addWatch},
		{"removeWatch", _removeWatch},
		{NULL, NULL}
	};

	luaL_register(state, 0, regTable);
}

bool Titan::Update()
{
	mFileWatcher.update();
	return mRunning;
}

void Titan::handleFileAction(WatchID watchId, const String& dir, const String& filename, Action action)
{
	WatchIDMap::iterator itr = mWatchFunctions.find(watchId);
	if(itr != mWatchFunctions.end())
	{
		MOAIScopedLuaState state = itr->second->GetSelf();
		itr->second->PushRef(state);
		state.Push(watchId);
		state.Push(dir.c_str());
		state.Push(filename.c_str());
		state.Push(action);
		state.DebugCall(4, 0);
	}
}

int Titan::_restart(lua_State* L)
{
	Titan::Get().mRunning = false;
	return 0;
}

int Titan::_addWatch(lua_State* L)
{
	MOAILuaState state(L);
	if(!state.CheckParams(1, "SF")) return 0;

	Titan& titan = Get();
	WatchID watchId = titan.mFileWatcher.addWatch(state.GetValue<cc8*>(1, "."), &titan, true);
	MOAILuaRef* ref = new MOAILuaRef;
	ref->SetStrongRef(state, 2);
	titan.mWatchFunctions.insert(std::make_pair(watchId, ref));

	state.Push(watchId);
	return 1;
}

int Titan::_removeWatch(lua_State* L)
{
	MOAILuaState state(L);
	if(!state.CheckParams(1, "N")) return 0;

	Titan& titan = Get();
	WatchID watchId = state.GetValue<WatchID>(1, 0);
	WatchIDMap::iterator itr = titan.mWatchFunctions.find(watchId);
	if(itr != titan.mWatchFunctions.end())
	{
		titan.mFileWatcher.removeWatch(watchId);
		delete itr->second;
		titan.mWatchFunctions.erase(itr);
	}

	return 0;
}