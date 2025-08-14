# Shell

## Introduction

This project consists of the development of a command-line Shell designed to interact with the operating system in a Unix-like environment. It was created as a university group assignment by a team of four students, including myself.

The goal was to implement a fully functional user interface that accepts commands, interprets them, and executes the corresponding processes, supporting both built-in and external commands. The system handles argument parsing, process creation, and termination, as well as basic error reporting to ensure smooth and predictable interaction.

A clarification: The project was developed for the University, so all analysis, code, and documentation are written in Spanish.

## Respuestas teóricas

Todas las consignas planteadas a la hora del desarrollo del trabajo práctico se encuentran respondidas en el archivo 'shell.md'.

## Compilar

```bash
make
```

## Pruebas

- Ejecutar todas las pruebas

```bash
make test
```

- Ejecutar una **única** prueba

```bash
make test-TEST_NAME
```

Por ejemplo:

```bash
make test-env_empty_variable
```

Cada identificador de una prueba se muestra entre paréntesis `(TEST_NAME)` al lado de cada _test_ cuando se ejecutan todas las pruebas.

```
=== Temporary files will be stored in: /tmp/tmp0l10br1k-shell-test ===

PASS 1/26: cd . and cd .. work correctly by checking pwd (no prompt) (cd_back)
PASS 2/26: cd works correctly by checking pwd (no prompt) (cd_basic)
PASS 3/26: cd with no arguments takes you home (/proc/sys :D) (cd_home)
PASS 4/26: empty variables are not substituted (env_empty_variable)
...
```

## Ejecutar

```bash
./sh
```

## Linter

```bash
make format
```
