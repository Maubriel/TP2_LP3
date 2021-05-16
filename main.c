#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ncurses.h>

typedef struct coche
{
	int id;
	struct coche *sgte;
}list;

int pos = 0, nro1 = 0, nro2 = 0, nro3 = 0;
int contador = 0;
int terminacion = 0;
int flag = 0;
char cadena[1000];
sem_t mutex;
list *inicio_izq = NULL;
list *inicio_der = NULL;

void imprimir_lista(list *izq, list *der);
void imprimir_puente(int nro1, int nro2, int nro3, int lado);
int extraer_vehiculo(int lado);
void handler(void *ptr);
void lectura();
void colocar(int lado);

int main(void)
{
	inicio_izq = (list*) malloc(sizeof(list));
	inicio_der = (list*) malloc(sizeof(list));
	
	pthread_t hilo_izq;
   	pthread_t hilo_der;
   	pthread_t hilo_lectura;
	
	sem_init(&mutex, 0, 1);
	
	int k[] = {0, 1};
	
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	
	pthread_create(&hilo_lectura, NULL, (void*)&lectura, NULL);
	while(flag == 0 && terminacion == 0);
	
	pthread_create(&hilo_izq, NULL, (void*)&handler, (void*)&k[0]);
	sleep(0.5);
    	pthread_create(&hilo_der, NULL, (void*)&handler, (void*)&k[1]);
	
	pthread_join(hilo_izq, NULL);
	pthread_join(hilo_der, NULL);
	pthread_join(hilo_lectura, NULL);
	
	endwin();
	
	sem_destroy(&mutex);
	printf("Termina\n");
	return 0;
}

void handler(void *ptr)
{
	int pasaron = 0, actus = 0, max_actus = 7;
	
	while(terminacion != 1)
	{
		sem_wait(&mutex);
		
		pos = *((int *) ptr);
		clear();
		imprimir_puente(nro1, nro2, nro3, pos);
		imprimir_lista(inicio_izq, inicio_der);
		printw("\n> car %s ", cadena);
		refresh();
		sleep(1);
		
		while(actus < max_actus || nro1 > 0 || nro2 > 0 || nro3 > 0)
		{
			if(pos == 0)
			{
				nro3 = nro2;
				nro2 = nro1;
				if(pasaron < 4)
					nro1 = extraer_vehiculo(pos);
				else
					nro1 = 0;
				if(nro1 < 1 && pasaron == 0)
					break;
				if(nro1 < 1)
					max_actus = pasaron+3;
				else
					pasaron++;
			}
			else
			{
				nro1 = nro2;
				nro2 = nro3;
				if(pasaron < 4)
					nro3 = extraer_vehiculo(pos);
				else
					nro3 = 0;
				if(nro3 < 1 && pasaron == 0)
					break;
				if(nro3 < 1)
					max_actus = pasaron+3;
				else
					pasaron++;
			}
			
			clear();
			imprimir_puente(nro1, nro2, nro3, pos);
			imprimir_lista(inicio_izq, inicio_der);
			printw("\n> car %s", cadena);;
			refresh();
			
			actus++;
			sleep(1);
		}
		pasaron = 0;
		actus = 0;
		max_actus = 7;
		
		sem_post(&mutex);
		sleep(1);
	}
	pthread_exit(0);
}

void imprimir_puente(int nro1, int nro2, int nro3, int lado)
{
	char dir;
	if(lado == 0)
		dir = '>';
	else
		dir = '<';
	
	printw("\t\t\t%c%c%cSIMULACION%c%c%c\n\n", dir,dir,dir,dir,dir,dir);
	
	printw("\t=============================================\n");
	if(nro1 > 0)
		printw("\t-%cauto%02d%c---------", dir, nro1, dir);
	else
		printw("\t------------------");
	if(nro2 > 0)
		printw("%cauto%02d%c----------", dir, nro2, dir);
	else
		printw("------------------");
	if(nro3 > 0)
		printw("%cauto%02d%c-\n", dir, nro3, dir);
	else
		printw("---------\n");
	printw("\t=============================================\n");
}

