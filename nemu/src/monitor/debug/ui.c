#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
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
	//printf("123"); read the src 
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char*args){
	int count=0;
	if (args == NULL){                       //default val , why C dont have deafult val in func?
		count = 1;
	} else {
		sscanf(args,"%d",&count);
	}
	cpu_exec(count);
	return 0;
}

 static int cmd_info(char*args){
	if(args == NULL) {
		printf("lack of options ,please input some .\n");
		return 0; 
	}
	char opt;
	sscanf(args, " %c", &opt);
	if(opt=='r'){
		printf("eax: 0x%08x \n", cpu.eax);
		printf("ecx: 0x%08x \n", cpu.ecx);
		printf("edx: 0x%08x \n", cpu.edx);
		printf("ebx: 0x%08x \n", cpu.ebx);
		printf("esp: 0x%08x \n", cpu.esp);
		printf("ebp: 0x%08x \n", cpu.ebp);
		printf("esi: 0x%08x \n", cpu.esi);
		printf("edi: 0x%08x \n", cpu.edi);
		printf("eip: 0x%08x \n", cpu.eip);
	}
	return 0;
} 

static int cmd_x(char*args){
	if(args == NULL) {
		printf("lack of options ,please input some .\n");
		return 0; 
	}
	//// easy way to scan mem, but nearly no use for expr :(
	// uint32_t num, addr;
	// sscanf(args, "%d", &num);
	// printf("%d\n%x\n%s\n",num,addr,args);
	// while (num--){
	// 	printf("address 0x%x: 0x%x\n", addr, swaddr_read(addr, 4));
	// 	addr += 4;
	// }/
	uint32_t num,addr;
	bool success;
	char *num_s=strtok(args," ");
	if(args==NULL){
		printf("lack of options ,please input some .\n");
		return 0; 
	}
	num=atoi(num_s);            
	char *arg = strtok(NULL, "");
	if(arg == NULL){
		printf("scan for what? \n");
		return 0;
	}
	addr = expr(arg, &success);
	if(!success) {
		printf("unexpected token before something\n");
		return 0;
	}
	// printf("%d\n",addr);
	  while (num--){
	  	printf("address 0x%x: 0x%x\n", addr, swaddr_read(addr, 4));
	  	addr += 4;
	 }
	return 0;
}

static int cmd_p(char*args){
	if(args == NULL) {
		printf("lack of options ,please input some .\n");
		return 0; 
	}
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "Step into implementation of N instructions after the suspension of execution.When N is notgiven,the default is 1.", cmd_si},
	{ "info", "r for print register state \n w for print watchpoint information", cmd_info},
	{ "p", "Expression evaluation", cmd_p}, //   should before scan :(  waste time
	{ "x", "Scan the memory", cmd_x },
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
