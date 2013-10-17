#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include "comun.h"

typedef struct suscriptor {

//Aca va la informacion para conectarse

} suscriptor;

typedef suscriptor * suscript;

typedef struct tema {
	char * tema;
	int cantSuscript;
	suscript * suscriptores;
} tema;

typedef struct lista_temas {
	tema** temas;
	int cant_temas;
} lista_temas;

void imprimir_temas(const lista_temas* temas);
void imprimir_tema(const tema* tema);
lista_temas* inicializar_temas(const char* archivo);
tema* crear_tema(const char* tematica);
int agregar_tema(lista_temas* temas, const char* tematica);
void traza_estado(const char *mensaje, const char* port);
tema* buscarTema(char* tema, lista_temas * listaTemas);
int procesarNotificacion(notif notificacion, lista_temas* listaTemas);
int procesarBaja(char* tematica, lista_temas * listaTemas);
int procesarAlta(char* tematica, lista_temas* listaTemas);
int procesarMensaje(char* tematica, char* mensaje, lista_temas* listaTemas);

void traza_estado(const char *mensaje, const char* port) {
	printf("\n------------------- %s --------------------\n", mensaje);
	system("netstat -at | head -2 | tail -1");
	char comando[30];
	strcpy(comando, "netstat -at | grep ");
	strcat(comando, port);
	system(comando);
//	system("netstat -at | grep 56789");
	printf("---------------------------------------------------------\n\n");
}

lista_temas* inicializar_temas(const char* archivo) {

	lista_temas* resp = malloc(sizeof(lista_temas));
	resp->cant_temas = 0;
	resp->temas = NULL;

	FILE* file = fopen(archivo, "r");
	char* tematica = NULL;
	int longitud = 0;
	int c;
	while ((c = getc(file)) != -1) {
		if (c == '\n') {
			tematica[longitud] = '\0';

			agregar_tema(resp, tematica);

			longitud = 0;
		} else {
			if (longitud % BLOQUE == 0)
				tematica = realloc(tematica,
						(longitud + BLOQUE) * sizeof(char));
			tematica[longitud++] = c;
		}

	}
	free(tematica);
	fclose(file);
	return resp;
}

int agregar_tema(lista_temas* temas, const char* tematica) {
	if (temas->cant_temas % BLOQUE == 0) {
		temas->temas = realloc(temas->temas,
				(temas->cant_temas + BLOQUE) * sizeof(tema*));
	}

	temas->temas[temas->cant_temas] = crear_tema(tematica);
	temas->cant_temas++;

	return 1;
}

tema* crear_tema(const char* tematica) {

	tema * resp = malloc(sizeof(tema));
	resp->cantSuscript = 0;
	resp->suscriptores = malloc(sizeof(suscript *));
	resp->tema = NULL;
	int i = 0;
	while (tematica[i] != '\0') {
		if (i % BLOQUE == 0)
			resp->tema = realloc(resp->tema, (i + BLOQUE) * sizeof(char));
		resp->tema[i] = tematica[i];
		i++;
	}
	resp->tema[i] = '\0';
	return resp;
}
tema* buscarTema(char* tema, lista_temas * listaTemas) {
	int i = 0;
	for (i = 0; i < listaTemas->cant_temas; i++) {
		if (strcmp(listaTemas->temas[i]->tema, tema) == 0) {
			return listaTemas->temas[i];
		}
	}
	return NULL ;
}
void imprimir_temas(const lista_temas* temas) {
	int i = 0;
	printf("Hay %d temas disponibles \n", temas->cant_temas);
	for (i = 0; i < temas->cant_temas; i++) {
		imprimir_tema(temas->temas[i]);
	}
}

void imprimir_tema(const tema* tema) {
	printf("El nombre del tema es %s y hay %d procesos suscriptos \n",
			tema->tema, tema->cantSuscript);

}

int procesarNotificacion(notif notificacion, lista_temas* listaTemas) {

	switch (notificacion->opt) {
	case BAJA:
		return procesarBaja(notificacion->tema, listaTemas);
	case ALTA:
		return procesarAlta(notificacion->tema, listaTemas);

	case MENSAJE:
		return procesarMensaje(notificacion->tema, notificacion->mensaje,
				listaTemas);
	}
	return -1;
}

