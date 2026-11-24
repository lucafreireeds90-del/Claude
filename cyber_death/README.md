# Cyber Death (app para Flipper Zero)

App externo (.fap) em C com menu e três telas: **Contador**, **Gerador de senha** e **Sobre**.
Serve de base para você adicionar seus próprios módulos.

## Como compilar e instalar

```bash
git clone --recursive https://github.com/flipperdevices/flipperzero-firmware.git
cp -r cyber_death flipperzero-firmware/applications_user/
cd flipperzero-firmware
./fbt fap_cyber_death          # gera build/.../cyber_death.fap
./fbt launch APPSRC=applications_user/cyber_death   # com o Flipper via USB
```

Ou copie o `.fap` para `SD Card/apps/Tools/` pelo qFlipper.

## Controles
- Menu: Cima/Baixo navega, OK abre, Voltar sai.
- Contador: Cima +1, Baixo -1, OK zera.
- Senha: OK gera outra (12 caracteres, aleatório do hardware).

## Observação
Não testei em um aparelho real nem compilei com o SDK aqui; se o seu "Cyber Death" usar
outro firmware/hardware, me diga qual para adaptar.
