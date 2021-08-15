# NGINX (documented)

This is a fork of the official [NGINX Git mirror](https://github.com/nginx/nginx),
but with more implementation specific documentation and code comments in areas that I explore.

The intention is to fill the gap between online documentation that explains NGINX's
functionality and how to use it, and spending hours reading the thousands of lines
of code to understand how NGINX actually implements things. In some ways this is
intended to complement [NGINX's development guide](http://nginx.org/en/docs/dev/development_guide.html)
which does explain a lot of the basic concepts, but I felt wasn't enough, especially
if wanting to make modules or contribute to the source.

Currently this documentation is nowhere near complete, so expect gaps in coverage
and questions for things I have not yet explored.

# Directory Structure

See subfolders for READMEs going into more details on individual areas.

* auto: shell scripts for building the source.
* conf:
* contrib: scripts and tooling provided by people to the NGINX project.
* docs: source files for official documentation such as man page.
* misc:
* src: all the server's code written in C
    * core: entrypoint (nginx.c), data structures and other stuff that is important to the system as a whole.
    * event: implementations of the event loop.
    * http: the "core" http module which is the foundation for all http features, as well
        as the source for other http modules which build on it.
    * mail: modules implementing the mail protocols.
    * misc:
    * os: APIs that abstract the underlying operating system.
    * stream: modules implementing TCP/UDP proxy support.
