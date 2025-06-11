#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//agregar imputados, verificaciones para que no se agreguen datos invalidos.
struct nodoCausa {
    struct Causa *causa;//head
    struct nodoCausa *izquierda;
    struct nodoCausa *derecha;
};

struct Causa {
    char ruc[20];
    char estado[100];
    char decisiones[100];
    struct nodoDenuncia *denuncias;
    int claveDeAcceso;
    struct carpetaInvestigativa *carpeta;
};

struct nodoDenuncia {
    struct Denuncia *denuncia;
    struct nodoDenuncia *siguiente;
}; 

struct Denuncia {
    char denunciante[50];
    char fecha[11]; //DD/MM/AAAA
    char descripcion[1000];
}; 

struct carpetaInvestigativa {
    struct nodoDeclaracion *declaraciones;
    struct nodoPrueba *pruebas;
    struct nodoDiligencias *diligencias;
    struct nodoResolucion *resoluciones;
    struct Imputado *imputado;
};

struct Imputado {
    char nombre[50];
    char rut[15];
    char medidasCautelares[1000];
    char estadoProcesal[1000];
    struct nodoDeclaracion *declaraciones;
}; 

struct nodoResolucion {
    struct Resolucion *resolucion;
    struct nodoResolucion *siguiente;
};

struct Resolucion {
    char tipo[50]; 
    char contenido[1000];
}; 

struct nodoDiligencias {
    struct Diligencia *diligencia;
    struct nodoDiligencias *siguiente;
}; 

struct Diligencia {
    char descripcion[1000];
    char prioridad[20];  
}; 

struct nodoPrueba {
    struct Prueba *prueba;
    struct nodoPrueba *siguiente;
}; 

struct Prueba {
    char tipo[30];// documental, pericial, etc.
    char descripcion[1000];
}; 

struct nodoDeclaracion {
    struct Declaracion *declaracion;
    struct nodoDeclaracion *siguiente;
}; 

struct Declaracion {
    char tipo[20]; //victima, testigo, imputado
    char contenido[1000];
};

//todavía no hay ninguna causa guardada en el arbol, inicio apunta a NULL al principio, luego ira apuntando a todas las causas que se ingresen, pero en específico a la primera que se ingrese
struct nodoCausa *inicio = NULL;
int contarMedidasProteccionActivas(struct nodoCausa *nodo) {
    if (nodo == NULL) return 0;

    int contador = 0;

    if (nodo->causa->carpeta != NULL && nodo->causa->carpeta->imputado != NULL) {
        if (strlen(nodo->causa->carpeta->imputado->medidasCautelares) > 0) {
            contador++;
        }
    }

    contador += contarMedidasProteccionActivas(nodo->izquierda);
    contador += contarMedidasProteccionActivas(nodo->derecha);

    return contador;
}
int contarArchivosProvisionales(struct nodoCausa *nodo) {
    if (nodo == NULL) return 0;

    int contador = 0;

    if (nodo->causa->carpeta != NULL) {
        struct nodoResolucion *actual = nodo->causa->carpeta->resoluciones;
        while (actual != NULL) {
            if (strcmp(actual->resolucion->tipo, "archivo provisional") == 0) {
                contador++;
            }
            actual = actual->siguiente;
        }
    }

    contador += contarArchivosProvisionales(nodo->izquierda);
    contador += contarArchivosProvisionales(nodo->derecha);

    return contador;
}
int contarSentenciasCondenatorias(struct nodoCausa *nodo) {
    if (nodo == NULL) {
        return 0;
    }

    int contador = 0;

    // Recorrer resoluciones
    if (nodo->causa->carpeta != NULL) {
        struct nodoResolucion *actual = nodo->causa->carpeta->resoluciones;
        while (actual != NULL) {
            if (strcmp(actual->resolucion->tipo, "condenatoria") == 0) {
                contador++;
            }
            actual = actual->siguiente;
        }
    }

    // Recursión sobre subárbol izquierdo y derecho
    contador += contarSentenciasCondenatorias(nodo->izquierda);
    contador += contarSentenciasCondenatorias(nodo->derecha);

    return contador;
}
int validarFecha(char *fecha) {
    if (strlen(fecha) != 8) return 0;
    if (fecha[2] != '/' || fecha[5] != '/') return 0;
    for (int i = 0; i < 8; i++) {
        if (i == 2 || i == 5) continue;
        if (fecha[i] < '0' || fecha[i] > '9') return 0;
    }
    return 1;
}

int validarRUTchile(char *rut) {
    int largo = strlen(rut);
    if (largo != 11 && largo != 12) return 0;
    int puntos = 0, guion = 0;
    for (int i = 0; i < largo; i++) {
        if (rut[i] >= '0' && rut[i] <= '9') continue;
        else if (rut[i] == '.') puntos++;
        else if (rut[i] == '-') guion++;
        else return 0;
    }
    if (puntos != 2 || guion != 1) return 0;
    if (rut[largo - 2] != '-') return 0;
    return 1;
}

//nodoCausa* definido con * porque devolverá un puntero al nodo del RUC que estamos buscando
//*nodo es el nodo actual que estamos revisando
struct nodoCausa* buscarCausa(struct nodoCausa *nodo, char rucBuscado[20]) {
    //no se encontro el RUC en ningún nodo por lo que llego al ultimo (NULL)
    if (nodo == NULL){
        return NULL;
    } 
    //encontramos el nodo del ruc buscado
    if (strcmp(nodo->causa->ruc, rucBuscado) == 0){
        return nodo;
    }
    //si el ruc es menor que el actual vamos a tener que seguir buscando por el lado izquierdo
    if (strcmp(rucBuscado, nodo->causa->ruc) < 0){
        return buscarCausa(nodo->izquierda, rucBuscado);
    }
    //si es mayor, por el lado derecho
    else{
        return buscarCausa(nodo->derecha, rucBuscado);
    }
}

