#include "builtin.h"

#define SALIDA_EXITOSA 0
#define CERO 0
#define UNO 1
#define CD_CMD_LEN 3

// returns true if the 'exit' call
// should be performed.
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == CERO)
		return UNO;

	return SALIDA_EXITOSA;
}

//  returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory').
// 	2. $ cd (change to $HOME).
//  it has to be executed and then return true.
//
//  Remember to update the 'prompt' with the
//  new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0'].
//  2. cmd = ['c','d', '\0'].
int
cd(char *cmd)
{
	char *dir;
	if (strcmp(cmd, "cd") == CERO) {
		dir = getenv("HOME");
	} else if (strncmp(cmd, "cd ", CD_CMD_LEN) == CERO) {
		dir = cmd + CD_CMD_LEN;
	} else {
		return SALIDA_EXITOSA;
	}

	if (chdir(dir) < CERO) {
		perror("error al cambiar de directorio");
		return SALIDA_EXITOSA;
	}

	snprintf(prompt, sizeof(prompt), "%s", getcwd(dir, PRMTLEN));
	return UNO;
}

// returns true if 'pwd' was invoked
// in the command line.
//
// (It has to be executed here and then
// return true).
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == CERO) {
		char *pwd = getcwd(NULL, CERO);
		printf("%s\n", pwd);
		free(pwd);
		return UNO;
	}
	return SALIDA_EXITOSA;
}

// returns true if `history` was invoked
// in the command line.
//
// (It has to be executed here and then
// 	return true).
int
history(char *cmd)
{
	if (strcmp(cmd, "history") == CERO) {
		return UNO;
	}

	return SALIDA_EXITOSA;
}
