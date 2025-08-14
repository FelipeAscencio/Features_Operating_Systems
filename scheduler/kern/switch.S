/*
 * Your code here for context_switch
 *
 * It should restore the register values in the Trapframe with the 'iret' instruction.
 * This exits the kernel and starts executing some environment's code.
 *
 * This function does not return.
 */

# PRE: La pila actual debe tener un 'Trapframe' que contenga los registros 
#      guardados del entorno al que vamos a cambiar.
# POST: Todos los registros generales y los registros de segmento estaran
#       restaurados para el entorno/proceso al que se cambia.
.globl context_switch;
context_switch:
	# Incrementamos el puntero de pila '%esp' en 4 bytes.
	add $4, %esp
	# Movemos el valor almacenado en la direccion apuntada al propio %esp
	mov (%esp), %esp
	# Se restauran todos los registros generales desde la pila.
	popal
	# Se restauran los valores de los registros de segmento.
	pop %es
	pop %ds
	# Se incrementa el puntero de pila '%esp' en 8 bytes.
	add $8, %esp
	# Hacemos el retorno de interrupcion.
	iret

spin:
	jmp spin