//*agregarCausa con * para que la nueva causa que estamos agregando este conectada con el arbol (por su dirección de memoria)
//*inicio con * para modificar el nodo original, no una copia
struct nodoCausa *agregarCausa(struct nodoCausa *inicio, struct Causa *nueva) {
    //creamos el nuevo nodo para la nueva causa
    struct nodoCausa *nuevoNodo = malloc(sizeof(struct nodoCausa));
    //asignamos la causa nueva al nodo nuevo
    nuevoNodo->causa = nueva;
    nuevoNodo->izquierda = NULL;
    nuevoNodo->derecha = NULL;

    //inicio == NULL el arbol esta vacio, queda como el primer nodo
    if (inicio == NULL) {
        return nuevoNodo;
    }

    struct nodoCausa *actual = inicio;
    struct nodoCausa *anterior = NULL;
    //recorremos los nodos hasta que se encuentre un espacio vacio
    while (actual != NULL) {
        //le asignamos el nodo de actual al anterior para que en el caso de que actual llegue a ser NULL guardemos el ultimo nodo valido
        anterior = actual;
        //compara el ruc de la nueva causa con el del nodo, si es menor (el nuevo) se va por la izquierda
        if (strcmp(nueva->ruc, actual->causa->ruc) < 0) {
            actual = actual->izquierda;
        }
        //si es mayor, se va por la derecha
        else {
            actual = actual->derecha;
        }
    }

    //y ahora como anterior tenia el valor del ultimo nodo valido, si es menor el ruc del nuevo que el del anterior se va agrega la izquierda
    if (strcmp(nueva->ruc, anterior->causa->ruc) < 0) {
        anterior->izquierda = nuevoNodo;
    //si es mayor, se agrega por la derecha
    } else {
        anterior->derecha = nuevoNodo;
    }
    //devolvemos ahora el inicio, pero con la nueva causa ya agregada
    return inicio;
}


void opcionesEstado(char estado[])
{
    int opcion;
    printf("Seleccione el estado de la causa:\n");
    printf("1. En investigación\n");
    printf("2. Cerrada\n");
    printf("3. En juicio\n");
    printf("4. Archivada\n");
    printf("Opción: ");
    scanf(" %d", &opcion);

    if (opcion == 1) {
        strcpy(estado, "en investigacion");
    } 
    else if (opcion == 2) {
        strcpy(estado, "cerrada");
    } 
    else if (opcion == 3) {
        strcpy(estado, "en juicio");
    } 
    else if (opcion == 4) {
        strcpy(estado, "archivada");
    } 
    else {
        printf("Opción no válida. Intente de nuevo.\n");
        opcionesEstado(estado);
    }
}

int esEstadoAbierto(const char *estado) {
    const char *ref = "en investigacion";
    if(strcmp(ref, estado) == 0)
        return 1;
    return 0;
}
int contarCausasAbiertas(struct nodoCausa *nodo) {
    int contador = 0;

    if (nodo == NULL) {
        return 0;
    }

    contador += contarCausasAbiertas(nodo->izquierda);
    contador += contarCausasAbiertas(nodo->derecha);

    if (esEstadoAbierto(nodo->causa->estado)) {
        contador++;
    }

    return contador;
}
//*causa con * porque al agregar una denuncia a una causa que ya existe, tenemos que tener la real, no la copia
void agregarDenuncia(struct Causa *causa) {
    //Crear el nuevo nodo denuncia para la nueva denuncia
    struct nodoDenuncia *nueva = malloc(sizeof(struct nodoDenuncia));
    //ahora creamos la estructura de una denuncia (denunciante, fecha, descripcion)
    nueva->denuncia = malloc(sizeof(struct Denuncia));
    //la nueva denuncia siguiente apunta a el primer nodo de denuncias, por lo que queda de las primeras
    nueva->siguiente = causa->denuncias;

    printf("Ingrese nombre del denunciante: ");
    scanf(" %[^\n]", nueva->denuncia->denunciante);
    do {
        printf("Ingrese fecha (DD/MM/AA): ");
        scanf(" %[^\n]", nueva->denuncia->fecha);
        if (!validarFecha(nueva->denuncia->fecha)) {
            printf("Formato inválido. Intente nuevamente.\n");
        }
    } while (!validarFecha(nueva->denuncia->fecha));
    printf("Ingrese descripción: ");
    scanf(" %[^\n]", nueva->denuncia->descripcion);
    printf("\n");
    //guardamos toda la info que recopilamos de la denuncia
    causa->denuncias = nueva;
}

