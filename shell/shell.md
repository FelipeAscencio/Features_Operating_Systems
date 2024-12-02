# shell

---

## Búsqueda en $PATH

### ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?.

execve() ejecuta el programa al que hace referencia el nombre de pathname, con los argumentos que se le envían por argv[].

Esto hace que el programa que está ejecutando actualmente por el proceso llamador **sea reemplazado por un nuevo programa.**

- Crea un nuevo heap, stack y data segments.
- Cambia la imagen del proceso actual → memoria virtual, argumentos y entorno.
- mantiene todo lo de mas (Incluido el PID).

La principal diferencia entre **execve(2)** y la familia de los **exec(3)** es en como se reciben los argumentos y de donde proviene el entorno.

La familia de **exec(3)** esta compuesta por:

`execl, execlp, execle, execv, execvp, execvpe`

Donde:

- **v** = llamadas que toman un vector para recibir los argumentos del nuevo programa a generar. El final de los argumentos se indica mediante un elemento de matriz que contiene el valor de NULL.
- **l** = llamadas que toman una lista de argumentos de longitud variable para recibir los argumentos del nuevo programa a generar. El final de los argumentos se indica mediante un NULL.
- **e** = llamadas que toman un argumento adicional (o argumentos si hay **l** ) para brindarle un nuevo entorno al programa. Caso contrario el programa hereda el entorno del proceso actual.
- **p** = lamadas que buscan la variables en el entorno de PATH para encontrar el programa, si este no posee un directorio, caso contrario, el nombre del programa se tratara como una ruta ejecutable.

### ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?.

Si, la llamada exec(3) no esta libre de poder fallar, y cuando esto sucede esta retorna un errno indicando el error y un -1.

Segun los manuales de linux entendemos que:

- Si se nos deniega el permiso a un archivo (Error EACCES) 
a shell podría continuar buscando en otras rutas de PATH antes de retornar el error, pero si no encuentra nada más, devolverá EACCES.
- Si no logra reconocer el encabezado del archivo indicado (Error ENOEXEC), las funciones se ejecutaran en la shell con la ruta del archivo posicionandolo como primer argumento.
    - Si esto llegase a fallar tambien, no se realizarian mas busquedas.

---

## Procesos en segundo plano

### Explicar detalladamente el mecanismo completo utilizado. ¿Por qué es necesario el uso de señales? (Items 4 a 6).

El manejo de procesos en segundo plano en una shell es esencial para permitir que el usuario continúe interactuando con el entorno mientras uno o más procesos se ejecutan sin bloquear el terminal.

Esto requiere una correcta gestión de señales y grupos de procesos, especialmente para evitar la acumulación de procesos zombi y liberar recursos adecuadamente.

1. Detección de comandos en segundo plano: Cuando el usuario ejecuta un comando con el operador &, el shell debe identificar que este comando debe correr en segundo plano. En nuestro código, esto se maneja en el bloque case BACK dentro de la función exec_cmd(). Este bloque se activa cuando un comando se ejecuta en segundo plano, permitiendo que el shell devuelva el prompt inmediatamente sin esperar la finalización del proceso hijo.

2. Grupos de procesos en segundo plano: Para gestionar eficientemente los procesos en segundo plano, se hace uso de grupos de procesos. Cuando se ejecuta el primer proceso en segundo plano, su PID se utiliza como el ID del grupo de procesos en segundo plano (bg_pgid). Todos los procesos que se ejecuten posteriormente en segundo plano se agregarán a este grupo mediante la llamada a setpgid(0, bg_pgid). Este agrupamiento facilita la diferenciación entre procesos en primer y segundo plano, lo que es esencial para un manejo adecuado de la finalización y recolección de recursos de los procesos en segundo plano.

