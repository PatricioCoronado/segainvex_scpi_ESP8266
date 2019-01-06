/*

 Fichero:PuertoCom.c
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
	
	This file is part of PROYECTO ó PRY.

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
// Include files
#include <userint.h>
#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include "PuertoCOM.h"
#include "ErroresRS232.h"
#include <rs232.h>
#include "listaPaco.h"
#include "PanelesPuertoCom.h" 
//----------------------------------------------------------------------------
//Constantes
//IDN_DEL_SISTEMA  // Declaración de la cadena de identificación del sistema. Ver PuertoCOM.h
#define  ERROR_DATOS_EXCESIVOS -9// Código no utilizado en la libreria RS-232
//----------------------------------------------------------------------------
//Variables privadas
int Depuracion=0; // Variable para poner el programa en estado de depuración
listaPaco sListaCOM; // Lista para anotar los puertos encontrados de puertos
static short PuertoCOM;
//Handlers
int MensajesHandle=0; // Manipuladores de los paneles que usa el puerto COM
int ConfigHandle=0;
int	estatusHandle=0;
int EsperaHandle=0;
int VentanitaHandle=0;
int ModoPanelConfiguracion;
int VentanitaAbierta; //Flag para indicar que la ventanita de enviar a mano está abierta
// Variables privadas del puerto serie
static int FlagPanelConfigRecienAbierto=1;// Para reconfigurar el panel cada vez que se abra  
static int VelocidadPuerto;
static int Paridad;
static int BitsDatos;
static int BitsDeStop;
static int ColaDeEntrada;  		// Establece la long.de la cola de recepción
static int ColaDeSalida; 		// Establece la long.de la cola de transmisión
static int ModoX;
static int ModoCTS;
static int ErrorRS232;      //Código de error del puerto serie 
static int FlagPuertoAbierto=0;  // Flag que indica si hay algún puerto abierto  
static int TerminadorTransmision; // Selección del terminador de transmisión 
static int TerminadorRecepcion; // Seleccion del terminador de recepción 
static int CaracterTerminador;		// Caracter a incluir en la cadena de emisión como terminador 
double Timeout;
double Retardo;
char NombrePuertoCOM[STRCORTO];  		// Nombre lógico del puerto COM1, COM2, etc. 
char ComMsg[STRCORTO];
char Msg1[STRCORTO];   // Cadenas de caracteres auxiliares
char Msg2[STRCORTO];
char Msg3[STRCORTO];
char IDNinstrumento[STRMEDIO];
// Retardo para la apertura del puerto. Para un Arduino AVR es mayor, 1 seg
// Para un DUE se puede dejar en 0.3
float	RetardoAperturaPuerto=3; 
 // Variables para manejar el fichero de datos del puerto COM
int ErrorFichero;
long Dummy;
FILE *ptrFichero;
static char NombreFichero[]="PuertoCOM.ini";
struct strPuertoCOM
{
	char IDNinstrumento[STRLARGO];
	char NombrePuertoCOM[STRCORTO];
	int  PuertoCOM;
	int  VelocidadPuerto;
	int  Paridad;
	int  BitsDatos;
	int  BitsDeStop;
	int  ColaDeEntrada;
	int  ColaDeSalida;
	int	 ModoX;
	int	 ModoCTS;
	int	 Timeout;
	int TerminadorTransmision; 
	int TerminadorRecepcion; 
};
//Estructura global con los datos del puerto que hay en fichero
struct strPuertoCOM filePuertoCOM;
//............................................................................ 
// Terminadores para recepción automática. El Serial.println() de Arduino funciona así:
// Prints data to the serial port as human-readable ASCII text followed by a carriage 
// return character (ASCII 13, or '\r' or CR) and a newline character (ASCII 10, or '\n' or LF). 
// This command takes the same forms as Serial.print().
// La cadena terminandola con "13 10" ó CR LF, por eso la recepción automática se 
// proboca con LF. Así la lectura quita el LF y deja el CR que me sirve para detectar
// el final de cadena.
char Terminador='\n';  //es LF ó 10
char Terminador2='\r'; //es CR ó 13
//==============================================================================
// Funciones privadas
void com_display_error_RS232 (void);
void com_establece_parametros (void);
void com_salva_parametros (void);
void com_muestra_mensaje_status (int);
void CVICALLBACK com_terminador_detectado_en_buffer_de_entrada (int portNo,int eventMask,void *callbackData); 
int	com_hacer_lista_de_puertos_del_sistema(void);
int pcom_lee_puerto_com_de_fichero(void);
int pcom_guarda_puerto_com_en_fichero(void);

//==============================================================================
// Variables globales
// Cadenas públicas de comunicación con el programa principal
char CadenaComando[STRMEDIO];   // Aquí se pone el comando a enviar al instrumento
char CadenaRespuesta[STRLARGO]; // Aquí esta la respuesta del instrumento
int DatosEnCadenaRespuesta=0;   // Flag para saber si se han recibido datos automáticamente
//==============================================================================       
// Definición de funciones
/************************************************************************************
 GUARDA EN EL FICHERO "configuracion.com" LOS DATOS DEL PUERTO QUE SE HA ABIERTO
 Y HA FUNCIONADO
 *************************************************************************************/