void imprimir_lista(list *izq, list *der)
{
	printw("\n\nEsperando en la izq:\t\t\tEsperando en la der:\n");
	
	if(izq == NULL && der == NULL)
		printw("\n");
		
	while(izq != NULL || der != NULL)
	{
		if(izq != NULL)
		{
			if(izq->id > 0)
				printw("Auto%02d\t\t\t\t\t", izq->id);
			else
				printw("\t\t\t\t\t");
			izq = izq->sgte;
		}
		else
			printw("\t\t\t\t\t");
		if(der != NULL)
		{
			if(der->id > 0)
				printw("Auto%02d\n", der->id);
			else
				printw("\n");
			der = der->sgte;
		}
		else
			printw("\n");
	}
}

int extraer_vehiculo(int lado)
{
	int aux = 0;
	if(lado == 0)
	{
		if(inicio_izq == NULL)
		{
			inicio_izq = (list*) malloc(sizeof(list));
			return 0;
		}
		aux = inicio_izq->id;
		inicio_izq = inicio_izq->sgte;
		return aux;
	}
	if(lado == 1)
	{
		if(inicio_der == NULL)
		{
			inicio_der = (list*) malloc(sizeof(list));
			return 0;
		}
		aux = inicio_der->id;
		inicio_der = inicio_der->sgte;
		return aux;
	}
	exit(0);
}

void lectura()
{
	char ch;
	printw("\t\t\tIngrese los autos que esperarán en cada lado\n");
	printw("> ");
	refresh();
	while(1)
	{
		ch = getch();
		if(ch == 10) //tecla ENTER
		{
			if((strcmp(cadena, "car izq") == 0 && flag == 0) || (strcmp(cadena, "izq") == 0 && flag == 1))
			{
				colocar(0);
			}
			else if((strcmp(cadena, "car der") == 0 && flag == 0) || (strcmp(cadena, "der") == 0 && flag == 1))
			{
				colocar(1);
			}
			else if(strcmp(cadena, "status") == 0 && flag == 0)
			{
				imprimir_lista(inicio_izq, inicio_der);
				refresh();
			}
			else if(strcmp(cadena, "start") == 0 && flag == 0)
			{
				flag = 1;
				clear();
			}
			else if(strcmp(cadena, "clear") == 0 && flag == 0)
			{
				clear();
				printw("\t\t\tIngrese los autos que esperarán en cada lado\n");
			}
			else if(strcmp(cadena, "exit") == 0 || strcmp(cadena, "car exit") == 0)
				break;
			else
				printw("\n¡Comando invalido!\n");
			
			printw("\n> ");
			if(flag == 0)
				strcpy(cadena, "");
			else
			{
				clear();
				strcpy(cadena, "");
				imprimir_puente(nro1, nro2, nro3, pos);
				imprimir_lista(inicio_izq, inicio_der);
				printw("\n> car %s", cadena);
				refresh();
			}
		}
		else if(ch == 7) //tecla BACKSPACE
		{
			printw("\b \b");
			cadena[strlen(cadena)-1] = '\0';
			refresh();
		}
		else //tecla normal
		{
			strcat(cadena, &ch);
			printw("%c", ch);
			refresh();
		}
	}
	terminacion = 1;
	pthread_exit(0);
}

void colocar(int lado)
{
	list *aux = NULL;
	if(lado == 0)
	{
		if(inicio_izq == NULL)
			inicio_izq = (list*) malloc(sizeof(list));
		aux = inicio_izq;
	}
	else
	{
		if(inicio_der == NULL)
			inicio_der = (list*) malloc(sizeof(list));
		aux = inicio_der;
	}
	
	while(aux->id > 0)
		aux = aux->sgte;
	aux->id = ++contador;
	aux->sgte = (list*) malloc(sizeof(list));
}

