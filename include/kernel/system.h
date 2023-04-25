#ifndef SYSTEM_H
#define SYSTEM_H

// build settings

#define KERNEL_VERSION  "SCS-14"
#define KERNEL_EDITING  "scubasuit"

#define PROCESS_MAX     20
#define KERNEL_PRIORITY 5           // default kernel process priority
#define RAMDISK_SECTOR  4096
#define SCUBA_MAP_TO    0x8000000   // scuba map to 128MB
#define RUN_DEFAULT     "/bin/shell.bin"

#define RATE_TIMER_TICK 1000        // cpu ticks per second
#define RATE_SCHEDULER  100         // schedule per second

#define RUN_BIN_VBASE   0xC0000000  // virtual base address for binary
#define RUN_BIN_VCUNT   0x80000     // virtual memory count for binary
#define RUN_BIN_STACK   0x10000     // stack size for binary file

#define RUN_LIB_STACK_L 0x1000      // left stack size for library
#define RUN_LIB_STACK_R 0x4000      // right stack size for library

#define PROCESS_ESP     0x4000      // process stack size

#define WATFUNC_ADDR    0x1ffff7
#define WATDILY_ADDR    0x1ffffb
#define MEM_BASE_ADDR   0x1fffff

#define GRUBMOD_START   0x118000    // grub module start (increase 0x1000)


// system.c
void sys_reboot();
void sys_shutdown();
void sys_stop();

int  sys_warning(char msg[]);
int  sys_error(char msg[]);
void sys_fatal(char msg[]);
void sys_interrupt(int code, int err_code); // reserved cpu interrupt

int sys_init_fpu();
void sys_kinfo(char *dest);

// kshell.c
void start_kshell();
void kernel_switch_back();

// runtime.c
int run_ifexist(char path[], int argc, char **argv);

// dily.c
int dily_does_loaded(int lib_id);
int dily_get_func(int lib_id, int func_id);
int dily_load(char path[], int lib_id);
void dily_unload(int lib_id);
int dily_init();

// watfunc.c
int init_watfunc();

#endif
