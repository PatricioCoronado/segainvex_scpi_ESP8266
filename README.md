# segainvex_scpi_ESP8266

Librería para controlar un ESP8266 (plataforma Arduino)con un PC.

El PC envía comandos al ESP8266 y este ejecuta funciones definidas en la 
aplicación como respuesta a los comandos. Los comandos pueden incluir 
parámetros. 

Para probar la librería se incluye una aplicación de Pc escrito con el entorno 
LabWindowsCVI de "National Instruments". Para que funcione tiene que estar instalada
el "runtime engine" de LabWindows™/CVI. 

http://www.ni.com/es-es/support/downloads/software-products/download.labwindows-cvi-runtime.html

Hay que compilar y cargar el ejemplo test_segainvex_scpi_ESP8266 en el ESP8266.

Posteriormente se ejecuta Plantilla_CVI.exe. (también se aporta el código fuente)
Este código incluye una versión de la librería PuertoCOM para utilizar el puerto
COM en el que se conecte el ESP8266 de forma sencilla. Una versión actualizada
de esta librería para LabWindows CVI se puede encontrar en:
https://github.com/PatricioCoronado/Virtual-instrumentation-Arduino-LabWindows-CVI

En el textbox superior de la ventana "comunicacion" se clica con el boton derecho
del ratón y aparece una ventana "enviar comando" en la que se puede escribir un
comando que se enviará al ESP8266.
