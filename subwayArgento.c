#include <stdio.h>      // libreria estandar
#include <stdlib.h>     // para usar exit y funciones de la libreria standard
#include <string.h>
#include <pthread.h>    // para usar threads
#include <semaphore.h>  // para usar semaforos
#include <unistd.h>


#define LIMITE 50

//creo los mutex
pthread_mutex_t lock;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
pthread_mutex_t lock3;

//punteros a un archivo
FILE *fptr;
FILE *fptr1;

//creo estructura de semaforos 
struct semaforos {
    sem_t sem_mezclar;
	sem_t sem_salar;
	sem_t sem_agregar;
	sem_t sem_hornear;
	sem_t sem_cocinar;
	sem_t sem_empanar;
	sem_t sem_armar;
	sem_t sem_cortaExtras;
};

//creo los pasos con los ingredientes
struct paso {
   char accion [LIMITE];
   char ingredientes[6][LIMITE];
   
};

//creo los parametros de los hilos 
struct parametro {
 int equipo_param;
 struct semaforos semaforos_param;
 struct paso pasos_param[9];
};

//funcion para leer la receta desde un archivo y pasar la informacion a la estructura parametro(solo lee con el formato que se paso en receta.txt)
void* pasaInfo(void *fp,void *data){
	int c; //caracter actual
	char accion[20];
	char ingrediente[20];
	int k = 0; //posicion caracter actual
	int j = 0; //conteo de pasos
	int m = 0; //conteo de ingrediente;
	int accionLeida = 0;
	struct parametro *mydata = data;

	//obtiene accion e ingredientes
	while ((c = getc(fp)) != EOF){
		if(c == '\n'){
			continue;
		}
		if(c != ':' && accionLeida == 0){
			accion[k] = c;
			k++;
		}else if(c == ':'){
			accion[k] = '\0';
			accionLeida = 1;
			k = 0;
			strcpy(mydata->pasos_param[j].accion, accion);
		}else if(c != ',' && c != '.' && accionLeida == 1){
			ingrediente[k] = c;
			k++;
		}else if(c == ','){
			ingrediente[k] = '\0';
			k = 0;
			strcpy(mydata->pasos_param[j].ingredientes[m], ingrediente);
			m++;
		}else if(c == '.'){
			ingrediente[k] = '\0';
			strcpy(mydata->pasos_param[j].ingredientes[m], ingrediente);
			k = 0;
			m = 0;
			accionLeida = 0;
			j++;
		}
	}
}

//funcion para imprimir las acciones y los ingredientes de la accion
void* imprimirAccion(void *data, char *accionIn) {
	pthread_mutex_lock(&lock3); // para que me imprima bien (sino se saltea impresiones)
	struct parametro *mydata = data;
	//calculo la longitud del array de pasos 
	int sizeArray = (int)( sizeof(mydata->pasos_param) / sizeof(mydata->pasos_param[0]));
	//indice para recorrer array de pasos 
	int i;
	for(i = 0; i < sizeArray; i ++){
		//pregunto si la accion del array es igual a la pasada por parametro (si es igual la funcion strcmp devuelve cero)
		if(strcmp(mydata->pasos_param[i].accion, accionIn) == 0){
			printf("\tEquipo %d - accion %s \n " , mydata->equipo_param, mydata->pasos_param[i].accion);
			//imprime accion del equipo actual en un archivo
			fprintf(fptr,"\tEquipo %d - accion %s \n ",mydata->equipo_param, mydata->pasos_param[i].accion);
		    //calculo la longitud del array de ingredientes
		    int sizeArrayIngredientes = (int)( sizeof(mydata->pasos_param[i].ingredientes) / sizeof(mydata->pasos_param[i].ingredientes[0]) );
		    //indice para recorrer array de ingredientes
		    int h;
			for(h = 0; h < sizeArrayIngredientes; h++) {
				//consulto si la posicion tiene valor porque no se cuantos ingredientes tengo por accion 
				if(strlen(mydata->pasos_param[i].ingredientes[h]) != 0) {
							printf("\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
							//imprime ingrediente actual en un archivo
							fprintf(fptr,"\tEquipo %d ingrediente  %d : %s \n",mydata->equipo_param,h,mydata->pasos_param[i].ingredientes[h]);
				}
			}
		}
	}
	pthread_mutex_unlock(&lock3);
}

//funciones para cada accion
void* cortar(void *data) {
	//creo el nombre de la accion de la funcion 
	char *accion = "cortar";
	//creo el puntero para pasarle la referencia de memoria (data) del struct pasado por parametro (la cual es un puntero). 
	struct parametro *mydata = data;
	//llamo a la funcion imprimir le paso el struct y la accion de la funcion
	imprimirAccion(mydata,accion);
	//uso sleep para simular que que pasa tiempo
	usleep( 20000 );
	//doy la señal a la siguiente accion (cortar me habilita mezclar)
    sem_post(&mydata->semaforos_param.sem_mezclar);
	
    pthread_exit(NULL);
}

void* mezclar(void *data){
	char *accion = "mezclar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_mezclar);
	imprimirAccion(mydata,accion);
	usleep(20000);
	sem_post(&mydata->semaforos_param.sem_salar);

	pthread_exit(NULL);
}

void* salar(void *data){

	pthread_mutex_lock(&lock); //solo uno entra a la vez
	char *accion = "salar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_salar);
	imprimirAccion(mydata,accion);
	fprintf(fptr,"\tEquipo %d termino de usar la sal \n",mydata->equipo_param);
	usleep(20000);
	sem_post(&mydata->semaforos_param.sem_agregar);
	pthread_mutex_unlock(&lock);

	pthread_exit(NULL);
}

void* agregar(void *data){

	char *accion = "agregar carne";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_agregar);
	imprimirAccion(mydata,accion);
	usleep(20000);
	sem_post(&mydata->semaforos_param.sem_empanar);

	pthread_exit(NULL);
}

