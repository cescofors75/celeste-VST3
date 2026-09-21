# Validación — 2026-09-21

Compilación Release Windows x64: JUCE 7.0.12, GCC/MinGW-w64 16.2, CMake 4.4.3.

## Pruebas realizadas

- Motor DSP a 44.1, 48 y 96 kHz: identidad exacta con mezcla seca, bypass exacto incluso con Output a +6 dB, silencio sin señal generada, muestras finitas y salida acotada con realimentación y efectos al máximo.
- Cambios reales de señal comprobados para Wavefolder, filtro, Motion, Space y topología de los delays. No son únicamente cambios visuales.
- Bloques de 1024 muestras y cambios de topología durante procesamiento; internamente el motor divide bloques grandes en tramos de hasta 512.
- Binario VST3 final cargado con Pedalboard 0.9.25. Procesamiento estéreo a las tres frecuencias anteriores, bloques de 64, 257, 512 y 1024 según prueba; parámetros enumerados, estado guardado/restaurado y diferencia de respuesta entre serie y paralelo.
- Render a 96 kHz de diez segundos de audio en aproximadamente 0.92 segundos en esta máquina. Es una medida offline, no una garantía de carga CPU en cualquier DAW.
- Demo de 54 segundos renderizada mediante el VST3 con el preset Celestial Bloom, incluyendo seis segundos de cola y un fundido final. Pico absoluto 0.443, sin NaN ni infinito.
- Editor real renderizado e inspeccionado: ocho mandos, unidades, rutas, preset selector, bypass y colores. Las barras usan medidas del motor; las ondas son decorativas.
- Dependencias del módulo inspeccionadas: DLL del sistema Windows; runtimes GCC enlazados estáticamente.

Los logs adjuntos conservan los resultados. `host_test.py` recibe la carpeta VST3 y abre su módulo `Contents/x86_64-win` porque el escáner de Pedalboard para Windows requiere el archivo.

## Límites

No se ha hecho una sesión manual en un DAW comercial ni una prueba prolongada en directo. La automatización extrema no está verificada exhaustivamente en todos los hosts. El editor tiene tamaño fijo de 1180 x 760. La aplicación Standalone está compilada, pero no se ha probado con una interfaz física de audio.

La política Control de aplicaciones de Windows bloqueó algunos ejecutables auxiliares de compilación/captura. Se usaron recursos de versión estáticos para compilar. El módulo VST3 final sí se cargó y procesó audio en las pruebas. No se modificó la política de seguridad del equipo. La captura incluida procede de la ejecución correcta anterior del editor final; la recompilación posterior del auxiliar, que solo corrige cómo sobrescribe el PNG, fue bloqueada.
