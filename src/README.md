# src

# Modules

NGINX is implemented as a modular system. Much of the built in functionality is
implemented as modules, and these built in modules do not generally have
anything that makes them special compared to third party modules.

See [this section](http://nginx.org/en/docs/dev/development_guide.html#Modules)
of the NGINX development guide for some basic information about modules. The
important bit is that modules fill in a `ngx_module_t` object that represents them
at runtime and defines the hooks the module is plugged into, and a `config` file
that is used by the build system to compile the module. The `ngx_module_t` object
has a "context", which is actually more like a interface that differs
per module type, that the module should implement to get access to more hooks/
abilities.

Note that for built in modules, what would be defined in a `config` file still exists, but is
instead integrated directly into NGINX's build system. See the `auto` folder in the project's root directory.

The list of builtin NGINX modules can be found in the "Modules reference" section of [the official documentation](https://nginx.org/en/docs/).

Modules can interact with each other, an example is how the many HTTP
modules depend on a single "core" HTTP module (i.e a HTTP module of type NGX_CORE_MODULE
instead of type NGX_HTTP_MODULE).
In fact, that "core" HTTP module, is the reason the NGX_HTTP_MODULE type exists to begin with.
See `http/ngx_http_core_module.c/h`.

## Configuration (aka conf)

Configuration in the code often does not refer to a user's `nginx.conf`; at least not directly.
Usually it refers to the module's runtime data which of course,
is usually based on the user's `nginx.conf` been parsed, and then the values
being saved in memory. But still, that means conf is used a lot to refer to module runtime configuration,
rather than the user's static configuration on disk.

Each module is responsible for storing its own configuration. `NGX_CORE_MODULE`s
do this by putting their data in their entry in the `ngx_cycle_t.conf_ctx` via
the `create_conf` callback they are given access to. Other modules have other
mechanisms.

## Module Types

### NGX_CORE_MODULE

Context type: `ngx_core_module_t` defined in `core/ngx_module.c/h`

The context for these modules allows them to hook into the creation of the cycle/
parsing of the `nginx.conf` and store a pointer to arbitrary data in the
`ngx_cycle_t.conf_ctx` array, which they can access later.

### NGX_EVENT_MODULE

### NGX_HTTP_MODULE

### NGX_MAIL_MODULE

### NGX_STREAM_MODULE

# Next

`core/nginx.c` contains `int main(int argc, char *const *argv)`: the executable's entrypoint.

See `core/README.md` for more information on code in core.
