/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_SCHED_H
#define JOS_KERN_SCHED_H
#ifndef JOS_KERNEL
#error "This is a JOS kernel header; user programs should not #include it"
#endif

// Para el "scheduler" decidimos implementar un "MLFQ".
// MLFQ es un acronimo de Multi-Level Feedback Queue.
// Es un algoritmo de planificacoon de procesos utilizado para gestionar
// la ejecucion de tareas que tiene 3 caracteristicas basicas:
// 1_ Niveles de Prioridad: MLFQ utiliza multiples colas de prioridad
// para clasificar las tareas segun su necesidad de CPU.
// 2_ Retroalimentacion: Las tareas pueden moverse entre las diferentes
// colas basandose en su comportamiento durante la ejecucion.
// 3_ Temporalidad: MLFQ tambien puede implementar un sistema de tiempo
// de espera (quantum) para cada cola

// Declaracion de la variable global "mlfq_scheduler".
extern struct MLFQ_scheduler mlfq_scheduler;

// Definicion del struct "MLFQ_cola" que utiliza el "scheduler".
struct MLFQ_cola {
	envid_t envs[NENV];  // Arreglo que almacena IDs de procesos.
	int ultimo;          // Indice del ultimo entorno en la cola.
	int primero;         // Indice del principio de la cola.
};

// Definicion del struct "MLFQ_scheduler".
struct MLFQ_scheduler {
	struct MLFQ_cola cola_a;  // Cola de alta prioridad.
	struct MLFQ_cola cola_b;  // Cola de prioridad media.
	struct MLFQ_cola cola_c;  // Cola de baja prioridad.
	struct MLFQ_cola cola_d;  // Cola de prioridad muy baja.
	int ejecuciones_totales;  // Contador total de ejecuciones.
};

// PRE: El identificador de entorno (env_id) debe ser valido y corresponder a un
// entorno que esta presente en alguna cola. La cola a la que pertenece el
// entorno debe ser valida. POST: El entorno identificado por env_id se elimina
// de su respectiva cola si es encontrado. La funcion itera sobre los elementos
// de la cola y elimina el primero que coincida con el env_id.
void sched_eliminar_env(envid_t env_id);

// PRE: El identificador del entorno (env_id) debe ser valido y el valor de cola
// (valor_cola) debe corresponder a una cola existente (>= 0). POST: El entorno
// identificado por env_id se agrega a la cola correspondiente, y se incrementa
// el contador de la posicion final de la cola.
void sched_agregar_env(envid_t env_id, int valor_cola);

void sched_yield(void) __attribute__((noreturn));

#endif  // !JOS_KERN_SCHED_H