3. Manejo de señales (y la señal SIGCHLD): Cada vez que un proceso hijo termina su ejecución, el sistema operativo genera una señal SIGCHLD, que se envía al proceso padre (en este caso, la shell) para notificarle de la terminación del proceso hijo. En el caso de los procesos que se ejecutan en primer plano, la shell utiliza la función waitpid() en la función run_cmd() para esperar explícitamente a que estos procesos terminen. Como la shell espera directamente a la finalización del proceso, no es necesario un manejador de señales SIGCHLD para los procesos en primer plano. Sin embargo, cuando se ejecutan comandos en segundo plano, no se usa waitpid() de forma inmediata, y es aquí donde el uso de señales se vuelve esencial. Si no se gestiona la señal SIGCHLD para los procesos en segundo plano, estos podrían convertirse en procesos zombis, es decir, procesos que han terminado su ejecución, pero cuyos recursos no han sido liberados, lo que podría agotar los recursos del sistema si se generan muchos procesos zombis.

4. Liberación de recursos y notificación: Para evitar procesos zombis, se implementa un manejador para la señal SIGCHLD. En nuestro código, la función sigchld_handler() se encarga de gestionar esta señal. Cuando un proceso hijo termina, el kernel genera la señal SIGCHLD, y el manejador registrado captura esta señal. Dentro del manejador, la función waitpid() es llamada con la opción WNOHANG, lo que permite verificar si hay procesos hijos que han terminado sin bloquear la ejecución del proceso padre. De esta manera, se liberan los recursos asociados a los procesos hijos terminados sin detener la ejecución de la shell. Además, el shell puede notificar al usuario inmediatamente cuando un proceso en segundo plano ha terminado, imprimiendo su PID. La opción WNOHANG es crucial porque evita que la shell se bloquee esperando la finalización de procesos en segundo plano, permitiendo que continúe respondiendo a las solicitudes del usuario.

5. Reinicio de llamadas interrumpidas y SA_RESTART: Cuando un manejador de señales interrumpe ciertas llamadas al sistema, como la lectura de entrada o salida, estas llamadas pueden fallar y no se reanudarían automáticamente. Para evitar este comportamiento no deseado, se utiliza el flag SA_RESTART al registrar el manejador con sigaction().  Esto asegura que las llamadas al sistema que fueron interrumpidas por señales se reinicien automáticamente una vez que el manejador de señales haya terminado su ejecución, evitando que la shell se vea afectada por interrupciones no esperadas durante su funcionamiento.

6. Manejo de Señales No Deseadas con SA_NOCLDSTOP: En nuestro contexto, solo nos interesa capturar la señal SIGCHLD cuando un proceso hijo ha terminado, no cuando un proceso ha sido detenido o suspendido. Para evitar que el manejador de señales procese señales de detención, se utiliza el flag SA_NOCLDSTOP. Esto asegura que el manejador solo capture las señales SIGCHLD generadas por la terminación de procesos, lo que mejora la eficiencia al evitar la gestión innecesaria de señales generadas por pausas de procesos en segundo plano.

7. Diferenciación entre Procesos de Primer y Segundo Plano: Para evitar que el manejador de SIGCHLD interfiera con los procesos en primer plano, agrupamos los procesos en segundo plano bajo un mismo ID de grupo de procesos, utilizando setpgid(). Esto permite que el manejador de señales identifique específicamente los procesos en segundo plano. En el manejador, se utiliza waitpid(-bg_pgid, NULL, WNOHANG) para recolectar y liberar los recursos únicamente de los procesos que pertenecen al grupo de procesos en segundo plano (bg_pgid), sin afectar a los procesos en primer plano.

#### Al gestionar los procesos de esta manera, se logra:

- Liberar los recursos de los procesos terminados.
- Mantener la interactividad de la shell, permitiendo que el usuario siga ejecutando comandos sin necesidad de esperar la finalización de procesos en segundo plano.
- Evitar la acumulación de procesos zombis, lo que es crucial para mantener el rendimiento del sistema.

---

## Flujo estándar

### Investigar el significado de 2>&1, explicar cómo funciona su forma general.

En el número `1` se representa la salida estándar (`stdout`), mientras que el número `2` representa la salida de errores estándar (`stderr`). La expresión `2>&1` redirige la salida de errores (`stderr`, descriptor de archivo 2) hacia la salida estándar (`stdout`, descriptor de archivo 1).

- `> out.txt`: Redirige la salida estándar (`stdout`) al archivo `out.txt`.
- `2>&1`: Redirige la salida de errores (`stderr`) a la misma ubicación a la que apunta la salida estándar (`stdout`), que en este caso es `out.txt`.

