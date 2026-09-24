# CELESTE Parallel — Native VST3 0.1

Efecto estéreo para Windows x64, independiente de la Tang Nano, USB y navegador.

## Instalar y probar

1. Copia **la carpeta completa `CELESTE Parallel.vst3`**, conservando `Contents`, a `C:\Program Files\Common Files\VST3` (Windows puede pedir permisos de administrador).
2. Abre tu DAW, vuelve a escanear los plugins e inserta **CELESTE Parallel** como efecto en una pista estéreo que tenga audio.
3. Reproduce la pista y selecciona **Celestial Bloom** para ambient, **Prism Cascade** para ecos rítmicos o **Neon Dust** para una textura más agresiva.
4. Compara con **BYPASS**. **MASTER MIX** controla la mezcla original/procesada. Empieza con un volumen de escucha moderado.

No es un sintetizador: necesita audio de entrada. No necesita la FPGA. La aplicación Standalone incluida permite probarlo con un dispositivo de entrada/salida; para una pista de audio, el DAW es la opción más sencilla.

## Controles

| Control | Función |
| --- | --- |
| Delay A / B | Tiempos independientes: 20–1200 / 20–1600 ms, con pequeñas diferencias entre L/R |
| Feedback | Realimentación compartida, hasta 85%; filtrada y suavemente saturada |
| Filter | Filtro paso bajo de la rama Wavefolder, entre 80 Hz y 18 kHz |
| Wavefolder | Cantidad e intensidad del plegado de onda; sobremuestreo 2× |
| Space | Reverberación estéreo alimentada por la suma de las ramas procesadas |
| Motion | Profundidad del LFO sobre el filtro y la amplitud del bus procesado |
| Master Mix | Mezcla original/procesada; 0% devuelve el original a la ganancia de salida elegida |
| LFO Rate | Velocidad libre entre 0,05 y 8 Hz |
| Output | Ganancia final; BYPASS vuelve a unidad |
| Series A → B | Alimenta Delay B desde Delay A; desactivado, ambos reciben la entrada |

Doble clic en un mando restaura su valor inicial. Puedes escribir valores en las casillas, automatizar los parámetros desde el DAW y guardar el estado dentro de la sesión. Los cambios de parámetros y de ruta se suavizan durante 40 ms. Cambiar el tiempo de delay puede producir un deslizamiento de tono intencionado.

## Qué significa Parallel Fabric aquí

Tres ramas lógicas procesan la misma fuente: Delay A, Delay B y Wavefolder → Filter. Su suma recibe modulación de amplitud y alimenta Space. Existe además una ruta seca. SERIES sustituye las dos ramas de delay por la cascada A → B; conserva la rama Wavefolder/Filter y la ruta seca.

El procesamiento se ejecuta en la CPU del ordenador. La topología paralela es real en el audio, pero esta versión no afirma simultaneidad física de FPGA. Las ondas son firmas visuales animadas; las barras miden picos del motor. Los medidores internos pueden seguir mostrando actividad durante bypass porque las colas internas continúan procesándose.

## Alcance de esta primera versión

Cinco presets y una topología fija con conmutación paralelo/serie. No incluye todavía editor libre de cables, duplicación arbitraria de módulos, Glitch, Chaos, sincronía BPM, entrada mono ni control de hardware. VST3 de 64 bits para Windows; no sirve como binario para macOS. No equivale a una certificación de compatibilidad con todos los DAW.

## Compilar el código

Requiere CMake 3.22+, C++17 y JUCE **7.0.12**. Se ha compilado con MinGW-w64; también puede configurarse con un toolchain compatible de Visual Studio.

```powershell
git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git JUCE
cmake -S source -B build -DJUCE_SOURCE_DIR="$PWD/JUCE"
cmake --build build --config Release
```

`CelesteCheck` ejecuta pruebas del motor y puede renderizar un WAV: `CelesteCheck.exe entrada.wav salida.wav`. `CelestePreview` renderiza una imagen del editor sin necesitar un DAW. `host_test.py` comprueba el VST3 usando Python, NumPy y Pedalboard.

## Dependencias

JUCE 7.0.12 y su SDK VST3 integrado. Se adjuntan los avisos de JUCE y Steinberg. Este paquete se entrega como prototipo local con código fuente; antes de distribuirlo públicamente, aplica la modalidad de licencia correspondiente de JUCE. La licencia del resto del proyecto CELESTE no se modifica.

## Integracion Tang en desarrollo

La pagina TANG CONTROL y su estado de validacion se describen en [TANG-CONTROL.md](TANG-CONTROL.md). Los binarios antiguos no incluyen esta funcion.
