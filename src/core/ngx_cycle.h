
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_CYCLE_H_INCLUDED_
#define _NGX_CYCLE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


#ifndef NGX_CYCLE_POOL_SIZE
#define NGX_CYCLE_POOL_SIZE     NGX_DEFAULT_POOL_SIZE
#endif


#define NGX_DEBUG_POINTS_STOP   1
#define NGX_DEBUG_POINTS_ABORT  2


typedef struct ngx_shm_zone_s  ngx_shm_zone_t;

typedef ngx_int_t (*ngx_shm_zone_init_pt) (ngx_shm_zone_t *zone, void *data);

struct ngx_shm_zone_s {
    void                     *data;
    ngx_shm_t                 shm;
    ngx_shm_zone_init_pt      init;
    void                     *tag;
    void                     *sync;
    ngx_uint_t                noreuse;  /* unsigned  noreuse:1; */
};


/**
 * A structure holding all the runtime information of a running NGINX process.
 * There should only be one currently in use for a given process, though that
 * cycle refers to the previously used cycle.
 *
 * In the case where it is a fresh start, the cycle still refers to a "previous"
 * cycle, which is initialised with the startup options.
 *
 * Reloading NGINX includes creating a new object and inheriting some stuff
 * from old one.
 *
 * Use ngx_cycle_init to initialize all the important fields before use.
 */
struct ngx_cycle_s {
    /**
     * A array of void* that is used to store pointers to runtime configuration
     * information for all modules. Modules allocate data from pools, and store
     * the pointer to that data in this array.
     *
     * The index into the array is a given module's ngx_module_t.ctx_index.
     *
     * Use the ngx_get_conf macro to access this array.
     *
     * The size is bound by the maximum number of modules. So there should
     * never be a need to re-sized.
     */
    void                  ****conf_ctx;
    /**
     * The primary pool from which all memory needed for this cycle is
     * allocated.
     * In fact, the memory for the cycle itself is allocated from this pool.
     *
     * For the initial cycle, this will pretty much only include data needed
     * while processing options, as soon afterwards a new cycle is created with
     * its own pool that will be used for the rest of the processes' lifetime.
     */
    ngx_pool_t               *pool;

    /**
     * The logger for this cycle.
     */
    ngx_log_t                *log;
    ngx_log_t                 new_log;

    ngx_uint_t                log_use_stderr;  /* unsigned  log_use_stderr:1; */

    ngx_connection_t        **files;
    ngx_connection_t         *free_connections;
    ngx_uint_t                free_connection_n;

    /**
     * Array of pointers to module objects for modules currently in use.
     *
     * Must be initialized. The size is bound by the maximum number of modules,
     * so there should never need to be re-sized.
     */
    ngx_module_t            **modules;
    /**
     * The number of modules currently in use.
     */
    ngx_uint_t                modules_n;
    ngx_uint_t                modules_used;    /* unsigned  modules_used:1; */

    /**
     * Defaults to an empty queue.
     */
    ngx_queue_t               reusable_connections_queue;
    ngx_uint_t                reusable_connections_n;
    time_t                    connections_reuse_time;

    /**
     * Defaults to array of capacity 10.
     */
    ngx_array_t               listening;
    /**
     * Defaults to array of capacity 10.
     */
    ngx_array_t               paths;

    /**
     * Defaults to array of capacity 1.
     */
    ngx_array_t               config_dump;
    ngx_rbtree_t              config_dump_rbtree;
    ngx_rbtree_node_t         config_dump_sentinel;

    /**
     * Defaults to list of capacity 20.
     */
    ngx_list_t                open_files;
    /**
     * Defaults to list of capacity 1.
     */
    ngx_list_t                shared_memory;

    ngx_uint_t                connection_n;
    ngx_uint_t                files_n;

    ngx_connection_t         *connections;
    ngx_event_t              *read_events;
    ngx_event_t              *write_events;

    /**
     * Points to the previous cycle. Even when this is a fresh start of NGINX,
     * this still points to a cycle, but that "old_cycle" will be configured
     * based on the options used during startup.
     */
    ngx_cycle_t              *old_cycle;

    /**
     * The fully qualified path to the main user configuration file.
     *
     * Use NGX_CONF_PATH as the default value.
     */
    ngx_str_t                 conf_file;
    /**
     * A string of normal NGINX configuration code that has been provided at
     * the command line.
     */
    ngx_str_t                 conf_param;
    /**
     * The same as prefix, but seemingly a new name that is more consistent with
     * other conf_ fields in this structure. Usage in some areas of the code is
     * hidden behind a flag NGX_CONF_PREFIX.
     */
    ngx_str_t                 conf_prefix;
    /**
     * The prefix for all server files referenced in the user's configuration.
     *
     * Use NGX_PREFIX as the default value.
     */
    ngx_str_t                 prefix;
    /**
     * Path to file specifically used for error logs.
     *
     * Use NGX_ERROR_LOG_PATH as the default value.
     */
    ngx_str_t                 error_log;
    ngx_str_t                 lock_file;
    /**
     * The hostname is all lowercase.
     */
    ngx_str_t                 hostname;
};


/**
 * The runtime configuration structure for the "core" module.
 * Populated by parsing the user's configuration and merging with defaults.
 */
typedef struct {
    ngx_flag_t                daemon;
    ngx_flag_t                master;

    ngx_msec_t                timer_resolution;
    ngx_msec_t                shutdown_timeout;

    ngx_int_t                 worker_processes;
    ngx_int_t                 debug_points;

    ngx_int_t                 rlimit_nofile;
    off_t                     rlimit_core;

    int                       priority;

    ngx_uint_t                cpu_affinity_auto;
    ngx_uint_t                cpu_affinity_n;
    ngx_cpuset_t             *cpu_affinity;

    char                     *username;
    ngx_uid_t                 user;
    ngx_gid_t                 group;

    ngx_str_t                 working_directory;
    ngx_str_t                 lock_file;

    ngx_str_t                 pid;
    ngx_str_t                 oldpid;

    ngx_array_t               env;
    char                    **environment;

    ngx_uint_t                transparent;  /* unsigned  transparent:1; */
} ngx_core_conf_t;


#define ngx_is_init_cycle(cycle)  (cycle->conf_ctx == NULL)


ngx_cycle_t *ngx_init_cycle(ngx_cycle_t *old_cycle);
ngx_int_t ngx_create_pidfile(ngx_str_t *name, ngx_log_t *log);
void ngx_delete_pidfile(ngx_cycle_t *cycle);
ngx_int_t ngx_signal_process(ngx_cycle_t *cycle, char *sig);
void ngx_reopen_files(ngx_cycle_t *cycle, ngx_uid_t user);
char **ngx_set_environment(ngx_cycle_t *cycle, ngx_uint_t *last);
ngx_pid_t ngx_exec_new_binary(ngx_cycle_t *cycle, char *const *argv);
ngx_cpuset_t *ngx_get_cpu_affinity(ngx_uint_t n);
ngx_shm_zone_t *ngx_shared_memory_add(ngx_conf_t *cf, ngx_str_t *name,
    size_t size, void *tag);
void ngx_set_shutdown_timer(ngx_cycle_t *cycle);


extern volatile ngx_cycle_t  *ngx_cycle;
extern ngx_array_t            ngx_old_cycles;
extern ngx_module_t           ngx_core_module;
extern ngx_uint_t             ngx_test_config;
extern ngx_uint_t             ngx_dump_config;
extern ngx_uint_t             ngx_quiet_mode;


#endif /* _NGX_CYCLE_H_INCLUDED_ */
