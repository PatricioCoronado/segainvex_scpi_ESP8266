#ifndef __PuertoCOM_H__
#define __PuertoCOM_H__
/*

 Fichero: PuertoCom.h
 Próposito: Implementa variables y funciones para manejar fácilmente la conexión      
 con un puerto COM.
 
 Para utilizar este módulo hay que incluir PuertoCOM.h en la compilación
 además de estar en el proyecto los módulos listaPaco.c y listaPaco.h


 V1.1 Incluye guardar el setup del puerto com en un fichero del que lee
 los datos para abrir el puerto.
 
 Para abrir el puerto lo habitual es ejecutar pcom_abre_puerto_automaticamente.
 Esta función abre los puertos existentes con el setup del panel configuracion.
 Si alguno le devuelve la identificación correcta IDN_DEL_SISTEMA, lo abre y 
 guarda el setup en un fichero. Así cuando se vuelva a ejecutar, abre directamente
 con los datos del fichero.
 
 Si hay problemas se habre el panel de "configuración" y con "busca puertos", se crea
 una lista de puertos que han abierto con el setup del panel "configuracion". Los 
 puertos se guadan en el "edit ring" de "busca puertos". hay que seleccionar uno.
 En el "sistema en puerto COM seleccionado" aparece el IDN_DEL_SISTEMA que está 
 en ese puerto.
 */
/*
	Copyright © 2017 Patricio Coronado
	
	This file is part of Caracterización de PROYECTO.

    PRY is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PRY is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CMM.  If not, see <http://www.gnu.org/licenses/>
*/
//==============================================================================
// ÚNICAS VARIABLES QUE HAY QUE MODIFICAR EN FUNCIÓN DEL SISTEMA
// Para hacer una recepción automática, hay que definir un carácter terminado de 
// las cadenas que envia el instrumento conectado. ese carácter se define aquí
//#define TERMINADOR ';' 
extern char Terminador;
// Para hacer apertura automática del puerto, el programa compara la cadena 
// declarada aquí con la que reciba del instrumento conectado al enviarle un "*IDN"

/*
#ifndef IDN_DEL_SISTEMA  
	#define IDN_DEL_SISTEMA char IdentidadDelSistema[]="Plantilla para SAM_D21";
#endif  
*/
//==============================================================================
// Types
//==============================================================================
// Constantes
#define STRLARGO 2048 // Longitud de una cadena larga	
#define STRCORTO 64 // Longitud de una cadena corto	
#define STRMEDIO 256// Longitud de una cadena media
#define SI 1
#define NO 0
#define PM_TOP  25		// Coordenadas para posicionar el panel de Mensajes
#define PM_LEFT 500
#define MOSTRAR 1
#define NO_MOSTRAR 0
//==============================================================================
// External variables: Declaración de variables visibles desde otros ficheros
extern char IdentidadDelSistema[];  //Cadena que se rellena el el programa principal con el nombre del sistema
extern char CadenaComando[STRMEDIO];   // Aquí se pone el comando a enviar al instrumento
extern char CadenaRespuesta[STRLARGO]; // Aquí esta la respuesta del instrumento
extern int DatosEnCadenaRespuesta;   // Flag para saber si han llegado datos
extern double Retardo;		// Tiempo de retardo entre envío y recepción
extern char IDNinstrumento[STRMEDIO];// Cadena con la identificació que ha mandado el sistema  
extern int Depuracion; // Variable para poner el programa en modo depuración (1 depuración, 0 normal)     
//==============================================================================
// MACROS			   
// Envía un comando err? al sistema sin mostrarlo en el control que muestra las cadenas de salida
#define	PEDIR_ERROR_AL_SISTEMA(MOSTRAR_O_NO) sprintf(CadenaComando,"%s","ERR?");\
								if(!pcom_test_puerto_abierto())	{ \
					pcom_enviar_datos(CadenaComando,MOSTRAR_O_NO);  \
					Delay (Retardo);}
//Envía por el puerto serie lo que se haya puesto en el array de char "CadenaComano"
//sin mostrarlo en el control que muestra las cadenas de salida 
#define ENVIAR_COMANDO_AL_SISTEMA(MOSTRAR_O_NO) if(!pcom_test_puerto_abierto()){ \
				pcom_enviar_datos(CadenaComando,MOSTRAR_O_NO);		   \
				Delay (Retardo);								   \
			}
//==============================================================================
// Funciones públicas
extern void pcom_datos_recibido(void);// Función virtual, a desarrollar en el programa principal
int pcom_recibir_datos (char[]);// Recibe por el puerto serie la cadena pasada como argumento
int pcom_enviar_datos (char[],int); // Envia por el puerto serie la cadena pasada como argumento
int pcom_test_puerto_abierto(void);// Devuelve 0 si el puerto estáabierto y -3 si está cerrado
void pcom_cierra_puerto_serie(void);//Cierra el puerto serie
int pcom_abre_puerto_serie(void);  // Abre el puerto serie
void pcom_limpia_pila_errores(void); //Limpia la pila de errores SCPI
void pcom_activa_controles_mensajes_para_comunicar(int short);// Activa o desactiva controles que comunican
// 	Busca puertos, los abre con los parámetros del instrumento a controlar,
// 	lee la identificación del instrumento y la compara con la que tiene
// 	en la cadena "IdentidadDelSistema". Si son la misma abre el puerto. 
//  Devuelve el número del COM abierto y 0 si no abrió ningún puerto.
int pcom_abre_puerto_serie_automaticamente(void);
// Funciones para usar el panel de mensajes
int pcom_carga_panel_de_mensajes(void);
int pcom_muestra_el_panel_de_mensajes(int,int);//Los parámetros son las coordenadas top y left
int pcom_estado_radio_button_de_depuracion(void);
int pcom_carga_panel_de_configuracion(void);
void pcom_descarga_paneles_del_puerto_com(void);
void pcom_instala_el_panel_de_configuracion(int/*PANEL_MODO_HIJO o PANEL_MODO_TOP*/,int,int);
#define PANEL_MODO_HIJO 0//Para instalar el panel de configuración del puerto modo hijo
#define PANEL_MODO_TOP 1 //Para instalar el panel de configuración del puerto modo top
//Los otros dos argumentos son las coordenas top y left para posicionar el panel
void pcom_recepcion_automatica(int);
/*
	Muestra la cadena de caracteres pasada como parámetro en el text box 
	CADENA_RECIBIDA. Si el segun parámetro es 1, además incluye un retorno de carro 
*/
void pcom_mostrar_datos_en_textbox_cadena_recibida (char[],int);
//==============================================================================            	
#endif  /* ndef __PuertoCOM_H__ */  
