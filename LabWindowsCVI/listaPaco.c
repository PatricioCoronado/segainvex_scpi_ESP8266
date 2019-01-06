/*

	Fichero:listaPaco.c
	SISTEMA PARA CREAR Y MANEJAR UNA LISTA DOBLEMENTE ENLAZADA
	TIPO listaPaco, CON NODOS TIPO nodoStructPaco. EL CONTENIDO
	DE LA ESTRUCTURA DEL NODO SE PUEDE MODIFICAR SEGUN TU
	APLICACION
	POR: Patricio Coronado Collado
	Enero de 2016
*/
/*
	Copyright © 2017 Patricio Coronado
	
	This file is part of Caracterización de Materiales Magnetorresistivos ó CMM.

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

#include <userint.h>
#include <ansi_c.h>
//#include <malloc.h>
#include <stdio.h>
#include "listaPaco.h"

//#include "stdafx.h" // Depende del entorno de programación
/******************************************************************
					FUNCIONES PUBLICAS
******************************************************************/
static int CuentaNodosStructPaco=0;
/******************************************************************
					Pone los punteros a NULL
******************************************************************/
int inicializa_lista_paco(listaPaco *plp)
{
	plp->primer_nodo = NULL;
	plp->ultimo_nodo = NULL;
	return 1;
}
//......................................................................................
int inicializa_nodo_struct_paco(nodoStructPaco **pnlp)
{

    	// Pide memoria al sistema para una estructura ListaPaco
     CuentaNodosStructPaco++;
	*pnlp = (nodoStructPaco *)malloc(sizeof(nodoStructPaco));
	if (!(*pnlp))// Si el puntero sigue a NULL...
	{
		(*pnlp)=NULL;
		return 0; // Sale con fracaso. No hay memoria para el nodo
	}
	else // Si se asigna memoria correctamente...
	{	// se inicializan las variables del nodo.
		(*pnlp)->anterior = NULL;
		(*pnlp)->siguiente = NULL;
		(*pnlp)->indice=CuentaNodosStructPaco;
		return 1; // Crea el nodo con éxito
	}
}
//......................................................................................
int inserta_nodo_lista_paco(listaPaco *pl,nodoStructPaco *pn,lugarInsercionListaPaco li)
{
	listaPaco *pLista; // Puntero auxiliar tipo lista
	pLista = pl;   // Hacemos que apunte a la lista
	// En el caso particular de que el entrante sea el primer nodo, se hace lo mismo sea
	// insertar delante o detrás
	if (pLista->primer_nodo==NULL)	// Si la lista no tiene nodos, el actual es el primero...
		{
		pLista->primer_nodo=pn;		// tanto los punteros primer_nodo y ultimo_nodo, apuntan
		pLista->ultimo_nodo=pn;		// al nodo actual que es el primero y último.
		pn->anterior=NULL;			//Y los punteros del nuevo nodo a NULL
		pn->siguiente=NULL;
		//NumeroNodosStructPaco++;
		return 1; // Sale con éxito. Insertó el primer nodo
		}

	if(li==DELANTE)
	{
		pn->siguiente=pLista->primer_nodo;// El nuevo nodo apunta al que antes era el primero
		pLista->primer_nodo->anterior=pn; // El que era el primero apunta al nuevo con "anterior"
		pLista->primer_nodo=pn; // El nodo nuevo es apuntado por el puntero "primer_nodo" de la lista
		pn->anterior=NULL; // El puntero "anterior" del nuevo primer nodo a null.
		//NumeroNodosStructPaco++;
		return 1; // Sale con éxito. Insetó un nodo por delante
	}
	if(li==DETRAS)
	{
		pn->anterior=pLista->ultimo_nodo; // El nuevo nodo apunta al que era el último
		pLista->ultimo_nodo->siguiente=pn; // El que era el último nodo, apunta al recien llegado
		pLista->ultimo_nodo=pn; // ultimo_nodo es el recien llegado
		pn->siguiente=NULL; // El nuevo último nodo apunta a NULL
		//NumeroNodosStructPaco++;
		return 1; // Sale con éxito. Insertó un nodo por detrás
	}
	return 0; // Sale con fracaso
}
//......................................................................................
int elimina_nodos_lista_paco(listaPaco *pLista)
{
	nodoStructPaco *pNodo;// puntero para apuntar a los nodos a eliminar
	pNodo=pLista->ultimo_nodo; // Apuntamos al último nodo

	while(pNodo!=NULL)  // Si se ha eliminado ya el último nodo, o la lista estava vacía no hace más
	{
		pLista->ultimo_nodo=pNodo->anterior; // El que era penultimo nodo es ahora el último
		free(pNodo);
		//MessagePopup ("TEST", "elimina un nodo");
		//NumeroNodosStructPaco--;
		pNodo=pLista->ultimo_nodo; // el puntero que borra apunta al último nodo
	}
	CuentaNodosStructPaco=0;
	inicializa_lista_paco(pLista);
	return 1;
}
int copia_cadena_de_lista_paco(listaPaco *pLista, int Numero, char *Cadena)
{
	nodoStructPaco *pNodo;// puntero para apuntar a los nodos a eliminar
	pNodo=pLista->primer_nodo; // Apuntamos al primer

	while(pNodo!=NULL)  
	{
		if(pNodo->NumeroPuerto==Numero)
		{
			strcpy (Cadena, pNodo->NombreInstrumento);
			return 1;
		}
		pNodo=pNodo->siguiente; // Apunta al siguiente nodo
	}
	return 0;
}
//......................................................................................

/******************************************************************
					FUNCIONES PRIVADAS
******************************************************************/