int pcom_guarda_puerto_com_en_fichero(void)
{
	ptrFichero=fopen(NombreFichero,"wb");// Crea el fichero
   	if (!ptrFichero){return 0;} //Si falla la apertura sale con 0
	//Si abre el fichero rellena la estructura con los datos del fichero
    strcpy(filePuertoCOM.IDNinstrumento,IDNinstrumento);
	strcpy(filePuertoCOM.NombrePuertoCOM,NombrePuertoCOM);
	filePuertoCOM.PuertoCOM=PuertoCOM;
	filePuertoCOM.VelocidadPuerto=VelocidadPuerto;
	filePuertoCOM.Paridad=Paridad;
	filePuertoCOM.BitsDatos=BitsDatos;
	filePuertoCOM.BitsDeStop=BitsDeStop;
	filePuertoCOM.ColaDeEntrada=ColaDeEntrada;
	filePuertoCOM.ColaDeSalida=ColaDeSalida;
	filePuertoCOM.ModoX=ModoX;
	filePuertoCOM.ModoCTS=ModoCTS;
	filePuertoCOM.Timeout=Timeout;
	filePuertoCOM.PuertoCOM=PuertoCOM;
	filePuertoCOM.TerminadorTransmision=TerminadorTransmision;
	filePuertoCOM.TerminadorRecepcion=TerminadorRecepcion; 
	// Si abre el fichero sin errores..guarda la estructura en el fichero.
	fwrite(&filePuertoCOM,sizeof(struct strPuertoCOM),1,ptrFichero);
	fclose(ptrFichero);//Cierra el fichero
	return filePuertoCOM.PuertoCOM; 
}
/************************************************************************************
 BUSCA EL FICHERO "configuracion.com" Y SI EXISTE Y ABRE, LEE LA UNA ESTRUCTURA
 structPueertoCOM DONDE ESTAN LOS DATOS DEL PUERTO CON EL QUE FUNCIONA EL SISTEMA
 Si no hay fichero ó no se abre devuelve la variable de la estructura global 
 filePuertoCOM.NumeroPuerto=0; Si lee con éxito esta variable vale el nº del puerto.
 y el resto de las variables del puerto están rellenas
*************************************************************************************/
int pcom_lee_puerto_com_de_fichero(void)
{
	
	//INTENTA ABRIR EL PUERTO GUARDADO EN EL FICHERO
	DisableBreakOnLibraryErrors ();//Para evitar errores en debug  
	ErrorFichero=GetFileSize (NombreFichero, &Dummy);//Pregunta el tamaño del fichero    
	if (ErrorFichero>=0)//Si no da error, el fichero existe
	{
		ptrFichero=fopen(NombreFichero,"rb");// Abre el fichero
		if (!ptrFichero){filePuertoCOM.PuertoCOM=0;return 0;} //Si falla la apertura, sale con 0
		// Si abre el fichero sin errores lee los datos del puerto COM
		fread(&filePuertoCOM,sizeof(struct strPuertoCOM),1,ptrFichero);
		fclose(ptrFichero);//Cierra el fichero
		return filePuertoCOM.PuertoCOM; //Sale con un número de puerto
	}//if (ErrorFichero>=0)
	// Si el fichro no existe sale con 0
	else {filePuertoCOM.PuertoCOM=0;return 0;}
}
/**************************************************************************************
	   			SI HAY UN FICHERO DE CONFIGURACIÓN, LO APLICA
***************************************************************************************/
int pcom_abre_puerto_desde_fichero(void)
{
	// Si no encuentra el fichero o no lo puede abrir sale con 0 
	if (!pcom_lee_puerto_com_de_fichero()) return 0;
	// Si encuentra el fichero y lee una estructura con los datos, los procesa  
	FlagPuertoAbierto = 0;  // inicializa el flag a puerto no abierto 
    // Abre el puerto con la nueva configuración 
    DisableBreakOnLibraryErrors ();// Inhabilita mensajes de error en tiempo de ejecución
	ErrorRS232 = OpenComConfig (filePuertoCOM.PuertoCOM, filePuertoCOM.NombrePuertoCOM,
		filePuertoCOM.VelocidadPuerto,filePuertoCOM.Paridad,filePuertoCOM.BitsDatos, 
		filePuertoCOM.BitsDeStop, filePuertoCOM.ColaDeEntrada, filePuertoCOM.ColaDeSalida);
	EnableBreakOnLibraryErrors (); // Habilita mensajes de error en tiempo de ejecución
    // Puerto abierto 
    if (ErrorRS232) 
	{
	//com_display_error_RS232 ();//No muestra el error
		return 0;//   devuelve el control a la función que llamó  
	}
    else //(ErrorRS232 == 0) Si no hay errores configura el protocolo 
    {
        // Configura la opción protocolo software 
        SetXMode (filePuertoCOM.PuertoCOM,filePuertoCOM.ModoX);
        // Configura la opción protocolo hardware 
        SetCTSMode (filePuertoCOM.PuertoCOM, filePuertoCOM.ModoCTS);
        // Configura la opción "Timeout" 
        SetComTime (filePuertoCOM.PuertoCOM, filePuertoCOM.Timeout);
		// Indica al sistema que puerto se ha abierto
		PuertoCOM=filePuertoCOM.PuertoCOM;
		//Configura los terminadores
		TerminadorTransmision=filePuertoCOM.TerminadorTransmision; 
		TerminadorRecepcion=filePuertoCOM.TerminadorRecepcion; 
		// Indica al sistema que hay puerto abierto
		FlagPuertoAbierto = 1;
 		//Anota el puerto en el panel de configuración
		SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);
		//Identifica al instrumento
		strcpy (IDNinstrumento, filePuertoCOM.IDNinstrumento);   
		// Activa la recepción automática para ese puerto
		pcom_recepcion_automatica(SI);
		//sale con el puerto COM abierto 
		return PuertoCOM;
	}
}
/**************************************************************************************
 		       	FUNCION DEL PANEL DE CONFIGURACION                          
				ESTA FUNCIÓN PUEDE CERRAR EL PANEL 
				Recupera la configuración realizada por el usuario	
***************************************************************************************/
int CVICALLBACK panel_configuracion (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{   
	if( event==EVENT_GOT_FOCUS)
	{   // Si el panel está recien abierto...
		if(FlagPanelConfigRecienAbierto) 	// Usa el flag para que esto lo haga una sola vez
											// cada vez que se abre el panel
		{	
			// Inicializa la lista de puertos. La idea es que esté viva mientras el 
			// panel esté instalado en memoria
			//if(Depuracion)MessagePopup ("TEST POINT", "se inicializa la lista de puertos");
			elimina_nodos_lista_paco(&sListaCOM); 
			inicializa_lista_paco(&sListaCOM); 
			// La lista de puertos disponibles incialmente dimado
			SetCtrlAttribute (ConfigHandle,CONFIGURAP_RINGCOM, ATTR_DIMMED, 1);
			com_establece_parametros ();// Llama a la función que establece parámetros del panel
			FlagPanelConfigRecienAbierto=0;
		}
	// Si no hay un puerto abierto borra la identificación del sistema
	if(!PuertoCOM) SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, "");			
	}	
	
	// Para cerrar el panel sin aplicar cambios
	if (event==EVENT_CLOSE)
	{
		elimina_nodos_lista_paco(&sListaCOM);//Elimina la lista de puertos   
		FlagPanelConfigRecienAbierto=1;  
		if(ModoPanelConfiguracion==PANEL_MODO_HIJO)
		RemovePopup (0);
		if(ModoPanelConfiguracion==PANEL_MODO_TOP)
		HidePanel (ConfigHandle);
	}
	return 0;
}
/**************************************************************************************
         FUNCION PARA APLICAR LAS OPCIONES DEL PANEL DE CONFIGURACION ABRIR PUERTO 
		 Y OCULTAR EL PANEL
		 Intenta abrir el puerto, con los parámetros  seleccionado en el 
		 panelde configuración y cierra el panel de configuración.
***************************************************************************************/
int CVICALLBACK AplicarConfigCallback (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if(event== EVENT_COMMIT)
	{
			// Lee el valor del puerto seleccionado para abrir
			pcom_cierra_puerto_serie();  
			GetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, &PuertoCOM);
			copia_cadena_de_lista_paco(&sListaCOM,  PuertoCOM, IDNinstrumento);
			SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, IDNinstrumento);	
			// AQUI SE LLAMA A LA FUNCION QUE ABRE EL PUERTO SERIE
			if(PuertoCOM!=0) pcom_abre_puerto_serie();
			///Para finalizar Descarga el panel de configuración
            FlagPanelConfigRecienAbierto=1;//Para que cuando vuelva a cargar el panel
											//lo configure con los parámetos actualizados
			// Antes de salir elimina la lista de puertos COM
			//if(Depuracion)MessagePopup ("TEST POINT", "se elimina la lista de puertos");
			elimina_nodos_lista_paco(&sListaCOM);   
			// Si la cadena de identificación recibida del instrumento coincide 
			//con la IdentidadDelSistema que tiene la aplicación, guarda el setup en el fichero
			if(!strcmp (IDNinstrumento, IdentidadDelSistema))  
				pcom_guarda_puerto_com_en_fichero();//Guarda la configuracion en un fichero
			HidePanel(ConfigHandle);
	}
	return 0;
}
/************************************************************************************
			ABRE EL PUERTO SERIE Y CONFIGURA EL PROTOCOLO Y TIMEOUT
			Si no se abre el puerto muestra un pop-up indicandolo y devuelve 0
			Si se abre devuelve el número de puerto abierto
			Abre el puerto seleccinado en el control PUERTOCOM
*************************************************************************************/
int pcom_abre_puerto_serie(void)
{
	FlagPuertoAbierto = 0;  // inicializa el flag a puerto no abierto 
    com_salva_parametros (); // Lee los parámetros de configuración 
    // Abre el puerto con la nueva configuración 
    DisableBreakOnLibraryErrors ();// Inhabilita mensajes de error en tiempo de ejecución
	ErrorRS232 = OpenComConfig (PuertoCOM, NombrePuertoCOM, VelocidadPuerto,
		Paridad,BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida);
	
	EnableBreakOnLibraryErrors (); // Habilita mensajes de error en tiempo de ejecución
    // Puerto abierto 
    if (ErrorRS232) 
	{
		com_display_error_RS232 ();
		return 0;
	}
    else //(ErrorRS232 == 0) Si no hay errores configura el protocolo 
    {
          // Lee y configura la opción protocolo software 
           GetCtrlVal (ConfigHandle, CONFIGURAP_MODOXONOFF, &ModoX);
           SetXMode (PuertoCOM,ModoX);
           // Lee y configura la opción protocolo hardware 
           GetCtrlVal (ConfigHandle, CONFIGURAP_MODOCTS, &ModoCTS);
           SetCTSMode (PuertoCOM, ModoCTS);
           // Lee y configura la opción "Timeout" 
           GetCtrlVal (ConfigHandle, CONFIGURAP_TIMEOUT, &Timeout);
           SetComTime (PuertoCOM, Timeout);
		   // Activa la recepción automática
		   pcom_recepcion_automatica(SI);
		   FlagPuertoAbierto = 1;
 		   SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);
		   return PuertoCOM;
	}
}
/************************************************************************************
			FUNCION QUE ACTIVA O DESACTIVA LA RECEPCION AUTOMATICA DEL PUERTO COM
*************************************************************************************/
void pcom_recepcion_automatica(int activar)
{
	if(activar){
	InstallComCallback (PuertoCOM, LWRS_RXFLAG, 0,
		Terminador,com_terminador_detectado_en_buffer_de_entrada, NULL);}
	else
	{InstallComCallback (PuertoCOM, 0, 0, Terminador,
		 com_terminador_detectado_en_buffer_de_entrada, 0);}
}
/************************************************************************************
	BUSCA EL SISTEMA ENTRE LOS PUERTOS DEL SISTEMA Y SI LO ENCUENTRA ABRE EL PUERTO 
	SERIE Y CONFIGURA EL PROTOCOLO Y TIMEOUT.
	Intenta abrir todos los puertos de 1 a 255.Con la configuración 
	del panel de configuración. Si puede abrir alguno, le envia
	un comando de identificación. Y si la cadena devuelta es la esperada, 
	IDN_DEL_SISTEMA,termina de configurar el puerto y sale devolviendo el numero 
	del puerto COM abierto; si es 0 ningún puerto se ha abierto	
*************************************************************************************/
int pcom_abre_puerto_serie_automaticamente(void) 
{
	int Indice;
	int status_puerto;
	int LongMensaje;
	char CadenaComando[STRLARGO];
	//Primero intenta abrir el puerto desde un fichero
	if (pcom_abre_puerto_desde_fichero())
	{
		
		return PuertoCOM;//Si tiene éxito sale ya
	}
	//Si no puede abrir el puerto desde el fichero, explora los puerto buscando el sistema
	//...............................................................................
	FlagPuertoAbierto = 0;  // inicializa el flag a "puerto si abrir"
    com_salva_parametros (); // Lee los parámetros de configuración
	DisableBreakOnLibraryErrors (); // Función obsoleta 						TO DO
	//SetBreakOnLibraryErrors (0);// Inhabilita errores en tiempo de ejecución     
	for (PuertoCOM=1;PuertoCOM <=255;PuertoCOM++)// Recorre los puertos posibles
	{
		// Intenta abrirlos
		status_puerto = OpenComConfig (PuertoCOM, NombrePuertoCOM, VelocidadPuerto, Paridad,
                                       BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida);
		if (status_puerto>=0) // Si el puerto está presente en el equipo y abre... 
		{
			Delay(RetardoAperturaPuerto);// Espera un poco para que termine de configurarse el puerto	
			SetComTime (PuertoCOM, 0.1); 
			Fmt ( CadenaComando, "%s", "*IDN"); // Prepara la cadena con el comando de identificación
			FlagPuertoAbierto=1; // Indica al sistema que el puerto está abierto
			pcom_enviar_datos( CadenaComando,NO_MOSTRAR);// Envia la cadena
			Delay(Retardo); // Espera un poco
			pcom_recibir_datos(IDNinstrumento);// Lee la posible respuesta
			// Busca el terminador (ver de #define Terminador) y pone tras el un finalizador de cadena '\0'
			Indice=0;
			LongMensaje=strlen (IDNinstrumento);
			while(IDNinstrumento[Indice]!=Terminador && Indice<=LongMensaje)Indice++;
			IDNinstrumento[Indice+1]='\0';
			// Compara la cadena recibida con la identificación del sistema
			if(!strcmp (IDNinstrumento, IdentidadDelSistema))
			{  // Y si es correcta termina de configurar el puerto
				// Lee y configura la opción protocolo software 
               GetCtrlVal (ConfigHandle, CONFIGURAP_MODOXONOFF, &ModoX);
               SetXMode (PuertoCOM,ModoX);
               // Lee y configura la opción protocolo hardware 
               GetCtrlVal (ConfigHandle, CONFIGURAP_MODOCTS, &ModoCTS);
               SetCTSMode (PuertoCOM, ModoCTS);
               // Lee y configura la opción "Timeout" 
               GetCtrlVal (ConfigHandle, CONFIGURAP_TIMEOUT, &Timeout);
               SetComTime (PuertoCOM, Timeout);
			   //Callback para respuesta automática
			   pcom_recepcion_automatica(SI);
			   //InstallComCallback (PuertoCOM, LWRS_RXFLAG, 0, Terminador,
				//   com_terminador_detectado_en_buffer_de_entrada, 0);
			   FlagPuertoAbierto = 1; // Indica al sistema que el puerto está abierto 
			   EnableBreakOnLibraryErrors (); // Habilita errores en tiempo de ejecución
			   break; // Sale del bucle for
			}
			else
			{
				CloseCom (PuertoCOM); // No dejamos el puerto abierto
				FlagPuertoAbierto=0;
			}
		
		}//if (status_puerto>=0)
	}// del for
	if (FlagPuertoAbierto){
		SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);//Actualiza el panel de configuración
		pcom_guarda_puerto_com_en_fichero();//Guarda los datos del puerto en un fichero 
		return PuertoCOM;
	}
	else
	{ 
		PuertoCOM=0;
		SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);
		return PuertoCOM;
	}
}
/*************************************************************************************
							CIERRA EL PUERTO SERIE
	Si el puerto está abierto, limpia los
	buffers de entrada y salida y cierra
	el puerto.
**************************************************************************************/
void pcom_cierra_puerto_serie(void)
{
	if (FlagPuertoAbierto)
    {
		FlagPuertoAbierto=0; // Para que el programa sepa que el puerto está cerrado
		FlushOutQ (PuertoCOM);
        FlushInQ (PuertoCOM);  
		ErrorRS232 = CloseCom (PuertoCOM);
	    if (ErrorRS232) com_display_error_RS232 ();
		PuertoCOM=0;
		//SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);		
    }
}
/***************************************************************************************
	FUNCION PARA ENVIAR DATOS
	Función que comprueba si se pueden enviar datos por el puerto serie. Si se puede
	 añade el terminador y los envia; si no, de un mensaje de error en pantalla.
	 "Mostrar"  se pone a 1 para que la cadena se vea en el test box MENSAJES_CADENA_ENVIAR
*****************************************************************************************/
int pcom_enviar_datos (char CadenaEnviar[], int Mostrar)  {
// Si el pueto no está abierto no podemos transmitir
	int LongitudString;
	if (ErrorRS232 = pcom_test_puerto_abierto()) 
	{
		com_display_error_RS232();
		return 0;
	}
    // Si el puerto está abierto transmitimos
    else 
	{
    	switch (TerminadorTransmision) 
    	{
    	case 1:
       		strcat(CadenaEnviar, "\r");
       		break;
    	case 2:
       		strcat(CadenaEnviar, "\n");
       		break;
		case 3:
       		strcat(CadenaEnviar, "\r\n");
       		break;
    	}
	    LongitudString = StringLength (CadenaEnviar);
		// Envía la cadena  por el puerto
		ComWrt (PuertoCOM, CadenaEnviar, LongitudString); 
		ErrorRS232 = ReturnRS232Err ();
		if (ErrorRS232)
		{ com_display_error_RS232 ();
		return 0;
		}
		else 
		{
			//if(Mostrar)ResetTextBox (MensajesHandle, MENSAJES_CADENA_ENVIAR, CadenaEnviar);
			 if(Mostrar){
			if(MensajesHandle)SetCtrlVal (MensajesHandle, MENSAJES_CADENA_ENVIAR, CadenaEnviar);
			//if(MensajesHandle)InsertListItem (MensajesHandle, MENSAJES_LISTCOMANDOS, 0,CadenaEnviar,23);
				 
			 }
		}
		return ErrorRS232;
	}
}
/**************************************************************************************
	FUNCION QUE COMPRUEBA LA VARIABLE FlagPuertoAbierto
	Que está a 1 si el puerto está abierto
***************************************************************************************/
int pcom_test_puerto_abierto (void)
{
		if(FlagPuertoAbierto) return 0;
		else return -3;	   // Devuelve error de puerto no abierto
}
/****************************************************************************************
	LEE LOS PUERTOS DEL SISTEMA, LOS PONE EN UNA LISTA DINÁMICA Y LOS PONE EN UN RING
	Intenta abrir todos los puertos con los parámetros del sistema conectado. Los que
	consigue abrir les mando un *IDN y lee la respuesta. Pone el número y nombre del 
	puerto, así como el identificador, lo pone en un nodo y lo añade a una lista
	listaPaco. Finalmente con los datos de la lista rellena un control RING y
	elimina la lista tipo listaPaco.
	DEVUELVE:El número de puertos encontrados.
****************************************************************************************/
int com_hacer_lista_de_puertos_del_sistema(void) 
{
	int Indice;
	int status_puerto;
	int LongMensaje;
	int NumeroDePuerto;
	int PuertosEncontrados;
	char CadenaComando[STRLARGO];
	char NombrePuerto[STRCORTO];
	nodoStructPaco *pNodoCOM; // Puntero auxiliar a nodo con la información de un puerto 
	//
	elimina_nodos_lista_paco(&sListaCOM); // Elimina el contenido de la lista antigua
	inicializa_lista_paco(&sListaCOM);	
	inicializa_nodo_struct_paco( &pNodoCOM);// El primer nodo es el del COM0 ó ningún puerto
	pNodoCOM->NumeroPuerto=0; // Apunta en el nodo creado el número del puerto
	strcpy (pNodoCOM->NombrePuerto,"COM 0");
	pNodoCOM->VelocidadPuerto=VelocidadPuerto; // Copia la cadena de identificación en el nodo  
	strcpy (pNodoCOM->NombreInstrumento,"ningún puerto abierto;");
	inserta_nodo_lista_paco(&sListaCOM,pNodoCOM,DETRAS);// Inserta el nodo en la lista
	FlagPuertoAbierto = 0;  // inicializa el flag a puerto no abierto
    com_salva_parametros (); // Lee los parámetros de configuración
	DisableBreakOnLibraryErrors (); // Inhabilita errores en tiempo de ejecución     
	PuertosEncontrados=0;
	for (NumeroDePuerto=1;NumeroDePuerto <=255;NumeroDePuerto++)// Recorre los puertos posibles
	{
		// Intenta abrirlos
		status_puerto = OpenComConfig (NumeroDePuerto, NombrePuertoCOM, VelocidadPuerto, Paridad,
                                       BitsDatos, BitsDeStop, ColaDeEntrada, ColaDeSalida);
		if (status_puerto>=0) // Si el puerto está presente en el equipo y abre... 
		{
			SetComTime (PuertoCOM, 0.1); 
			Delay(RetardoAperturaPuerto);// Espera un poco para que termine de configurarse el puerto	
			Fmt ( CadenaComando, "%s", "*IDN"); // Prepara la cadena con el comando de identificación
			FlagPuertoAbierto=1; // Indica al sistema que el puerto está abierto
			PuertoCOM=NumeroDePuerto; // Para poder enviar y recibir
			pcom_enviar_datos( CadenaComando,NO_MOSTRAR);// Envia la cadena
			Delay(Retardo); // Espera un poco
			IDNinstrumento[0]='\0'; // Límpia la cadena antes de leer el puerto			
			pcom_recibir_datos(IDNinstrumento);// Lee la posible respuesta
			FlagPuertoAbierto=0;// Indica al sistema que el puerto está cerrado
			CloseCom (NumeroDePuerto); // Cierra el puerto
			// Busca el terminador (ver de #define Terminador) y pone tras el un finalizador de cadena '\0'
			Indice=0;
			LongMensaje=strlen (IDNinstrumento);
			while(IDNinstrumento[Indice]!=Terminador && Indice<=LongMensaje && Indice<STRLARGO)Indice++;
			IDNinstrumento[Indice+1]='\0'; // Cierra la cadena con un fin de cadena
			// Crea un nodo en la lista y guarda la información encontrada
			inicializa_nodo_struct_paco( &pNodoCOM); // Crea un nuevo nodo para la lista de puertos
			Fmt (NombrePuerto, "%s<COM %i", NumeroDePuerto);  // Crea la cadena de identificación del puerto
			strcpy (pNodoCOM->NombrePuerto, NombrePuerto); // Copia la cadena de identificación en el nodo
			if (!strcmp(IDNinstrumento,""))strcpy(IDNinstrumento,"instrumento sin identificación;");
			strcpy (pNodoCOM->NombreInstrumento, IDNinstrumento);
			pNodoCOM->NumeroPuerto=NumeroDePuerto; // Apunta en el nodo creado el número del puerto
			pNodoCOM->VelocidadPuerto=VelocidadPuerto; // Copia la cadena de identificación en el nodo  
			inserta_nodo_lista_paco(&sListaCOM,pNodoCOM,DETRAS);// Inserta el nodo en la lista
			PuertosEncontrados++;
		}//if (status_puerto>=0)
		
	}// del for
	//if(Depuracion && sListaCOM.primer_nodo != NULL )
	{
		//if(Depuracion)MessagePopup ("TEST POINT", "se ha rellenado la lista de puertos");
	}
	
	// Rellena el control RINGCOM con los puertos encontrados
	SetCtrlAttribute (ConfigHandle,CONFIGURAP_RINGCOM, ATTR_DIMMED, 0);
	ClearListCtrl (ConfigHandle,CONFIGURAP_RINGCOM);// Primero limpia el control destino 
	// Rellena el control del panel para visualizar la lista
	pNodoCOM=sListaCOM.primer_nodo;// Con el puntero auxiliar apunta al primer nodo de la lista
	while (pNodoCOM!=NULL)
	{
		InsertListItem (ConfigHandle,CONFIGURAP_RINGCOM,0, pNodoCOM->NombrePuerto, pNodoCOM->NumeroPuerto); 
		pNodoCOM=pNodoCOM->siguiente;
	}	
	return PuertosEncontrados;
}


