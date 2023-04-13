// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

// LAB 1: add your command to here...
static struct Command commands[] = {
    { "help", "Display this list of commands", mon_help },
    { "kerninfo", "Display information about the kernel", mon_kerninfo },
    { "backtrace", "Print backtrace of kernel stack", mon_backtrace },
    { "debuginfo", "Display information about a given address in kernel", mon_debuginfo }
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}


int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
    uint32_t* ebp = (uint32_t*) read_ebp(); // Get the base pointer (ebp)
    cprintf("Stack backtrace:\n");

    while (ebp) { // As long as ebp is not null
        cprintf("  ebp %08x eip %08x args ", ebp, ebp[1]);
        int i;
        for (i = 2; i < 7; i++) { // Print the next 5 arguments on the stack
            cprintf("%08x ", ebp[i]);
        }
        cprintf("\n");

        struct Eipdebuginfo info;
        debuginfo_eip(ebp[1], &info);
        cprintf("\t%s:%d: ", info.eip_file, info.eip_line);
        cprintf("%.*s", info.eip_fn_namelen, info.eip_fn_name);
        cprintf("+%d\n", ebp[1] - info.eip_fn_addr);

        ebp = (uint32_t*) ebp[0]; // Move up the stack by setting ebp to the value at the current ebp address
    }

    return 0;
}


int
mon_debuginfo(int argc, char **argv, struct Trapframe *tf)
{
    // Check if user provided an address argument
    if (argc != 2) {
        cprintf("Usage: debuginfo <address>\n");
        return 0;
    }

    uintptr_t addr = strtol(argv[1], NULL, 0);

    // Fill the Eipdebuginfo struct with information about the given address
    struct Eipdebuginfo info;
    if (debuginfo_eip(addr, &info) < 0) {
        cprintf("Error: could not get debug info for address 0x%08x\n", addr);
        return 0;
    }

    // Print the debug info
    cprintf("Function name: %.*s+%u\n", info.eip_fn_namelen, info.eip_fn_name, addr - info.eip_fn_addr);
    cprintf("Source file: %.*s:%u\n", info.eip_file_namelen, info.eip_file_name, info.eip_line);
    cprintf("Arguments: %u\n", info.eip_fn_narg);
    for (int i = 0; i < info.eip_fn_narg; i++) {
        cprintf("    arg %u: %08x\n", i, *((uint32_t *) (addr + (i+1)*4)));
    }

    return 0;
}


/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");


	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
