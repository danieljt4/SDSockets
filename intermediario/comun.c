/*
 Incluya en este fichero todas las implementaciones que pueden
 necesitar compartir todos los módulos (editor, subscriptor y
 proceso intermediario), si es que las hubiera.
 */

#include "comun.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char* marshallMsg(const int opt, char ** argumento, int argumentos,
		int* longitud) {

	int i = 0;
	char* msg = malloc(sizeof(int) + sizeof(char));

	*longitud = 0;
	memcpy((void*) msg, (void *) &opt, sizeof(int));
	*longitud += sizeof(int);
	msg[(*longitud)] = 0;
	*longitud += sizeof(char);

	for (i = 0; i < argumentos; i++) {
		msg = realloc(msg, *longitud + strlen(argumento[i]) + sizeof(char));
		memcpy((void*) (msg + *longitud), (void*) argumento[i],
				strlen(argumento[i]));
		*longitud += strlen(argumento[i]);
		msg[(*longitud)] = 0;
		*longitud += sizeof(char);
	}
	return msg;
}

notif unMarshallMsg(char* msg) {

	notif resp = malloc(sizeof(notificacion));
	int longitud = 0;
	int longArgumento = 0	;
	memcpy((void *) &(resp->opt), (void *) msg, sizeof(int));
	longitud += sizeof(int) + sizeof(char);
	longArgumento = strlen(msg + longitud) + 1;
	resp->tema = malloc(longArgumento);
	memcpy((void*) resp->tema, (void*) (msg + longitud), longArgumento);
	longitud += longArgumento;
	if (resp->opt == MENSAJE) {
		longArgumento = strlen(msg + longitud) + 1;
		resp->mensaje = malloc(longArgumento);
		memcpy((void*) resp->mensaje, (void*) (msg + longitud), longArgumento);

	} else if (resp->opt != ALTA && resp->opt == BAJA) {
		free(resp->tema);
		free(resp);
		return NULL ;
	}
	return resp;
}