# Make uses /bin/sh by default, which is incompatible with the bashisms seen
# below.
SHELL := /bin/bash

ifeq ($(origin MAKEFILE_DIR), undefined)
	MAKEFILE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
endif

BUILDROOT ?= $(MAKEFILE_DIR)/..
BUILDDIR := $(BUILDROOT)/out
TOBUILDDIR = $(addprefix $(BUILDDIR)/,$(1))
MKDIR = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
CC=gcc
CXX=g++
FLATC=flatc

LIBEDGETPU_CFLAGS := \
	-fPIC \
	-Wall \
	-std=c99

LIBEDGETPU_CXXFLAGS := \
	-fPIC \
	-Wall \
	-std=c++17 \
	-DDARWINN_PORT_DEFAULT

LIBEDGETPU_LDFLAGS := \
	-Wl,-Map=$(BUILDDIR)/output.map \
	-shared \
	-Wl,--soname,libedgetpu.so.1 \
	-Wl,--version-script=$(BUILDROOT)/tflite/public/libedgetpu.lds \
	-fuse-ld=gold \
	-lflatbuffers \
	-labsl_flags_commandlineflag \
	-labsl_flags_internal \
	-labsl_flags_reflection \
	-labsl_flags_marshalling \
	-labsl_hash \
	-labsl_hashtablez_sampler \
	-labsl_raw_hash_set \
	-labsl_str_format_internal \
	-labsl_bad_optional_access \
	-lusb-1.0

LIBEDGETPU_FLATC_SRCS := \
	$(BUILDROOT)/executable/executable.fbs \
	$(BUILDROOT)/api/driver_options.fbs
LIBEDGETPU_FLATC_OBJS := $(call TOBUILDDIR,$(patsubst %.fbs,%_generated.h,$(LIBEDGETPU_FLATC_SRCS)))

LIBEDGETPU_FW_SRCS := \
	$(BUILDROOT)/driver/usb/apex_latest_single_ep.bin \
	$(BUILDROOT)/driver/usb/apex_latest_multi_ep.bin
LIBEDGETPU_FW_OUTPUT := $(call TOBUILDDIR, $(BUILDROOT)/driver/usb/usb_latest_firmware.h)

LIBEDGETPU_INCLUDES := \
	$(BUILDROOT) \
	$(TFROOT) \
	$(BUILDDIR) \
	$(BUILDDIR)/$(BUILDROOT)
LIBEDGETPU_INCLUDES := $(addprefix -I,$(LIBEDGETPU_INCLUDES))

