#include <stdio.h>
#include <stdlib.h>
#include <utilidades/general.h>
#include "lib/fumc.h"

int main(void) {

	leerArchivoDeConfiguracion("config.txt"); // Abro archivo configuración

	escucharANucleo(); // Conexión con Núcleo

	escucharACPU(); // Conexión con CPU

	return EXIT_SUCCESS;
}
