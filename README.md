# SOR-Semáforos-1S-2021
Trabajo Práctico Semáforos primer semestre año 2021
Universidad Nacional de General Sarmiento
Alumno: Tomás Moscarelli

### Introducción
En el presente trabajo se realizo un programa en el que se hizo uso de **Semáforos** y **Threads** para simular una competencia por equipos en la que se realizan tareas de manera concurrente. 
Para este trabajo se solicitaron varias cosas, y hubieron algunas que presentaron dificultades las cuales fueron :

 - La sincronización de los hilos para realizar las acciones
 - La lectura de la receta desde un archivo
 - La escritura de la salida de competencia en un archivo

### Solución a los problemas
#### Sincronización de los hilos
El problema estaba en que varios acciones se realizaban a la misma vez y había veces en que la impresión de las acciones no era en el orden correcto, esto se soluciono al crear el wait para cada función y la señal a la siguiente función y de esta forma se mantuvo un orden:
``` c
    accion="mezclar"
    sem_wait(sem_mezclar);
    imprimirAccion()
    sem_post(sem_salar)
```
#### Lectura de la receta desde un archivo
Otra dificultad fue la lectura de la receta desde un archivo , pues ya que realizando un strcpy para cada acción e ingrediente no era la mejor forma de obtener la información de la receta, se realizo la lectura desde un archivo, este se hizo en un .txt y con cierto formato para la función. 
El formato es: 

> **[Acción] : [Ingrediente1],[Ingrediente2],[Ingrediente N].**

El problema estaba en que cuando se realizaba la lectura hasta el final de una linea, la cadena que terminaba al final tomaba un \n y luego cuando se realizaba la comparación de cadenas en la función de imprimirAccion , este daba false y no se imprimía la salida de competencia completa. 
Ejemplo:
**acción: "mezclar\n"**
``` c
    strcmp(accion, accionIn) == 0 --> False
```
**Output**:
```
    Equipo 1 - accion cortar
    Equipo 1 ingrediente 0 : ajo
    Equipo 1 ingrediente 1 : perejil
```
y ahí termina.

Esto se soluciono realizando una verificación de si el carácter actual es igual a \n y si es true entonces se continua a la siguiente lineal y así hasta EOF (End of Line).
``` c
    while((c = getc(fp)) != EOF){
         if(c == '\n'){
         continue;
         }
         ...
    }
```
#### Escritura de la salida de competencia
El problema estaba en que se mostraba mal el formato de los strings en la salida de competencia y en el orden incorrecto. También sucedía que al imprimir el ganador, luego se continuaba las impresiones de acciones de otros equipos como si no hubiera terminado.
```
    Equipo 3 - accion armar sandwich
    Equipo 3 ingrediente  0 : pan
    Equipo 3 ingrediente  1 : lechuga
    Equipo 3 ingrediente  2 : tomate
    Equipo 3 ingrediente  3 : cebolla
    Equipo 3 ingrediente  4 : pepino
    Equipo 3 ingrediente  5 : milanesa
    Equipo 3 - Es el Ganador!
    Equipo 2 - accion cocinar milanesa
    Equipo 2 ingrediente  0 : carne mezclada
    ...
```
Esto se soluciono poniendo dos impresiones en la funcion imprimirAccion las cuales tienen un formato determinado y un puntero FILE, y luego se puso un exit(0) al final de la función armado para que no continue con las impresiones de los otros equipos.
``` c
    ...
    fprintf(fptr,"\tEquipo %d - accion %s \n ",equipo, accion);
    ...
    fprintf(fptr,"\tEquipo %d ingrediente %d : %s \n",equipo, n, ingrediente);
```
``` c
    void* armado(void *data){
       ...
       sem_wait(armado)
       imprimirAccion(accion)
       fprintf(fptr,"\tEquipo %d - Es el Ganador!\n ",numero_equipo);
       ...
       exit(0);
    }
```
### Análisis y prueba de escritorio en Pseudocódigo
```
Semaforos: (ver en archivo de texto toda la explicación)
sem_agregar =      0 0 1 0 0 0 0 0
sem_mezclar =      1 0 0 0 0 0 0 0 
sem_salar =        0 1 0 0 0 0 0 0
sem_hornear =      0 0 0 0 0 1 0 0
sem_cocinar =      0 0 0 0 1 0 0 0
sem_empanar =      0 0 0 1 0 0 0 0
sem_armar =        0 0 0 0 0 0 0 1
sem_cortaExtras =  0 0 0 0 0 0 1 0
```
Prueba de escritorio en pseudocódigo y explicación de funciones se encuentra en el archivo : **PseudocodigoTPSOR.txt**

**Código**:
Para compilar: `gcc subwayArgento.c -o ejecutable -lpthread`

Para ejecutar: `./ejecutable`
