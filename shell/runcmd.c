#include "runcmd.h"

#define POS_PRIMER_ELEMENTO 0
#define PID_PROC_HIJO 0
#define SALIDA_EXITOSA 0
#define WAITPID_OPCION_DEFAULT 0

// Declaración de variables globales.
int status = 0;
struct cmd *parsed_pipe;

// runs the command in 'cmd'.
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;

	// if the "enter" key is pressed.
	// just print the prompt again.
	if (cmd[POS_PRIMER_ELEMENTO] == END_STRING)
		return SALIDA_EXITOSA;

	// "history" built-in call.
	if (history(cmd))
		return SALIDA_EXITOSA;

	// "cd" built-in call.
	if (cd(cmd))
		return SALIDA_EXITOSA;

	// "exit" built-in call
	if (exit_shell(cmd))
		return EXIT_SHELL;

	// "pwd" built-in call.
	if (pwd(cmd))
		return SALIDA_EXITOSA;

	// parses the command line.
	parsed = parse_line(cmd);

	// forks and run the command.
	if ((p = fork()) == PID_PROC_HIJO) {
		// keep a reference.
		// to the parsed pipe cmd.
		// so it can be freed later.
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
		exit(SALIDA_EXITOSA);
	}

	// stores the pid of the process.
	parsed->pid = p;

	if (parsed->type == BACK) {
		print_back_info(parsed);

	} else {
		// waits for the process to finish.
		waitpid(p, &status, WAITPID_OPCION_DEFAULT);
		print_status_info(parsed);
	}
	free_command(parsed);

	return SALIDA_EXITOSA;
}
