/* Common Gasket device kernel and user space declarations. */
#ifndef __WINDOWS_GASKET_IOCTL_H__
#define __WINDOWS_GASKET_IOCTL_H__

#ifndef __GASKET_IOCTL_H__
#error This file should not be included directly include gasket_ioctl.h instead
#endif

#ifndef __KERNEL__
#include "port/fileio.h"
#include <winioctl.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define CORAL_DOS_DRIVER_NAME L"\\\\?\\ApexDriver"
#define APEX_DEVICE_NAME_BASE "ApexDevice"
#define APEX_MAX_DEVICES 256  // arbitrary

#define APEX_DRIVER_TYPE 43000
#define APEX_DRIVER_IOCTL_ENUM_BASE 2833

/*
 * Common structure for ioctls associating an eventfd with a device interrupt,
 * when using the Gasket interrupt module.
 */
struct gasket_set_event_ioctl {
  uint64_t int_num;
  WCHAR event_name[MAX_PATH];
};

/*
 * Common structure for ioctls mapping or unmapping kernel address space into
 * user space.
 */
struct gasket_address_map_ioctl {
  uint64_t base_addr;
  uint64_t offset;
  uint64_t size;
  uint64_t dev_dma_addr;
  uint32_t flags;
  uint64_t* virtaddr;
};

// Device type -- in the "User Defined" range."
#define GASKET_TYPE 43000
#define WINDOWS_GASKET_IOCTL_BASE 2833

#define _CTL_CODE(base, nr, buffering, access)                                \
  CTL_CODE(GASKET_TYPE, WINDOWS_GASKET_IOCTL_BASE + (base) + (nr), buffering, \
           access)

#define _IO(type, nr) _CTL_CODE(type, nr, METHOD_NEITHER, FILE_ANY_ACCESS)
#define _IOW(type, nr, size) \
  _CTL_CODE(type, nr, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define _IOR(type, nr, size) \
  _CTL_CODE(type, nr, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define _IOWR(type, nr, size) \
  _CTL_CODE(type, nr, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

// Ioctl numbers "nr" should not overlap with ones defined in
// linux_gasket_ioctl.h


/*
 * Tells the kernel to map "size" bytes at offset "offset" from device BAR2
 * returns user mode virtual address in "virtaddr".
 */
#define GASKET_IOCTL_MAP_HDW_VIEW \
  _IOWR(GASKET_IOCTL_BASE, 14, struct gasket_address_map_ioctl)

/*
 * Unmaps device BAR2 region previously mapped to user mode virtual address in
 * "virtaddr".
 */
#define GASKET_IOCTL_UNMAP_HDW_VIEW \
  _IOWR(GASKET_IOCTL_BASE, 15, struct gasket_address_map_ioctl)

/*
 * Tells the kernel to map "size" bytes of kernel allocated common buffer which
 * was previously mapped to device's "dev_dma_addr" address.
 * Returns user mode virtual address in "virtaddr".
 */
#define GASKET_IOCTL_MAP_UMDMA_VIEW \
  _IOWR(GASKET_IOCTL_BASE, 16, struct gasket_address_map_ioctl)

/*
 * Unmaps kernel common buffer mapped to "virtaddr" user mode virtual address.
 */
#define GASKET_IOCTL_UNMAP_UMDMA_VIEW \
  _IOWR(GASKET_IOCTL_BASE, 17, struct gasket_address_map_ioctl)


#ifndef __KERNEL__

#include "port/logging.h"
#include "port/stringprintf.h"

namespace platforms {
namespace darwinn {
namespace driver {

template <typename IPT>
int ioctl(FileDescriptor fd, ULONG ctlcode, IPT* ipt) {
  ULONG access = (ctlcode >> 14) & 0x00000003;
  ULONG transfer_type = ctlcode & 0x00000003;
  //
  BOOL rc = 0;

  if (transfer_type != METHOD_BUFFERED) {
    VLOG(1) << StringPrintf(
        "Parameter error: Windows ioctl(x%X): invalid transfer-type", ctlcode);
    return -EINVAL;
  }

  if (access == FILE_ANY_ACCESS) {
    // No data transfer: we should not have a buffer
    if ((ipt != NULL) || (sizeof(*ipt) != 0)) {
      VLOG(1) << StringPrintf(
          "Parameter error: Windows ioctl(x%X): no file access with a buffer",
          ctlcode);
      return -EINVAL;
    }
  } else {
    // We should have a buffer
    if ((ipt == NULL) || (sizeof(*ipt) == 0)) {
      VLOG(1) << StringPrintf(
          "Parameter error: Windows ioctl(x%X): file access without a buffer",
          ctlcode);
      return -EINVAL;
    }
  }

  switch (access) {
    case FILE_ANY_ACCESS:
      rc = DeviceIoControl(fd, ctlcode, NULL, 0, NULL, 0, NULL, NULL);
      break;

    case FILE_WRITE_DATA:
      rc = DeviceIoControl(fd, ctlcode, ipt, sizeof(*ipt), NULL, 0, NULL, NULL);
      break;

    case FILE_READ_DATA:
      rc = DeviceIoControl(fd, ctlcode, NULL, 0, ipt, sizeof(*ipt), NULL, NULL);
      break;

    case (FILE_WRITE_DATA | FILE_READ_DATA):
      rc = DeviceIoControl(fd, ctlcode, ipt, sizeof(*ipt), ipt, sizeof(*ipt),
                           NULL, NULL);
  }

  if (rc == 0) {
    DWORD gle = GetLastError();
    VLOG(1) << StringPrintf(
        "Execute error: Windows ioctl(x%X): gle=%d strerror=%s", ctlcode, gle,
        strerror(errno));
  }

  return (rc != 0 /*good result*/) ? 0 /*linux-style no error*/ : errno;
}

}  // namespace driver
}  // namespace darwinn
}  // namespace platforms
#endif  // ifndef __KERNEL__

#endif /* __WINDOWS_GASKET_IOCTL_H__ */