void agregarAcarpetainvestigativa(struct Causa *causa, int opcion){
    //incializar la carpeta si es null
    if (causa->carpeta == NULL) {
        causa->carpeta = malloc(sizeof(struct carpetaInvestigativa));
        causa->carpeta->declaraciones = NULL;
        causa->carpeta->pruebas = NULL;
        causa->carpeta->diligencias = NULL;
        causa->carpeta->resoluciones = NULL;
        causa->carpeta->imputado = NULL;
    }

    int accionFinal;
    if(opcion == 1){
        //agregar declaracion
        //crear la nueva declaración
        struct nodoDeclaracion *nuevaDeclaracion = malloc(sizeof(struct nodoDeclaracion));
        //crear memoria para la estructura de la declaración
        nuevaDeclaracion->declaracion = malloc(sizeof(struct Declaracion));
        printf("Ingrese tipo de declaración (victima, testigo, imputado): ");
        scanf(" %[^\n]", nuevaDeclaracion->declaracion->tipo);
        printf("Ingrese contenido de la declaración: ");
        scanf(" %[^\n]", nuevaDeclaracion->declaracion->contenido);
        //la nueva declaración siguiente apunta a el primer nodo de declaraciones, por lo que queda de las primeras
        nuevaDeclaracion->siguiente = causa->carpeta->declaraciones;
        //agregamos la nueva declaración a la carpeta investigativa de la causa
        causa->carpeta->declaraciones = nuevaDeclaracion;
        printf("Declaración agregada correctamente \n");
        printf("¿Desea realizar alguna otra acción? (Ingrese el número correspondiente) \n");
        //mandar al menu principal del main
        printf("1. Sí \n");
        printf("2. No \n");
        scanf(" %d",&accionFinal);
        //si elige 1, volver al main
        if(accionFinal == 1){
            //volver al main
            return;
        }
        //si elige 2, salir del programa
        if(accionFinal == 2){
            exit(0);
        }
    }
    if(opcion == 2){
        //agregar pruebas
        //crear la nueva prueba
        struct nodoPrueba *nuevaPrueba = malloc(sizeof(struct nodoPrueba));
        //crear memoria para la estructura de la prueba
        nuevaPrueba->prueba = malloc(sizeof(struct Prueba));
        printf("Ingrese tipo de prueba (documental, pericial, etc.): ");
        scanf(" %[^\n]", nuevaPrueba->prueba->tipo);
        printf("Ingrese descripción de la prueba: ");
        scanf(" %[^\n]", nuevaPrueba->prueba->descripcion);
        //la nueva prueba siguiente apunta a el primer nodo de pruebas, por lo que queda de las primeras
        nuevaPrueba->siguiente = causa->carpeta->pruebas;
        //agregamos la nueva prueba a la carpeta investigativa de la causa
        causa->carpeta->pruebas = nuevaPrueba;
        printf("Prueba agregada correctamente \n");
        printf("¿Desea realizar alguna otra acción? (Ingrese el número correspondiente) \n");
        //mandar al menu principal del main
        printf("1. Sí \n");
        printf("2. No \n");
        scanf(" %d",&accionFinal);
        //si elige 1, volver al main
        if(accionFinal == 1){
            //volver al main
            return;
        }
        //si elige 2, salir del programa
        if(accionFinal == 2){
            exit(0);
        }
    }
    if(opcion == 3){
        //agregar diligencias
        //crear la nueva diligencia
        struct nodoDiligencias *nuevaDiligencia = malloc(sizeof(struct nodoDiligencias));
        //crear memoria para la estructura de la diligencia
        nuevaDiligencia->diligencia = malloc(sizeof(struct Diligencia));
        printf("Ingrese descripción de la diligencia: ");
        scanf(" %[^\n]", nuevaDiligencia->diligencia->descripcion);
        printf("Ingrese prioridad de la diligencia (alta, media, baja): ");
        scanf(" %[^\n]", nuevaDiligencia->diligencia->prioridad);
        //la nueva diligencia siguiente apunta a el primer nodo de diligencias, por lo que queda de las primeras
        nuevaDiligencia->siguiente = causa->carpeta->diligencias;
        //agregamos la nueva diligencia a la carpeta investigativa de la causa
        causa->carpeta->diligencias = nuevaDiligencia;
        printf("Diligencia agregada correctamente \n");
    }
    if(opcion == 4){
        //agregar resoluciones
        //crear la nueva resolución
        struct nodoResolucion *nuevaResolucion = malloc(sizeof(struct nodoResolucion));
        //crear memoria para la estructura de la resolución
        nuevaResolucion->resolucion = malloc(sizeof(struct Resolucion));
        printf("Ingrese tipo de resolución: ");
        scanf(" %[^\n]", nuevaResolucion->resolucion->tipo);
        printf("Ingrese contenido de la resolución: ");
        scanf(" %[^\n]", nuevaResolucion->resolucion->contenido);
        //la nueva resolución siguiente apunta a el primer nodo de resoluciones, por lo que queda de las primeras
        nuevaResolucion->siguiente = causa->carpeta->resoluciones;
        //agregamos la nueva resolución a la carpeta investigativa de la causa
        causa->carpeta->resoluciones = nuevaResolucion;
        printf("Resolución agregada correctamente \n");
        printf("¿Desea realizar alguna otra acción? (Ingrese el número correspondiente) \n");
        //mandar al menu principal del main
        printf("1. Sí \n");
        printf("2. No \n");
        scanf(" %d",&accionFinal);
        //si elige 1, volver al main
        if(accionFinal == 1){
            //volver al main
            return;
        }
        //si elige 2, salir del programa
        if(accionFinal == 2){
            exit(0);
        }
    }
    if(opcion == 5){
        //agregar imputado
        //crear el nuevo imputado
        struct Imputado *nuevoImputado = malloc(sizeof(struct Imputado));
        //crear memoria para la estructura del imputado
        printf("Ingrese nombre del imputado: ");
        scanf(" %[^\n]", nuevoImputado->nombre);
        do {
            printf("Ingrese RUT del imputado (formato 1.111.111-1 o 11.111.111-1): ");
            scanf(" %[^\n]", nuevoImputado->rut);
            if (!validarRUTchile(nuevoImputado->rut)) {
                printf("Formato inválido. Intente nuevamente.\n");
            }
        } while (!validarRUTchile(nuevoImputado->rut));
        printf("Ingrese medidas cautelares del imputado: ");
        scanf(" %[^\n]", nuevoImputado->medidasCautelares);
        printf("Ingrese estado procesal del imputado: ");
        scanf(" %[^\n]", nuevoImputado->estadoProcesal);
        //inicializar las declaraciones del imputado
        nuevoImputado->declaraciones = NULL;
        //agregar el imputado a la carpeta investigativa de la causa
        causa->carpeta->imputado = nuevoImputado;
        printf("Imputado agregado correctamente \n");
        printf("¿Desea realizar alguna otra acción? (Ingrese el número correspondiente) \n");
        //mandar al menu principal del main
        printf("1. Sí \n");
        printf("2. No \n");
        scanf(" %d",&accionFinal);
        //si elige 1, volver al main
        if(accionFinal == 1){
            //volver al main
            return;
        }
        //si elige 2, salir del programa
        if(accionFinal == 2){
            exit(0);
        }
    }
}

