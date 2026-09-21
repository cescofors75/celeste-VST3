# CELESTE Parallel — macOS

VST3 y Audio Unit (AU), con la misma interfaz y motor de Windows. Universal: Apple Silicon + Intel; objetivo mínimo macOS 11. El AU permite usarlo en Logic Pro. El plugin necesita una pista estéreo con audio.

**Descarga:** usa el artefacto `CELESTE-Parallel-macOS-Universal` de una ejecución verde de GitHub Actions. Contiene los plugins compilados y los logs de validación. El ZIP denominado `macOS-source` contiene únicamente fuentes.

## Compilar en tu Mac

Necesitas las herramientas de desarrollo de Apple y CMake 3.22 o superior. En Terminal, dentro de esta carpeta:

```sh
bash build-macos.command
```

El script descarga JUCE 8.0.12, compila VST3, AU y la aplicación, ejecuta las pruebas DSP y comprueba que los tres binarios contienen arm64 y x86_64. Deja los resultados en `mac-release` y crea `CELESTE-Parallel-macOS-Universal.zip`. Los fallos detienen el proceso; no se anuncia éxito si las pruebas fallan.

Para comprobar el VST3 mediante un host real, instala NumPy y Pedalboard en un entorno Python local y ejecuta:

```sh
python3 -m venv .mac-test-env
source .mac-test-env/bin/activate
python -m pip install numpy pedalboard
python host_test.py "mac-release/CELESTE Parallel.vst3"
```

## Instalar después de compilar

Copia las carpetas completas, conservando `Contents`:

- `CELESTE Parallel.vst3` → `~/Library/Audio/Plug-Ins/VST3/`
- `CELESTE Parallel.component` → `~/Library/Audio/Plug-Ins/Components/` (Logic y otros hosts AU).

Vuelve a abrir el DAW, reescanea los plugins y selecciona CELESTE Parallel. En Logic puedes comprobar el AU instalado con `auval -v aufx Cpf1 Clst` desde Terminal. Prueba inicialmente Parallel Dreams o Celestial Bloom.

Este prototipo no incluye certificado Apple Developer ID ni notarización para distribución pública. No cambia Gatekeeper ni borra atributos de seguridad. Los logs del paquete indican las pruebas ejecutadas en el Mac de compilación; la prueba manual en tu DAW sigue siendo necesaria. Comprobar ambas arquitecturas con `lipo` no sustituye ejecutarlas en ambas.

Las funciones, controles y límites del prototipo se describen en `README.md`. Las pruebas Windows de `VALIDATION.md` no certifican macOS.
