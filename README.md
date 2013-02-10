# Hardware breakpoints

This is a debugging helper class which lets you set breakpoints on the fly from
within code. This is mainly useful for the case where you have a variable that
you know is getting trashed, but you have no idea who is trashing it. You can
cause the debugger to break in at the very moment the variable is changed. The
really cool thing is that this makes use of the Intel Pentium's built-in debug
registers, which means that it really will stop no matter _what_ code is
executing, even if it's down in the NT kernel, in a different thread, or
whatever.

(Visual C++ has the ability to set a breakpoint when a variable's value
changes, but these breakpoints can be very hard to use, especially on local
variables.)

## Usage

The class is called `HardwareBreakpoint`. If, for example, you have a `DWORD`
called `x` and you want to break the next time it's written to, do this:

```c++
    DWORD x = 1;

    HardwareBreakpoint bp;
    bp.Set(&x, sizeof(x), HardwareBreakpoint::Write);
```

As your code continues to execute, if Visual C++ stops at a location where you
didn't have a VC++ breakpoint set, it's possible that the `HardwareBreakpoint`
triggered. If it did, the assembly instruction _immediately before_ the
instruction pointer is the one that caused it to trigger.

The breakpoint will be cleared automatically when the `HardwareBreakpoint`
object falls out of scope. Or, to clear it explicitly, do this:

```c++
    bp.Clear();
```

You can also break the moment any code even tries to read the value of the
variable x! (Actually, this will break when someone tries to read _or_ write
it.)

```c++
    bp.Set(&x, sizeof(x), HardwareBreakpoint::Read);
```

## Notes

This code is Intel-specific. It will run on Win95/98/ME and on WinNT/2000/XP,
as long as the machine is running an Intel or compatible chip (e.g. not Alpha).

There are certain limitations when the breakpoint is triggered inside system
code (these same limitations apply to hardware breakpoints set by Visual C++ or
any other debugger):

*   On WinNT/2000/XP, some system code runs in ring 3 (with user privileges),
    and other system code runs in ring 0. If a hardware breakpoint triggers
    inside ring 0 code, the debugger *will* stop, but not exactly when the data
    write happens -- it will stop after execution transitions back to ring 3
    code. In practice, this is not a problem at all -- when the breakpoint
    triggers, it's usually quite easy to figure out what happened.
*   On Win95/98/ME, the situation is not as good. _All_ system code is treated
    by the debugger as untouchable. Worse, whenever any hardware breakpoint
    triggers inside system code, you'll never see the breakpoint trigger. For
    example, if a call to strcpy() would cause your breakpoint to trigger,
    you'll see it, because strcpy() is just a regular part of your application.
    But if a call to lstrcpy() would cause your breakpoint to trigger, you
    won't see it, because lstrcpy() is system code.

You can do "bp.Clear()" from the QuickWatch dialog to clear a breakpoint from
within the debugger.

The second parameter is the number of bytes to watch. This _must_ be either 1,
2, or 4. Also, the variable being watched _must_ be aligned appropriately (e.g.
if you're watching 4 bytes, they must be DWORD-aligned). If you don't do this,
you'll get an error when you try to set the breakpoint. There is a limit of 4
hardware breakpoints. Because of this (and also because of common sense), don't
check in code that uses breakpoints -- just write the code temporarily to find
bugs, but delete it before checking in your source into your source control
system.

It's important to pass in the address that you actually want to watch. Consider
these two similar but different cases:

```c++
    long *my_array;
    my_array = new long[3];
    
    HardwareBreakpoint bpArrayPtr, bpArrayFirstElement;
    
    // break if someone changes the POINTER
    bpArrayPtr.Set(&my_array, sizeof(long*), HardwareBreakpoint::Write);
    
    // break if someone changes the FIRST ELEMENT POINTED TO
    bpArrayFirstElement.Set(&my_array[0], sizeof(long), HardwareBreakpoint::Write);
```

## How does it work?

The Intel x86 CPUs have some special registers which are intended for debugging
use only. By storing special values into these registers, a program can ask the
CPU to execute an INT 1 (interrupt 1) instruction immediately whenever a
specified memory location is read from or written to. (They can also stop when
a memory address is about to be executed as code, but my `HardwareBreakpoint`
class doesn't use that functionality.)

INT 1 also happens to be the interrupt that's executed by the CPU after a
debugger asks the CPU to single-step one assembly line of the program.  And by
good fortune, when Visual C++ encounters an INT 1 that it wasn't expecting to
see (as is the case when a `HardwareBreakpoint` breakpoint is triggered), it
responds gracefully, stopping the debugger at the appropriate instruction.