void actualizarCarpetaInvestigativa(struct Causa *causa, int opcion){
    // incializar la carpeta si es null, osea crearla
    if (causa->carpeta == NULL) {
        causa->carpeta = malloc(sizeof(struct carpetaInvestigativa));
        causa->carpeta->declaraciones = NULL;
        causa->carpeta->pruebas = NULL;
        causa->carpeta->diligencias = NULL;
        causa->carpeta->resoluciones = NULL;
        causa->carpeta->imputado = NULL;
    }

    int opcionAccion;
    if(opcion == 1){
        printf("¿Qué desea actualizar en declaraciones? (Ingrese el número correspondiente)\n");
        printf("1. Agregar declaración\n");
        printf("2. Actualizar declaración\n");
        printf("3. Eliminar declaración\n");
        scanf(" %d",&opcionAccion);
        if(opcionAccion == 1){
            //agregar declaración
            agregarAcarpetainvestigativa(causa, 1);
        }
        if(opcionAccion == 2){
            //actualizar declaración
            //buscar declaración por tipo (victima, testigo, imputado)
            char tipoDeclaracion[20];
            printf("Ingrese el tipo de declaración a actualizar (victima, testigo, imputado): ");
            scanf(" %[^\n]", tipoDeclaracion);
            //buscar la declaración en la carpeta investigativa
            struct nodoDeclaracion *actual = causa->carpeta->declaraciones;
            while(actual != NULL){
                //si son iguales, actualizamos la declaración
                if(strcmp(actual->declaracion->tipo, tipoDeclaracion) == 0){
                    //encontramos la declaración, actualizarla
                    printf("Ingrese nuevo contenido de la declaración: ");
                    scanf(" %[^\n]", actual->declaracion->contenido);
                    printf("Declaración actualizada correctamente \n");
                    //como ya se actualizo, salimos del ciclo
                    return;
                }
                actual = actual->siguiente;
            }
            printf("Declaración no encontrada \n");
        }
        if(opcionAccion == 3){
            //eliminar declaración
            //buscar declaración por tipo (victima, testigo, imputado)
            char tipoDeclaracion[20];
            printf("Ingrese el tipo de declaración a eliminar (victima, testigo, imputado): ");
            scanf(" %[^\n]", tipoDeclaracion);
            //buscar la declaración en la carpeta investigativa
            struct nodoDeclaracion *actual = causa->carpeta->declaraciones;
            struct nodoDeclaracion *anterior = NULL;
            while(actual != NULL){
                //si son iguales, eliminamos la declaración
                if(strcmp(actual->declaracion->tipo, tipoDeclaracion) == 0){
                    //encontramos la declaración, eliminarla
                    if(anterior == NULL){
                        //es el primer nodo
                        causa->carpeta->declaraciones = actual->siguiente;
                    } else {
                        anterior->siguiente = actual->siguiente;
                    }
                    printf("Declaración eliminada correctamente \n");
                    //ya se elimino, asi que salimos del ciclo
                    return;
                }
                anterior = actual;
                actual = actual->siguiente;
            }
            printf("Declaración no encontrada \n");
        }
    }
    if(opcion == 2){
        //actualizar pruebas
        printf("¿Qué desea actualizar en pruebas? (Ingrese el número correspondiente)\n");
        printf("1. Agregar prueba\n");
        printf("2. Actualizar prueba\n");
        printf("3. Eliminar prueba\n");
        scanf(" %d",&opcionAccion);
        if(opcionAccion == 1){
            //agregar prueba
            agregarAcarpetainvestigativa(causa, 2);
        }
        if(opcionAccion == 2){
            //actualizar prueba
            //buscar prueba por tipo (documental, pericial, etc)
            char tipoPrueba[30];
            printf("Ingrese el tipo de prueba a actualizar (documental, pericial, etc.): ");
            scanf(" %[^\n]", tipoPrueba);
            //buscar la prueba en la carpeta investigativa
            struct nodoPrueba *actual = causa->carpeta->pruebas;
            while(actual != NULL){
                //si son iguales, actualizamos la prueba
                if(strcmp(actual->prueba->tipo, tipoPrueba) == 0){
                    //encontramos la prueba, la actualizamos
                    printf("Ingrese nueva descripción de la prueba: ");
                    scanf(" %[^\n]", actual->prueba->descripcion);
                    printf("Prueba actualizada correctamente \n");
                    //como ya se actualizo, salimos del ciclo
                    return;
                }
                actual = actual->siguiente;
            }
            //si termina el ciclo y no se encontro la prueba:
            printf("Prueba no encontrada \n");
        }
        if(opcionAccion == 3){
            //eliminar prueba
            //buscar prueba por tipo (documental, pericial, etc)
            char tipoPrueba[30];
            printf("Ingrese el tipo de prueba a eliminar (documental, pericial, etc.): ");
            scanf(" %[^\n]", tipoPrueba);
            //buscar la prueba en la carpeta investigativa
            struct nodoPrueba *actual = causa->carpeta->pruebas;
            struct nodoPrueba *anterior = NULL;
            while(actual != NULL){
                //si son iguales, eliminamos la prueba
                if(strcmp(actual->prueba->tipo, tipoPrueba) == 0){
                    //encontramos la prueba, eliminarla
                    if(anterior == NULL){
                        //es el primer nodo
                        causa->carpeta->pruebas = actual->siguiente;
                    } else {
                        anterior->siguiente = actual->siguiente;
                    }
                    printf("Prueba eliminada correctamente \n");
                    //ya se elimino, asi que salimos del ciclo
                    return;
                }
                anterior = actual;
                actual = actual->siguiente;
            }
            printf("Prueba no encontrada \n");
        }
    }
    if(opcion == 3){
        //actualizar diligencias
        printf("¿Qué desea actualizar en diligencias? (Ingrese el número correspondiente)\n");
        printf("1. Agregar diligencia\n");
        printf("2. Actualizar diligencia\n");
        printf("3. Eliminar diligencia\n");
        scanf(" %d",&opcionAccion);
        if(opcionAccion == 1){
            //agregar diligencia
            agregarAcarpetainvestigativa(causa, 3);
        }
        if(opcionAccion == 2){
            //actualizar diligencia
            //buscar diligencia por descripción
            char descripcionDiligencia[1000];
            printf("Ingrese la descripción de la diligencia a actualizar: ");
            scanf(" %[^\n]", descripcionDiligencia);
            //buscar la diligencia en la carpeta investigativa
            struct nodoDiligencias *actual = causa->carpeta->diligencias;
            while(actual != NULL){
                //si son iguales, actualizamos la diligencia
                if(strcmp(actual->diligencia->descripcion, descripcionDiligencia) == 0){
                    //encontramos la diligencia, actualizarla
                    printf("Ingrese nueva prioridad de la diligencia (alta, media, baja): ");
                    scanf(" %[^\n]", actual->diligencia->prioridad);
                    printf("Diligencia actualizada correctamente \n");
                    //como ya se actualizo, salimos del ciclo
                    return;
                }
                actual = actual->siguiente;
            }
            printf("Diligencia no encontrada \n");
        }
        if(opcionAccion == 3){
            //eliminar diligencia
            //buscar diligencia por descripción
            char descripcionDiligencia[1000];
            printf("Ingrese la descripción de la diligencia a eliminar: ");
            scanf(" %[^\n]", descripcionDiligencia);
            //buscar la diligencia en la carpeta investigativa
            struct nodoDiligencias *actual = causa->carpeta->diligencias;
            struct nodoDiligencias *anterior = NULL;
            while(actual != NULL){
                //si son iguales, eliminamos la diligencia
                if(strcmp(actual->diligencia->descripcion, descripcionDiligencia) == 0){
                    //encontramos la diligencia, eliminarla
                    if(anterior == NULL){
                        //es el primer nodo
                        causa->carpeta->diligencias = actual->siguiente;
                    } else {
                        anterior->siguiente = actual->siguiente;
                    }
                    printf("Diligencia eliminada correctamente \n");
                    //ya se elimino, asi que salimos del ciclo
                    return;
                }
                anterior = actual;
                actual = actual->siguiente;
            }
            printf("Diligencia no encontrada \n");
        }
    }
    if(opcion == 4){
        //actualizar resoluciones
        printf("¿Qué desea actualizar en resoluciones? (Ingrese el número correspondiente)\n");
        printf("1. Agregar resolución\n");
        printf("2. Actualizar resolución\n");
        printf("3. Eliminar resolución\n");
        scanf(" %d",&opcionAccion);
        if(opcionAccion == 1){
            //agregar resolución
            agregarAcarpetainvestigativa(causa, 4);
        }
        if(opcionAccion == 2){
            //actualizar resolución
            //buscar resolución por tipo
            char tipoResolucion[50];
            printf("Ingrese el tipo de resolución a actualizar: ");
            scanf(" %[^\n]", tipoResolucion);
            //buscar la resolución en la carpeta investigativa
            struct nodoResolucion *actual = causa->carpeta->resoluciones;
            while(actual != NULL){
                //si son iguales, actualizamos la resolución
                if(strcmp(actual->resolucion->tipo, tipoResolucion) == 0){
                    //encontramos la resolución, actualizarla
                    printf("Ingrese nuevo contenido de la resolución: ");
                    scanf(" %[^\n]", actual->resolucion->contenido);
                    printf("Resolución actualizada correctamente \n");
                    //como ya se actualizo, salimos del ciclo
                    return;
                }
                actual = actual->siguiente;
            }
            printf("Resolución no encontrada \n");
        }
        if(opcionAccion == 3){
            //eliminar resolución
            //buscar resolución por tipo
            char tipoResolucion[50];
            printf("Ingrese el tipo de resolución a eliminar: ");
            scanf(" %[^\n]", tipoResolucion);
            //buscar la resolución en la carpeta investigativa
            struct nodoResolucion *actual = causa->carpeta->resoluciones;
            struct nodoResolucion *anterior = NULL;
            while(actual != NULL){
                //si son iguales, eliminamos la resolución
                if(strcmp(actual->resolucion->tipo, tipoResolucion) == 0){
                    //encontramos la resolución, eliminarla
                    if(anterior == NULL){
                        //es el primer nodo
                        causa->carpeta->resoluciones = actual->siguiente;
                    } else {
                        anterior->siguiente = actual->siguiente;
                    }
                    printf("Resolución eliminada correctamente \n");
                    //ya se elimino, asi que salimos del ciclo
                    return;
                }
                anterior = actual;
                actual = actual->siguiente;
            }
            printf("Resolución no encontrada \n");
        }
    }
    if(opcion == 5){
        //actualizar imputado
        printf("¿Qué desea actualizar en imputado? (Ingrese el número correspondiente)\n");
        printf("1. Agregar imputado\n");
        printf("2. Actualizar imputado\n");
        printf("3. Eliminar imputado\n");
        scanf(" %d",&opcionAccion);
        if(opcionAccion == 1){
            //agregar imputado
            agregarAcarpetainvestigativa(causa, 5);
        }
        if(opcionAccion == 2){
            //actualizar imputado
            //buscar imputado por nombre
            char nombreImputado[50];
            printf("Ingrese el nombre del imputado a actualizar: ");
            scanf(" %[^\n]", nombreImputado);
            //buscar el imputado en la carpeta investigativa
            if(causa->carpeta->imputado != NULL && strcmp(causa->carpeta->imputado->nombre, nombreImputado) == 0){
                //encontramos el imputado,hay que actualizarlo
                printf("Ingrese nuevo RUT del imputado: ");
                scanf(" %[^\n]", causa->carpeta->imputado->rut);
                printf("Ingrese nuevas medidas cautelares del imputado: ");
                scanf(" %[^\n]", causa->carpeta->imputado->medidasCautelares);
                printf("Ingrese nuevo estado procesal del imputado: ");
                scanf(" %[^\n]", causa->carpeta->imputado->estadoProcesal);
                printf("Imputado actualizado correctamente \n");
                return;
            }
            printf("Imputado no encontrado \n");
        }
        if(opcionAccion == 3){
            //eliminar imputado
            //buscar imputado por nombre
            char nombreImputado[50];
            printf("Ingrese el nombre del imputado a eliminar: ");
            scanf(" %[^\n]", nombreImputado);
            //buscar el imputado en la carpeta investigativa
            if(causa->carpeta->imputado != NULL && strcmp(causa->carpeta->imputado->nombre, nombreImputado) == 0){
                //encontramos el imputado, eliminarlo
                causa->carpeta->imputado = NULL;
                printf("Imputado eliminado correctamente \n");
                return;
            }
            printf("Imputado no encontrado \n");
        }
    }
}

