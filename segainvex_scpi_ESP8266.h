/*******************************************************************************
	DEPARTAMENTO DE ELECTRÓNICA DE SEGAINVEX. UNIVERSIDA AUTONOMA DE MADRID				
				LIBRERIA PARA ARDUINO Segainvex_scpi_ESP8266 V1.0
 SISTEMA PARA COMUNICAR UNA COMPUTADORA CON ARDUINO MEDIANTE PUERTO SERIE 
 Fichero de cabecera segainvex_scpi_ESP8266.h
/******************************************************************************
/*
	Copyright © 2017 Mariano Cuenca, Patricio Coronado
	
	This file is part of segainvex_scpi_ESP8266.

    segainvex_scpi_ESP8266 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    segainvex_scpi_ESP8266 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with segainvex_scpi_ESP8266.  If not, see <http://www.gnu.org/licenses/>.

*/
/* 
		Mejoras futuras:
		1)Poner el menú de segainvex_scpi_ESP8266 en flash para no consumir RAM
		2)Mejorar la función int cambia_variable_double_del_sistema(double *,double,double);
 */
  /***********************************************************************************
  DESDE AQUÍ ESTE FICHERO NO DEBE SER EDITADO. ES EL MISMO EN TODAS LAS APLICACIONES
 ***********************************************************************************/
