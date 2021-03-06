#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
	char *arg = strtok ( NULL," " );
	int i;
	if(arg == NULL)
	{
		cpu_exec(1);
	}
	else
	{
	i =atoi(arg);
	if(i==0)
	{
		cpu_exec(1);
	}
	if(i==-1)
	{
		cpu_exec(-1);
	}
	else
	{
		if(i<0)
		{
			printf("Error!\n");

		}
		else
		{
			cpu_exec(i);
		}
	}
	}
	return 0;
}

static int cmd_info(char *args)
{
	char *a[8]={"eax","ecx","edx","ebx","esp","ebp","esi","edi"};
	char *arg =strtok (NULL , " ");
	int i;
	if(arg == NULL)
	{
		printf("Error!\n");
	}
	else if(strcmp(arg,"r")==0)
	{
		for(i=0;i<8;i++)
		{
			printf("%s\t%0x\t%d\n",a[i],cpu.gpr[i]._32,cpu.gpr[i]._32);
		}
    	printf("eip\t%0x\t%d\n",cpu.eip,cpu.eip);
	}
	return 0;
}

static int cmd_x(char *args)
{
	char *arg=strtok(NULL," ");
	int  n=atoi(arg);
	char *address=strtok(NULL," ");
	int  addr =strtol(address ,NULL ,16);
	int i,j;
	if (args == NULL || address == NULL || arg == NULL)
	{
		printf("ERROR!\n");
	}
	else
	{
		printf("dump memory start address:%0x\tlen:%d\n",addr,n);
		printf("\n");
		for(j=0;j<n;j++)
		{
			printf("%08x\t", addr);        //print address
			
			for(i=0;i<4;i++)
			{
				printf("%02x\t",swaddr_read(addr,1));
				addr++;
			}
			printf("\n");
		}
	}
	return 0;
}

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si","Single step", cmd_si },
	{ "info", "Print state", cmd_info},
	{ "x", "Scan the address", cmd_x},
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