/*****************************************************************************************
   SELECCIONA UN PUERTO DEL RING DONDE ESTÁN LOS PUERTOS DEL SISTEMA Y LO COLOCA
   EN EL CONTROL DEL QUE SE LEE EL PUERTO A ABRIR.


******************************************************************************************/
int CVICALLBACK seleciona_un_puerto_de_la_lista (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int indice;
	int ValorPuerto;
	int Resultado;
	//nodoStructPaco *pNodoCOM;
	switch (event)
	{
		case EVENT_VAL_CHANGED:
			GetCtrlIndex (ConfigHandle, CONFIGURAP_RINGCOM, &indice);//Lee el índice del puerto seleccionado
			Resultado=GetValueFromIndex (ConfigHandle, CONFIGURAP_RINGCOM, indice,&ValorPuerto);// Le el valor apuntado por le índice 
			if(Resultado<0)
			{
				MessagePopup ("ERROR", "No hay puertos en la lista");
			}
			else
			{
				copia_cadena_de_lista_paco(&sListaCOM,  ValorPuerto, IDNinstrumento);
				SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, IDNinstrumento);				
				SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, ValorPuerto); // Anota el valor del puerto seleccionado   
			}
			break;
	}
	return 0;
}
/****************************************************************************************
	FUNCION QUE SE EJECUTA CUANDO SE PULSA EL BOTON DE "buscar"
	Pone un pop-up de espera y ejecuta 	com_hacer_lista_de_puertos_del_sistema(); 
*****************************************************************************************/
int CVICALLBACK buscar_puertos_serie (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int Resultado=0;
	switch (event)
	{
		case EVENT_LEFT_CLICK:
			if (FlagPuertoAbierto){
				pcom_cierra_puerto_serie();
			}
			SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION,"Selecciona un puerto");	
			EsperaHandle = LoadPanel (ConfigHandle, "PanelesPuertoCom.uir", ESPERA);
	    	InstallPopup (EsperaHandle); // Pone el panel en memoria
			Resultado=com_hacer_lista_de_puertos_del_sistema();
			RemovePopup (0);
			break;
	}
	return 0;
}
/****************************************************************************************
		FUNCION QUE CARGA EL PANEL DE ESTATUS
****************************************************************************************/

