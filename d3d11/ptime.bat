
@setlocal enableextensions disabledelayedexpansion
@echo off
if not [%1]==[] powershell -nol -noni -nop -c "measure-command { invoke-expression -command '%*' | write-host };"
