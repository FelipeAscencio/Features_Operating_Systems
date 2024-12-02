#include "exec.h"

#define PID_PROC_HIJO 0
#define POS_PRIMER_ELEMENTO 0
#define CERO 0
#define UNO 1
#define POS_FD_LEC 0
#define POS_FD_ESC 1
#define VALOR_DE_ERROR -1
#define OVERWRITE_ENV 1
#define TAMANIO_PIPE 2

extern pid_t bg_pgid;  // Permitimos el acceso a 'bg_pgid' en este archivo.

// sets "key" with the key part of "arg"
// and null-terminates it.
//
// Example:
//  - KEY=value.
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0'].
//  key = "KEY".
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = CERO; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it.
// "idx" should be the index in "arg" where "=" char
// resides.
//
// Example:
//  - KEY=value.
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0'].
//  value = "value".
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + UNO), j = CERO; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line.
static void
set_environ_vars(char **eargv, int eargc)
{
	char key[BUFLEN];
	char value[BUFLEN];

	for (int i = CERO; i < eargc; i++) {
		int idx = block_contains(eargv[i], '=');
		if (idx == VALOR_DE_ERROR) {
			fprintf(stderr, "Formato incorrecto de variable de entorno: %s\n", eargv[i]);
			continue;
		}

		// Extrae la clave y el valor.
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, idx);

		// Establece la variable de entorno.
		if (setenv(key, value, OVERWRITE_ENV) == VALOR_DE_ERROR) {
			perror("Error al establecer la variable de entorno");
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor.
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	mode_t mode = 0;

	if (flags & O_CREAT) {
		mode = S_IWUSR | S_IRUSR;
	}
	fd = open(file, flags, mode);
	if (fd < PID_PROC_HIJO) {
		perror("Error al abrir el archivo");
		return VALOR_DE_ERROR;
	}
	return fd;
}

// Abre un archivo y redirige el flujo especificado (stdin, stdout, stderr).
static void
redirect_fd(char *file, int flags, int fd_to_redirect)
{
	int fd = open_redir_fd(file, flags);
	if (fd < PID_PROC_HIJO) {
		perror("Error al abrir el archivo");
		_exit(VALOR_DE_ERROR);
	}
	if (dup2(fd, fd_to_redirect) < PID_PROC_HIJO) {
		perror("Error al redirigir el flujo");
		_exit(VALOR_DE_ERROR);
	}
	close(fd);  // Cierra el descriptor de archivo ya que no es necesario.
}

// Redirige el flujo de entrada estándar (stdin) si es necesario.
static void
redirect_stdin(char *infile)
{
	if (infile && strlen(infile) > CERO) {
		redirect_fd(infile, O_RDONLY, STDIN_FILENO);
	}
}

// Redirige el flujo de salida estándar (stdout) si es necesario.
static void
redirect_stdout(char *outfile)
{
	if (outfile && strlen(outfile) > CERO) {
		redirect_fd(outfile, O_CREAT | O_WRONLY | O_TRUNC, STDOUT_FILENO);
	}
}

// Redirige el flujo de error estándar (stderr) si es necesario.
static void
redirect_stderr(char *errfile)
{
	if (errfile && strlen(errfile) > CERO) {
		redirect_fd(errfile, O_CREAT | O_WRONLY | O_TRUNC, STDERR_FILENO);
	}
}

// executes a command - does not return.
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases.
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);

		if (execvp(e->argv[POS_PRIMER_ELEMENTO], e->argv) < CERO) {
			perror("Error en execvp");
			free_command((struct cmd *) e);
			exit(VALOR_DE_ERROR);
		}
		break;
	}
	case BACK: {
		// runs a command in background.
		b = (struct backcmd *) cmd;

		if (bg_pgid == PID_PROC_HIJO) {
			bg_pgid = getpid();  // Primer proceso en segundo plano establece bg_pgid.
		}
		setpgid(CERO, bg_pgid);  // Asigna al grupo de procesos de fondo.
		exec_cmd(b->c);
		break;
	}
	case REDIR: {
		// changes the input/output/stderr flow.
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero.
		r = (struct execcmd *) cmd;

		redirect_stdin(r->in_file);
		redirect_stdout(r->out_file);

		if (strcmp(r->err_file, r->out_file) == CERO) {
			dup2(STDOUT_FILENO, STDERR_FILENO);
		} else {
			redirect_stderr(r->err_file);
		}

		r->type = EXEC;
		exec_cmd((struct cmd *) r);

		break;
	}
	case PIPE: {
		// pipes two commands.
		p = (struct pipecmd *) cmd;

		int pipefd[TAMANIO_PIPE];
		if (pipe(pipefd) < PID_PROC_HIJO) {
			perror("Error en pipe");
			exit(VALOR_DE_ERROR);
		}

		pid_t pid1 = fork();
		if (pid1 < PID_PROC_HIJO) {
			perror("Error en fork");
			exit(VALOR_DE_ERROR);
		}

		if (pid1 == PID_PROC_HIJO) {  // Proceso hijo 1.
			close(pipefd[POS_FD_LEC]);
			if (dup2(pipefd[POS_FD_ESC], STDOUT_FILENO) < CERO) {
				perror("Error al redirigir stdout");
				exit(VALOR_DE_ERROR);
			}
			close(pipefd[POS_FD_ESC]);

			// Ejecuta el primer comando.
			exec_cmd(p->leftcmd);
		}

		// Proceso padre.
		close(pipefd[POS_FD_ESC]);
		pid_t pid2 = fork();
		if (pid2 < PID_PROC_HIJO) {
			perror("Error en fork");
			exit(VALOR_DE_ERROR);
		}

		if (pid2 == PID_PROC_HIJO) {  // Proceso hijo 2.
			if (dup2(pipefd[POS_FD_LEC], STDIN_FILENO) < CERO) {
				perror("Error al redirigir stdin");
				exit(VALOR_DE_ERROR);
			}
			close(pipefd[POS_FD_LEC]);

			// Ejecuta el segundo comando.
			exec_cmd(p->rightcmd);
		}

		// Proceso padre.
		close(pipefd[POS_FD_LEC]);
		wait(NULL);  // Espera a que termine un hijo.
		wait(NULL);  // Espera a que termine el otro hijo.

		break;
	}
	}
}
