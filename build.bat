@echo off
REM Build portable core + selftest (MinGW). STM32 port is Cube project only.
set CC=gcc
set CFLAGS=-std=c99 -Wall -Wextra -O2 -Iconfig -Icore/include -Iport/include

%CC% %CFLAGS% -c core/src/modbus_types.c -o core/src/modbus_types.o
%CC% %CFLAGS% -c core/src/modbus_crc.c   -o core/src/modbus_crc.o
%CC% %CFLAGS% -c core/src/modbus_rtu.c   -o core/src/modbus_rtu.o
%CC% %CFLAGS% -c core/src/modbus_map.c   -o core/src/modbus_map.o
%CC% %CFLAGS% -c core/src/modbus_master.c -o core/src/modbus_master.o
%CC% %CFLAGS% -c core/src/modbus_slave.c  -o core/src/modbus_slave.o

ar rcs libmodbus_rtu_core.a core/src/modbus_types.o core/src/modbus_crc.o core/src/modbus_rtu.o core/src/modbus_map.o core/src/modbus_master.o core/src/modbus_slave.o

%CC% %CFLAGS% examples/selftest.c -L. -lmodbus_rtu_core -o selftest.exe
echo.
selftest.exe
