@echo off
:: show PHP version
%~dp0\php\php -v
:: start boxing the Qt application
%~dp0\php\php build-boxed-scp.php
pause