#ifndef SEGAINVEX_SCPI_ESP8266_H_INCLUDED
#define SEGAINVEX_SCPI_ESP8266_H_INCLUDED
//
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
/*******************************************************************************
							CONSTANTES EXPORTADAS
Hay que tener en cuenta que el buffer de puerto serie de Arduino es 64 bytes							
Por lo que BUFFCOM_SIZE y LONG_SCPI no pueden ser mayores de 64 bytes.
Estas constantes pueden ser redefinidas.
********************************************************************************/
#define BUFFCOM_SIZE 32 //Longitud del buffer de lectura de de segainvex_scpi_ESP8266
#define LONG_SCPI 32 // Longitud máxima del comando mas largo sin contar parámetros
#define MAXERR 3   // Profundidad de la pila de errores
/*******************************************************************************
						DEFINICION DE TIPOS EXPORTADOS
*******************************************************************************/            
//TIPOS PARA LA ESTRUCTURA DEL MENU DE SEGAINVEX-SCPI
// Declaración de tipo estructura para menús de comandos SEGAINVEX-SCPI
typedef struct tipoNivel
{             
  int   NumNivInf;       // Número de niveles por debajo de este
  char *largo, *corto;   // Nombre largo y nombre corto del comando
  unsigned char handler; // Número de función a llamar con este comando 
  tipoNivel *sub;     // Puntero a la estructura de niveles inferiores
};
// TIPO PARA EL ARRAY DE ERRORES
typedef char *tipoCodigoError[];
/********************************************************************************
      PROTOTIPOS DE FUNCIONES DE SEGAINVEX-SCPI_ESP8266 QUE TIENEN QUE SER VISIBLES

*********************************************************************************/
/********************************************************************************
      EjecutaComandos() hay que definirla el el código de la aplicación
	  porque tiene que ejecutar las funciones de cada comando que tienen
	  que conocer el sistema y sus variables
*********************************************************************************/
extern void EjecutaComandos(unsigned char);
/*********************************************************************************
	Funcion que gestiona la pila de errores
	El parámetro de entrada es el número del error a poner el la pila de errores
**********************************************************************************/
extern void errorscpi(int); // 
/*********************************************************************************
	Funcion principal de SEGAINVEX-SCPI
	Se debe ejecutar cuando se detecten datos en el puerto serie
**********************************************************************************/
extern void scpi(void);     
/**********************************************************************************
 FUNCIONES AUXILIARES, PARA LEER UN ARGUMENTO ENTERO QUE SE PASA CON EL COMANDO,
 CUYO FIN ES ACTUALIZAR EL VALOR DE UNA VARIABLE ENTERA O BOOLENAN DEL SISTEMA.
 SI EL COMANDO VA SEGUIDO DE ? LO QUE HACE LA FUNCIÓN ES DEVOLVER EL VALOR DE 
 LA VARIABLE.
 Leen el parámetro del array de char FinComando[]. El carácter FinComando[0] se 
 espera que sea un espacio. A continuación tiene que estar el parámetro a leer.
*********************************************************************************/
/*******************************************************************************
	Cambia la variable entera cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable entera, el valor máximo, el valor mínimo.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable.
*******************************************************************************/
extern int cambia_variable_int_del_sistema(int *,int,int);//
/*******************************************************************************
	Cambia la variable entera discreta cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable entera,array de entero con los valores posibles,
	tamaño del array.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable.
*******************************************************************************/
extern int cambia_variable_int_discreta_del_sistema(int *,int*,int);//
/*******************************************************************************
	Cambia la variable booleana cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable entera, el valor máximo, el valor mínimo y el
	número de error a anotar si la variable está fuera de rango.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable.
********************************************************************************/	
extern int cambia_variable_bool_del_sistema(bool *);
/*******************************************************************************
	Cambia la variable double cuya dirección se pasa como argumento.
	Los argumentos de entrada son:
	La dirección de la variable float, el valor máximo, el valor mínimo.
	Devuelve 1 si cambió la variable. 0, si no cambió la variable por errores.
	Y 2 si devolvió el valor de la variable. 
*******************************************************************************/
extern int cambia_variable_double_del_sistema(double *,double,double);
/********************************************************************************
   VARIABLES GLOBALES EXPORTADAS DE SEGAINVEX-SCPI QUE TIENEN QUE SER VISIBLES
  Son globales todos los modulos con el include a segainvex_scpi_ESP8266.h  						
********************************************************************************/
//Esta variable tiene que estar definida en el código del usuario.
extern tipoNivel Raiz[]; //Array de la estructura raiz de comandos
//Estas variable tiene que estar definida en el código del usuario.
extern tipoCodigoError CodigoError;//Puntero al array de cadenas explicativas del error
// FinComando es un array definido en seginvex_scpi.c de char que contiene los 
//parámetros que envia el PC tras el comando.
extern char *FinComando;// Puntero al final del comando para leer parámetros
//Cadena donde el usuario debe escribir el nombre de su sistema. La cadena se 
// envía al PC cuando este envíe a Arduino el comando *IDN
extern char IdentificacionDelSistema[];
/********************************************************************************
									MACROS EXPORTADAS 
*********************************************************************************/      
/*
// Para definir submenús 
#define SCPI_SUBMENU(X,Y) sizeof(X)/sizeof(*X), #X,#Y,NULL,X,  
// Para definir comandos
#define SCPI_COMANDO(X,Y,Z) 0, #X,#Y,Z,NULL, //Para definir comandos 
//Para definir el nivel raiz de comandos
#define SCPI_NIVEL_RAIZ tipoNivel Raiz[]={sizeof(NivelDos)/sizeof(*NivelDos),"","",NULL,NivelDos};
*/
//Macro que pone en el array "IdentificacionDelSistema" el nombre del sistema que implementa Arduino
#define NOMBRE_DEL_SISTEMA_64B(X) strcpy(IdentificacionDelSistema,#X);
/*********************************************************************************
 EDITABLE
 PROTOTIPOS DE FUNCIONES DEL PROGRAMA PRINCIPAL LLAMADAS POR SEGAINVEX-SCPI
 Añadir tantas funciones como se necesiten.
 *********************************************************************************/
