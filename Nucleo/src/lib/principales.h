#ifndef LIB_PRINCIPALES_H_
#define LIB_PRINCIPALES_H_

#include "funciones.h"

/*** FUNCIONES PRINCIPALES ***/
void inicializarColecciones();
void crearLoggerNucleo();
void iniciarEscuchaDeInotify();
void leerConfiguracionNucleo();
void llenarDiccionarioSemaforos();
void llenarDiccionarioVarCompartidas();
void lanzarHilosIO();
int conexionConUMC();
void esperar_y_PlanificarProgramas();
void unirHilosIO();
void liberarRecursosUtilizados();

#endif /* LIB_PRINCIPALES_H_ */