LIBEDGETPU_CCSRCS := \
	$(BUILDROOT)/api/allocated_buffer.cc \
	$(BUILDROOT)/api/buffer.cc \
	$(BUILDROOT)/api/driver_options_helper.cc \
	$(BUILDROOT)/api/layer_information.cc \
	$(BUILDROOT)/api/tensor_util.cc \
	$(BUILDROOT)/api/watchdog.cc \
	$(BUILDROOT)/driver/aligned_allocator.cc \
	$(BUILDROOT)/driver/allocator.cc \
	$(BUILDROOT)/driver/beagle/beagle_kernel_top_level_handler.cc \
	$(BUILDROOT)/driver/beagle/beagle_pci_driver_provider.cc \
	$(BUILDROOT)/driver/beagle/beagle_pci_driver_provider_linux.cc \
	$(BUILDROOT)/driver/beagle/beagle_top_level_handler.cc \
	$(BUILDROOT)/driver/beagle/beagle_top_level_interrupt_manager.cc \
	$(BUILDROOT)/driver/beagle/beagle_usb_driver_provider.cc \
	$(BUILDROOT)/driver/device_buffer.cc \
	$(BUILDROOT)/driver/device_buffer_mapper.cc \
	$(BUILDROOT)/driver/dma_chunker.cc \
	$(BUILDROOT)/driver/dma_info.cc \
	$(BUILDROOT)/driver/dma_info_extractor.cc \
	$(BUILDROOT)/driver/driver.cc \
	$(BUILDROOT)/driver/driver_factory.cc \
	$(BUILDROOT)/driver/driver_factory_default.cc \
	$(BUILDROOT)/driver/executable_util.cc \
	$(BUILDROOT)/driver/instruction_buffers.cc \
	$(BUILDROOT)/driver/interrupt/grouped_interrupt_controller.cc \
	$(BUILDROOT)/driver/interrupt/interrupt_controller.cc \
	$(BUILDROOT)/driver/interrupt/top_level_interrupt_manager.cc \
	$(BUILDROOT)/driver/kernel/kernel_coherent_allocator.cc \
	$(BUILDROOT)/driver/kernel/kernel_event_handler.cc \
	$(BUILDROOT)/driver/kernel/kernel_interrupt_handler.cc \
	$(BUILDROOT)/driver/kernel/kernel_mmu_mapper.cc \
	$(BUILDROOT)/driver/kernel/kernel_registers.cc \
	$(BUILDROOT)/driver/kernel/linux/kernel_coherent_allocator_linux.cc \
	$(BUILDROOT)/driver/kernel/linux/kernel_event_handler_linux.cc \
	$(BUILDROOT)/driver/kernel/linux/kernel_event_linux.cc \
	$(BUILDROOT)/driver/kernel/linux/kernel_registers_linux.cc \
	$(BUILDROOT)/driver/memory/buddy_address_space.cc \
	$(BUILDROOT)/driver/memory/buddy_allocator.cc \
	$(BUILDROOT)/driver/memory/dual_address_space.cc \
	$(BUILDROOT)/driver/memory/mmio_address_space.cc \
	$(BUILDROOT)/driver/memory/mmu_mapper.cc \
	$(BUILDROOT)/driver/memory/nop_address_space.cc \
	$(BUILDROOT)/driver/mmio/coherent_allocator.cc \
	$(BUILDROOT)/driver/mmio_driver.cc \
	$(BUILDROOT)/driver/package_registry.cc \
	$(BUILDROOT)/driver/package_verifier.cc \
	$(BUILDROOT)/driver/real_time_dma_scheduler.cc \
	$(BUILDROOT)/driver/registers/registers.cc \
	$(BUILDROOT)/driver/request.cc \
	$(BUILDROOT)/driver/run_controller.cc \
	$(BUILDROOT)/driver/scalar_core_controller.cc \
	$(BUILDROOT)/driver/single_queue_dma_scheduler.cc \
	$(BUILDROOT)/driver/single_tpu_request.cc \
	$(BUILDROOT)/driver/usb/libusb_options_default.cc \
	$(BUILDROOT)/driver/usb/local_usb_device.cc \
	$(BUILDROOT)/driver/usb/usb_dfu_commands.cc \
	$(BUILDROOT)/driver/usb/usb_dfu_util.cc \
	$(BUILDROOT)/driver/usb/usb_driver.cc \
	$(BUILDROOT)/driver/usb/usb_io_request.cc \
	$(BUILDROOT)/driver/usb/usb_ml_commands.cc \
	$(BUILDROOT)/driver/usb/usb_registers.cc \
	$(BUILDROOT)/driver/usb/usb_standard_commands.cc \
	$(BUILDROOT)/driver_shared/time_stamper/driver_time_stamper.cc \
	$(BUILDROOT)/port/blocking_counter.cc \
	$(BUILDROOT)/port/default/port_from_tf/logging.cc \
	$(BUILDROOT)/port/default/port_from_tf/status.cc \
	$(BUILDROOT)/port/default/port_from_tf/statusor.cc \
	$(BUILDROOT)/port/default/stringprintf.cc \
	$(BUILDROOT)/port/shared_mutex.cc \
	$(BUILDROOT)/port/timer_portable.cc \
	$(BUILDROOT)/tflite/custom_op.cc \
	$(BUILDROOT)/tflite/custom_op_data.cc \
	$(BUILDROOT)/tflite/custom_op_direct.cc \
	$(BUILDROOT)/tflite/custom_op_user_data_direct.cc \
	$(BUILDROOT)/tflite/edgetpu_c.cc \
	$(BUILDROOT)/tflite/edgetpu_delegate_for_custom_op.cc \
	$(BUILDROOT)/tflite/edgetpu_delegate_for_custom_op_tflite_plugin.cc \
	$(TFROOT)/tensorflow/lite/core/c/common.cc \
	$(TFROOT)/tensorflow/lite/util.cc \
	$(TFROOT)/tensorflow/lite/array.cc
LIBEDGETPU_CCOBJS := $(call TOBUILDDIR,$(patsubst %.cc,%.o,$(LIBEDGETPU_CCSRCS)))

# In order to support direct and throttled mode - we need to compile two files
# twice (changing -DTHROTTLE_EDGE_TPU).
LIBEDGETPU_MAX_CCSRCS := \
	$(BUILDROOT)/tflite/edgetpu_context_direct.cc \
	$(BUILDROOT)/tflite/edgetpu_manager_direct.cc
LIBEDGETPU_MAX_CCOBJS := $(call TOBUILDDIR,$(patsubst %.cc,%.o,$(LIBEDGETPU_MAX_CCSRCS)))

