## x86_64 Operating System

<img src="./images/lateOS.png" width="80%"/>

Operating System implementation as a course project for CSE 506 under Prof. Michael Ferdman. It has Cooperative multi-tasking with below functionalities :

  1) All functions from include/*.h must work
  2) Virtual memory/ring-3 user processes
  3) COW fork(), auto-growing stack, graceful SEGV
  4) tarfs: open, read, close, opendir, readdir, closedir
  5) read(stdin), write(stdout), write(stderr)
  6) Binaries: echo, sleep, cat, ls, kill -9, ps
  7) Command prompt - sbush

#### Suggested Plan of attack :

  1) Memory Subsystem (page descriptors, free list, page tables, kmalloc)
  2) rocess Subsystem (kernel threads, context switch)
  3) User-level Subsystem (VMAs/vm_map_entrys, switch to ring 3, page faults)
  4) I/O subsystem (syscalls, terminals, VFS, tarfs file access)
  5) Implement /bin/init (call fork()+exec()+wait() on /etc/rc, then exec() sbush)

#### The provided Makefile:
  1) builds a kernel
  2) copies it into rootfs/boot/kernel/kernel
  3) creates an ISO CD image with the rootfs/ contents

#### To boot the system in QEMU, run:
    qemu-system-x86_64 -curses -drive id=boot,format=raw,file=$USER.img,if=none -drive id=data,format=raw,file=$USER-data.img,if=none -device ahci,id=ahci -device ide-drive,drive=boot,bus=ahci.0 -device ide-drive,drive=data,bus=ahci.1 -gdb tcp::9999

Explanation of parameters:
  -curses         use a text console (omit this to use default SDL/VNC console)
  -drive ...      connect a CD-ROM or hard drive with corresponding image
  -device ...     configure an AHCI controller for the boot and data disks
  -gdb tcp::9999  listen for "remote" debugging connections on port NNNN
  -S              wait for GDB to connect at startup
  -no-reboot      prevent reboot when OS crashes

When using the -curses mode, switch to the qemu> console with ESC-2.

#### To connect a remote debugger to the running qemu VM, from a different window:
    gdb ./kernel

At the (gdb) prompt, connect with:

    target remote localhost:9999

### Help
<a href="http://wiki.osdev.org/Expanded_Main_Page">OSdev.org</a>

<a href="http://fxr.watson.org/">Browseable modern OS source codes</a>

<a href="http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html">GCC Inline Assembly</a>

<a href="https://eli.thegreenplace.net/2011/09/06/stack-frame-layout-on-x86-64/">x86_64 calling conventions</a>

<a href="https://eli.thegreenplace.net/2012/08/13/how-statically-linked-programs-run-on-linux/">How statically linked 
  programs run on linux</a>
  
<a href="http://www.unknownroad.com/rtfm/gdbtut/gdbtoc.html">GDB Tutorial</a>

<a href="http://www.gnu.org/copyleft/gpl.html">GNU Public License v3.0</a>

<a href="http://blog.raveendrasoori.com/2017/10/01/gdb-fix/"> Fixing GDB on local machine - Raveendra Soori</a>


### To work on your own machine
We do not provide support for it, but you can get your own usable environment for the course projects up and running by installing Ubuntu 16.04 and:

    apt-get install build-essential gdb git screen tmux emacs exuberant-ctags ccache eclipse xterm gtkwave tcl-dev zsh strace curl mkisofs qemu dosfstools syslinux
    
If you are working on your own machine, you will need to build gdb from source and apply a <a href="https://sourceware.org/bugzilla/attachment.cgi?id=8512">patch</a>.
