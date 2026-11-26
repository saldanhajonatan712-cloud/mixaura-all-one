MIX AURA ALL-IN-ONE - Projeto VST3 (JUCE / C++ / CMake)
=======================================================

O pacote Windows gerado pelo GitHub Actions contem o binario VST3 compilado
para Windows x64. O pacote deve ser baixado na pagina Actions do repositorio.

REQUISITOS (Windows 10/11)
- Visual Studio 2022 com o workload "Desenvolvimento para desktop com C++"
- CMake 3.22 ou superior (ja vem com o Visual Studio 2022)
- Git (o CMake usa para baixar o JUCE 8 automaticamente)
- Internet na primeira compilacao

COMO COMPILAR
Opcao A - Clique duplo em build_windows.bat
Opcao B - No "Developer PowerShell for VS 2022", dentro desta pasta:
    cmake --preset vs2022
    cmake --build --preset release
Opcao C - Visual Studio 2022 > Arquivo > Abrir > Pasta... (abre o CMakeLists.txt direto)

RESULTADO
    build\MixAura_artefacts\Release\VST3\MIX AURA ALL-IN-ONE.vst3
(tambem gera uma versao Standalone em build\MixAura_artefacts\Release\Standalone)

INSTALAR NO FL STUDIO
1. Copie a pasta "MIX AURA ALL-IN-ONE.vst3" para:
       C:\Program Files\Common Files\VST3\
2. FL Studio > Options > Manage plugins > Find installed plugins
3. No Mixer, clique num slot vazio e escolha "MIX AURA ALL-IN-ONE"

ESTRUTURA
  CMakeLists.txt, CMakePresets.json, build_windows.bat
  Source\Params.h              - parametros automatizaveis
  Source\PluginProcessor.*     - DSP: gate, EQ, de-esser, transientes, saturacao,
                                 compressor, stereo width M/S, chorus, phaser,
                                 delay sincronizado ao BPM, reverb, limiter, medidores, presets
  Source\PluginEditor.*        - interface (knobs, espectro, medidores, redimensionavel)
  Source\AvatarComponent.*     - avatar dancarino (desenho vetorial 2D, 5 dancas no BPM do DAW)
