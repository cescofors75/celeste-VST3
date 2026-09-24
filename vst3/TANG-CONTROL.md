# Tang Control — primera integración (fuentes, pendiente de binario validado)

El efecto nativo se conserva. TANG CONTROL añade 44 parámetros de hardware independientes, automatizables y guardados en la sesión del DAW. No transforma la Tang en una interfaz de audio USB.

## Conexión

1. Cerrar CONNECTED en la web: COM14 sólo admite un propietario. Usar una sola instancia del plugin para cada Tang.
2. Abrir TANG CONTROL, indicar COM14 en Windows o el dispositivo /dev/cu.usbserial-… en macOS y pulsar CONNECT. No abre puertos automáticamente al cargar proyectos.
3. Elegir READ TANG para importar valores reales o SEND SESSION para aplicar los valores guardados del plugin. Conectar solo no escribe controles ni modifica el audio. Leer la Tang sustituye los parámetros hardware guardados en la instancia; enviar la sesión escribe los 44 controles.
4. Activar CONTROLLER ONLY si se desea que el audio del DAW pase intacto por el plugin mientras controla el equipo externo. Desactivado, el motor nativo continúa procesando la pista. Los presets nativos no son presets de la Tang.
5. Automatizar los parámetros que empiezan por Tang. Automatización de control a 20 Hz, no precisa a nivel de muestra ni sincronizada al transporte/BPM. READ TANG actualiza movimientos de los encoders físicos; no hay seguimiento bidireccional continuo en esta primera versión.

## Audio real

Salida de audio del ordenador/interfaz → PCM1808 Line-In → FPGA → PCM5102. Para grabar el retorno en el DAW se necesita una entrada de línea de una interfaz de audio. Evitar enviar ese retorno otra vez a la salida que alimenta la Tang. No hay retorno de audio USB ni compensación automática de latencia.

## Controles

Porcentajes normalizados iguales al protocolo web/HDMI, no los rangos del efecto nativo. Incluye Glitch, Filter, ambos delays, Wavefolder, VCA, LFO, Chaos, Chorus, Flanger, Crusher, Freeze, Tremolo, Auto-pan, Envelope, mezclas, Master, bypass, mute, solo y rutas. Freeze Hold por encima del umbral del firmware congela; no es una duración. El nivel audible depende de ruta habilitada y mezcla no nula.

Las ultimas filas de la interfaz presentan interruptores por componente y un selector Off/Parallel/Series para Delay 2. El DAW guarda estos grupos como mascaras enteras automatizables. Referencia de los bits:

- Routes: Dry=1, Glitch=2, Delay=4, Filter=8, Wavefolder=16, VCA=32.
- Delay 2 routing: 0 apagado, 1 paralelo, 3 serie desde Delay; 2 no habilita salida.
- Expanded routes: Chorus=1, Flanger=2, Crusher=4, Freeze=8, Tremolo=16, Auto-pan=32, Envelope→Filter=64.
- LFO/Chaos targets: Filter=1, VCA=2, ambos=3.
- Bypass mask: Glitch=1, Delay=2, Filter=4, Wavefolder=8, VCA=16, Delay2=32, LFO=64, Chaos=128.
- Expanded bypass: Chorus=1, Flanger=2, Crusher=4, Freeze=8, Tremolo=16, Auto-pan=32, Envelope=64.
- Mute/Solo: Glitch=1, Delay=2, Filter=4, Wavefolder=8, VCA=16, Delay2=32, Chorus=64, Flanger=128, Crusher=256, Freeze=512, Tremolo=1024, Auto-pan=2048, Dry=4096. Cero limpia la máscara.

## Arquitectura y pruebas

Puerto exclusivo a 3 Mbaud, protocolo CELESTE/1 con CRC16/CCITT, secuencia y tipo de respuesta comprobados, límites de tamaño/tiempo, cola de últimos valores por parámetro. Hilo serie independiente: processBlock no toca el puerto ni espera respuestas. Al perder conexión no reconecta ni reenvía un patch automáticamente. Sólo acepta la versión Line-In ampliada verificada (capabilities 59391). No cambia firmware, clocks ni cableado.

Windows: compilación de objetos C++ verificada; Device Guard bloquea ar.exe en el enlazado. No hay aún VST3 nuevo validado ni prueba física de este controlador. macOS: implementación termios/IOSSIOSPEED incluida, pendiente de compilación y prueba.

Después de compilar: host_test.py prueba identidad del audio en Controller Only y guardado de parámetros, además de regresión DSP existente. CelesteTangCheck COM14 comprueba identificación, lectura de los 44 controles, exclusividad, cambio de un LSB de feedback y restauración con lectura real. Requiere cerrar la web. No se afirma que esas pruebas hayan pasado antes de ejecutarlas.