void CVICALLBACK Estatus (int menuBar, int menuItem, void *callbackData,
		int panel)
{	
		char MensajeDeError[STRMEDIO];
			if(pcom_test_puerto_abierto()==0)
			{
				estatusHandle = LoadPanel (MensajesHandle, "PanelesPuertoCom.uir", ESTATUS);
	    		InstallPopup (estatusHandle); // Pone el panel en memoria 
			}
			else
			{
				Fmt (MensajeDeError, "%s", "Error:¡No hay ningún puerto abierto!\n\n"
					"Abra el panel de configuración\n"
                 "en el menú COM/Configurar\n");
            	MessagePopup ("Mensaje RS232", MensajeDeError);
			}
}
/*****************************************************************************************
					FUNCIONES DEL PANEL MENSAJES
****************************************************************************************/
// Función del Panel
int CVICALLBACK panel_mensajes (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	Rect CoordenadasPanel;
	switch (event)
	{
		case EVENT_PANEL_MOVING:
			GetPanelEventRect (eventData2, &CoordenadasPanel/*Rect *rectangle*/);
			SetCtrlVal(MensajesHandle,MENSAJES_X_POS,CoordenadasPanel.top);
			SetCtrlVal(MensajesHandle,MENSAJES_Y_POS,CoordenadasPanel.left);
		break;		
		case EVENT_GOT_FOCUS:
			
			if(pcom_test_puerto_abierto()==0)
			{
				pcom_activa_controles_mensajes_para_comunicar(SI);// Activa los controles que comunican. 	
			}
			else
			{
				pcom_activa_controles_mensajes_para_comunicar(NO);// Activa los controles que comunican. 	
			}
			
			
			break;
		case EVENT_LOST_FOCUS:
			break;
		case EVENT_CLOSE:
			HidePanel(MensajesHandle);
		break;
	}
	return 0;
}
/****************************************************************************************
					       FUNCION PARA EL BOTÓN "RECIBIR"
****************************************************************************************/
int CVICALLBACK Recibir (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
		 if(pcom_recibir_datos (CadenaRespuesta)>0)
		 {		 
		 	SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA,CadenaRespuesta);  
		 }
		 break;
		case EVENT_RIGHT_CLICK:
		break;
		}
	return 0;
}
/****************************************************************************************
      				 FUNCION PARA LIMPIAR LA VENTANA DE RECEPCION          
****************************************************************************************/
int CVICALLBACK LimpiarRecibir (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{																						 
			case EVENT_COMMIT:
			ResetTextBox (MensajesHandle, MENSAJES_CADENA_RECIBIDA, "");
			break;
		}
	return 0;
}
/****************************************************************************************
      				 FUNCION PARA LIMPIAR LA VENTANA DE COMANDOS          
****************************************************************************************/
int CVICALLBACK LimpiarEnviar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	if (event==EVENT_COMMIT)
	{
		ResetTextBox (MensajesHandle, MENSAJES_CADENA_ENVIAR,"");
	}
	return 0;
}
/****************************************************************************************
	FUNCION QUE SE EJECUTA AL PULSAR EL BOTON "abrir/cerrar puerto"
	Si el puerto está abierto, cierra el puerto serie abierto y 
	desactiva los controles que comunican
****************************************************************************************/
int CVICALLBACK cerrar_puerto (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
	case EVENT_COMMIT:
		if (pcom_test_puerto_abierto()==0) // Si el puerto está abierto...
		{				
			pcom_cierra_puerto_serie(); // cierra el puerto
			if(pcom_test_puerto_abierto!=0)  // Si se cerró el puerto
				pcom_activa_controles_mensajes_para_comunicar(NO);
		}
		else // Si el puerto está cerrado,
		{
			// Pone el cartel de espera, intenta abrir el puerto y quita el cartel de espera
			EsperaHandle = LoadPanel (MensajesHandle, "PanelesPuertoCom.uir", ESPERA);
    		InstallPopup (EsperaHandle); 
			pcom_abre_puerto_serie_automaticamente();
			RemovePopup (0);
				
				if(pcom_test_puerto_abierto()==0) // Si se abrió el puerto
					pcom_activa_controles_mensajes_para_comunicar(SI);// Activa los controles que comunican. 	
				else // Si no se abre el puerto da la opción de abrir el panel de configuración
				{
					if(ConfirmPopup ("Error de puerto COM", "No se ha encontrado el sistema en ningún puerto COM\n\n"
					"¿  Quieres abrir la configuración del puerto?"))
					InstallPopup (ConfigHandle);
					//DisplayPanel (ConfigHandle); // Pone el panel de configuración en memoria
				}
		}
	break;
	}