### Mostrar qué sucede con la salida de cat out.txt en el ejemplo.

Cuando se ejecuta el comando `ls -C /home /... > out.txt 2>&1`:

1. `ls -C /home /...` intenta listar los contenidos de `/home` (que existe) y `/...` (que no existe).
2. La salida de listar `/home` se redirige a `out.txt`.
3. El error generado por intentar listar `/...` se redirige a la salida estándar (gracias a `2>&1`) y, por lo tanto, también se guarda en `out.txt`.

Al ejecutar `cat out.txt`, se mostrará el contenido del archivo, que incluye tanto la lista de archivos de `/home` como el error producido al intentar acceder a `/...`.

### Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).

Se redirige la salida estándar (`stdout`) a la salida de errores (`stderr`), pero el archivo de salida (`out.txt`) solo recibe la salida de errores.

1. `ls -C /home /...`:
    - Intenta listar `/home`, pero como se ha redirigido `stdout` a `stderr`, esa salida no se guarda en `out.txt`.
    - El error por intentar listar `/...` se imprime directamente en pantalla, ya que `stderr` no ha sido redirigido al archivo.
2. Al ejecutar `cat out.txt`, el archivo estará vacío porque la salida estándar nunca fue redirigida al archivo, solo se redirigió a `stderr`, que no se guarda en `out.txt`.

#### Diferencia clave:

- En el primer caso (`2>&1`), tanto la salida estándar como los errores se almacenan en el archivo.
- En el segundo caso (`1>&2`), la salida estándar se redirige a los errores, y no se almacena en el archivo, dejando el archivo vacío. Los errores siguen apareciendo en pantalla.

---

## Tuberías múltiples

### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe. ¿Cambia en algo?.

Cuando se utiliza un pide (`|`) en Bash, el comportamiento del **exit code es el siguiente:**

- El **exit code** es el código de salida del **último comando** en el pipe, sin importar el fallo de algun comando previo.
- El Bash nos proporciona una llamada `PIPESTATUS`, que es un array que contiene los **códigos de salida de todos los comandos** involucrados el pipe.
    - Esto nos permite permite capturar y verificar si alguno de los comandos en el pipe falló.
- Si uno de los primeros comandos falla pero el último tiene éxito, el **exit code será de éxito** (`0`)

### ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.

**Funciona solo 1 comando:**

```jsx
$ echo "helloww" | grep "si"
$ echo $?
1
```

`echo "hello"` se ejecuta correctamente, pero `grep "no"` no , por lo tanto, falla con un código de salida `1`.

**No funcionan ambos comandos:**

```jsx
$ ls /... | grep "test"
ls: cannot access '/...': No such file or directory
$ echo $?
0
```

El comando `ls /...`falla porque el directorio no existe. Sin embargo, como `grep "test"` no falló , el **código de salida reportado es `0`**, indicando éxito, ya que el último comando en la tubería se ejecutó sin errores.

---

## Variables de entorno temporarias

### ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?.

Se debe realizar luego de la llamada a fork(2) debido a que las variables de entorno temporales se utilizan en comandos específicos, para que estén disponibles en el proceso donde se ejecutará el binario correspondiente.

### En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3). ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

Cuando se pasan nuevas variables de entorno como parte de ese tercer argumento _envp_, estas variables son creadas en la memoria del nuevo proceso que se ejecuta mediante _exec()_. Por el otro lado, las funciones de la familia _exec(3)_ que no tienen el sufijo “e” al final usan una variable de entorno externa environ que posee el proceso actual y la utiliza como tabla con las variables de entorno para el nuevo proceso que se va a ejecutar, lo que significa que las variables de entorno del nuevo proceso son copias de las del proceso actual.

De esta forma, el comportamiento no es el mismo que si se utilizara _setenv(3)_, ya que esta función agrega variables al entorno actual, lo que hace que al llamar a _exec()_ estas variables ya se encuentren en el entorno del proceso, mientras que las funciones _execle()_ y _execvpe()_ solamente envían como entorno a las variables que se encuentran en el array _envp_.

