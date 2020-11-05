/* Common Gasket device kernel and user space declarations. */

#ifndef __GASKET_IOCTL_H__
#error This file should not be included directly include gasket_ioctl.h instead
#endif

/* ioctl structure declarations */

/* Ioctl structures are padded to a multiple of 64 bits */
/* and padded to put 64 bit values on 64 bit boundaries. */
/* Unsigned 64 bit integers are used to hold pointers. */
/* This helps compatibility between 32 and 64 bits. */

/*
 * Common structure for ioctls associating an eventfd with a device interrupt,
 * when using the Gasket interrupt module.
 */
struct gasket_interrupt_eventfd {
  uint64_t interrupt;
  uint64_t event_fd;
};

/*
 * Common structure for ioctls mapping and unmapping buffers when using the
 * Gasket page_table module.
 */
struct gasket_page_table_ioctl {
  uint64_t page_table_index;
  uint64_t size;
  uint64_t host_address;
  uint64_t device_address;
};

/*
 * Definitions for gasket_page_table_ioctl_flags.flags bitfield.
 */
#define GASKET_PT_FLAGS_DMA_DIRECTION_SHIFT 1
#define GASKET_PT_FLAGS_DMA_DIRECTION_WIDTH 2

/*
 * Value for gasket_page_table_ioctl_flags.flags.DMA_DIRECTION.
 * Mirrors kernel dma_data_direction definition in dma-direction.h.
 */
enum dma_data_direction {
  DMA_BIDIRECTIONAL = 0,
  DMA_TO_DEVICE = 1,
  DMA_FROM_DEVICE = 2,
  DMA_NONE = 3,
};

/*
 * Structure for ioctl mapping buffers with flags when using the Gasket
 * page_table module.
 */
struct gasket_page_table_ioctl_flags {
  struct gasket_page_table_ioctl base;
  /*
   * Flags indicating status and attribute requests from the host.
   * NOTE: Set RESERVED bits to 0 to ensure backwards compatibility.
   *
   * Bitfields:
   *   [0]     - RESERVED
   *   [2:1]   - DMA_DIRECTION: dma_data_direction requested by host
   *   [31:3]  - RESERVED
   */
  uint32_t flags;
};

/*
 * Common structure for ioctls mapping and unmapping buffers when using the
 * Gasket page_table module.
 * dma_address: phys addr start of coherent memory, allocated by kernel
 */
struct gasket_coherent_alloc_config_ioctl {
  uint64_t page_table_index;
  uint64_t enable;
  uint64_t size;
  uint64_t dma_address;
};

/*
 * Common structure for ioctls mapping and unmapping dma-bufs when using the
 * Gasket page_table module.
 * map: boolean, non-zero to map, 0 to unmap.
 * flags: see gasket_page_table_ioctl_flags.flags.
 */
struct gasket_page_table_ioctl_dmabuf {
  uint64_t page_table_index;
  uint64_t device_address;
  int dmabuf_fd;
  uint32_t num_pages;
  uint32_t map;
  uint32_t flags;
};


/* Reset the device. */
// NOLINTNEXTLINE
#define GASKET_IOCTL_RESET _IO(GASKET_IOCTL_BASE, 0)

/* Associate the specified [event]fd with the specified interrupt. */
#define GASKET_IOCTL_SET_EVENTFD \
  _IOW(GASKET_IOCTL_BASE, 1, struct gasket_interrupt_eventfd)

/*
 * Clears any eventfd associated with the specified interrupt. The (ulong)
 * argument is the interrupt number to clear.
 */
// NOLINTNEXTLINE
#define GASKET_IOCTL_CLEAR_EVENTFD _IOW(GASKET_IOCTL_BASE, 2, unsigned long)

/*
 * [Loopbacks only] Requests that the loopback device send the specified
 * interrupt to the host. The (ulong) argument is the number of the interrupt to
 * send.
 */
#define GASKET_IOCTL_LOOPBACK_INTERRUPT \
  _IOW(GASKET_IOCTL_BASE, 3, unsigned long)  // NOLINT

/* Queries the kernel for the number of page tables supported by the device. */
#define GASKET_IOCTL_NUMBER_PAGE_TABLES _IOR(GASKET_IOCTL_BASE, 4, uint64_t)

/*
 * Queries the kernel for the maximum size of the page table.  Only the size and
 * page_table_index fields are used from the struct gasket_page_table_ioctl.
 */
#define GASKET_IOCTL_PAGE_TABLE_SIZE \
  _IOWR(GASKET_IOCTL_BASE, 5, struct gasket_page_table_ioctl)

/*
 * Queries the kernel for the current simple page table size.  Only the size and
 * page_table_index fields are used from the struct gasket_page_table_ioctl.
 */
#define GASKET_IOCTL_SIMPLE_PAGE_TABLE_SIZE \
  _IOWR(GASKET_IOCTL_BASE, 6, struct gasket_page_table_ioctl)

/*
 * Tells the kernel to change the split between the number of simple and
 * extended entries in the given page table. Only the size and page_table_index
 * fields are used from the struct gasket_page_table_ioctl.
 */
#define GASKET_IOCTL_PARTITION_PAGE_TABLE \
  _IOW(GASKET_IOCTL_BASE, 7, struct gasket_page_table_ioctl)

/*
 * Tells the kernel to map size bytes at host_address to device_address in
 * page_table_index page table.
 */
#define GASKET_IOCTL_MAP_BUFFER \
  _IOW(GASKET_IOCTL_BASE, 8, struct gasket_page_table_ioctl)

/*
 * Tells the kernel to unmap size bytes at host_address from device_address in
 * page_table_index page table.
 */
#define GASKET_IOCTL_UNMAP_BUFFER \
  _IOW(GASKET_IOCTL_BASE, 9, struct gasket_page_table_ioctl)

/* Clear the interrupt counts stored for this device. */
#define GASKET_IOCTL_CLEAR_INTERRUPT_COUNTS _IO(GASKET_IOCTL_BASE, 10)

/* Enable/Disable and configure the coherent allocator. */
#define GASKET_IOCTL_CONFIG_COHERENT_ALLOCATOR \
  _IOWR(GASKET_IOCTL_BASE, 11, struct gasket_coherent_alloc_config_ioctl)

/*
 * Tells the kernel to map size bytes at host_address to device_address in
 * page_table_index page table. Passes flags to indicate additional attribute
 * requests for the mapped memory.
 */
#define GASKET_IOCTL_MAP_BUFFER_FLAGS \
  _IOW(GASKET_IOCTL_BASE, 12, struct gasket_page_table_ioctl_flags)

/*
 * Tells the kernel to map/unmap dma-buf with fd to device_address in
 * page_table_index page table.
 */
#define GASKET_IOCTL_MAP_DMABUF \
  _IOW(GASKET_IOCTL_BASE, 13, struct gasket_page_table_ioctl_dmabuf)