return 0;
}
/****************************************************************************************
		FUNCION PARA DIMAR O DESDIMAR LOS CONTROLES QUE COMUNICAN
		Si "Accion" vale 1 desdima los controles y se pueden usar
		Si "Acción" vale 0 dima los controles y no se pueden usar 
*****************************************************************************************/
void pcom_activa_controles_mensajes_para_comunicar( int short Accion)
{

	if(Accion)  // Desbloquea/activa los controles
	{
		
		//SetCtrlAttribute (MensajesHandle,MENSAJES_ENVIAR, ATTR_DIMMED, 0);
		SetCtrlAttribute (MensajesHandle,MENSAJES_RECIBIR, ATTR_DIMMED, 0);
		SetCtrlVal(MensajesHandle,MENSAJES_LEDCOM,1); 
		SetCtrlAttribute (MensajesHandle, MENSAJES_CERRARCOM, ATTR_LABEL_TEXT, "cerrar puerto");
	}
	else   // Bloquea/Desactiva los controles
	{
		//SetCtrlAttribute (MensajesHandle,MENSAJES_ENVIAR, ATTR_DIMMED, 1);
		SetCtrlAttribute (MensajesHandle,MENSAJES_RECIBIR, ATTR_DIMMED, 1);
		SetCtrlVal(MensajesHandle,MENSAJES_LEDCOM,0); 
		SetCtrlAttribute (MensajesHandle, MENSAJES_CERRARCOM, ATTR_LABEL_TEXT, "abrir puerto");
	}
}
/**************************************************************************************
 		       				FUNCIONES DEL PANEL STATUS                           
***************************************************************************************/
int CVICALLBACK com_cierra_panle_status (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
		{
		case EVENT_COMMIT:
			DiscardPanel (estatusHandle);
			break;
		case EVENT_RIGHT_CLICK:

			break;
		}
	return 0;
}
/**************************************************************************************
			FUNCION QUE  MUESTRA EL NUMERO DE BYTES EN LA COLA DE ENTRADA             
***************************************************************************************/
int CVICALLBACK com_longitud_cola_entrada (int panel, int control, int event,
                                void *callbackData, int eventData1,
                                int eventData2)
{
    int BytesEnBufferDeEntrada; 
	if (event == EVENT_COMMIT)
        {
        BytesEnBufferDeEntrada = GetInQLen (PuertoCOM);
        Fmt (Msg1, "%s<Longitud de la cola de entrada = %i", BytesEnBufferDeEntrada);
        MessagePopup ("Mensaje RS232", Msg1);
        }
    return 0;
}
/**************************************************************************************
		FUNCION QUE  MUESTRA EL NUMERO DE BYTES EN LA COLA DE SALIDA              
*******	********************************************************************************/
int CVICALLBACK com_longitud_cola_salida (int panel, int control, int event,
                                 void *callbackData, int eventData1,
                                 int eventData2)

