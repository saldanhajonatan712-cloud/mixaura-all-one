@echo off
cmake --preset vs2022 || goto :err
cmake --build --preset release || goto :err
echo.
echo OK: build\MixAura_artefacts\Release\VST3\MIX AURA ALL-IN-ONE.vst3
pause
exit /b 0
:err
echo Falha na compilacao. Veja as mensagens acima.
pause
