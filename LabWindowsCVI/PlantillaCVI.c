/********************************************************************************************
	FICHERO:  PlantillaCVI.c                                                    
	
	Aplicación: Plantilla para controlar instrumentación virtual mediante comandos SCPI 
	
	Por Patricio Coronado Collado.17/07/2017.


	Para comunicar con el puerto COM se utilizan funciones y variables exportadas de 
	PuertoCom.h
*********************************************************************************************/
																								
/*********************************************************************************************
	Copyright © 2017 Patricio Coronado
	
	This file is part of PlantillaCVI 

    PlantillaCVI is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PlantillaCVI is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PlantillaCVI  If not, see <http://www.gnu.org/licenses/>

*********************************************************************************************/	
/*
	Esta cadena es la respuesta que tiene que dar el sistema que queremos controlar
	al recibir un comando SCPI *IDN.
	Obligatorio rellenar la cadena IdentidadDelSistema que debe conincidir con 
	NOMBRE_DEL_SISTEMA_64B(Plantilla para SAM_D21) en la aplicación de Arduino
*/
//char IdentidadDelSistema[] ="Plantilla para SAM_D21";   
char IdentidadDelSistema[] ="Test de la libreria segainvex_scpi_ESP8266 para Arduino.";
//------------------------------------------------------------------------------------------
// Ficheros include                                                          
//------------------------------------------------------------------------------------------
#include <ansi_c.h>
#include <cvirte.h>
#include <userint.h>
#include <rs232.h>
#include <utility.h>
#include <formatio.h>
#include <string.h>
#include <analysis.h>
#include "PlantillaCVI.h"
#include "PuertoCOM.h"
#include "PanelesPuertoCom.h"
//------------------------------------------------------------------------------------------
// Constantes
//------------------------------------------------------------------------------------------
#define SI 1	  
#define NO 0
// Coordenadas de los paneles
#define PP_TOP  60	   // Panel Principal
#define PP_LEFT 60	
// Colección de posibles valores de la variable DatoEsperado (dato esperado por el puerto COM)
#define NINGUNO 0
#define DATOS_RECIBIDOS 2
//-----------------------------------------------------------------------------------------
// Variables globales 
extern int Depuracion; // Variable para poner el programa en modo depuración
char Comando[STRMEDIO]; //Cadena para poner un comando antes de copiarlo en CadenaComando[]
char SParametro1[STRCORTO];	// Cadena para poner un parámetros en los comandos
int Parametro1; //Valor del parámetro 1
int PrincipalHandle; //Handler del panel principal
int DatoEsperado;/*Se utiliza para decir al PC que datos se esperan del sistema 
ver la función pcom_datos_recibido() en este módulo */
enum  {Test,Normal} Modo=Test; //Para poner el sistema en modo test o en modo de funcionamiento normal
//----------------------------------------------------------------------------------------
// Prototipo de funciones propias                                            
//----------------------------------------------------------------------------------------
void activa_controles_principal_para_comunicar(int short);// Dimar y desdimar controles
/****************************************************************************************
		                   FUNCION PRINCIPAL                                     
****************************************************************************************/
 int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	// Carga el panel principal como top-level
	if ((PrincipalHandle = LoadPanel (0, "PlantillaCVI.uir", PRINCIPAL)) < 0)
		return -1;
	// Carga el panel de configuración del puerto como top-level
	pcom_carga_panel_de_configuracion();
	// Abre el puerto serie
	if(!pcom_abre_puerto_serie_automaticamente())
	{
		if(ConfirmPopup ("Error de puerto COM", "No se ha podido abrir ningún puerto COM\n\n"
			"¿  Quieres intentarlo manualmente?"))
			//DisplayPanel (ConfigHandle); // Pone el panel de configuración en memoria
			pcom_instala_el_panel_de_configuracion(PANEL_MODO_HIJO,PP_TOP+25,PP_LEFT+25);
			//InstallPopup (ConfigHandle);
	}
	//Además de cargar el panel de configuración del puerto hay que cargar el de mensajes
	pcom_carga_panel_de_mensajes();
	// Muestra el panel principal
	DisplayPanel (PrincipalHandle);
	// Mira en el panel de comunicación si el sistema está o no por defecto en modo depuración
	Depuracion=pcom_estado_radio_button_de_depuracion();
	if(Depuracion) pcom_muestra_el_panel_de_mensajes(PP_TOP,PP_LEFT+530);// En modo depuración muestra el panel de mensajes		
	// Coloca el panel principal en una posición determinada en la pantalla
	SetPanelAttribute (PrincipalHandle, ATTR_TOP,PP_TOP );
	SetPanelAttribute (PrincipalHandle, ATTR_LEFT,PP_LEFT);
	SetCtrlVal(PrincipalHandle,PRINCIPAL_X_POS,PP_TOP); // Muestra la posición
	SetCtrlVal(PrincipalHandle,PRINCIPAL_Y_POS,PP_LEFT);

	// Si el puerto está abierto activa los  controles que lo utilizan
	if(pcom_test_puerto_abierto()==0) 
	{
		activa_controles_principal_para_comunicar(SI);
		pcom_activa_controles_mensajes_para_comunicar(SI);		
		pcom_limpia_pila_errores(); // Limpia la pila de errores SCPI del micro
	}
	// Si el puerto no está abierto desactiva los controles que comunican
	else 
	{
		activa_controles_principal_para_comunicar(NO);
		pcom_activa_controles_mensajes_para_comunicar(NO);
	}
		// Corre la aplicación........................................................
		RunUserInterface (); 
	// Aqui hay que descargar todos lo paneles utilizados
	pcom_descarga_paneles_del_puerto_com();
	DiscardPanel (PrincipalHandle);
	return 0;
}
/****************************************************************************************
					FUNCIONES DEL PANEL PRINCIPAL
 	Se utiliza para salir de la aplicación.Cierra el puerto y sale de la aplicación.
****************************************************************************************/
int CVICALLBACK panel_principal (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	Rect CoordenadasPanel;
	//Rect CoordenadasPanelFijas={1,2,3,4};
	
	switch (event)
	{
		case EVENT_PANEL_MOVING:
		GetPanelEventRect (eventData2, &CoordenadasPanel/*Rect *rectangle*/);
		SetCtrlVal(PrincipalHandle,PRINCIPAL_X_POS,CoordenadasPanel.top);
		SetCtrlVal(PrincipalHandle,PRINCIPAL_Y_POS,CoordenadasPanel.left);
		break;		
		case EVENT_GOT_FOCUS:// || EVENT_LOST_FOCUS:
			if(pcom_test_puerto_abierto()==0) 
			{
				activa_controles_principal_para_comunicar(SI);
				SetPanelAttribute (PrincipalHandle, ATTR_TITLE,IDNinstrumento);
			}
		// Si el puerto no está abierto desactiva los controles que comunican
		else 
			{
				activa_controles_principal_para_comunicar(NO);
				SetPanelAttribute (PrincipalHandle, ATTR_TITLE,"Instrumento sin identificar"); 
			}
		break;
		//case EVENT_LOST_FOCUS:
		//break;
		case EVENT_CLOSE:
			pcom_cierra_puerto_serie();
			QuitUserInterface (0);
		break;
		case EVENT_RIGHT_CLICK:
			
		break;
	}
	return 0;
}
/****************************************************************************************
		FUNCION QUE CARGA EL PANEL DE CONFIGURACION

****************************************************************************************/
void CVICALLBACK Configurar (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	/* Carga el panel de configuración como child del Principal*/
    //pcom_instala_el_panel_de_configuracion(PANEL_MODO_HIJO,PP_TOP+25,PP_LEFT+25);
	pcom_instala_el_panel_de_configuracion(PANEL_MODO_TOP,PP_TOP+25,PP_LEFT+25);
}
/****************************************************************************************
			FUNCION QUE CARGA EL PANEL DE MENSAJES
			Para que se vean los mensajes de entrada salida por el puerto COM
****************************************************************************************/
void CVICALLBACK comunicacion_menu (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	pcom_muestra_el_panel_de_mensajes(PP_TOP,PP_LEFT+530);
	
}
/****************************************************************************************
		FUNCION DE MENU PARA MOSTRAR INFORMACION DEL AUTOR
****************************************************************************************/
void CVICALLBACK About (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	int LongMensaje;
	MessagePopup 
	   ("   Autor: ", "         Patricio Coronado Collado\n"
		"        UAM-SEGAINVEX Electrónica\n"
		"        patricio.coronado@uam.es\n"
		"Este software no forma parte de ningún proyecto\n"
		"realizado por la UAM SEGAINVEX-Electrónica.\n"
		"Su único fin es para test\n"
		);
}
/*****************************************************************************************

						PONER AQUÍ EL CÓDIGO DE CADA APLICACION		

******************************************************************************************/




