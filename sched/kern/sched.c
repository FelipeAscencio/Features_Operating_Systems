#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <kern/sched.h>

#define MAX_EJECUCIONES_MLFQ 50
#define COLAS_MAXIMAS 4
#define CERO 0
#define UNO 1
#define DOS 2
#define MENOS_UNO -1

// Definicion de variables globales.
static int runs_del_sched;
struct MLFQ_scheduler mlfq_scheduler = {
	.cola_a = { .ultimo = CERO, .primero = CERO },
	.cola_b = { .ultimo = CERO, .primero = CERO },
	.cola_c = { .ultimo = CERO, .primero = CERO },
	.cola_d = { .ultimo = CERO, .primero = CERO },
	.ejecuciones_totales = CERO,
};

// Definicion de funciones.

// PRE: Se debe proporcionar un numero de cola valido (>= 0).
// POST: Devuelve un puntero a la cola correspondiente dentro del scheduler, o NULL si el numero de cola es negativo.
struct MLFQ_cola *obtener_cola(int cola);

// PRE: La cola debe ser valida y no estar vacia. La posicion debe estar dentro
// del rango entre "primero" y "ultimo" de la cola. POST: El entorno en la posicion
// indicada se elimina de la cola, y la posicion "primero" de la cola se incrementa.
// Si la posicion era igual a "primero", solo se incrementa "primero".
void sched_borrar_env(struct MLFQ_cola *cola, int posicion);

// PRE: - .
// POST: Las colas se vacian completamente, sus indices "primero" y "ultimo" se reinician a CERO.
void vaciar_colas();

// PRE: La cola debe ser valida y contener entornos con identificadores validos.
// POST: Todos los entornos de la cola indicada se promueven.
void promover_cola(struct MLFQ_cola *cola);

// PRE: Las colas deben contener entornos validos. La funcion asume que los valores
// de "primero" y "ultimo" en cada cola son correctos. POST: Todos los entornos de todas las colas se promueven.
void promover_colas();

// Implementacion del 'scheduler round robin' pedido en el enunciado.
void round_robin();

// PRE: La cola debe ser valida y contener identificadores de entornos validos.
// POST: Devuelve la posicion de la cola si al menos uno de los entornos es ejecutable
// (tiene estado ENV_RUNNABLE). Si no hay entornos ejecutables, devuelve -1.
int tiene_entornos_ejecutables(struct MLFQ_cola *cola, int posicion_cola);

// PRE: "cola_param" debe ser un puntero a una variable donde se almacenara la
// cola de mejor prioridad. El sistema debe tener al menos una cola valida con
// entornos ejecutables. POST: Devuelve el indice de la cola con entornos
// ejecutables de mejor prioridad, y guarda un puntero a esa cola en
// "cola_param". Si no hay entornos ejecutables, devuelve -1.
int obtener_cola_mejor_prioridad(struct MLFQ_cola **cola_param);

// Implementacion del 'scheduler con prioridad' pedido en el enunciado.
void prioridad_MLFQ();

void sched_halt(void);

struct MLFQ_cola *
obtener_cola(int cola)
{
	if (cola < CERO) {
		return NULL;
	}

	switch (cola) {
	case CERO:
		return &mlfq_scheduler.cola_a;
	case UNO:
		return &mlfq_scheduler.cola_b;
	case DOS:
		return &mlfq_scheduler.cola_c;
	default:
		return &mlfq_scheduler.cola_d;
	}
}

void
sched_agregar_env(envid_t env_id, int valor_cola)
{
	struct MLFQ_cola *cola = obtener_cola(valor_cola);
	int index = cola->ultimo % NENV;
	envs[ENVX(env_id)].cola_actual = valor_cola;
	cola->envs[index] = env_id;
	cola->ultimo++;
}

void
sched_borrar_env(struct MLFQ_cola *cola, int posicion)
{
	if (posicion == cola->primero) {
		cola->primero++;
		return;
	}

	envid_t primero = cola->envs[cola->primero % NENV];
	envid_t objetivo = cola->envs[posicion];
	envid_t auxiliar = primero;
	cola->envs[cola->primero % NENV] = objetivo;
	cola->envs[posicion] = auxiliar;
	cola->primero++;
}

void
sched_eliminar_env(envid_t env_id)
{
	struct Env *env = &envs[ENVX(env_id)];
	int valor_cola = env->cola_actual;
	struct MLFQ_cola *cola = obtener_cola(valor_cola);
	for (int i = cola->primero; i < cola->ultimo; i++) {
		if (cola->envs[i % NENV] == env_id) {
			sched_borrar_env(cola, i % NENV);
			break;
		}
	}
}

