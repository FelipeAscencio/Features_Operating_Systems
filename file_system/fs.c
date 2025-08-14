#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "fs.h"

// PRE: `path` apunta a una cadena válida y terminada en '\0'. El sistema de archivos debe estar inicializado correctamente.
// POST: Devuelve el índice del inodo correspondiente a `path` si existe, o ERROR si no se encuentra.
int
search_inode(const char *path)
{
	int i = 0;
	while (i < INODES_AMOUNT &&
	       (superblock.inode_bitmap[i] == 0 ||
	        strcmp(superblock.inodes[i].path, path) != 0)) {
		i++;
	}

	if (i == INODES_AMOUNT) {
		return ERROR;
	}
	return i;
}

// PRE: `content` es un puntero válido con datos terminados en '\0'. `offset` apunta a una posición inicial dentro de `content`. 
// `buff` debe ser un arreglo suficientemente grande para almacenar una entrada completa.
// POST: Copia la siguiente entrada delimitada por '\n' o '\0' desde `content` a `buff` y actualiza `offset`.
void
get_next_entry(char *content, off_t *offset, char *buff)
{
	off_t aux=*offset;
	int i = 0;

	while (content[aux] != '\n' && content[aux] != '\0') {
		buff[i] = content[aux];
		i++;
		aux++;
	}
	if (content[aux] == '\0' && i == 0) {
		return;
	}

	buff[i++] = '\0';
	*offset += i;
}

// PRE: El bitmap de inodos (`inode_bitmap`) debe estar inicializado.
// POST: Devuelve el índice de un inodo libre si existe, o -1 si no hay inodos libres.
int
search_free_inode()
{
	int i = 0;
	while (i < INODES_AMOUNT && superblock.inode_bitmap[i] != 0) {
		i++;
	}
	if(i == INODES_AMOUNT){
		return -1;
	}
	return i;
}

// PRE: `path` apunta a una cadena válida y terminada en '\0' con longitud menor a `MAX_PATH`. 
// POST: Crea un inodo con los valores especificados y devuelve su índice, o un código de error si no hay espacio o memoria insuficiente.
int create_inode(const char *path, mode_t mode, enum inode_type type)
{
    // Verificar que el tamaño del path no exceda el máximo permitido
    if (strlen(path) >= MAX_PATH) {
        return -ENAMETOOLONG;
    }

    // Buscar un índice libre en el bitmap de inodos
    int free_index = search_free_inode();
    if (free_index == -1) {
        return -ENOSPC; 
    }

    inode_t *inode = &superblock.inodes[free_index];

    strcpy(inode->path, path);
	inode->content = calloc(INITIAL_CONTENT_LENGTH, sizeof(char));
	if (inode->content == NULL) {
		return -ENOMEM;
	}

    inode->content[0] = '\0'; // Contenido vacío inicial
    inode->size = 0;          
    inode->type = type;       
    inode->last_access = time(NULL);
    inode->last_modification = time(NULL);
    inode->creation_time = time(NULL);
    inode->group = getgid();
    inode->owner = getuid();
    inode->permissions = mode;

    superblock.inode_bitmap[free_index] = 1;
	superblock.inode_amount++;
    return free_index; 
}

// PRE: `path` apunta a una cadena válida y terminada en '\0'.
// POST: Devuelve un puntero al inodo del directorio padre de `path` si existe, o NULL y establece `error` en -ENOENT si no se encuentra.
inode_t *
get_parent(const char path[MAX_PATH], int *error)
{
    char *last_slash = strrchr(path, '/');

    if (strlen(path) == 1) {
		*error = -ENOENT;
        return NULL;
    }

    size_t parent_length = (last_slash == path) ? 1 : last_slash - path;

    char parent[MAX_PATH];
    strncpy(parent, path, parent_length);
    parent[parent_length] = '\0';  
	
    int parent_index = search_inode(parent);
    if (parent_index == ERROR) {
        *error = -ENOENT;
        return NULL;
    }

    return &superblock.inodes[parent_index];
}

// PRE: `content` es un puntero válido a una cadena dinámicamente asignada. `content_size` debe apuntar al tamaño actual de `content`. 
// `dentry` debe ser un puntero a una cadena válida terminada en '\0'.
// POST: Agrega `dentry` como una nueva entrada al contenido, expandiendo su tamaño si es necesario.
void
add_dentry_to_content(char **content, int *content_size, char *dentry)
{
    int dentry_size = strlen(dentry);
    dentry[dentry_size] = '\n';        
    dentry[++dentry_size] = '\0';     

    int content_length = strlen(*content);

    if (content_length + dentry_size > *content_size) {
        int new_size = *content_size + dentry_size + INITIAL_CONTENT_LENGTH;

        char *new_content = realloc(*content, new_size);
        if (new_content == NULL) {
            errno = ENOMEM;
            return;
        }
        *content = new_content;
        *content_size = new_size;
    }

    // Copiar la nueva entrada al contenido
    strcpy(*content + content_length, dentry);
}

