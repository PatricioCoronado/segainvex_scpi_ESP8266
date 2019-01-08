# segainvex_scpi_ESP8266

Librería para controlar un ESP8266 (plataforma Arduino)con un PC.
El PC envía comandos al ESP8266 y este ejecuta funciones definidas en la 
aplicación como respuesta a los comandos. Los comandos pueden incluir 
parámetros. 

Se incluye software de Pc escrito con LabWindowsCVI de "National Instruments".

Este software tiene como finalidad probar la librería segainvex_scpi_ESP8266.
Para que funcione tiene que estar instalada el "runtime engine" de LabWindows™/CVI. 

http://www.ni.com/es-es/support/downloads/software-products/download.labwindows-cvi-runtime.html

Hay que compilar y cargar el ejemplo test_segainvex_scpi_ESP8266 en el ESP8266.
Posteriormente se ejecuta Plantilla_CVI.exe. (también se aporta el código fuente)
En el textbox superior de la ventana "comunicacion" se clica con el boton derecho
del ratón y aparece una ventana "enviar comando" en la que se puede escribir un
comando que se enviará al ESP8266.
