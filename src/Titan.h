#pragma once

#include <moai-core/headers.h>
#include <moai-core/MOAILua.h>
#include <unordered_map>
#include <FileWatcher/FileWatcher.h>

class Titan:
	public MOAIGlobalClass<Titan, MOAILuaObject>,
	private FW::FileWatchListener
{
public:
	DECL_LUA_SINGLETON(Titan);

	Titan();
	virtual ~Titan();

	void RegisterLuaClass(MOAILuaState& state);

	bool Update();

	void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename, FW::Action action);

private:
	bool mRunning;
	FW::FileWatcher mFileWatcher;
	typedef unordered_map<FW::WatchID, MOAILuaRef*> WatchIDMap;
	WatchIDMap mWatchFunctions;

	static int _restart(lua_State* L);
	static int _addWatch(lua_State* L);
	static int _removeWatch(lua_State* L);
};