void* empanar(void *data){
	char *accion = "empanar";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_empanar);
	imprimirAccion(mydata,accion);
	usleep(20000);
	sem_post(&mydata->semaforos_param.sem_cocinar);

	pthread_exit(NULL);
}

void* cocinar(void *data){
	pthread_mutex_lock(&lock1);
	char *accion = "cocinar milanesa";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_cocinar);
	imprimirAccion(mydata,accion);
	fprintf(fptr,"\tEquipo %d termino de cocinar \n",mydata->equipo_param);
	usleep(20000);
	sem_post(&mydata->semaforos_param.sem_hornear);
	pthread_mutex_unlock(&lock1);

	pthread_exit(NULL);
}

void* hornear(void *data){
	pthread_mutex_lock(&lock2);
	char *accion = "hornear pan";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_hornear);
	imprimirAccion(mydata,accion);
	fprintf(fptr,"\tEquipo %d termino de hornear \n",mydata->equipo_param);
	usleep(20000);
	sem_post(&mydata->semaforos_param.sem_cortaExtras);
	pthread_mutex_unlock(&lock2);

	pthread_exit(NULL);
}

void* cortarExtras(void *data){
	char *accion = "cortar extras";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_cortaExtras);
	imprimirAccion(mydata,accion);
	usleep(20000);
	sem_post(&mydata->semaforos_param.sem_armar);

	pthread_exit(NULL);
}

void* armado(void *data){
	char *accion = "armar sandwich";
	struct parametro *mydata = data;
	sem_wait(&mydata->semaforos_param.sem_armar);
	imprimirAccion(mydata,accion);
	printf("\tEquipo %d - Es el Ganador!\n ",mydata->equipo_param);
	//se imprime el ganador y termina el programa (el que termina de armar el sandwich primero )
	fprintf(fptr,"\tEquipo %d - Es el Ganador!\n ",mydata->equipo_param);
	//se cierran los archivos
	fclose(fptr);
	fclose(fptr1);
	//termina aca para que no continue impresiones de los demas hilos
	exit(0);

	pthread_exit(NULL);
}