//verificamos que el RUC tenga el formato del ejemplo 123456789-2025
int validarRUC(char *ruc) {
    int i;
    //validar que los primeros 9 caracteres sean digitos
    for (i = 0; i < 9; i++) {
        if (ruc[i] < '0' || ruc[i] > '9'){
            return 0;
        } 
    }
    //el caracter 9 debe ser un guion 
    if (ruc[9] != '-'){
        return 0;
    } 
    //validar que los siguientes 4 caracteres (despues del guion) sean digitos
    for (i = 10; i < 14; i++) {
        if (ruc[i] < '0' || ruc[i] > '9'){
            return 0;
        } 
    }

    //verificar que no haya mas caracteres despues del ultimo
    if (ruc[14] != '\0'){
        return 0;
    } 
    return 1; // El RUC cumple
}

void consultarCausaExistente(struct nodoCausa *nodo) {
    if (nodo == NULL) {
        printf("No hay causas registradas.\n");
        return;
    }

    int opcionConsulta;
    int claveDeAccesoCMP;
    char rucBuscado[20];  // Ya estaba antes
    struct nodoCausa *encontrada;  // Ya estaba antes

    printf("Ingrese el RUC de la causa: ");
    scanf(" %[^\n]", rucBuscado);

    encontrada = buscarCausa(nodo, rucBuscado);

    if (encontrada == NULL) {
        printf("Causa no encontrada.\n");
        return;
    }

    printf("Ingrese la clave de acceso: ");
    scanf(" %d", &claveDeAccesoCMP);

    if (claveDeAccesoCMP != encontrada->causa->claveDeAcceso) {
        printf("Acceso denegado\n");
        return;
    }

    //si la clave coincide, se muestra el menu
    printf("¿Qué desea consultar? (Ingrese el número correspondiente)\n");
    printf("1. RUC de la causa\n");
    printf("2. Estado de la causa\n");
    printf("3. Decisiones de la causa\n");
    printf("4. Denuncias de la causa\n");
    printf("5. Carpeta investigativa de la causa\n");
    scanf(" %d", &opcionConsulta);

    //si se encuentra la causa, dependiendo de la opcion elegida se muestra la info
    if(opcionConsulta == 1) {
        printf("--------------------------\n");
        printf("\n");
        printf("RUC de la causa: %s\n", encontrada->causa->ruc);
        printf("\n");
        printf("--------------------------\n");
    } 
    else if(opcionConsulta == 2) {
        printf("--------------------------\n");
        printf("\n");
        printf("Estado de la causa: %s\n", encontrada->causa->estado);
        printf("\n");
        printf("--------------------------\n");
    } 
    else if(opcionConsulta == 3) {
        printf("--------------------------\n");
        printf("\n");
        printf("Decisiones de la causa: %s\n", encontrada->causa->decisiones);
        printf("\n");
        printf("--------------------------\n");
    } 
    else if(opcionConsulta == 4) {
        struct nodoDenuncia *denunciaActual = encontrada->causa->denuncias;
        if (denunciaActual == NULL) {
            printf("--------------------------\n");
            printf("\n");
            printf("No hay denuncias registradas para esta causa.\n");
            printf("\n");
            printf("--------------------------\n");
        } 
        else {
            printf("--------------------------\n");
            printf("\n");
            printf("Denuncias de la causa:\n");
            printf("\n");
            while (denunciaActual != NULL) {
                printf("Denunciante: %s, Fecha: %s, Descripción: %s\n",denunciaActual->denuncia->denunciante, denunciaActual->denuncia->fecha, denunciaActual->denuncia->descripcion);
                printf("\n");
                denunciaActual = denunciaActual->siguiente;
            }
            printf("--------------------------\n");
        }
    } 
    else if(opcionConsulta == 5) {
        //mostrar carpeta investigativa
        if (encontrada->causa->carpeta == NULL) {
            printf("--------------------------\n");
            printf("\n");
            printf("No hay carpeta investigativa registrada para esta causa.\n");
            printf("\n");
            printf("--------------------------\n");
        } 
        else {
            //mostrar las declaraciones, pruebas, diligencias, resoluciones e imputado
            printf("--------------------------\n");
            printf("\n");
            printf("Carpeta investigativa:\n");
            printf("\n");
            //declaraciones
            printf("--------------------------\n");
            printf("\n");
            struct nodoDeclaracion *declaracionActual = encontrada->causa->carpeta->declaraciones;
            if (declaracionActual == NULL) {
                printf("No hay declaraciones registradas.\n");
                printf("--------------------------\n");
            } 
            else {
                printf("Declaraciones:\n");
                while (declaracionActual != NULL) {
                    printf("Tipo: %s, Contenido: %s\n",declaracionActual->declaracion->tipo, declaracionActual->declaracion->contenido);
                    printf("\n");
                    declaracionActual = declaracionActual->siguiente;
                }
                printf("--------------------------\n");
            }
            //pruebas
            struct nodoPrueba *pruebaActual = encontrada->causa->carpeta->pruebas;
            if (pruebaActual == NULL) {
                printf("No hay pruebas registradas.\n");
                printf("\n");
                printf("--------------------------\n");
            } 
            else {
                printf("Pruebas:\n");
                printf("\n");
                while (pruebaActual != NULL) {
                    printf("Tipo: %s, Descripción: %s\n", pruebaActual->prueba->tipo, pruebaActual->prueba->descripcion);
                    printf("\n");
                    pruebaActual = pruebaActual->siguiente;
                }
                printf("--------------------------\n");
            }
            //diligencias
            struct nodoDiligencias *diligenciaActual = encontrada->causa->carpeta->diligencias;
            if (diligenciaActual == NULL) {
                printf("No hay diligencias registradas.\n");
                printf("\n");
                printf("--------------------------\n");
            } 
            else {
                printf("Diligencias:\n");
                printf("\n");
                while (diligenciaActual != NULL) {
                    printf("Descripción: %s, Prioridad: %s\n",diligenciaActual->diligencia->descripcion, diligenciaActual->diligencia->prioridad);
                    printf("\n");
                    diligenciaActual = diligenciaActual->siguiente;
                }
                printf("--------------------------\n");
            }
            //resoluciones
            struct nodoResolucion *resolucionActual = encontrada->causa->carpeta->resoluciones;
            if (resolucionActual == NULL) {
                printf("No hay resoluciones registradas.\n");
                printf("\n");
                printf("--------------------------\n");
            } 
            else {
                printf("Resoluciones:\n");
                printf("\n");
                while (resolucionActual != NULL) {
                    printf("Tipo: %s, Contenido: %s\n",resolucionActual->resolucion->tipo, resolucionActual->resolucion->contenido);
                    printf("\n");
                    resolucionActual = resolucionActual->siguiente;
                }
                printf("--------------------------\n");
            }
            //imputado
            if (encontrada->causa->carpeta->imputado == NULL) {
                printf("No hay imputado registrado.\n");
                printf("\n");
                printf("--------------------------\n");
            } 
            else {
                printf("Imputado:\n");
                printf("\n");
                printf("Nombre: %s, RUT: %s, Medidas cautelares: %s, Estado procesal: %s\n",encontrada->causa->carpeta->imputado->nombre, encontrada->causa->carpeta->imputado->rut, encontrada->causa->carpeta->imputado->medidasCautelares,encontrada->causa->carpeta->imputado->estadoProcesal);
                printf("\n");
                printf("--------------------------\n");
            }
        }
    }
}