int procesarBaja(char* tematica, lista_temas * listaTemas) {

	int i = 0;
	tema * temaBus = buscarTema(tematica, listaTemas);
	if (temaBus == NULL )
		return -1;
	for (i = 0; i < temaBus->cantSuscript; i++) {
		//TODO BUSCAR SUSCRIPTOR SI NO EXISTE DEVOLVER ERROR;
		temaBus->cantSuscript--;
	}

	return -1;
}

int procesarAlta(char* tematica, lista_temas* listaTemas) {

	tema* temaBus = buscarTema(tematica, listaTemas);
	if (temaBus == NULL )
		return -1;
	int cantSus = temaBus->cantSuscript;
	if (cantSus % BLOQUE == 0)
		temaBus->suscriptores = realloc(temaBus->suscriptores,
				(temaBus->cantSuscript + BLOQUE) * sizeof(suscript));
	temaBus->suscriptores[cantSus] = malloc(sizeof(suscriptor));
	temaBus->cantSuscript += 1;
	//TODO AGREGAR EL SUSCRIPTOR A LA LISTA

	return 1;
}

int procesarMensaje(char* tematica, char* mensaje, lista_temas* listaTemas) {

	tema * temaBus = buscarTema(tematica, listaTemas);
	if (temaBus == NULL ) {
		printf("SE RECIBIO EL TEMA %s Y NO EXISTE", tematica);
		return -1;

	}

	printf("SE RECIBIO UN MENSAJE: \n TEMA: %s \n CON MENSAJE: %s \n", tematica,
			mensaje);
	return 1;

}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		perror("La cantidad de argumentos es invalida");
		return 1;
	}
	char * port = argv[1];
	int portInt = atoi(port);
	char* archivo = argv[2];
	lista_temas* listaTemas = inicializar_temas(archivo);
	imprimir_temas(listaTemas);

	int s, s_conec, leido;
	unsigned int tam_dir;
	struct sockaddr_in dir, dir_cliente;
	char buf[TAM];
	char mensajeRsp[5];
	int opcion = 1, resp = -1;
	char* mensaje = NULL;
	int mensajeLong = 0;

	if ((s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("error creando socket");
		return 1;
	}

	/* Para reutilizar puerto inmediatamente
	 */
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion)) < 0) {
		perror("error en setsockopt");
		return 1;
	}

	dir.sin_addr.s_addr = INADDR_ANY;
	dir.sin_port = htons(portInt);
	dir.sin_family = PF_INET;
	if (bind(s, (struct sockaddr *) &dir, sizeof(dir)) < 0) {
		perror("error en bind");
		close(s);
		return 1;
	}

	if (listen(s, 5) < 0) {
		perror("error en listen");
		close(s);
		return 1;
	}

	traza_estado("Despu�s de listen", port);

	while (1) {
		tam_dir = sizeof(dir_cliente);
		if ((s_conec = accept(s, (struct sockaddr *) &dir_cliente, &tam_dir))
				< 0) {
			perror("error en accept");
			close(s);
			return 1;
		}

		traza_estado("Despu�s de accept", port);

		while ((leido = read(s_conec, buf, TAM)) > 0) {

			mensaje = realloc(mensaje, mensajeLong + leido * sizeof(char));

			memcpy((void*) mensaje, (void*) buf, leido);

		}

		notif notificacion = unMarshallMsg(mensaje);
		resp = procesarNotificacion(notificacion, listaTemas);
		memcpy((void *) mensajeRsp, (void*) &resp, sizeof(int));
		mensajeRsp[4] = 0;

		// TODO SE TRABA ACAAAAAAAAAAAAAA
		if (write(s_conec, mensajeRsp, TAMANIO_RESPUESTA) < 0) {
			perror("error en write");
			close(s);
			close(s_conec);
			return 1;
		}

		close(s_conec);
		traza_estado("Despu�s de close de conexi�n", port);

	}

	close(s);

	return 0;

}