// PRE: `path` apunta a una cadena válida y terminada en '\0'. `inode_index` debe ser un índice válido dentro de `superblock.inodes`.
// POST: Marca el inodo como libre, actualiza el contenido del directorio padre y libera la memoria asociada.
void
remove_inode(const char *path, int inode_index)
{
	superblock.inode_bitmap[inode_index] = 0;

	int error=0;
	inode_t *parent = get_parent(path, &error);
	if (!parent || error) {
		fprintf(stderr, "Error: Unable to find parent directory\n");
        return;
    }

	char *dir_entry = strrchr(path, '/');
	if (!dir_entry) {
        fprintf(stderr, "Error: Invalid path\n");
        return;
    }
	dir_entry++;

	char *new_content = calloc(parent->size, sizeof(char));
	if (!new_content) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }
	int new_size = parent->size;

	off_t offset = 0;
	int content_length = strlen(parent->content);
	while (offset < content_length) {
		char buff[MAX_PATH + 1];
		get_next_entry(parent->content, &offset, buff);

		if (strcmp(buff, dir_entry) != 0) {
			printf("dentry: %s\n",buff);
			add_dentry_to_content(&new_content, &new_size, buff);
		}
	}

	free(parent->content);
	free(superblock.inodes[inode_index].content);
	parent->content = new_content;
	parent->size = new_size;
	superblock.inode_amount--;
}

// PRE: `path` apunta a una cadena válida y terminada en '\0'. 
// POST: Agrega una nueva entrada al contenido del directorio padre, o devuelve un error si el padre no es un directorio.
int
add_dentry_to_parent_dir(const char *path)
{
    int error=0;
	inode_t *parent_inode = get_parent(path,&error);
    if (!parent_inode || error) {
		fprintf(stderr, "Error: Unable to find parent directory\n");
        return -ERROR;
    }
    
	if (parent_inode->type != INODE_DIR) {
		errno = ENOTDIR;
		return -ENOTDIR;
	}

	char *dir_entry = strrchr(path, '/');
	dir_entry++;

	add_dentry_to_content(&parent_inode->content,
	                      &parent_inode->size,
	                      dir_entry);
	return 0;
}

// PRE: `fp` debe ser un descriptor de archivo válido apuntando a un sistema de archivos serializado.
// POST: Carga el estado del sistema de archivos en `superblock`. En caso de error, libera recursos ya asignados.
void
deserialize(int fp)
{
	int i = 0;
	bool error = false;
	int inodes_amount = 0;
	int amount = read(fp, &inodes_amount, sizeof(int));
	if (amount<0){
		perror("Error loading filesystem\n");
		return;
	}	
	superblock.inode_amount = inodes_amount;

	while (i < inodes_amount) {
		inode_t *inode = &superblock.inodes[i];

		int read_inode = read(fp, inode, sizeof(inode_t));
		if (read_inode <= 0) {
			error = read_inode != 0;
			break;
		}

		inode->content = calloc(inode->size, sizeof(char));
		if (inode->content == NULL) {
			error = true;
			break;
		}

		int read_content = read(fp, inode->content, inode->size);
		if (read_content < 0) {
			error = true;
			break;
		}
		superblock.inode_bitmap[i] = 1;
		i++;
	}

	if (error) {
		perror("Error loading filesystem\n");
		for (int j = 0; j < i; j++) {
			free(superblock.inodes[j].content);
		}
		superblock.inode_amount = 0;
	}
}

// PRE: `fp` debe ser un descriptor de archivo válido para escribir el estado del sistema de archivos. 
// El sistema de archivos debe estar correctamente inicializado en `superblock`.
// POST: Escribe el estado del sistema de archivos en el archivo y libera la memoria asociada a los contenidos de los inodos.
void
serialize(int fp)
{
	int amount = write(fp, &superblock.inode_amount, sizeof(int));
	if (amount<0)
	{
		perror("Error saving filesystem\n");
		return;
	}
	
	bool error = false;
	for (int i = 0; i < superblock.inode_amount; i++) {
		inode_t *inode = &superblock.inodes[i];
		if (superblock.inode_bitmap[i] != 1)
			continue;

		int write_inode = write(fp, inode, sizeof(inode_t));
		int write_content = write(fp, inode->content, inode->size);

		if (write_inode < 0 || write_content < 0) {
			perror("Error saving filesystem\n");
			error = true;
			break;
		}
	}

	if (error) {
		return;
	}

	for (int i = 0; i < superblock.inode_amount; i++) {
		if (superblock.inode_bitmap[i] == 1) {
			free(superblock.inodes[i].content);
		}
	}
}