//ingresar una nueva denuncia o consultar por una que ya exista
void ingresarNuevaCausaoDenuncia(){
    //existe: para saber que opción eligirá el usuario
    int existe;
    int causaExistenteOpciones;
    int claveDeAccesoCMP;
    int carpetaInvestigativaOpciones;
    int carpetaInvestigativaOpcionesNuevaCausa;
    int opcionAccionNuevaCarpetaInvestigativa;
    int accionFinal;
    //si elige consultar una existente se le pedira el ruc
    char rucExistente[20];
    //inicializar las variables de la nueva causa en caso de que elija esa opción
    struct Causa *nuevaCausa = NULL;
    struct nodoCausa *encontrada = NULL;
    struct nodoDenuncia *nuevaDenuncia = NULL;
    //preguntar
    printf("¿Ya existe la causa o desea crear una? (Ingrese el número correspondiente)\n");
    printf("1.Existe\n");
    printf("2.Crear\n");
    scanf(" %d",&existe);
    //caso en el que desea consultar
    if (existe == 1) {
        printf("Ingrese RUC de la causa existente: ");
        scanf(" %[^\n]", rucExistente);
        //buscar la causa
        encontrada = buscarCausa(inicio, rucExistente);
        //si no se encuentra
        if (encontrada == NULL) {
            printf("Causa no encontrada \n");
            return;
        }
        //Clave de acceso
        printf("Ingrese la clave de acceso: ");
        scanf(" %d",&claveDeAccesoCMP);
        //si la clave de acceso ingresada es la misma que la de la causa, se le pregunta que desea hacer
        if(claveDeAccesoCMP == encontrada->causa->claveDeAcceso){
            //preguntar que desea hacer
            printf("¿Qué acción desea realizar? (Introduzca el número correspondiente).\n");
            printf("1. Agregar nueva denuncia \n");
            printf("2. Actualizar carpeta investigativa \n");
            printf("3. Actualizar estado de la causa \n");
            //scanf la opción de la acción que desea realizar
            scanf(" %d",&causaExistenteOpciones);
            //quiere agregar una denuncia
            if(causaExistenteOpciones == 1){
                //llamar a la función para agregar la denuncia
                agregarDenuncia(encontrada->causa);
                printf("Denuncia agregada correctamente \n");
            }
            if(causaExistenteOpciones==2){
                // AQUIIIIIIIIIIIIIIIIIIIIIIIIIIIIII !!!!!!!!!!!!!!!!!!!!!!!! //
                printf("¿Que desea actualizar? (Introduzca el número correspondiente). \n"); 
                //opciones
                printf("1. Declaraciones \n");
                printf("2. Pruebas \n");
                printf("3. Diligencias \n");
                printf("4. Resoluciones \n");
                printf("5. Imputado \n");
                //scanf la opción de la acción que desea realizar
                scanf(" %d",&carpetaInvestigativaOpciones);
                //* REALIZAR LOS CAMBIOS DEPENDIENDO DE LA OPCIÓN ELEGIDA */
                actualizarCarpetaInvestigativa(encontrada->causa, carpetaInvestigativaOpciones);
            }
            if(causaExistenteOpciones==3){
                opcionesEstado(encontrada->causa->estado);
                printf("Estado actualizado correctamente \n");
            }
        }
        //clave de acceso distinta a la de la causa 
        else{
            printf("Acceso denegado");
        }
    }
    //si quiere crear la causa
    if (existe == 2) {
        //crear memoria para la nuevaCausa
        nuevaCausa = malloc(sizeof(struct Causa));
        //no se pudo reservar memoria (lo hacemos por buena práctica para evitar segmentation fault)
        if (nuevaCausa == NULL) {
            printf("No se pudo crear la nueva causa. Inténtelo más tarde\n");
            return;
        }
        //mientras tanto, ya que todavía falta pedir los demás datos
        nuevaCausa->carpeta = NULL;
        //crear memoria para la nueva denuncia, primero el nodo
        nuevaDenuncia = malloc(sizeof(struct nodoDenuncia));
        //despues la estructura de la denuncia (denunciante , .... , ...)
        nuevaDenuncia->denuncia = malloc(sizeof(struct Denuncia));
        nuevaDenuncia->siguiente = NULL;
        //primera denuncia sera la nueva denuncia
        nuevaCausa->denuncias = nuevaDenuncia;
        //Agregar la información de la nueva causa pero del struct Causa
        printf("Ingrese el nombre del denunciante: ");
        scanf(" %[^\n]",nuevaDenuncia->denuncia->denunciante);
        //ingresar el RUC de la causa, pero validandolo y haciendo un ciclo hasta que se cumpla
        do {
            printf("Ingrese RUC de la causa (formato 123456789-2025): ");
            scanf(" %[^\n]", nuevaCausa->ruc);
            //mientras el ruc sea distinto a lo que se pide, se le pide nuevamente
            if (!validarRUC(nuevaCausa->ruc)) {
                printf("Formato inválido (Ej: 123456789-2025)\n");
            }
        }while (!validarRUC(nuevaCausa->ruc));
        do {
            printf("Ingrese fecha (formato DD/MM/AA): ");
            scanf(" %[^\n]", nuevaDenuncia->denuncia->fecha);
            if (!validarFecha(nuevaDenuncia->denuncia->fecha)) {
                printf("Formato inválido. Intente nuevamente.\n");
            }
        } while (!validarFecha(nuevaDenuncia->denuncia->fecha));
        printf("Ingrese descripción: ");
        scanf(" %[^\n]",nuevaDenuncia->denuncia->descripcion);
        printf("Cree clave de acceso: ");
        scanf(" %d",&nuevaCausa->claveDeAcceso);
        printf("Ingrese el estado de la causa (ingrese el número correspondiente.): ");
        opcionesEstado(nuevaCausa->estado);
        printf("Ingrese la decisión tomada: ");
        scanf(" %[^\n]",nuevaCausa->decisiones);
        //agregar la causa al árbol
        inicio = agregarCausa(inicio, nuevaCausa);
        printf("Causa agregada correctamente \n");
        //ahora consultar si desea agregar algo dentro de la carpeta investigativa
        printf("¿Desea agregar algún documento en la carpeta investigativa? (Ingrese el número correspondiente) \n");
        printf("1. Sí \n");
        printf("2. No \n");
        scanf(" %d",&opcionAccionNuevaCarpetaInvestigativa);
        if(opcionAccionNuevaCarpetaInvestigativa == 1){
            //crear carpeta investigativa
            printf("¿Qué desea agregar? (Ingrese el número correspondiente)\n");
            printf("1.Declaraciones\n");
            printf("2.Pruebas\n");
            printf("3.Diligencias\n");
            printf("4.Resoluciones\n");
            printf("5.Imputado\n");
            //scanf la opción de la acción que desea realizar
            scanf(" %d",&carpetaInvestigativaOpcionesNuevaCausa);
            //crear función para agregar declaraciones, pruebas, diligencias, resoluciones e imputados
            agregarAcarpetainvestigativa(nuevaCausa, carpetaInvestigativaOpcionesNuevaCausa);
            printf("¿Desea realizar alguna otra acción? (Ingrese el número correspondiente) \n");
            printf("1. Sí \n");
            printf("2. No \n");
            scanf(" %d",&accionFinal);
            if(accionFinal == 1){
                //volver al main
                return;
            }
            if(accionFinal == 2){
                //salir del programa
                exit(0);
            }
        }
    }
}

