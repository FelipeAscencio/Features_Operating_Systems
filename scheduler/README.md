# Scheduler

## Introduction

This project consists of the development of a process scheduler within a Unix-like operating system environment. It was created as a university group assignment by a team of four students, including myself.

The goal was to implement a scheduling mechanism capable of managing multiple processes, allocating CPU time efficiently, and ensuring fairness while maintaining system responsiveness. The scheduler supports different scheduling policies, manages process states, and handles context switching while respecting the constraints of an educational operating system model.

A clarification: The project was developed for the University, so all analysis, code, and documentation are written in Spanish.

## Respuestas teóricas

Todas las consignas planteadas a la hora del desarrollo del trabajo práctico se encuentran respondidas en el archivo 'scheduler.md'.

## Compilar

Por _default_ se compilará el _scheduler_ en versión **round-robin**.

```bash
make
```

## Compilación condicional de _schedulers_

Para compilar y probar el kernel y poder probar ambos planificadores, se puede:

- **round-robin**:

```bash
make <target> USE_RR=1
```

- **priorities**:

```bash
make <target> USE_PR=1
```

## Pruebas

```bash
make grade
```

## Docker

Se provee un _script_ `dock` que permite ejecutar los siguientes comandos:

- **build**: genera la imagen del proyecto usando el `Dockerfile` provisto
- **run**: genera un _container_ a partir de la imagen anterior y lo corre
- **exec**: permite abrir una nueva _shell_ en el _container_ anterior

Dentro del _container_ se pueden ejecutar todos los comandos provistos por el `GNUmakefile` como `make grade` o `make qemu-nox`.

El _container_ utiliza [mount volumes](https://docs.docker.com/storage/volumes/) con lo cual los cambios que se realicen por fuera del mismo, serán visibles de forma automática.

## Linter

```bash
$ make format
```
