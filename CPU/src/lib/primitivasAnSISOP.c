#include "primitivasAnSISOP.h"

/* FUNCIONES EXTRA */

bool esArgumento(t_nombre_variable identificador_variable){
	if(isdigit(identificador_variable)){
		return true;
	}else{
		return false;
	}
}

/* --- PRIMITIVAS --- */
t_puntero definirVariable(t_nombre_variable var_nombre){
	/* Le asigna una posición en memoria a la variable,
	 y retorna el offset total respecto al inicio del stack. */

	if(!esArgumento(var_nombre)){
		log_info(logger, "Definiendo nueva variable: %c.", var_nombre);
	}
	else{
		log_info(logger, "Definiendo nuevo argumento: %c.", var_nombre);
	}

		char * var_id = strdup(charAString(var_nombre));

		direccion * var_direccion = malloc(sizeof(direccion));
		var_direccion->pagina = pcbActual->primerPaginaStack;
		var_direccion->offset = pcbActual->stackPointer;
		var_direccion->size = INT;

		while(var_direccion->offset > tamanioPagina){
			(var_direccion->pagina)++;
			var_direccion->offset -= tamanioPagina;
		}
		// Verifico si se desborda la pila en memoria:
		if(pcbActual->stackPointer + 4 > (tamanioPagina*tamanioStack)){
				printf("Hubo stack overflow. Se finalizará el proceso actual #%d.\n", pcbActual->pid);
				huboStackOverflow = true;

			return ERROR;
		}else{
			// Agrego un nuevo registro al índice de stack:
		registroStack* regStack = list_get(pcbActual->indiceStack, pcbActual->indexActualStack);
			if(regStack == NULL){
				regStack = reg_stack_create(); // TODO: Ver si pasar tamaño(s) como argumento del creator
			}
			//(stackPointer: desplazamiento desde la primer página del stack hasta donde arranca mi nueva variable)
			if(!esArgumento(var_nombre)){
				dictionary_put(regStack->vars, var_id, var_direccion);
			}
			else{
				dictionary_put(regStack->args, var_id, var_direccion);
			}
			log_debug(logger, "'%c' -> Dirección lógica: %i, %i, %i", var_id, var_direccion->pagina, var_direccion->offset,var_direccion->size);
			free (var_id); var_id = NULL;
			free(var_direccion); var_direccion = NULL;
			// Guardo la nueva variable en el índice:
			list_add(pcbActual->indiceStack, regStack);
			// Valor a retornar:
			int var_stack_offset = pcbActual->stackPointer;
			// Actualizo parámetros del PCB:
			pcbActual->stackPointer += INT;
			int total_heap_offset = ((pcbActual->paginas_codigo*tamanioPagina)+pcbActual->stackPointer);
			pcbActual->paginaActualStack = total_heap_offset/tamanioPagina;

		return var_stack_offset;
		} // fin else ERROR
}

t_puntero obtenerPosicionVariable(t_nombre_variable var_nombre){
	/* En base a la posición de memoria de la variable,
	 retorna el offset total respecto al inicio del stack. */

	if(!esArgumento(var_nombre)){
		log_debug(logger, "Obteneniendo posición de la variable: '%c'.", var_nombre);
	}
	else{
		log_debug(logger, "Obteneniendo posición de la variable: '%c'.", var_nombre);
	}
	char* var_id = strdup(charAString(var_nombre));
	// Obtengo el registro del stack correspondiente al contexto de ejecución actual:
	registroStack* regStack = list_get(pcbActual->indiceStack, pcbActual->indexActualStack);
	// Me posiciono al inicio de este registro y busco la variable del diccionario que coincida con el nombre solicitado:

	if(!esArgumento(var_nombre)){
		if(dictionary_size(regStack->vars) > 0){

				if(dictionary_has_key(regStack->vars, var_id)){
					direccion * var_direccion = malloc(sizeof(direccion));
					var_direccion = (direccion*)dictionary_get(regStack->vars, var_id);
					free(var_id); var_id = NULL;

					int var_stack_page = var_direccion->pagina - pcbActual->primerPaginaStack;
					int var_stack_offset = (var_stack_page*tamanioPagina) + var_direccion->offset;
					free(var_direccion); var_direccion = NULL;

					return var_stack_offset;
				}
			log_error(logger, "La variable buscada no se encuentra en el registro actual de stack.");
			return ERROR;
		}
		log_error(logger, "No hay variables en el registro actual de stack.");
		return ERROR;
	}
	else{
		if(dictionary_size(regStack->args) > 0){

				if(dictionary_has_key(regStack->args, var_id)){
					direccion * var_direccion = malloc(sizeof(direccion));
					var_direccion = (direccion*)dictionary_get(regStack->args, var_id);
					free(var_id); var_id = NULL;

					int var_stack_page = var_direccion->pagina - pcbActual->primerPaginaStack;
					int var_stack_offset = (var_stack_page*tamanioPagina) + var_direccion->offset;
					free(var_direccion); var_direccion = NULL;

					return var_stack_offset;
				}
			log_error(logger, "El argumento buscado no se encuentra en el registro actual de stack.");
			return ERROR;
		}
		log_error(logger, "No hay argumentos en el registro actual de stack.");
		return ERROR;
	}
}