{
int BytesEnBufferDeSalida;   
	if (event == EVENT_COMMIT)
        {
        BytesEnBufferDeSalida = GetOutQLen (PuertoCOM);
        Fmt (Msg1, "%s<Longitud de la cola de salida = %i", BytesEnBufferDeSalida);
        MessagePopup ("Mensaje RS232", Msg1);
        }
    return 0;
}
/**************************************************************************************
             FUNCION QUE LIMPIA LA COLA DE ENTRADA                       
***************************************************************************************/
int CVICALLBACK com_borra_cola_entrada (int panel, int control, int event,
                                 void *callbackData, int eventData1,
                                 int eventData2)
{
    if (event == EVENT_COMMIT)
        {
        FlushInQ (PuertoCOM);
        MessagePopup ("Mensaje RS232", "¡Cola de entrada limpia!");
        }
    return 0;
}

/**************************************************************************************
                FUNCION QUE LIMPIA LA COLA DE SALIDA                      
***************************************************************************************/
int CVICALLBACK com_borra_cola_salida (int panel, int control, int event,
                                   void *callbackData, int eventData1,
                                   int eventData2)
{
    if (event == EVENT_COMMIT)
        {
        FlushOutQ (PuertoCOM);
		MessagePopup ("Mensaje RS232", "¡Cola de salida limpia!");
        }
    return 0;
}
/**************************************************************************************
           FUNCION QUE LEE EL ESTATUS DEL PUERTO SERIE                     
***************************************************************************************/
int CVICALLBACK com_muestra_status (int panel, int control, int event,
                                   void *callbackData, int eventData1,
                                   int eventData2)
{
    int com_status;      // Código de estatus del puerto
	if (event == EVENT_COMMIT)
        {
        com_status = GetComStat (PuertoCOM);
        com_muestra_mensaje_status (com_status);
        }
    return 0;
}
/**************************************************************************************
 		       FUNCION QUE LEE ERRORES DEL PUERTO                            
***************************************************************************************/
int CVICALLBACK com_muestra_error (int panel, int control, int event,
                               void *callbackData, int eventData1,
                               int eventData2)
{
    switch (event)
        {
        case EVENT_COMMIT:
            ErrorRS232 = ReturnRS232Err (); // El sistema informa del error registrado
            com_display_error_RS232 ();			// en la última operación del puerto
            break;
        case EVENT_RIGHT_CLICK :
            break;
        }
    return 0;
}
/**************************************************************************************
          FUNCION QUE MUESTRA EL ESTATUS DEL PUERTO SERIE                  
***************************************************************************************/
void com_muestra_mensaje_status (com_status)
{
    ComMsg[0] = '\0';
    if (com_status & 0x0001)
        strcat (ComMsg, "Input lost: Input queue"
                " filled and characters were lost.\n");
    if (com_status & 0x0002)
        strcat (ComMsg, "Asynch error: Problem "
                "determining number of characters in input queue.\n");
    if (com_status & 0x0010)
        strcat (ComMsg, "Paridad error.\n");
    if (com_status & 0x0020)
        strcat (ComMsg, "Overrun error: Received"
                " characters were lost.\n");
    if (com_status & 0x0040)
        strcat (ComMsg, "Framing error: Stop bits were not received"
                " as expected.\n");
    if (com_status & 0x0080)
        strcat (ComMsg, "Break: A break signal was detected.\n");
    if (com_status & 0x1000)
        strcat (ComMsg, "Remote XOFF: An XOFF character was received."
                "\nIf XON/XOFF was enabled, no characters are removed"
                " from the output queue and sent to another device "
                "until that device sends an XON character.\n");
    if (com_status & 0x2000)
        strcat (ComMsg, "Remote XON: An XON character was received."
                "\nTransmisson can resume.\n");
    if (com_status & 0x4000)
        strcat (ComMsg, "Local XOFF: An XOFF character was sent to\n"
                " the other device.  If XON/XOFF was enabled, XOFF is\n"
                " transmitted when the input queue is 50%, 75%, and 90%\n"
                " full.\n");
    if (com_status & 0x8000)
        strcat (ComMsg, "Local XON: An XON character was sent to\n"
                " the other device.  If XON/XOFF was enabled, XON is\n"
                " transmitted when the input queue empties after XOFF\n"
                " was sent.  XON tells the other device that it can \n"
                " resume sending data.\n");
    if (strlen (ComMsg) == 0)
        strcat (ComMsg, "No status bits are set.");
    MessagePopup ("Mensaje RS232", ComMsg);
}
/****************************************************************************************
			FUNCIONE PARA LIMPIAR LA PILA DE ERRORES SCPI DEL MICRO
			
****************************************************************************************/
void pcom_limpia_pila_errores()
{
	int LongCadenaCls;
	LongCadenaCls = StringLength ("*CLS");
	CopyString (CadenaComando, 0,"*CLS", 0,LongCadenaCls);
	pcom_enviar_datos(CadenaComando,NO_MOSTRAR); // envia el comando completo por el puerto	
}


/****************************************************************************************
			FUNCIONE PONER A 1 O 0 EL FLAG DE DEPURACION
			
****************************************************************************************/
int CVICALLBACK modo_depuracion (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{

	int LongCadena;
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (MensajesHandle, MENSAJES_DEPURACION, &Depuracion);
			if(Depuracion) sprintf(CadenaComando,"%s","TS:DP 1");	
			else sprintf(CadenaComando,"%s","TS:DP 0");	
			pcom_enviar_datos(CadenaComando,NO_MOSTRAR); // envia el comando completo por el puerto	
		break;
	}
	return 0;
}