void* ejecutarReceta(void *i) {
	
	//variables semaforos
	sem_t sem_cortar;
	sem_t sem_mezclar;
	sem_t sem_salar;
	sem_t sem_agregar;
	sem_t sem_hornear;
	sem_t sem_cocinar;
	sem_t sem_empanar;
	sem_t sem_armar;
	sem_t sem_cortaExtras;
	
	//variables hilos
	pthread_t p1;
	pthread_t p2;
	pthread_t p3;
	pthread_t p4;
	pthread_t p5;
	pthread_t p6;
	pthread_t p7;
	pthread_t p8;
	pthread_t p9;
	
	//numero del equipo (casteo el puntero a un int)
	int p = *((int *) i);

	//reservo memoria para el struct
	struct parametro *pthread_data = malloc(sizeof(struct parametro));

	//seteo los valores al struct
	
	//seteo numero de grupo
	pthread_data->equipo_param = p;

	//seteo semaforos
	pthread_data->semaforos_param.sem_mezclar = sem_mezclar;
	pthread_data->semaforos_param.sem_salar = sem_salar;
	pthread_data->semaforos_param.sem_agregar = sem_agregar;
	pthread_data->semaforos_param.sem_hornear = sem_hornear;
	pthread_data->semaforos_param.sem_cocinar = sem_cocinar;
	pthread_data->semaforos_param.sem_empanar = sem_empanar;
	pthread_data->semaforos_param.sem_armar = sem_armar;
	pthread_data->semaforos_param.sem_cortaExtras = sem_cortaExtras;

	//seteo las acciones y los ingredientes leyendo desde un archivo

	fptr1 = fopen("receta.txt","r");

	pasaInfo(fptr1,pthread_data);
	
	//inicializo los semaforos

	sem_init(&(pthread_data->semaforos_param.sem_mezclar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_salar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_agregar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_empanar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_cocinar),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_hornear),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_cortaExtras),0,0);
	sem_init(&(pthread_data->semaforos_param.sem_armar),0,0);


	//creo los hilos a todos les paso el struct creado (el mismo a todos los hilos) ya que todos comparten los semaforos 
    int rc;
    rc = pthread_create(&p1,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortar,             //funcion a ejecutar
                                pthread_data);                     //parametros de la funcion a ejecutar, pasado por referencia
	rc = pthread_create(&p2,                           //identificador unico
                            NULL,                          //atributos del thread
                                mezclar,             //funcion a ejecutar
                                pthread_data);
	rc = pthread_create(&p3,                           //identificador unico
                            NULL,                          //atributos del thread
                                salar,             //funcion a ejecutar
                                pthread_data);
	rc = pthread_create(&p4,                           //identificador unico
                            NULL,                          //atributos del thread
                                agregar,             //funcion a ejecutar
                                pthread_data);							
	rc = pthread_create(&p5,                           //identificador unico
                            NULL,                          //atributos del thread
                                empanar,             //funcion a ejecutar
                                pthread_data);
	rc = pthread_create(&p6,                           //identificador unico
                            NULL,                          //atributos del thread
                                cocinar,             //funcion a ejecutar
                                pthread_data);
	rc = pthread_create(&p7,                           //identificador unico
                            NULL,                          //atributos del thread
                                hornear,             //funcion a ejecutar
                                pthread_data);
	rc = pthread_create(&p8,                           //identificador unico
                            NULL,                          //atributos del thread
                                cortarExtras,             //funcion a ejecutar
                                pthread_data);
	rc = pthread_create(&p9,                           //identificador unico
                            NULL,                          //atributos del thread
                                armado,             //funcion a ejecutar
                                pthread_data);																																										
	
	//join de todos los hilos
	pthread_join (p1,NULL);
	pthread_join (p2,NULL);
	pthread_join (p3,NULL);
	pthread_join (p4,NULL);
	pthread_join (p5,NULL);
	pthread_join (p6,NULL);
	pthread_join (p7,NULL);
	pthread_join (p8,NULL);
	pthread_join (p9,NULL);

	//valido que el hilo se alla creado bien 
    if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     }

	 
	//destruccion de los semaforos
	sem_destroy(&sem_mezclar);
	sem_destroy(&sem_salar);
	sem_destroy(&sem_agregar);
	sem_destroy(&sem_empanar);
	sem_destroy(&sem_cocinar);
	sem_destroy(&sem_hornear);
	sem_destroy(&sem_cortaExtras);
	sem_destroy(&sem_armar);
	
	//salida del hilo
	 pthread_exit(NULL);
}


int main ()
{

	//creo los nombres de los equipos
     int rc;
     int *equipoNombre1 =malloc(sizeof(*equipoNombre1));
     int *equipoNombre2 =malloc(sizeof(*equipoNombre2));
     int *equipoNombre3 =malloc(sizeof(*equipoNombre3));
     int *equipoNombre4 =malloc(sizeof(*equipoNombre4));
  
     *equipoNombre1 = 1;
     *equipoNombre2 = 2;
     *equipoNombre3 = 3;
     *equipoNombre4 = 4;

	//inicializo los mutex
     pthread_mutex_init(&lock, NULL);
     pthread_mutex_init(&lock1, NULL);
     pthread_mutex_init(&lock2, NULL);
     pthread_mutex_init(&lock3, NULL);

	//creo las variables de los hilos de los equipos
     pthread_t equipo1; 
     pthread_t equipo2;
     pthread_t equipo3;
     pthread_t equipo4;

	//abro el archivo para la salida de competencia
    fptr = fopen("competencia.txt", "w");
  
	//inicializo los hilos de los equipos
    rc = pthread_create(&equipo1,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre1); 

    rc = pthread_create(&equipo2,                           //identificador unico
                            NULL,                          //atributos del thread
                                ejecutarReceta,             //funcion a ejecutar
                                equipoNombre2);

   rc = pthread_create(&equipo3,
	                       NULL,
				               ejecutarReceta,
				               equipoNombre3);

   rc = pthread_create(&equipo4,
	                      NULL,
				              ejecutarReceta,
				              equipoNombre4);

   if (rc){
       printf("Error:unable to create thread, %d \n", rc);
       exit(-1);
     } 

	//join de todos los hilos
	pthread_join (equipo1,NULL);
	pthread_join (equipo2,NULL);
	pthread_join (equipo3,NULL);
	pthread_join (equipo4,NULL);

	//destruccion de los mutex
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&lock1);
	pthread_mutex_destroy(&lock2);
	pthread_mutex_destroy(&lock3);

    pthread_exit(NULL);
}


//Para compilar:   gcc subwayArgento.c -o ejecutable -lpthread
//Para ejecutar:   ./ejecutable
