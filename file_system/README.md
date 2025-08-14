# File System

## Introduction

This project consists of the development of a simplified 'File System' for a Unix-like operating system environment. It was created as a university group assignment by a team of four students, including myself.

The goal was to implement a storage management layer capable of creating, reading, writing, and deleting files while organizing them within a structured directory hierarchy. The system also includes mechanisms for managing file metadata, handling allocation strategies, and ensuring data consistency during file operations.

A clarification: The project was developed for the University, so all analysis, code, and documentation are written in Spanish.

## Respuestas teóricas

Todas las consignas planteadas a la hora del desarrollo del trabajo práctico se encuentran respondidas en el archivo 'file_system.md'.

## Compilar

```bash
$ make
```

## Ejecutar

### Setup

Primero hay que crear un directorio de prueba:

```bash
$ mkdir prueba
```

### Iniciar el servidor FUSE

En el mismo directorio que se utilizó para compilar la solución, ejectuar:

```bash
$ ./fisopfs prueba/
```

Hay una flag `--filedisk NAME` para indicar que archivo se
 quiere utilizar como archivo de persistencia en disco. 
 El valor por defecto es "persistence_file.fisopfs"

```bash
$ ./fisopfs prueba/ --filedisk nuevo_disco.fisopfs
```

### Verificar directorio

```bash
$ mount | grep fisopfs
```

### Utilizar el directorio de "pruebas"

En otra terminal, ejecutar:

```bash
$ cd prueba
$ ls -al
```

### Limpieza

```bash
$ sudo umount prueba
```

## Docker

Existen tres _targets_ en el archivo `Makefile` para utilizar _docker_.

- `docker-build`: genera la imagen basada en "Ubuntu 20.04" con las dependencias de FUSE
- `docker-run`: crea un _container_ basado en la imagen anterior ejecutando `bash`
   - acá se puede ejecutar `make` y luego `./fisopfs -f ./prueba`
- `docker-attach`: permite vincularse al mismo _container_ anterior para poder realizar pruebas
   - acá se puede ingresar al directorio `prueba`

## Linter

```bash
$ make format
```