/***************************************************************************************
	FUNCION PARA RECIBIR DATOS
	Lee el terminador seleccionado y lee datos del puerto sobre la cadena
	global que le pasan como parámetro. 
	El parámetro SinPopUp se utiliza para que salte un popup si la cadena de entrada
	es demasiado larga. 
	
****************************************************************************************/
int pcom_recibir_datos (char CadenaRecibida[])
{
	int BytesEnBuffer;
	int BytesLeidos;
	char CadenaLeida[STRLARGO]="\0"; // Lee sobre una cadena limpia
	if (ErrorRS232 = pcom_test_puerto_abierto()) 
		{com_display_error_RS232();return 0;}
			// Si el pueto  está abierto  podemos recibir 
	else 
	{
       	// Selecciona el terminador de recepción 
       	switch (TerminadorRecepcion) 
        {
          	case 0:
            CaracterTerminador = 0;//NONE
            break;
         	case 1:
            CaracterTerminador = 13;  //CR
            break;
         	case 2:CaracterTerminador = 10; //LF
            break;
        }
           
		// RECEPION
		// Si el nº de bytes en el buffer del COM es razonable..
		BytesEnBuffer = GetInQLen (PuertoCOM);
		if(BytesEnBuffer<0)
		{com_display_error_RS232 (); return 0;}// Si la lectura da un error lo avisa y sale   		
		if(BytesEnBuffer > STRLARGO)
		{ErrorRS232=ERROR_DATOS_EXCESIVOS;com_display_error_RS232 ();return 0;}
		if (CaracterTerminador) // Lee con terminador 
			{   BytesLeidos = ComRdTerm (PuertoCOM, CadenaLeida, STRLARGO,CaracterTerminador);
				if(BytesLeidos <0)
				{ErrorRS232=BytesLeidos; com_display_error_RS232 (); return 0;}// Si la lectura da un error lo avisa y sale   	
			
			}
	           else // Lee sin terminador.  
			{   BytesLeidos = ComRd (PuertoCOM, CadenaLeida, BytesEnBuffer);
				if(BytesLeidos<0)
				{ErrorRS232=BytesLeidos;com_display_error_RS232 (); return 0;}// Si la lectura da un error lo avisa y sale   	
			}
           	// Tras hacer una lectura copia lo leido en CadenaRecibida y limpia el buffer del puerto COM
		CopyString (CadenaRecibida, 0, CadenaLeida, 0, BytesLeidos);
		return BytesLeidos;
		}//if (ErrorRS232 = pcom_test_puerto_abierto()) 
}
/****************************************************************************************
	FUNCION PARA RECIBIR AUTOMATICAMENTE CUANDO SE DETECTA "Terminador" EN EL BUFFER
 	Esta función es llamada cuando se detecta Terminador en el buffer de entrada
	del puerto COM.
    Esta función no utiliza pcom_recibir_datos(..) sino que utiliza ComRd(..). No utiliza
	ComRdTerm(..) porque quita el terminador.
	Antes de salir limpia el buffer del COM abierto
	Devuelve el resultado de la lectura en CadenaRespuesta
*****************************************************************************************/
void CVICALLBACK com_terminador_detectado_en_buffer_de_entrada (int portNo,int eventMask,void *callbackData)

{
	//void (*callbackData)();        
	int BytesLeidos;
	int Indice; // Para revisar la cadena de entrada
	CadenaRespuesta[0]='\0';// Resetea la cadena para leer datos del puerto
	BytesLeidos=ComRdTerm (PuertoCOM, CadenaRespuesta, STRLARGO,Terminador);//Lee
	// Si la lectura da un error lo avisa y sale 
	if (BytesLeidos<0){ErrorRS232=BytesLeidos;com_display_error_RS232 (); return;}//   
//--------------------------------------------------------------------------------------------		
	// Revisa la cadena buscando el Terminador y la cierra con '\n' y '\0'
	Indice=-1;
	do
	{
		Indice++;
	}while(CadenaRespuesta[Indice]!=Terminador2 && Indice<(STRLARGO-2));// Busca el terminador
	CadenaRespuesta[Indice]='\n'; // Le añade un retorno de carro
	CadenaRespuesta[Indice+1]='\0'; // Le añade un fin de cadena
	// Muestra la cadena
	SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA, CadenaRespuesta);// Muestra la cadena
	ErrorRS232 = ReturnRS232Err ();
	if (ErrorRS232) com_display_error_RS232 ();
	pcom_datos_recibido();//Llama a la función que se define en la aplicación principal 
return;
}	
/*********************************************************************************************
				PONE EN MEMORIA EL PANEL DE MENSAJES COMO TOP-LEVEL
**********************************************************************************************/
int pcom_carga_panel_de_mensajes(void)
{
	if ((MensajesHandle = LoadPanel (0, "PanelesPuertoCom.uir", MENSAJES)) < 0)
		return -1;
	
	return MensajesHandle;
}
/*********************************************************************************************
				MUESTRA EL PANEL DE MENSAJES
**********************************************************************************************/
int pcom_muestra_el_panel_de_mensajes(int Top,int Left)
{
	DisplayPanel(MensajesHandle);// Muestra el panel de mensajes
	// Coloca el panel en una posición determinada en la pantalla
	SetPanelAttribute (MensajesHandle, ATTR_TOP,Top);
	SetPanelAttribute (MensajesHandle, ATTR_LEFT,Left);
	SetCtrlVal(MensajesHandle,MENSAJES_X_POS,Top);
	SetCtrlVal(MensajesHandle,MENSAJES_Y_POS,Left);
	return 0;
}
/*********************************************************************************************
			LEE EL ESTADO DEL RADIOBOTTON DEPURACION Y LO DEVUELTE COMO ENTERO
**********************************************************************************************/
int pcom_estado_radio_button_de_depuracion(void)
{
	int Estado;
	GetCtrlVal(MensajesHandle,MENSAJES_DEPURACION,&Estado);
	return Estado;
}
/*********************************************************************************************
	CARGA EL PANEL DE CONFIGURACION EN MEMORIA COMO HIJO DEL PARÁMETRO DE ENTRADA
	Parámetro de entrada: tipo int; tiene que ser un Handle de panel activo
	o 0 para cargarlo como top-level.
**********************************************************************************************/
int pcom_carga_panel_de_configuracion(void)
{
		if ((ConfigHandle = LoadPanel (0, "PanelesPuertoCom.uir", CONFIGURAP)) < 0)
			return -1;
		else return ConfigHandle;
}
/*********************************************************************************************
			MUESTRA EL PANEL DE CONFIGURACION DEL PUERTO COM
**********************************************************************************************/
void pcom_instala_el_panel_de_configuracion(int ModoPanel,int Top,int Left)
{
	if(ModoPanel==PANEL_MODO_HIJO)	
	{if(ConfigHandle) InstallPopup (ConfigHandle);ModoPanelConfiguracion=PANEL_MODO_HIJO;}
	if(ModoPanel==PANEL_MODO_TOP)	
	{if(ConfigHandle) DisplayPanel (ConfigHandle);ModoPanelConfiguracion=PANEL_MODO_TOP;}
	SetPanelAttribute (ConfigHandle, ATTR_TOP,Top);
	SetPanelAttribute (ConfigHandle, ATTR_LEFT,Left);

}
/*********************************************************************************************
				QUITA DE LA MEMORIA LOS PANELES DEL PUERTO COM
**********************************************************************************************/
void pcom_descarga_paneles_del_puerto_com(void)
{
	if(ConfigHandle) DiscardPanel (ConfigHandle);
	if(MensajesHandle) DiscardPanel (MensajesHandle);
	//if(VentanitaHandle) DiscardPanel (VentanitaHandle);
	//if(EsperaHandle) DiscardPanel (EsperaHandle);
}
/*********************************************************************************************
			MUESTRA EN EL TEXTBOX DE ENTRADA LA CADENA QUE SE PASA COMO PARAMETRO
**********************************************************************************************/
void pcom_mostrar_datos_en_textbox_cadena_recibida (char CadenaMostrar[],int Enter)
{
	SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA,CadenaMostrar);
	if(Enter)SetCtrlVal (MensajesHandle, MENSAJES_CADENA_RECIBIDA,"\n");

}
/********************************************************************************************
	     FUNCIÓN PARA ABRIR EL PANEL CON EL TEXT BOX PARA ENVIAR UN COMANDO A MANO
*********************************************************************************************/
int CVICALLBACK cuadro_enviar (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	int Index,CodigoError;
	if(event==EVENT_RIGHT_CLICK && FlagPuertoAbierto && !VentanitaAbierta)
	{	
		//Carga el panel con el control para escribir el comando
		if ((VentanitaHandle = LoadPanel (0,"PanelesPuertoCom.uir", COMANDO)) < 0)
		return -1;
		//InstallPopup (VentanitaHandle);//Muestra el panel
		VentanitaAbierta=1;
		DisplayPanel(VentanitaHandle);
	}
/*
	if (event==EVENT_KEYPRESS && FlagPuertoAbierto)
	{
		switch (eventData1)
		{
			//case 768:// Tecla de escape
			//break;
			case 1280:// Tecla de enter
			
			Index=GetCtrlVal (MensajesHandle,MENSAJES_CADENA_ENVIAR ,CadenaComando);
			
			ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)
			break;
		}
	}
*/	
	return 0;
}
/********************************************************************************************
						FUNCIÓN PARA ENVIAR UN COMANDO A MANO
*********************************************************************************************/
int CVICALLBACK enviar_ventanita (int panel, int control, int event,
		void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal (VentanitaHandle,COMANDO_TEXTO ,CadenaComando);
			ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)
			break;
	}
	return 0;
}
/********************************************************************************************
	FUNCIÓN DEL PANEL CON EL TEXT BOX PARA ENVIAR UN COMANDO A MANO
********************************************************************************************/
int CVICALLBACK ventanita_comando (int panel, int event, void *callbackData,
		int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_CLOSE: 
			DiscardPanel (VentanitaHandle);
			VentanitaAbierta=0;//Para que pueda abrir de nuevo
			//HidePanel(VentanitaHandle);
			
			break;
		case EVENT_KEYPRESS:
			switch (eventData1)
			{
				case 768:// Tecla de escape
				HidePanel(VentanitaHandle);
				break;
				case 1280:// Tecla de enter
				GetCtrlVal (VentanitaHandle,COMANDO_TEXTO ,CadenaComando);
				ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)
				break;
			}
			
			break;
	}			   
	return 0;
}
/****************************************************************************************
				FUNCIONES DEL MENU CON COMANDOS SCPI COMUNES
			Envian por el puerto serie el comando seleccionado
			Además muestra en el panel MENSAJES las cadenas de salida
****************************************************************************************/
void CVICALLBACK error_scpi (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	sprintf(CadenaComando,"%s","ERR?"); 
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)
}

