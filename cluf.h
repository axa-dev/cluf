/*           Axa Lua Development          *\
**     Copyright 2022 Thadeu de Paula     **
**     https://github.com/axa-dev/cluf    **
\*       Licensed under MIT License       */

#include <lua.h>
#include <lauxlib.h>

/* Abstraction over Lua versions for module exports */
#if ( LUA_VERSION_NUM < 502 )
  #define luf_export(L,n,t) luaL_register(L,n,t);
#else
  #define luf_export(L,n,t) luaL_newlib(L,t);
#endif


/* Internally used to push luf_field_** macros */
#define luf_fpush(L,kt,k,vt,v) \
  lua_push ## kt((L),(k)); \
  lua_push ## vt((L),(v)); \
  lua_settable((L),(-3));

/* luf_tXY add keys to tables where X represents the key and Y the value,
** X and Y are represented by single letter of the Lua type.
** So, eost_si generate a string key with integer value
** Ex: (in C:) eost_si(L,"hello",10) (as in Lua:) t["hello"]=10
*/
#define luf_fpush_sb(L,k,v) luf_fpush((L),string,(k),boolean,(v));
#define luf_fpush_si(L,k,v) luf_fpush((L),string,(k),integer,(v));
#define luf_fpush_sn(L,k,v) luf_fpush((L),string,(k),number,(v));
#define luf_fpush_ss(L,k,v) luf_fpush((L),string,(k),string,(v));

#define luf_fpush_in(L,k,v) luf_fpush((L),integer,(k),number,(v));
#define luf_fpush_ii(L,k,v) luf_fpush((L),integer,(k),integer,(v));
#define luf_fpush_is(L,k,v) luf_fpush((L),integer,(k),string,(v));
#define luf_fpush_ib(L,k,v) luf_fpush((L),integer,(k),boolean,(v));


#define luf_error(L,requisite,message) \
  if ((requisite)) { \
    lua_pushstring((L),(message)); \
    lua_error(L); \
    return 0; \
  }

#define luf_failnil_m(L,t,m) \
  if ((t)) {\
    lua_pushnil((L)); \
    lua_pushstring((L),(const char *)(m)); \
    return 2; \
  }

#define luf_failboolean_m(L,t,m) \
  if ((t)) {\
    lua_pushboolean(L,0); \
    lua_pushstring((L),(const char *)(m)); \
    return 2; \
  }

#define luf_failnil(L,t) luf_failnil_m(L,t,strerror(errno))
#define luf_failboolean(L,t) luf_failboolean_m(L,t,strerror(errno))