void
vaciar_colas()
{
	mlfq_scheduler.cola_b.primero = CERO;
	mlfq_scheduler.cola_b.ultimo = CERO;
	mlfq_scheduler.cola_c.primero = CERO;
	mlfq_scheduler.cola_c.ultimo = CERO;
	mlfq_scheduler.cola_d.primero = CERO;
	mlfq_scheduler.cola_d.ultimo = CERO;
}

void
promover_cola(struct MLFQ_cola *cola)
{
	for (int i = cola->primero; i < cola->ultimo; i++) {
		envid_t env_id = cola->envs[i % NENV];
		sched_agregar_env(env_id, CERO);
	}
}

void
promover_colas()
{
	promover_cola(&mlfq_scheduler.cola_b);
	promover_cola(&mlfq_scheduler.cola_c);
	promover_cola(&mlfq_scheduler.cola_d);
	vaciar_colas();
}

void
round_robin()
{
	struct Env *siguiente_env = curenv;
	int index_inicial =
	        siguiente_env ? ENVX(siguiente_env->env_id) + UNO : CERO;

	for (int i = CERO; i < NENV; i++) {
		int actual_index = (index_inicial + i) % NENV;
		siguiente_env = &envs[actual_index];
		if (siguiente_env->env_status == ENV_RUNNABLE) {
			siguiente_env->ejecuciones++;
			env_run(siguiente_env);
		}
	}

	if (curenv && curenv->env_status == ENV_RUNNING) {
		curenv->ejecuciones++;
		env_run(curenv);
	}
}

int
tiene_entornos_ejecutables(struct MLFQ_cola *cola, int posicion_cola)
{
	for (int i = cola->primero; i < cola->ultimo; i++) {
		envid_t env_id = cola->envs[i % NENV];
		struct Env *env = &envs[ENVX(env_id)];
		if (env->env_status == ENV_RUNNABLE) {
			return posicion_cola;
		}
	}

	return MENOS_UNO;
}

int
obtener_cola_mejor_prioridad(struct MLFQ_cola **cola_param)
{
	for (int i = CERO; i < COLAS_MAXIMAS; i++) {
		struct MLFQ_cola *cola_local = obtener_cola(i);
		if (tiene_entornos_ejecutables(cola_local, i) != MENOS_UNO) {
			*cola_param = cola_local;
			return i;
		}
	}

	return MENOS_UNO;
}

void
prioridad_MLFQ()
{
	struct Env *env_actual = curenv;
	struct MLFQ_cola *cola_mejor_prioridad = NULL;
	int numero_cola = obtener_cola_mejor_prioridad(&cola_mejor_prioridad);
	if (!cola_mejor_prioridad) {
		if (curenv && curenv->env_status == ENV_RUNNING) {
			mlfq_scheduler.ejecuciones_totales++;
			curenv->ejecuciones++;
			env_run(curenv);
		} else {
			sched_halt();
		}
	}

	int principio_cola = cola_mejor_prioridad->primero;
	int final_cola = cola_mejor_prioridad->ultimo;
	for (int i = principio_cola; i < final_cola; i++) {
		envid_t siguiente_env_id = cola_mejor_prioridad->envs[i % NENV];
		struct Env *siguiente_env = &envs[ENVX(siguiente_env_id)];
		if (siguiente_env->env_status == ENV_RUNNABLE) {
			sched_borrar_env(cola_mejor_prioridad, i % NENV);
			int nuevo_numero_cola = numero_cola >= COLAS_MAXIMAS - UNO
			                                ? COLAS_MAXIMAS - UNO
			                                : numero_cola + UNO;
			sched_agregar_env(siguiente_env_id, nuevo_numero_cola);
			mlfq_scheduler.ejecuciones_totales++;
			siguiente_env->ejecuciones++;
			env_run(siguiente_env);
		}
	}
}

void
sched_yield(void)
{
	runs_del_sched++;

#ifdef SCHED_ROUND_ROBIN
	round_robin();

#endif

#ifdef SCHED_PRIORITIES
	if (mlfq_scheduler.ejecuciones_totales >= MAX_EJECUCIONES_MLFQ) {
		promover_colas();
		mlfq_scheduler.ejecuciones_totales = CERO;
	}

	prioridad_MLFQ();
#endif
	sched_halt();
	panic("should not return");
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
void
sched_halt(void)
{
	int i;
	for (i = CERO; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the "
		        "system!\n");

		cprintf("\nHistory of executed environments:\n\n");
		for (int i = CERO; i < NENV; i++) {
			struct Env *env = &envs[i];
			if (env->ejecuciones > CERO) {
				cprintf("env_id: %d, ejecuciones: %d\n",
				        env->env_id,
				        env->ejecuciones);
			}
		}

		cprintf("Scheduler was called a total of: %d times.\n\n",
		        runs_del_sched);

		while (true)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire
	// the big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print
	// statistics on performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
