/*
 * QEMU KVM support
 *
 * Copyright IBM, Corp. 2008
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#ifndef QEMU_KVM_H
#define QEMU_KVM_H

#include "config.h"
#include "cpu.h"
#include "qemu/queue.h"

#ifdef CONFIG_KVM

#ifdef TARGET_I386
extern int kvm_allowed;

#define kvm_enabled() (kvm_allowed)
#else
#define kvm_enabled() (0)
#endif

#else
#define kvm_enabled() (0)
#endif

struct kvm_run;

/* external API */

int kvm_init(int smp_cpus);

int kvm_init_vcpu(CPUOldState *env);
int kvm_sync_vcpus(void);

int kvm_cpu_exec(CPUOldState *env);

void kvm_set_phys_mem(hwaddr start_addr,
                      ram_addr_t size,
                      ram_addr_t phys_offset);

int kvm_physical_sync_dirty_bitmap(hwaddr start_addr,
                                   hwaddr end_addr);

int kvm_log_start(hwaddr phys_addr, ram_addr_t size);
int kvm_log_stop(hwaddr phys_addr, ram_addr_t size);
int kvm_set_migration_log(int enable);

int kvm_has_sync_mmu(void);

void kvm_setup_guest_memory(void *start, size_t size);

int kvm_coalesce_mmio_region(hwaddr start, ram_addr_t size);
int kvm_uncoalesce_mmio_region(hwaddr start, ram_addr_t size);

int kvm_insert_breakpoint(CPUOldState *current_env, target_ulong addr,
                          target_ulong len, int type);
int kvm_remove_breakpoint(CPUOldState *current_env, target_ulong addr,
                          target_ulong len, int type);
void kvm_remove_all_breakpoints(CPUOldState *current_env);
int kvm_update_guest_debug(CPUOldState *env, unsigned long reinject_trap);

/* internal API */

struct KVMState;
typedef struct KVMState KVMState;

int kvm_ioctl(KVMState *s, int type, ...);

int kvm_vm_ioctl(KVMState *s, int type, ...);

int kvm_vcpu_ioctl(CPUOldState *env, int type, ...);

int kvm_get_mp_state(CPUOldState *env);
int kvm_put_mp_state(CPUOldState *env);

/* Arch specific hooks */

int kvm_arch_post_run(CPUOldState *env, struct kvm_run *run);

int kvm_arch_vcpu_run(CPUOldState *env);

int kvm_arch_handle_exit(CPUOldState *env, struct kvm_run *run);

int kvm_arch_pre_run(CPUOldState *env, struct kvm_run *run);

int kvm_arch_get_registers(CPUOldState *env);

int kvm_arch_put_registers(CPUOldState *env);

int kvm_arch_init(KVMState *s, int smp_cpus);

int kvm_arch_init_vcpu(CPUOldState *env);

struct kvm_guest_debug;
struct kvm_debug_exit_arch;

struct kvm_sw_breakpoint {
    target_ulong pc;
    target_ulong saved_insn;
    int use_count;
    QTAILQ_ENTRY(kvm_sw_breakpoint) entry;
};

QTAILQ_HEAD(kvm_sw_breakpoint_head, kvm_sw_breakpoint);

int kvm_arch_debug(struct kvm_debug_exit_arch *arch_info);

struct kvm_sw_breakpoint *kvm_find_sw_breakpoint(CPUOldState *env,
                                                 target_ulong pc);

int kvm_sw_breakpoints_active(CPUOldState *env);

int kvm_arch_insert_sw_breakpoint(CPUOldState *current_env,
                                  struct kvm_sw_breakpoint *bp);
int kvm_arch_remove_sw_breakpoint(CPUOldState *current_env,
                                  struct kvm_sw_breakpoint *bp);
int kvm_arch_insert_hw_breakpoint(target_ulong addr,
                                  target_ulong len, int type);
int kvm_arch_remove_hw_breakpoint(target_ulong addr,
                                  target_ulong len, int type);
void kvm_arch_remove_all_hw_breakpoints(void);

void kvm_arch_update_guest_debug(CPUOldState *env, struct kvm_guest_debug *dbg);

int kvm_check_extension(KVMState *s, unsigned int extension);

uint32_t kvm_arch_get_supported_cpuid(CPUOldState *env, uint32_t function,
                                      int reg);

/* generic hooks - to be moved/refactored once there are more users */
#ifdef CONFIG_HAX
void hax_vcpu_sync_state(CPUOldState *env, int modified);
#endif
static inline void cpu_synchronize_state(CPUOldState *env, int modified)
{
    if (kvm_enabled()) {
        if (modified)
            kvm_arch_put_registers(env);
        else
            kvm_arch_get_registers(env);
    }
#ifdef CONFIG_HAX
    hax_vcpu_sync_state(env, modified);
#endif
}

int kvm_get_sregs(CPUOldState *env);


#endif