Manual de linux sobre _setenv(3)_: “_The setenv() function adds the variable name to the environment with the value value, if name does not already exist. If name does exist in the environment, then its value is changed to value if overwrite is nonzero; (…)_”.

### Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Una forma de resolver esto y que el comportamiento sea el mismo, es agregar al array de _chars_ _envp_ todas las variables del entorno del proceso actual, además de las nuevas variables que se le quieren pasar al nuevo proceso que se va a ejecutar. Las variables son de la forma “key=value”, y el array debe finalizar con un puntero a NULL. De esta forma, el nuevo proceso tendrá todas las variables del entorno del proceso actual, sumadas a las nuevas. Luego de esto, llamar alguna de las funciones _execvpe()_ o _execle()_ pasando como argumento _envp_ este array con las variables.

---

## Pseudo-variables

### Investigar al menos otras tres variables mágicas estándar, y describir su propósito.

Explicación del propósito de la variable _$?_: muestra el código del estado de salida para el último comando ejecutado. 

Otras 3 variables mágicas:

1. $$: muestra el ID del proceso de la Shell actual
2. $0: muestra el nombre del archivo del script actual
3. $*: Agrupa todos los argumentos dados y los conecta.

### Incluir un ejemplo de su uso en bash (u otra terminal similar).

Ejemplo de $$:

  $ /bin/bash

  $ echo $$

  37111


Ejemplo de $0:

  $ /bin/bash

  $ echo $0

  /bin/bash


Ejemplo de $*:

  $ /bin/bash

  $ echo "Uniendo" "estos" "argumentos" "e" "imprimiendo"; $*

  Uniendo estos argumentos e imprimiendo.

---

## Comandos built-in

### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

Para responder adecuadamente a esta pregunta, es importante primero definir qué es un comando built-in.

Un built-in es un comando que está integrado directamente en la shell, lo que significa que se ejecuta en el mismo proceso de la shell en lugar de crear un nuevo proceso hijo, como ocurre con los comandos externos. Esta integración permite que los built-ins interactúen directamente con el entorno de la shell, accediendo a sus variables y modificando su estado, lo que a menudo es necesario para ciertos comandos que afectan la shell misma.

En contraposición, los comandos externos requieren la creación de un proceso separado, lo que conlleva un cierto costo en términos de tiempo y recursos.

El comando pwd (print working directory) *puede implementarse como un comando externo*. Este comando simplemente imprime el directorio de trabajo actual, que es una información que puede obtenerse utilizando la llamada al sistema getcwd(3). No realiza ningún cambio en el entorno de la shell, por lo que no es estrictamente necesario que sea un built-in. 

De hecho, en muchos sistemas operativos existe un binario externo en "/bin/pwd" que puede ser invocado por procesos externos o scripts sin necesidad de una shell.

Sin embargo, muchas shells implementan pwd como un comando built-in por motivos de eficiencia. Dado que la shell ya mantiene información sobre su propio directorio de trabajo (CWD), es más rápido y eficiente que imprima directamente esa información en lugar de invocar un programa externo. 

Esto evita la sobrecarga de crear un proceso hijo, lo cual es especialmente relevante en sistemas donde se pueden ejecutar múltiples invocaciones de pwd en secuencia o en scripts interactivos.

Por otro lado, el comando cd (change directory) *debe ser un built-in* porque su propósito es modificar el directorio de trabajo actual (CWD, Current Working Directory) del proceso de la shell.

En los sistemas basados en Unix, el directorio de trabajo es una propiedad asociada a cada proceso. Si cd se implementara como un comando externo, es decir, en un proceso hijo, este cambio afectaría únicamente al proceso hijo y no tendría ningún efecto en la shell principal. Al finalizar el proceso hijo, este cambio se perdería y la shell volvería a su estado previo sin modificar el directorio de trabajo. 

Esto se debe a que, en Unix, los cambios en el estado de un proceso hijo (como su CWD) no afectan al proceso padre, en este caso, la shell. Por lo tanto, *cd no puede ser implementado como un comando externo* y debe ser un comando built-in para modificar correctamente el entorno de la shell.

---
