#include "parsing.h"

#define POS_PRIMER_ELEMENTO 0
#define CERO 0
#define UNO 1

// Declaración de variables globales.
extern int status;

// parses an argument of the command stream input.
static char *
get_token(char *buf, int idx)
{
	char *tok;
	int i;

	tok = (char *) calloc(ARGSIZE, sizeof(char));
	i = 0;

	while (buf[idx] != SPACE && buf[idx] != END_STRING) {
		tok[i] = buf[idx];
		i++;
		idx++;
	}

	return tok;
}

// parses and changes stdin/out/err if needed.
static bool
parse_redir_flow(struct execcmd *c, char *arg)
{
	int inIdx, outIdx;

	// flow redirection for output.
	if ((outIdx = block_contains(arg, '>')) >= CERO) {
		switch (outIdx) {
		// stdout redir.
		case CERO: {
			strcpy(c->out_file, arg + UNO);
			break;
		}
		// stderr redir.
		case UNO: {
			if (strcmp(&arg[outIdx + UNO], "&1") == CERO) {
				strcpy(c->err_file,
				       c->out_file);  // Apunta a lo mismo que stdout.
			} else {
				strcpy(c->err_file, &arg[outIdx + UNO]);
			}
			break;
		}
		}

		free(arg);
		c->type = REDIR;

		return true;
	}

	// flow redirection for input.
	if ((inIdx = block_contains(arg, '<')) >= CERO) {
		// stdin redir.
		strcpy(c->in_file, arg + UNO);

		c->type = REDIR;
		free(arg);

		return true;
	}

	return false;
}

// parses and sets a pair KEY=VALUE.
// environment variable.
static bool
parse_environ_var(struct execcmd *c, char *arg)
{
	// sets environment variables apart from the
	// ones defined in the global variable "environ".
	if (block_contains(arg, '=') > CERO) {
		// checks if the KEY part of the pair.
		// does not contain a '-' char which means
		// that it is not a environ var, but also
		// an argument of the program to be executed.
		// (For example:
		// 	./prog -arg=value
		// 	./prog --arg=value
		// )
		if (block_contains(arg, '-') < CERO) {
			c->eargv[c->eargc++] = arg;
			return true;
		}
	}

	return false;
}

// this function will be called for every token, and it should
// expand environment variables. In other words, if the token
// happens to start with '$', the correct substitution with the
// environment value should be performed. Otherwise the same
// token is returned. If the variable does not exist, an empty string should be
// returned within the token.
static char *
expand_environ_var(char *arg)
{
	if (arg[POS_PRIMER_ELEMENTO] != '$') {
		return arg;
	}

	if (strcmp(arg, "$?") == CERO) {
		sprintf(arg,
		        "%i",
		        status);  // escribo el entero status, en la cadena arg.
		return arg;
	}

	char *var_name = arg + UNO;
	char *value = getenv(var_name);

	if (value == NULL) {
		arg[POS_PRIMER_ELEMENTO] = '\0';
		return arg;
	}

	if (strlen(value) > strlen(arg)) {
		char *new_arg = realloc(arg, strlen(value) + UNO);
		if (new_arg == NULL) {
			perror("realloc failed");
			exit(EXIT_FAILURE);
		}
		arg = new_arg;
	}

	strcpy(arg, value);
	return arg;
}

// parses one single command having into account:
// - the arguments passed to the program.
// - stdin/stdout/stderr flow changes.
// - environment variables (expand and set).
static struct cmd *
parse_exec(char *buf_cmd)
{
	struct execcmd *c;
	char *tok;
	int idx = 0;
	int argc = 0;

	c = (struct execcmd *) exec_cmd_create(buf_cmd);

	while (buf_cmd[idx] != END_STRING) {
		tok = get_token(buf_cmd, idx);
		idx = idx + strlen(tok);

		if (buf_cmd[idx] != END_STRING)
			idx++;

		if (parse_redir_flow(c, tok))
			continue;

		if (parse_environ_var(c, tok))
			continue;

		tok = expand_environ_var(tok);

		if (strlen(tok) > CERO)
			c->argv[argc++] = tok;
	}

	c->argv[argc] = (char *) NULL;
	c->argc = argc;

	return (struct cmd *) c;
}

// parses a command knowing that it contains the '&' char.
static struct cmd *
parse_back(char *buf_cmd)
{
	int i = 0;
	struct cmd *e;

	while (buf_cmd[i] != '&')
		i++;

	buf_cmd[i] = END_STRING;

	e = parse_exec(buf_cmd);

	return back_cmd_create(e);
}

// parses a command and checks if it contains the '&'.
// (background process) character.
static struct cmd *
parse_cmd(char *buf_cmd)
{
	if (strlen(buf_cmd) == CERO)
		return NULL;

	int idx;

	// checks if the background symbol is after
	// a redir symbol, in which case
	// it does not have to run in in the 'back'.
	if ((idx = block_contains(buf_cmd, '&')) >= CERO &&
	    buf_cmd[idx - UNO] != '>')
		return parse_back(buf_cmd);

	return parse_exec(buf_cmd);
}

// parses the command line
// looking for the pipe character '|'.
struct cmd *
parse_line(char *buf)
{
	struct cmd *r, *l;

	char *right = split_line(buf, '|');

	l = parse_cmd(buf);
	if (block_contains(right, '|') > CERO) {
		r = parse_line(right);
	} else {
		r = parse_cmd(right);
	}

	return pipe_cmd_create(l, r);
}