t_valor_variable dereferenciar(t_puntero var_stack_offset){

	/* Retorna el valor leído a partir de var_stack_offset. */
	printf("Dereferenciando variable.\n");
	solicitudLectura * var_direccion = malloc(sizeof(solicitudLectura));

	int num_pagina =  var_stack_offset / tamanioPagina;
	int offset = var_stack_offset - (num_pagina*tamanioPagina);

	var_direccion->pagina = num_pagina;
	var_direccion->offset = offset;
	var_direccion->tamanio = INT;

	int head;
	void* entrada = NULL;
	int* valor_variable = NULL;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_LECTURA_VARIABLE, var_direccion);
	free(var_direccion); var_direccion = NULL;

	// Valido el pedido de lectura a UMC:
	if(!recibirYvalidarEstadoDelPedidoAUMC()){ // hubo error de lectura
		log_error(logger, "La variable no pudo dereferenciarse.");
		exitPorErrorUMC();
		return 0;
	}
	else{ // no hubo error de lectura
		entrada = aplicar_protocolo_recibir(fdUMC, &head); // respuesta OK de UMC, recibo la variable leída
		if(head == DEVOLVER_VARIABLE){
			valor_variable = (int*)entrada;
		}
		else{
			printf("Error al leer variable del proceso #%d.\n", pcbActual->pid);
		}
		return *valor_variable;
	}
}

void asignar(t_puntero var_stack_offset, t_valor_variable valor){

	/* Escribe en el stack de memoria el valor en la posición dada. */

	solicitudEscritura * var_escritura = malloc(sizeof(solicitudEscritura));

	int num_pagina =  var_stack_offset / tamanioPagina;
	int offset = var_stack_offset - (num_pagina*tamanioPagina);
		var_escritura->pagina = num_pagina;
		var_escritura->offset = offset;
		var_escritura->contenido = valor;

	aplicar_protocolo_enviar(fdUMC, PEDIDO_ESCRITURA, var_escritura);
	free(var_escritura); var_escritura = NULL;

	// Valido el pedido de lectura a UMC:
	if(!recibirYvalidarEstadoDelPedidoAUMC()){
		log_error(logger, "La variable no pudo asignarse.");
		exitPorErrorUMC();
	}
	return;
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida var_compartida_nombre){

	/* Solicita al Núcleo el valor de la variable compartida. */
	log_debug(logger, "Obteniendo el valor de la variable compartida: '%s'.", var_compartida_nombre);
	char * variableCompartida = malloc(strlen(var_compartida_nombre)+1);
	void* entrada = NULL;
	int* valor_variable = NULL;
	int head;

	variableCompartida = strdup((char*) var_compartida_nombre);

	aplicar_protocolo_enviar(fdNucleo, OBTENER_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida); variableCompartida = NULL;

	entrada = aplicar_protocolo_recibir(fdNucleo, &head);
	if(head == DEVOLVER_VAR_COMPARTIDA){
		valor_variable = (int*) entrada;
	}
	if(valor_variable == NULL){
		printf("Error al obtener variable compartida del proceso #%d.\n", pcbActual->pid);
		return ERROR;
	}
	else{
		return *valor_variable;
	}
}

t_valor_variable asignarValorCompartida(t_nombre_compartida var_compartida_nombre, t_valor_variable var_compartida_valor){

	log_debug(logger, "Asignando el valor %d a la variable compartida '%s'.", var_compartida_valor, var_compartida_nombre);
	var_compartida * variableCompartida = malloc(strlen(var_compartida_nombre)+ 5);

	variableCompartida->nombre = strdup((char*) var_compartida_nombre);
	variableCompartida->valor = var_compartida_valor;

	aplicar_protocolo_enviar(fdNucleo, GRABAR_VAR_COMPARTIDA, variableCompartida);
	free(variableCompartida->nombre); variableCompartida->nombre = NULL;
	free(variableCompartida); variableCompartida = NULL;

	return var_compartida_valor;
}

