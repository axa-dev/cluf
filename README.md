# cluf
Lua C Api Framework


`cluf.h` is just a C header with a set of macros to provide simple
data handling with Lua C Api as well as polyfilling gaps between
Lua versions, in a way to ease the usage of Lua C api targeting
multiple versions.

Also, `cluf.h` enforces some good patterns for Lua functions as
error message in second return.

* [Module creation](#module-creation)
* [Add table fields](#add-table-fields)
* [Fail Returns](#fail-returns)
* [Fail Errors](#fail-errors)


### Module creation

* `luf_export(L, n, t)`

Where:

* `L` is the `lua_State`
* `n` is the string name for the module
* `t` is the `luaL_Reg` array containing the methods for the module.


So, instead of writting:

```C
static const luaL_Reg mod[] = {
    {"myfunc", myfunc}
}

int luaopen_mymod(lua_State *L) {
  #if (LUA_VERSION_NUM < 502)
    luaL_register(L, "mymod", mod);
  #else
    luaL_newlib(L, mod);
  #endif
  return 1;
}
```

You can simply do with cluf:

```C
static const luaL_Reg mod[] = {
    {"myfunc", myfunc}
}

int luaopen_mymod(lua_State *L) {
  luf_export(L, "mymod", mod);
  return 1;
}
```


### Add table fields

These macros add fields to a Lua table. Fields usually have keys of type integer
(lists) or string (records). The macros indicate it by its two letters suffix,
where `b` stands for boolean, `i` for integer, `n` for number and `s` for string.

These functions receive three parameters: `lua_State`, `key` and `value`.

* `luf_fpush_ib(L,k,v)` - integer key / boolean value
* `luf_fpush_ii(L,k,v)` - integer key / integer value
* `luf_fpush_in(L,k,v)` - integer key / number value
* `luf_fpush_is(L,k,v)` - integer key / string value
* `luf_fpush_sb(L,k,v)` - string key / boolean value
* `luf_fpush_si(L,k,v)` - string key / integer value
* `luf_fpush_sn(L,k,v)` - string key / number value
* `luf_fpush_ss(L,k,v)` - string key / string value

Where:

* `L` is the `lua_State`
* `k` is the table key
* `v` is the table value

So, instead of writting:

```C
lua_create_table(L,1,2);
lua_pushinteger(L, 1);
lua_pushstring(L, "first list item");
lua_settable(L, -3);
lua_pushstring(L, "1st");
lua_pushstring(L, "first record item");
lua_settable(L, -3);
lua_pushstring(L, "2nd");
lua_pushstring(L, "second record item");
lua_settable(L, -3);
```

You can simply do with cluf:

```C
lua_create_table(L,1,2);
luf_fpush_is(L, 1, "first list item");
luf_fpush_ss(L, "fst", "first record item");
luf_fpush_ss(L, "snd", "first record item");
```

To be used in Lua:

```Lua
assert( x[1] == "first list item" )
assert( x.fst == "first record item" )
assert( x.snd == "second record item" )
```

These functions internally use a more abstract function that can be
use for less common operations that still were not used in Axa Project.

* `luf_fpush(L,kt,k,vt,v)` where `kt` and `vt` are the key type
and value type (Ex: `string`)


### Fail Returns

For "two returns on error" for Lua functions written in C, `cluf.h` the
functions:

* `luf_failboolean(L,t)`     - push `boolean`,`strerror(errno)` amd returns `2`.
* `luf_failboolean_m(L,t,m)` - push `boolean`, string `m` and returns `2`.
* `luf_failnil(L,t)`         - push `nil`, `strerror(errno)` and returns `2`.
* `luf_failnil_m(L,t,m)`     - push `nil`, string `m` and returns `2`.

Where:

* `L` is the `lua_State`,
* `t` a test, as used between parens of a C `if`
* `m` a string message


Lua has no exceptions in the way other languages have (what is very good!).
A good "soft approach" is to return a `nil` value and a second return with the
reason of the absent result.

Most of time this will occur on _impure_ functions, i.e. functions that mutate
an external data or depends on external data other than the received arguments.
Almost of the time these are functions that works on I/O of some kind: file
content manipulation, file system list, network communications,
database related etc.

Example 1:

```Lua
local ok, err = openConnection()
if ok
    then print("Connection opened")
    else print("Error:", err)
end
```

Example 2:

```Lua
local ok, err = openConnection()
assert(ok, err)
```

In the first example, the program only notifies that connection could'nt be
opened. In the second it aborts the program showing the value of `err` variable.

These do almost the same: If `a` and `b` are equal, they push a boolean or a nil
to the stack and a string and immediately `return 2` in the function they are
called.

Instead of:
```C
int fullmoon(lua_State *L) {
  int moon = luaL_checknumber(L,1);
  if (moon < FULL) {
    lua_pushnil(L);
    lua_pushstring(L,"wait some days");
    return 2;
  }
  if (moon > FULL) {
    lua_pushnil(L);
    lua_pushstring(L,"you are late");
    return 2;
  }

  lua_pushnumber(L, "howwwwwwl");
  return 1;
}
```

You can simply do with cluf:
```C
int fullmoon(lua_State *L) {
  int moon = luaL_checknumber(L,1);

  luf_failnil_m(L, moon < FULL, "wait some days");
  luf_failnil_m(L, moon > FULL, "you are late");

  lua_pushstring(L, "howwwwwwl");
  return 1;
}
```

```Lua
local ok, err = fullmoon(12)
if not ok then
  -- treat the exception...
end

-- or if you want to stop immediately
assert(ok, err)
```


### Fail Errors

Some errors should break the program and inform the stack from the calling if
not handled. It is typical on functions where you want to limit what and how
user passes to the function as well as force correct handling of critical
side effects of I/O functions.

In such occasions, you need to push a string to stack and call `lua_error()`.
To this task there is `luf_error(L, requisite, message)`

So instead of writting in C:
```C
int fullmoon(lua_State *L) {
  int moon = luaL_checkinteger(L);
  if (moon != FULL) {
    lua_pushstring(L, "wrong moon phase");
    lua_error(L);
    return 0;
  }
  lua_pushstring(L, "howwwwwwl");
  return 1;
}
```

You can simply do with cluf:
```C
inf fullmoon(lua_State *L) {
  int moon = luaL_checkinteger(L);
  luf_error(L, moon != FULL, "wrong moon phase");
  lua_pushstring(L, "howwwwwwl");
  return 1;
}
```

In Lua, you can treat the error using `pcall()`:

```lua
local err, ok = pcall(fullmoon, 12);

```

You may wonder why to have both `luf_fail*()` and `luf_error()` functions.

The `luf_fail` family are intended for things that can go wrong but are not
potentially explosive. In this case, the user asked for a value and you are
just returning what he wants or saying there is nothing to return giving them
the possibility of inspect why.

The `luf_error` function, nonetheless, are intended for explosive things,
things that by default should explode on error and must be treated properly.

