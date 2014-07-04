
#include "bridge/util.h"
#include "registry/registry.h"
#include "bindings/lua/lua_util.h"
#include "common/profiler/profiler.h"

using namespace std;
using namespace ug::bridge;
using namespace ug::script;

namespace ug{
namespace LuaShell{

/// error function to be used for lua_pcall
int LuaCallStackError( lua_State *L )
{
//	UG_LOG("LUA-ERROR! Call stack:\n");
//    ug::bridge::LuaStackTrace();
    return 1;
}

class LuaShell{
	public:
		LuaShell()
		{
		//	instead of the global lua state, one could also use a local one		
			m_luaState = GetDefaultLuaState();
		}

		void run(const char* buffer)
		{
			PROFILE_FUNC();
			lua_State* L = m_luaState;

			lua_pushcfunction(L, LuaCallStackError);

			int error = luaL_loadbuffer(L, buffer, strlen(buffer), "luashell-run-buffer");

			if(error == 0){
				error = lua_pcall(L, 0, 0, -2);
			}

			if(error){
				string msg = lua_tostring(L, -1);
				lua_pop(L, 1);
				if(msg.find("__UG__LUA__EMPTY__MSG__") == string::npos)
					throw(LuaError(msg.c_str()));
				else
					throw(LuaError());
			}
		}

		void set(const char* name, double value)
		{
			lua_pushnumber(m_luaState, value);
			lua_setglobal(m_luaState, name);
		}

		double get_number(const char* name)
		{
			lua_getglobal(m_luaState, name);
			if(lua_isnumber(m_luaState, -1))
				return lua_tonumber(m_luaState, -1);
			lua_pop(m_luaState, 1);
			UG_THROW("LuaShell error: Couldn't convert " << name << " to number.");
		}

	private:
		lua_State* m_luaState;
};

}// namespace LuaShell


extern "C" void
InitUGPlugin_LuaShell(Registry* reg, string grp)
{
	using namespace LuaShell;
	grp.append("LuaShell");

	typedef LuaShell::LuaShell	T;
	reg->add_class_<T>("LuaShell", grp)
		.add_constructor()
		.add_method("run", &T::run)
		.add_method("set", static_cast<void (T::*)(const char*, double)>(&T::set))
		.add_method("get_number", &T::get_number)
		.set_construct_as_smart_pointer(true);

}

}// namespace ug
