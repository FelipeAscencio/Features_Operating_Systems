#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

#define CERO 0
#define VALOR_DE_ERROR -1
#define SALIDA_EXITOSA 0

// Declaración del handler implementado.
void sigchld_handler(int signo);

// Declaración de variables globales.
pid_t bg_pgid = 0;
char prompt[PRMTLEN] = { 0 };

// Manejador de señales para SIGCHLD.
void
sigchld_handler(int signo)
{
	(void) signo;  // Evita la advertencia de parámetro no utilizado.
	int saved_errno = errno;
	pid_t pid;

	while ((pid = waitpid(-bg_pgid, NULL, WNOHANG)) > CERO) {
		printf_debug("\n==> terminado: PID=%d\n", pid);
	}
	errno = saved_errno;
}

// Inicializa el manejador de señales.
static void
init_signal_handlers()
{
	struct sigaction sa;

	sa.sa_handler = sigchld_handler;  // Asigna el manejador de SIGCHLD.
	sigemptyset(&sa.sa_mask);  // No bloquea ninguna otra señal durante el manejador.
	sa.sa_flags =
	        SA_RESTART |
	        SA_NOCLDSTOP;  // Reinicia llamadas interrumpidas y no captura señales de hijos detenidos.

	if (sigaction(SIGCHLD, &sa, NULL) == VALOR_DE_ERROR) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
}


// runs a shell command.
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell.
// with the "HOME" directory.
static void
init_shell()
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");

	if (chdir(home) < CERO) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}

	// Inicializa los manejadores de señales.
	init_signal_handlers();
}

int
main(void)
{
	init_shell();

	run_shell();

	return SALIDA_EXITOSA;
}
