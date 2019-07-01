call e:\ow19\owsetenv.cmd
e:
cd rxmous
rem Rxmous compile and link Openwatcom 1.9
wcc386 -wx -bd -d3 Rxmous.c
wlink @Rxmous