LIBEDGETPU_STD_CCSRCS := \
	$(BUILDROOT)/tflite/edgetpu_context_direct.cc \
	$(BUILDROOT)/tflite/edgetpu_manager_direct.cc
LIBEDGETPU_STD_CCOBJS := $(call TOBUILDDIR,$(patsubst %.cc,%-throttled.o,$(LIBEDGETPU_STD_CCSRCS)))

.PHONY: libedgetpu

all: libedgetpu libedgetpu-throttled

clean:
	rm -rf $(BUILDDIR)

$(LIBEDGETPU_FLATC_OBJS) : $(BUILDDIR)/%_generated.h: %.fbs
	@$(MKDIR)
	@$(FLATC) --cpp --gen-object-api --force-empty --gen-mutable -o $(dir $@) $<

firmware:
	@mkdir -p $(dir $(LIBEDGETPU_FW_OUTPUT))
	@echo "namespace {" > $(LIBEDGETPU_FW_OUTPUT)
	@for FILE in $(LIBEDGETPU_FW_SRCS); do \
	  FILENAME=$${FILE##*/} ; \
	  FILEBASE=$${FILENAME%.*} ; \
	  echo "const unsigned char ""$$FILEBASE"" [] = {" >> $(LIBEDGETPU_FW_OUTPUT) ; \
	  xxd -i < "$$FILE" >> $(LIBEDGETPU_FW_OUTPUT) ; \
	  echo "};" >> $(LIBEDGETPU_FW_OUTPUT) ; \
	  echo "constexpr unsigned int ""$$FILEBASE""_len = sizeof(""$$FILEBASE"")/sizeof(unsigned char);" >> $(LIBEDGETPU_FW_OUTPUT) ; \
	  echo "" >> $(LIBEDGETPU_FW_OUTPUT) ; \
	done
	@echo "} // namespace" >> $(LIBEDGETPU_FW_OUTPUT)


$(LIBEDGETPU_CCOBJS) : $(BUILDDIR)/%.o: %.cc
	@$(MKDIR)
	@echo "Compiling $<"
	@$(CXX) $(LIBEDGETPU_CXXFLAGS) $(LIBEDGETPU_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(LIBEDGETPU_MAX_CCOBJS) : $(BUILDDIR)/%.o: %.cc
	@$(MKDIR)
	@echo "Compiling $<"
	@$(CXX) $(LIBEDGETPU_CXXFLAGS) $(LIBEDGETPU_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

$(LIBEDGETPU_STD_CCOBJS) : $(BUILDDIR)/%-throttled.o: %.cc
	@$(MKDIR)
	@echo "Compiling $<"
	@$(CXX) -DTHROTTLE_EDGE_TPU $(LIBEDGETPU_CXXFLAGS) $(LIBEDGETPU_INCLUDES) -c $< -MD -MT $@ -MF $(@:%o=%d) -o $@

libedgetpu: | firmware $(LIBEDGETPU_FLATC_OBJS) $(LIBEDGETPU_CCOBJS) $(LIBEDGETPU_MAX_CCOBJS)
	@mkdir -p $(BUILDDIR)/direct/k8
	@echo "Building libedgetpu.so"
	@$(CXX) $(LIBEDGETPU_CCFLAGS) $(LIBEDGETPU_LDFLAGS) ${LDFLAGS} $(LIBEDGETPU_CCOBJS) $(LIBEDGETPU_MAX_CCOBJS) -o $(BUILDDIR)/direct/k8/libedgetpu.so.1.0
	@ln -sf $(BUILDDIR)/direct/k8/libedgetpu.so.1.0 $(BUILDDIR)/direct/k8/libedgetpu.so.1

libedgetpu-throttled: | firmware $(LIBEDGETPU_FLATC_OBJS) $(LIBEDGETPU_CCOBJS) $(LIBEDGETPU_STD_CCOBJS)
	@mkdir -p $(BUILDDIR)/throttled/k8
	@echo "Building throttled libedgetpu.so"
	@$(CXX) $(LIBEDGETPU_CCFLAGS) $(LIBEDGETPU_LDFLAGS) ${LDFLAGS} $(LIBEDGETPU_CCOBJS) $(LIBEDGETPU_STD_CCOBJS) -o $(BUILDDIR)/throttled/k8/libedgetpu.so.1.0
	@ln -sf $(BUILDDIR)/throttled/k8/libedgetpu.so.1.0 $(BUILDDIR)/throttled/k8/libedgetpu.so.1
