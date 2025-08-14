// Archivo creado e implementado de autoria propia.
#include <inc/lib.h>

#define CERO 0
#define TRES 3  // Prioridad de ejemplo que utilizamos para el TEST.

// Test para verificar que un proceso hijo no puede cambiar la prioridad del padre
// sin la autorizacion necesaria (corrobora que se cumple el orden jerarquico).
void
umain(int argc, char **argv)
{
	envid_t env_id_padre = sys_getenvid();
	envid_t id_proceso = fork();
	if (id_proceso == CERO) {
		int resultado_test = sys_asignar_prioridad(env_id_padre, TRES);
		cprintf("Resultado de una asignacion de prioridad invalida: "
		        "%i\n",
		        resultado_test);
	}
}
