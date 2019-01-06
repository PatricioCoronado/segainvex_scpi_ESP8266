/*

	Fichero:listaPaco.h
	SISTEMA PARA CREAR Y MANEJAR UNA LISTA DOBLEMENTE ENLAZADA
	TIPO listaPaco, CON NODOS TIPO nodoStructPaco. EL CONTENIDO
	DE LA ESTRUCTURA DEL NODO SE PUEDE MODIFICAR SEGUN TU
	APLICACION
	POR: Patricio Coronado Collado
	Enero de 2016
*/
/*
	Copyright � 2017 Patricio Coronado
	
	This file is part of Caracterizaci�n de Materiales Magnetorresistivos � CMM.

    CMM is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CMM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CMM.  If not, see <http://www.gnu.org/licenses/>
*/
/******************************************************************************
// EJEMPLO DE USO DE LA LIBRERIA. CREA UNA LISTA CON 10 NODOS Y LOS DESTRUYE

#include <stdio.h>
#include "listaPaco.h"
main()
{
	// Crea un puntero a nodo tipo nodoStructPaco
	nodoStructPaco *pNodo;
	// Crea una lista de nodos tipo nodoStructPaco
	listaPaco pMiLista;
	// Inicializa la lista
	inicializa_lista_paco(&pMiLista);
	do{
        // Inicializa un nodo
        inicializa_nodo_struct_paco( &pNodo);
        // Inserta el nodo creado
        inserta_nodo_lista_paco(&pMiLista,pNodo,DELANTE);
    }while(pNodo->indice!=10);
	// Para quitar la lista de la memoria
	elimina_lista_paco(&pMiLista);
    return 1;
}
*******************************************************************************/
/* Definicion de tipos para crear una lista genérica con 3 enteros y 3 cadenas */
#ifndef _structPaco_h
#define _structPaco_h
//#define NULL 0
//unsigned long int NumeroNodosStructPaco;
/********************************************************************
 Para declarar un nodo de la lista se usará el tipo "nodoStructPaco"
 Que contiene 3 enteros y tres cadenas. Este contenido se puede
 adaptar a cada aplicación
********************************************************************/
struct structPaco{ // estructura que soporta los datos del nodo
	int indice;
	int NumeroPuerto;
	//int entero2;
	int VelocidadPuerto;
	char NombrePuerto[32];
	char NombreInstrumento[128];
	//char stringLargo[128];
	struct structPaco* siguiente; // Puntero al tipo que los contiene para
	struct structPaco* anterior;  // manejar la lista.

};
typedef struct structPaco nodoStructPaco;
//............................................................
/********************************************************************
 Para declarar una lista, se utiliza el tipo "listaPaco"
 Que son dos punteros al tipo nodoStructPaco.
 Por ejemplo:
 lista Paco lista;
 lista.primer_nodo=NULL;
 lista.ultimo_nodo=NULL;
********************************************************************/
struct	lPaco
{
	struct structPaco* primer_nodo; // Puntero al primer y..
	struct structPaco* ultimo_nodo; // último nodo
};
typedef struct	lPaco listaPaco;
//..............................................................
enum lugarInsercionLP{DELANTE,DETRAS};
typedef enum lugarInsercionLP lugarInsercionListaPaco;
// Prototipos de funciones.
// Para manejar la lista se usarán las siguientes funciones:
/**********************************************************************
	Entra la dirección de una lista y la función pone los punteros
	primer_nodo y ultimo_nodo a NULL
	Ejemplo:
		listaPaco miLista;
		inicializa_lista_paco(&miLista);
	La lista existe en el ámbito en el que se crea y no es dinámica.
	solo son dinámicos los nodos.
***********************************************************************/
int inicializa_lista_paco(listaPaco *plp);
/***********************************************************************
	Inicializa un nodo nodoStructPaco. Pone los enteros a 0,
	vacia las cadenas y pone los punteros a NULL.
	El argumento de entrada es un puntero a un puntero tipo
	nodoStructPaco que apunta al nodo a inicializar
	Devuelve 1 si hay memoria y crea el nodo. Si no hay memoria
	para crear el nodo devuelve 0.
	Ejemplo:
	listaPaco miLista;
	nodoStructPaco *pNodo=NULL;
	inicializa_nodo_struct_paco(&pNodo);
	inserta_nodo_lista_paco(&miLista,pNodo ,DELANTE);
************************************************************************/
int inicializa_nodo_struct_paco(nodoStructPaco **pnlp);
//......................................................................
/************************************************************************
	Inserta un nodo en la lista tipo listaPaco.
	Segun el tercer	argumento se insertará al principio de la lista
	o al final de la misma.
	Los argumentos de entrada son la dirección de la lista tipo
	listaPaco, un puntero con la dirección del nuevo nodo y un
	tipo enumerado que decide si se inserta por delante o por detrás
	Devuelve 1 si inserta correctamente o 0 si no inserta el nodo.
	Ejemplo:
	listaPaco miLista;
	nodoStructPaco *pNodo=NULL;
	inicializa_nodo_struct_paco(&pNodo);
	inserta_nodo_lista_paco(&miLista,pNodo ,DELANTE);
*************************************************************************/
int inserta_nodo_lista_paco(listaPaco *,nodoStructPaco *,lugarInsercionListaPaco );
//..........................................................................
/************************************************************************
EXPLICACION: Entra la dirección de la lista y se va recorriendo de atrás
hacia delante eliminando nodos y devolviendo la memoria.
ARGUMENTOS DE ENTRADA: Dirección de una lista listaPaco
DEVUELVE: 1
************************************************************************/
int elimina_nodos_lista_paco(listaPaco *);
//........................................................................

/************************************************************************
EXPLICACION:Busca un valor n�merico en un miembro del nodo y cuando
lo encuentra copia una de sus cadenas miembro en la cadena que se le 
pasa por referencia.
ARGUMENTOS DE ENTRADA: El n�mero a buscar y la cadena a modificar
DEVUELVE:Modifica la cadena que se le pasa como segundo argumento y 
devuelve 1 si encontro el nodo con el valor numero y 0 si no.
************************************************************************/
int copia_cadena_de_lista_paco(listaPaco *pLista, int Numero, char *Cadena);
//........................................................................
/************************************************************************
EXPLICACION:
ARGUMENTOS DE ENTRADA:
DEVUELVE:
************************************************************************/

//........................................................................
#endif