void irAlLabel(t_nombre_etiqueta nombre_etiqueta){
	log_debug(logger, "Ir al Label: '%s'.", nombre_etiqueta);
	t_puntero_instruccion num_instruccion = metadata_buscar_etiqueta(nombre_etiqueta, pcbActual->indiceEtiquetas, pcbActual->tamanioIndiceEtiquetas);
	pcbActual->pc = num_instruccion - 1;
	return;
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){
	/* Reserva espacio para un nuevo contexto vacío preservando el contexto
	 *  de ejecución actual, para luego volver al mismo. */
	log_debug(logger, "Llamar con Retorno. Preservando el contexto de ejecución actual y la posición de retorno.");

	// Calculo la dirección de retorno y la guardo:
	registroStack * nuevoRegistroStack = reg_stack_create();
	nuevoRegistroStack->retVar.pagina = donde_retornar/tamanioPagina;
	nuevoRegistroStack->retVar.offset = donde_retornar%tamanioPagina;
	nuevoRegistroStack->retVar.size = INT;

	nuevoRegistroStack->retPos = pcbActual->pc; // Guardo el valor actual del program counter
	list_add(pcbActual->indiceStack, nuevoRegistroStack);

	pcbActual->indexActualStack++;

	irAlLabel(etiqueta);
	return;
}

void retornar(t_valor_variable var_retorno){

	log_debug(logger, "Llamada a la función 'retornar'.");
	// Tomo contexto actual y anterior:
	int index = pcbActual->indexActualStack;
	registroStack* registroActual = list_get(pcbActual->indiceStack, index);

	// Limpio los argumentos del registro y descuento el espacio que ocupan en el stack en memoria:
	pcbActual->stackPointer -= (4* dictionary_size(registroActual->args));

	// Limpio las variables del registro y descuento el espacio que ocupan en el stack en memoria:
	pcbActual->stackPointer -= (4 * dictionary_size(registroActual->vars));

	// Calculo la dirección de retorno a partir de retVar:
	t_puntero var_stack_offset = (registroActual->retVar.pagina * tamanioPagina) + registroActual->retVar.offset;
	asignar(var_stack_offset, var_retorno);

	// Elimino el contexto actual del índice de stack:
	// Luego, seteo el contexto de ejecución actual en el index anterior:
	pcbActual->pc =  registroActual->retPos;

	liberarRegistroStack(registroActual); // libero la memoria del registro
	free(list_remove(pcbActual->indiceStack, pcbActual->indexActualStack));
	pcbActual->indexActualStack--;
	return;
}

void imprimir(t_valor_variable valor_mostrar){
	printf("Solicitando imprimir variable.\n");
	int * valor = malloc(INT);
	*valor = valor_mostrar;
	aplicar_protocolo_enviar(fdNucleo,IMPRIMIR, valor);
	free(valor); valor = NULL;
}

void imprimirTexto(char* texto){
	printf("Solicitando imprimir texto.\n");
	char * txt = malloc(strlen(texto)+1);
	txt = strdup(texto);
	aplicar_protocolo_enviar(fdNucleo, IMPRIMIR_TEXTO, txt);
	free(txt);
}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

	log_info(logger, "Entrada/Salida para el dispositivo: '%s' durante '%i' unidades de tiempo.",dispositivo,tiempo);
	pedidoIO * pedidoEntradaSalida = malloc(strlen(dispositivo)+ 5);
	pedidoEntradaSalida->nombreDispositivo = strdup((char*) dispositivo);
	pedidoEntradaSalida->tiempo = tiempo;

	aplicar_protocolo_enviar(fdNucleo,ENTRADA_SALIDA, pedidoEntradaSalida);

	free(pedidoEntradaSalida->nombreDispositivo); pedidoEntradaSalida->nombreDispositivo = NULL;
	free(pedidoEntradaSalida); pedidoEntradaSalida = NULL;
	devolvioPcb = POR_IO;
}

void s_wait(t_nombre_semaforo nombre_semaforo){

	char* id_semaforo = malloc(strlen(nombre_semaforo)+1);
	id_semaforo = strdup((char*) nombre_semaforo);

	aplicar_protocolo_enviar(fdNucleo, WAIT_REQUEST, id_semaforo);
	free(id_semaforo); id_semaforo = NULL;

	int head;
	aplicar_protocolo_recibir(fdNucleo, &head);

	if(head == WAIT_CON_BLOQUEO){
		// Mando la pcb bloqueada y la saco de ejecución:
		devolvioPcb = POR_WAIT;
		log_info(logger, "Proceso bloqueado al hacer WAIT del semáforo: '%s'.", nombre_semaforo);
	}
	else{
		log_info(logger, "Proceso continúa ejecutando luego de hacer WAIT del semáforo: '%s'.", nombre_semaforo);
	}
}

void s_signal(t_nombre_semaforo nombre_semaforo){

	char* id_semaforo = malloc(strlen(nombre_semaforo)+1);
	id_semaforo = strdup((char*) nombre_semaforo);

	aplicar_protocolo_enviar(fdNucleo, SIGNAL_REQUEST, id_semaforo);

	log_info(logger, "SIGNAL del semáforo '%s'.", nombre_semaforo);
	free(id_semaforo); id_semaforo = NULL;
}
