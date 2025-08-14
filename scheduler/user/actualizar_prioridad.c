// Archivo creado e implementado de autoria propia.
#include <inc/lib.h>

#define CERO 0
#define TRES 3  // Prioridad de ejemplo que utilizamos para el TEST.

// Test para verificar que se puede actualizar la prioridad del proceso actual.
void
umain(int argc, char **argv)
{
	envid_t env_id = sys_getenvid();
	sys_asignar_prioridad(env_id, TRES);
	int id_proceso = fork();
	if (id_proceso == CERO) {
		envid_t env_id = sys_getenvid();
		int prioridad = sys_obtener_prioridad(env_id);
		cprintf("Prioridad del hijo: %i\n", prioridad);
	} else {
		envid_t env_id = sys_getenvid();
		int prioridad = sys_obtener_prioridad(env_id);
		cprintf("Prioridad del padre: %i\n", prioridad);
	}
}