/****************************************************************************************
		FUNCION PARA DIMAR O DESDIMAR LOS CONTROLES QUE COMUNICAN
		Si "Accion" vale 1 desdima los controles y se pueden usar 
		Si "Acción" vale 0 dima los controles y no se pueden usar 
*****************************************************************************************/
void activa_controles_principal_para_comunicar( int short Accion)
{

	if(Accion)  // Desbloquea/activa los controles
	{
	//	SetCtrlAttribute (PrincipalHandle,PRINCIPAL_CONTROL, ATTR_DIMMED, 0);
		
	}
	else   // Bloquea/Desactiva los controles
	{
	//	SetCtrlAttribute (PrincipalHandle,PRINCIPAL_CONTROL, ATTR_DIMMED, 1);
				
	}
}
/****************************************************************************************
 FUNCION DEL PUERTO COM QUE SE DEFINE AQUÍ PARA QUE HAGAS LO QUE QUIERAS CUANDO RECIBAS
 UN DATO DE FORMA AUTOMÁTICA.
 mirar en"CadenaRespuesta" que es donde están los datos recibidos
 
*****************************************************************************************/
void pcom_datos_recibido(void)
{
	unsigned int RDACRecibido;
	int Posicion,Muestra,MuestrasLeidas;
	double Medidas[257];
	switch(DatoEsperado)
	{	
		case DATOS_RECIBIDOS:
			
		break;	
	}
}
/****************************************************************************************
 								PIDE AL SISTEMA QUE SE IDENTIFIQUE
*****************************************************************************************/
int CVICALLBACK identifica_sistema (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	
	char DatosRecibidos[256]; //Cadena para recibir la firma del sistema   
	
	if(event==EVENT_COMMIT)       
	{
		pcom_recepcion_automatica(NO); //Evita la recepción automática	
		//
		sprintf(CadenaComando,"%s","*IDN"); //Pone el comando en la cadena de salida
		ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR) //Envia el comando en la cadena de salida
		Delay(0.3);	
		pcom_recibir_datos (DatosRecibidos);//Función definida en el módulo PuertoCOM.h  
		SetCtrlVal(panel,PRINCIPAL_SISTEMA,DatosRecibidos);	 
		
		pcom_recepcion_automatica(SI); //Habilita la recepción automática	
	}
	return 0;
}
/****************************************************************************************
 							ENVIA UN COMANDO AL SISTEMA
*****************************************************************************************/

int CVICALLBACK funcion1 (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			  sprintf(CadenaComando,"%s","c1:c11"); //
				ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR) //	
			break;
	}
	return 0;
}

//*****************************************  FIN  **************************************// 
