/*
  ESP8266_SCPI_test
  Patricio Coronado.  enero 2019.  
  Ejemplo de uso de la librería segainvex_scpi_ESP8266 para comunicar un PC con 
  ESP8266 a través del puerto serie.
  Estatus:  En pruebas
  NOTAS:
  
 */
#include <Arduino.h>
//....... SECCION PARA INCLUIR SEGAINVEX_SCPI_ESP8266...........................
#include <segainvex_scpi_ESP8266.h> // funciones y variables de segainvex_scpi_ESP8266
//Submenú
 tipoNivel SUBMENU1[] = //Array de estructuras tipo Nivel
{
	0,"COMANDO11","C11",11,NULL,
  0,"COMANDO12","C12",12,NULL,
};
 //Menú principal
 tipoNivel NivelDos[] = //Array de estructuras tipo Nivel
{
  0,"*IDN","*IDN",240,NULL, // Por compatibilidad identifica el instrumento	
	0,"ERROR","ERR",243,NULL, // Envia el último error	
	0,"*OPC","*OPC",248,NULL, // Por compatibilidad	devuelve un 1 al PC
	0,"*CLS","*CLS",255,NULL, // Borra la pila de errores
	0,"COMANDO1","C1",1,NULL, // Comando del nivel principal
  0,"COMANDO2","C2",2,NULL, // Comando del nivel principal 
  2,"SUBMENU1","SM1",NULL,SUBMENU1 //SUBMENU tiene 2 comandos  
};
//Menú raiz
tipoNivel  Raiz[]={7,"","",NULL,NivelDos};//El NivelDos[] tiene 7 comandos o submenús 
// Array de cadenas con información del error numeradas
tipoCodigoError CodigoError=
{
  // Errores del sistema SCPI 0...6
  " ",						// ERROR N. 0
  "1 Caracter no valido",                // ERROR N. 1
  "2 Comando desconocido",               // ERROR N. 2
  "3 Cadena demasiado larga",            // ERROR N. 3
  "4 Parametro inexistente",             // ERROR N. 4
  "5 Formato de parametro no valido",    // ERROR N. 5
  "6 Parametro fuera de rango",          // ERROR N. 6
  // Errores personalizados por el usuario 
  "7 El dato no esta listo",		// ERROR N. 7
};	
/*******************************************************************
    SWITCH para llamar a las funciones de comando de SCPI.
    Esta función forma parte deo sistema SCPI. Tiene que definirse
    en la aplicación del usuario.
********************************************************************/
void EjecutaComandos(unsigned char n)
{
  switch (n) 
  {
    case 1:
      comando1();
      break;
    case 2:
      comando2();
      break;
    case 11:
      comando11();
      break;
    case 12:
      comando12();
      break;
    // Comandos comunes a todos los sistemas
    case 240:
      idn();  //Identifica al sistema con una cadena
      break;
    case 243:
      err();  //Envía al PC el código y la cadena del último error
      break;
    case 248:
      opc();  //Devuelve 1 al PC
      break;
    case 255:
      cls();  //Limpia la pila de errores
      break;
  }
}
//...... FIN SECCION PARA INCLUIR SEGAINVEX_SCPI_ESP8266........................
//Ahora el código habitual de Arduino:
/**************************************************************
                            SETUP 
**************************************************************/
void setup() 
{
//Cadena para identificar el sistema ante el PC
NOMBRE_DEL_SISTEMA_64B(Test de la libreria segainvex_scpi_ESP8266 para Arduino.)
//Primero hacemos configuraciones previas y luego abrimos el puerto serie.
// TO DO: Configuraciones de la apllicación 
//
  Serial.begin(115200); 
  delay(1000);//Para que el puerto haga su tráfico inicial típico del ESP8266
  Serial.flush();
}
/****************************************************************
                            LOOP
*****************************************************************/
void loop()
{
  // Si recibe algo por el puerto serie lo procesa con segainvex_scpi_ESP8266
  if (Serial.available()){scpi();}
/*
TO DO: Poner aquí el código de usuario
*/
}// loop() 

/****************************************************************
 *          DEFINICION DE LAS FUNCIONES SCPI
*****************************************************************/

/****************************************************************
 *  Función del comando 1 COMANDO1 ó C1
*****************************************************************/
void comando1(void)
{
 Serial.println
 ("Se ha recibido el COMANDO1 y se ha ejecutado la funcion fs1");	
}
/****************************************************************
 *  Función del comando 2 COMANDO2 ó C2
*****************************************************************/
void comando2(void)
{
 Serial.println
 ("Se ha recibido el COMANDO2 y se ha ejecutado la funcion fs2");	
}
/****************************************************************
	Comando SUBMENU1:COMANDO11 ó SM1:C11
	Cambia el valor de la variable tipo double "Variable1"
*****************************************************************/
void comando11(void)
{
	static double Variable1=1.1;
	int Resultado;
	Resultado=cambia_variable_double_del_sistema(&Variable1,10.0,0.0);
	switch (Resultado)
	{
		case 0:
			Serial.println("No se cambio el valor de la Variable1");
			errorscpi(7);
		break;
		case 1:
			Serial.println("Se cambio el valor de la Variable1");
		break;
		case 2:
			Serial.println("Se envio al PC el valor de la Variable1");
		break;
	}
}
/*****************************************************************
	Comando SUBMENU1:COMANDO12 ó SM1:C12
	Ejecuta la función void fs4(void);
	Cambia el valor de la variable tipo int "Variable2"
*****************************************************************/
void comando12(void)
{
	int Resultado;
	static int Variable2=100;
	int ValoresVariable2[]={1,10,100};
	Resultado=cambia_variable_int_discreta_del_sistema(&Variable2,
ValoresVariable2,sizeof(ValoresVariable2));
	switch (Resultado)
	{
		case 0:
			Serial.println("No se cambio el valor de la Variable2");
		errorscpi(7);
		break;
		case 1:
			Serial.println("Se cambio el valor de la Variable2");
		break;
		case 2:
			Serial.println("Se envio al PC el valor de la Variable2");
		break;
	}	
}
/****************************************************************
 *          DEFINICION DE LAS FUNCIONES DE LA APLICACION
*****************************************************************/
/*
void hola_mundo(void)
{
  Serial.println("¡Hola Mundo!");
}
*/
/************************************/ /*FIN***************************/
