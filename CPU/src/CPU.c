#include <stdio.h>
#include <stdlib.h>
#include <parser/parser.h>
#include <utilidades/general.h>
#include "lib/fcpu.h"

int main(void) {

	leerArchivoDeConfiguracion("configCPU.txt"); // Abro archivo configuración

	testLecturaArchivoDeConfiguracion();

	conectarConNucleo(); // Conexión con Núcleo

	conectarConUMC(); // Conexión con UMC

	return EXIT_SUCCESS;
}
