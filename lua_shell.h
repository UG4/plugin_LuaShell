// created by Sebastian Reiter
// s.b.reiter@gmail.com

#ifndef __H__UG_lua_shell
#define __H__UG_lua_shell

#include "bindings/lua/lua_util.h"
#include "bindings/lua/lua_parsing.h"

namespace ug{
namespace luashell{

class LuaShell{
	public:
		LuaShell();

		void reset();

		void parse_file(const char* filename);
		void run(const char* buffer);

		void abort_run(const char* message);

		template <class TVal>
		void set(const char* name, TVal value)
		{
			bridge::lua::LuaParsing<TVal>::push(m_luaState, value);
			lua_setglobal(m_luaState, name);
		}

		void set(const char* name, void* pval, const char* className);
		void set(const char* name, const void* pval, const char* className);
		void set(const char* name, SmartPtr<void> pval, const char* className);
		void set(const char* name, ConstSmartPtr<void> pval, const char* className);

		template <class TVal>
		TVal get_val(const char* name)
		{
			lua_getglobal(m_luaState, name);
			if(!bridge::lua::LuaParsing<TVal>::check(m_luaState, -1)){
				lua_pop(m_luaState, 1);
				UG_THROW("LuaShell error: Couldn't convert " << name << " to requested type.");
			}

			TVal val = bridge::lua::LuaParsing<TVal>::get(m_luaState, -1);
			lua_pop(m_luaState, 1);
			return val;
		}

	private:
		lua_State* m_luaState;

		void init_lua_state();
};

}// namespace LuaShell
}//	end of namespace


#endif	//__H__lua_shell
