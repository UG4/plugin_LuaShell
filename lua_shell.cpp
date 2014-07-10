
#include "bridge/util.h"
#include "registry/registry.h"
#include "bindings/lua/lua_util.h"
#include "bindings/lua/lua_parsing.h"
#include "common/profiler/profiler.h"
#include "registry/class_name_provider.h"

using namespace std;
using namespace ug::bridge;
using namespace ug::bridge::lua;
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
		// replace LUAs print function with our own, to use UG_LOG
			lua_register(m_luaState, "print", UGLuaPrint );
			lua_register(m_luaState, "print_all", UGLuaPrintAllProcs );
			lua_register(m_luaState, "write", UGLuaWrite );

		//	todo: allow for argv arguments
			char* argv = 0;
			SetLuaUGArgs(m_luaState, 0, &argv, 0, 0);
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
			lua_pop(m_luaState, 1);
		}

		template <class TVal>
		void set(const char* name, TVal value)
		{
			LuaParsing<TVal>::push(m_luaState, value);
			lua_setglobal(m_luaState, name);
		}

		void set(const char* name, void* pval, const char* className)
		{
			LuaParsing<void*>::push(m_luaState, pval, className);
			lua_setglobal(m_luaState, name);
		}

		void set(const char* name, const void* pval, const char* className)
		{
			LuaParsing<const void*>::push(m_luaState, pval, className);
			lua_setglobal(m_luaState, name);
		}

		void set(const char* name, SmartPtr<void> pval, const char* className)
		{
			LuaParsing<SmartPtr<void> >::push(m_luaState, pval, className);
			lua_setglobal(m_luaState, name);
		}

		void set(const char* name, ConstSmartPtr<void> pval, const char* className)
		{
			LuaParsing<ConstSmartPtr<void> >::push(m_luaState, pval, className);
			lua_setglobal(m_luaState, name);
		}

		template <class TVal>
		TVal get_val(const char* name)
		{
			lua_getglobal(m_luaState, name);
			if(!LuaParsing<TVal>::check(m_luaState, -1)){
				lua_pop(m_luaState, 1);
				UG_THROW("LuaShell error: Couldn't convert " << name << " to requested type.");
			}

			TVal val = LuaParsing<TVal>::get(m_luaState, -1);
			lua_pop(m_luaState, 1);
			return val;
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

	RegisterDefaultLuaBridge(reg);
	
	typedef LuaShell::LuaShell	T;
	reg->add_class_<T>("LuaShell", grp)
		.add_constructor()
		.add_method("run", &T::run)
		.add_method("set", static_cast<void (T::*)(const char*, double)>(&T::set<double>))
		.add_method("set", static_cast<void (T::*)(const char*, bool)>(&T::set<bool>))
		.add_method("set", static_cast<void (T::*)(const char*, const char*)>(&T::set<const char*>))
		.add_method("set", static_cast<void (T::*)(const char*, std::string)>(&T::set<std::string>))
		.add_method("set", static_cast<void (T::*)(const char*, const std::string&)>(&T::set<const std::string&>))
		.add_method("set", static_cast<void (T::*)(const char*, void*, const char*)>(&T::set))
		.add_method("set", static_cast<void (T::*)(const char*, const void*, const char*)>(&T::set))
		.add_method("set", static_cast<void (T::*)(const char*, SmartPtr<void>, const char*)>(&T::set))
		.add_method("set", static_cast<void (T::*)(const char*, ConstSmartPtr<void>, const char*)>(&T::set))
		.add_method("get_number", &T::get_val<double>)
		.add_method("get_bool", &T::get_val<bool>)
		.add_method("get_string", &T::get_val<std::string>)
		.set_construct_as_smart_pointer(true);
}

}// namespace ug