extern void comando1(void);
extern void comando2(void);
extern void comando3(void);
extern void comando4(void);
extern void comando5(void);
extern void comando6(void);
extern void comando7(void);
extern void comando8(void);
extern void comando9(void);
extern void comando10(void);
extern void comando11(void);
extern void comando12(void);
extern void comando13(void);
extern void comando14(void);
extern void comando15(void);
extern void comando16(void);
extern void comando17(void);
extern void comando18(void);
extern void comando19(void);
extern void comando20(void);
extern void comando21(void);
extern void comando22(void);
extern void comando23(void);
extern void comando24(void);
extern void comando25(void);
extern void comando26(void);
extern void comando27(void);
extern void comando28(void);
extern void comando29(void);
extern void comando30(void);
extern void comando31(void);
extern void comando32(void);
extern void comando33(void);
extern void comando34(void);
extern void comando35(void);
extern void comando36(void);
extern void comando37(void);
extern void comando38(void);
extern void comando39(void);
extern void comando40(void);
extern void comando41(void);
extern void comando42(void);
extern void comando43(void);
extern void comando44(void);
extern void comando45(void);
extern void comando46(void);
extern void comando47(void);
extern void comando48(void);
extern void comando49(void);
extern void comando50(void);
extern void comando51(void);
extern void comando52(void);
extern void comando53(void);
extern void comando54(void);
extern void comando55(void);
extern void comando56(void);
extern void comando57(void);
extern void comando58(void);
extern void comando59(void);
extern void comando60(void);
extern void cls(void);
extern void err(void);
extern void opc(void);
extern void idn(void);
/**************************************************************************/
/*
NOTAS:
1)  El buffer de recepción de Arduino es de 64 bytes, por lo que hay que dimensionar
	la cadena de recepción como mucho a 64 bytes. BUFFCOM_SIZE=<64 
2)  Si se lee el puerto serie desde Arduino con 
	Serial.readBytesUntil('r',buffer,tamaño_buffer);
	hay que	asegurarse de que el terminador con que el PC cierre las cadenas a enviar
	sea '\r'.
3)	Si Arduino lee con Serial.readBytesUntil('\r',buffer,tamaño_buffer);.
	Esta función no mete	el terminador '\r' en la cadena que devuelve, por lo que 
	hay que ponerselo, ya que segainvex_scpi_ESP8266 necesita un terminador para dar por 
	buena una cadena. Al hacerlo hay que incrementar la variable que indica
	la longitud del comando "locCom" en 1. 
						
  Se ha definido un tipo de estructura denominado "tipoNivel". Las variables de este 
  tipo puede contener un nivel de comandos o un comando.
  
  Si contiene un nivel, el formato es:
  
  int número_de_comandos_del_nivel,
  char[] Nombre_largo_del_nivel,  
  char[] Nombre_corto_del_nivel,
  (*tpf)() NULL;                    //puntero a función   
  tipoNivel* Nombre_del_nivel,		//puntero a tipoNivel
  
  Y si contiene un comando, el formato es:
  
  int 0,
  char[] Nombre_largo_del_comando,
  char[] Nombre_corto_del_comando,
  (*tpf)() funcion_a_ejecutar,       //puntero a función
  tipoNivel* NULL,					//puntero a tipoNivel  
  
  Como se vé la diferencia es que si es un nivel, el puntero a función está a NULL, ya que
  al no ser un comando, no ejecuta ninguna función. Si es un comando, su puntero a función
  si que apunta a la función que se quiere ejecutar con ese comando. Pero su puntero a
  "tipoNivel" está a NULL, ya que de el no cuelga un nivel, porque es solo un comando.
  
  Se declaran array de variables tipoNivel.Siempre ha de existir este array:
  
  tipoNivel Raiz[] = {sizeof(NivelDos)/(5*sizeof(int)),"","",NULL, NivelDos};
	  
  Que tiene una única estructura "tipoNivel" y siempre el mismo formanto. 
  Es el nivel raiz del menú de submenús y comandos.
  
  Otro array que debe existir siempre es el denominado "NivelDos" (como se ve, 
  último miembro de la única estructura del "Raiz") 
	  
  El formato del array de estructuras "tipoNivel" "NivelDos" es:
  tipoNivel NivelDos[] ={
	  estructura_nivel_1,estructura_nivel_2,...,estructura_nivel_n,
	  estructura_comandos_1,estructura_comandos_2,..,estructura_comandos_n,
  };
  Cada nivel_estructura_i y nivel_comando_i tiene los formatos vistos arriba.
  
      El formato de cada array con un nivel de comandos es:
	tipoNivel NOMBRE_SUBNIVEL1[]={ 
		estructura_comandos_1,estructura_comandos_2,..,estructura_comandos_n,
  };
  
  Hay que añadir tantos arrays  "tipoNivel" como se necesiten. Y a su vez, en los
  arrays incluir tantos comandos como se necesiten. 
  
  Todo esto se puede declara en un fichero .h o dentro del .ino, lo que resulte
  más apropiado en cada caso.
*/
/**************************************************************************/
#endif // SEGAINVEX_SCPI_ESP8266_H_INCLUDED
/***********************************FIN************************************/
