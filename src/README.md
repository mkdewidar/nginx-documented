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

It is also common to see some data exist twice, once in the module's configuration,
and another in a `static` variable.
My theory so far is that this is for performance as it avoids the layer of indirection that would be required to get the value from the config object.

## Module Types

### NGX_CORE_MODULE

Context type: `ngx_core_module_t` defined in `core/ngx_module.c/h`

The context for these modules allows them to hook into the creation of the cycle/
parsing of the `nginx.conf` and store a pointer to arbitrary data in the
`ngx_cycle_t.conf_ctx` array, which they can access later.

### NGX_EVENT_MODULE

Context type: `ngx_event_module_t` defined in `core/ngx_event.c/h`

Provided by: The `NGX_CORE_MODULE` named `events`, declared as `ngx_module_t  ngx_events_module` in `core/ngx_event.c`.

These are modules that provide alternate implementations of the event poll that underlies
NGINX's event loop.

They are given hooks into the before, and after processing of the "events" block,
and are also able to store a pointer to arbitrary data in the `events` module's
context, similar to how `NGX_CORE_MODULE`s store their pointer in cycle's context (`ngx_cycle_t.conf_ctx`).

The event loop interacts with the module each iteration via the "actions" in the
`ngx_event_actions_t` data structure.

See the [events](#events) section later for more info.

### NGX_HTTP_MODULE

### NGX_MAIL_MODULE

### NGX_STREAM_MODULE

### NGX_CONF_MODULE (undocumented?)

# Events

NGINX operates using an event loop, similar to JavaScript.

The loop itself is part of OS dependent code in `ngx_process_cycle.c`.
Worker/single processes eventually call into `ngx_process_events_and_timers(ngx_cycle_t *cycle)` which
leaves the actual polling for events to the `NGX_EVENT_MODULE` currently in use.

How the event poll is setup:
* Configuration stage:
    * When the `events` block is encountered in the user's `nginx.conf`, the `events` module,
    which is of type `NGX_CORE_MODULE` triggers callbacks for `NGX_EVENT_MODULE`s.
    * One of the `NGX_EVENT_MODULE`s is `event_core`, which ensures that after the `events`
    block is processed that a event loop technology (epoll, kqueue etc) has been chosen
    for processing event loop messages. Each of these technologies have their own
    `NGX_EVENT_MODULE` in the `event/modules` directory.
* Processing stage:
    * At the start of the worker/single process's lifecycle, when `ngx_module_t.init_process`
    is called, the `event_core` module calls `ngx_event_actions_t.init` on the `NGX_EVENT_MODULE`
    that is in use.
    * The chosen module then configures its implementation, and sets its own actions
    in the global `ngx_event_actions`, so that it may be called by the event loop.

The contract a event polling system needs to implement is defined by `ngx_event_actions_t`.

One of the first events added is that to read from the listening socket.
This is done by the `event_core` module.

## Epoll

epoll == kernel API allowing monitoring of multiple file descriptors at once for read/write events
kqueue ~= epoll for linux
/dev/poll ~= epoll for solaris

For `ngx_event_actions_t.notify` support, the epoll module uses a eventfd object, which is in essence a counter,
exposed as a file descriptor.
Each call to `notify` results in the counter being incremented.
Since the eventfd object is a file descriptor, it is also being monitored by the epoll instance,
and so the write causes a `epoll_event` to be generated for that file descriptor with the handler provided by the caller of `notify`.

## Timers

Timed events are implemented using a red black tree `ngx_event_timer_rbtree` (defined in `ngx_event_timer.c`)
where the key is the milliseconds that the the event is expected to occur.
This means that finding what will expire next is as simple as finding
the minimum node, aka the leftmost node. Other functions in that file provide the ability to manage those timers.

Before calling the event poll implementation, NGINX will find the epoch at which the
next timer is due to expire and will pass that to the event poll implementation,
so that it does not delay beyond that point in time and end up delaying the execution
of timed tasks as well.

# Next

`core/nginx.c` contains `int main(int argc, char *const *argv)`: the executable's entrypoint.

See `core/README.md` for more information on code in core.
