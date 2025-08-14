// Archivo creado e implementado de autoria propia.
#include <inc/lib.h>

// Test para verificar que se puede leer la prioridad del proceso actual.
void
umain(int argc, char **argv)
{
	envid_t env_id = sys_getenvid();
	int prioridad = sys_obtener_prioridad(env_id);
	cprintf("La prioridad es: %i\n", prioridad);
}