int main(){
    int accion;

    while (1) {
        printf("¿Qué desea realizar? (Introduzca el número correspondiente) \n");
        printf("1. Agregar o actualizar causa \n");
        printf("2. Consultar causa\n");
        printf("3. Salir\n");
        printf("4. Contar causas abiertas\n");
        printf("5. Contar sentencias condenatorias\n");
        printf("6. Contar archivos provisionales\n");
        printf("7. Contar medidas de protección activas\n");
        scanf("%d",&accion);

        if(accion == 1){
            ingresarNuevaCausaoDenuncia();
        }
        else if(accion == 2){
            consultarCausaExistente(inicio);
        }
        else if(accion == 3){
            break;
        }
        else if(accion == 4){
            int total = contarCausasAbiertas(inicio);
             printf("Hay %d causas abiertas actualmente.\n", total);
        }
        else if(accion == 5){
            int total = contarSentenciasCondenatorias(inicio);
            printf("Hay %d sentencia(s) condenatoria(s) registradas.\n", total);
        }   
        else if (accion == 6) {
            int total = contarArchivosProvisionales(inicio);
            printf("Hay %d archivo(s) provisionales registrados.\n", total);
        }
        else if (accion == 7) {
            int total = contarMedidasProteccionActivas(inicio);
            printf("Hay %d causa(s) con medidas de protección activas.\n", total);
        }  
        else {
            printf("Opción no válida\n");
        } 
    }
    return 0;
}