void CVICALLBACK version_scpi (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	sprintf(CadenaComando,"%s","*IDN"); 
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)
}
void CVICALLBACK cls_scpi (int menuBar, int menuItem, void *callbackData,
		int panel)
{
	sprintf(CadenaComando,"%s","*CLS"); 
	ENVIAR_COMANDO_AL_SISTEMA(NO_MOSTRAR)
}
/**************************************************************************************
 				LEE LOS PARAMETROS DE CONFIGURACION DEL PUERTO                            
 ***************************************************************************************/
void com_salva_parametros (void)
{
    GetCtrlVal (ConfigHandle, CONFIGURAP_TIMEOUT, &Timeout);
	GetCtrlVal (ConfigHandle, CONFIGURAP_RETARDO, &Retardo);
    GetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, &PuertoCOM);
    GetCtrlVal (ConfigHandle, CONFIGURAP_VELOCIDADPUERTO, &VelocidadPuerto);
    GetCtrlVal (ConfigHandle, CONFIGURAP_PARIDAD, &Paridad);
    GetCtrlVal (ConfigHandle, CONFIGURAP_BITDATOS, &BitsDatos);
    GetCtrlVal (ConfigHandle, CONFIGURAP_BITSTOP, &BitsDeStop);
    GetCtrlVal (ConfigHandle, CONFIGURAP_COLAENTRADA, &ColaDeEntrada);
    GetCtrlVal (ConfigHandle, CONFIGURAP_COLASALIDA, &ColaDeSalida);
    GetCtrlVal (ConfigHandle, CONFIGURAP_MODOCTS, &ModoCTS);
    GetCtrlVal (ConfigHandle, CONFIGURAP_MODOXONOFF, &ModoX);
	GetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORLECTURA, &TerminadorRecepcion);
    GetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORESCRITURA, &TerminadorTransmision);
}
/**************************************************************************************
			ESTABLECE LOS PARAMETROS DE CONFIGURACION DEL PUERTO                     
***************************************************************************************/
void com_establece_parametros (void)
{
	
	SetCtrlVal (ConfigHandle, CONFIGURAP_TIMEOUT, Timeout);
	SetCtrlVal (ConfigHandle, CONFIGURAP_RETARDO, Retardo);
    SetCtrlVal (ConfigHandle, CONFIGURAP_PUERTOCOM, PuertoCOM);
    SetCtrlVal (ConfigHandle, CONFIGURAP_VELOCIDADPUERTO, VelocidadPuerto);
    SetCtrlVal (ConfigHandle, CONFIGURAP_PARIDAD, Paridad);
    SetCtrlVal (ConfigHandle, CONFIGURAP_BITDATOS, BitsDatos);
    SetCtrlVal (ConfigHandle, CONFIGURAP_BITSTOP, BitsDeStop);
    SetCtrlVal (ConfigHandle, CONFIGURAP_COLAENTRADA, ColaDeEntrada);
    SetCtrlVal (ConfigHandle, CONFIGURAP_COLASALIDA, ColaDeSalida);
    SetCtrlVal (ConfigHandle, CONFIGURAP_MODOCTS, ModoCTS);
    SetCtrlVal (ConfigHandle, CONFIGURAP_MODOXONOFF,ModoX);
    SetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORLECTURA, TerminadorRecepcion);
	SetCtrlIndex (ConfigHandle, CONFIGURAP_TERMINADORESCRITURA, TerminadorTransmision);
	SetCtrlVal (ConfigHandle, CONFIGURAP_IDENTIFICACION, IDNinstrumento);				
}
/**************************************************************************************
				FUNCION QUE MUESTRA LOS ERRORES DEL PUERTO                  
/**************************************************************************************/
void com_display_error_RS232 (void)
{
	int Respuesta;
	char ErrorMessage[STRLARGO];
    switch (ErrorRS232)
        {
        case 0  :
            MessagePopup ("Mensaje RS232", "No hay errores.");
            break;
        case -1  :
            //For error code -1 (UnknownSystemError), call the GetErrorRS232String 
			//function to obtain a specific Windows message string. 
			Respuesta=ConfirmPopup ("Mensaje RS232",
				"¡El puerto se desconectó!\n¿Quiere configurarlo de nuevo?");
			if(Respuesta==1)
			{pcom_cierra_puerto_serie();
    		InstallPopup (ConfigHandle); // Muestra el panel de configuración
			}
            break;
		case -2 :
            Fmt (ErrorMessage, "%s", "número de puerto invalido (debe valer "
                                     "entre 1 y 255).");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
        case -3 :
            Fmt (ErrorMessage, "%s", "El puerto no está abierto.\n"
                 "Entre en el menú  Com/configurar ");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
		case -5 :
            Fmt (ErrorMessage, "%s", "Error inesperado");
            MessagePopup ("Mensaje RS232", ErrorMessage);
        break;
		case -6 :
            Fmt (ErrorMessage, "%s", "Puerto no encontrado");
            MessagePopup ("Mensaje RS232", ErrorMessage);
        break;
        case -99 :
            Fmt (ErrorMessage, "%s", "error de Timeout.\n\n"
                 "Incremente el valor de Timeout,\n"
                 "       Compruebe la configuración, or\n"
                 "       Compruebe el puerto.");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
			case ERROR_DATOS_EXCESIVOS :
            Fmt (ErrorMessage, "%s", "error, demasiados datos en el buffer.\n");
            MessagePopup ("Mensaje RS232", ErrorMessage);
            break;
		default :
            if (ErrorRS232 < 0)
                {  
                Fmt (ErrorMessage, "%s<RS232 error número %i", ErrorRS232);
                MessagePopup ("Mensaje RS232", ErrorMessage);
                }
            break;
        }
}
/******************************************************************************************/

	 
