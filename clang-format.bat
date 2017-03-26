@echo off

FOR /f %%f IN ('DIR /s/b "src\*.h", "src\*.cpp"') DO (
    ECHO Formatting %%f 
    clang-format.exe -i %%f   
)