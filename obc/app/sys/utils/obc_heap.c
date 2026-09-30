#include "FreeRTOS.h"
#include "obc_privilege.h"
#include "os_portable.h"
#include "sys_heap.h"

/**
 * @brief Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *
 * @param size Size of memory to be allocated
 */
void *sysMalloc(size_t size) {
  BaseType_t runningPriv = prvRaisePrivilege();
  void *ptr = pvPortMalloc(size);
  portRESET_PRIVILEGE(runningPriv);
  return ptr;
}

/**
 * @brief Frees a block of memory previously allocated by a call to sysMalloc
 *
 * @param ptr Pointer to the beginning of the block of memory to be freed
 */
void sysFreeMem(void *ptr) {
  BaseType_t runningPriv = prvRaisePrivilege();
  vPortFree(ptr);
  portRESET_PRIVILEGE(runningPriv);
}
