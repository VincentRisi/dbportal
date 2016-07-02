# dbportal - Oracle Database Portal Domain Specific Language

This is a code generator using a middle out or star mechanism. This allows for it being mostly computer language independant, as well as not caring what operating system it is run on. It is however Oracle centric.
The code is written using Bison (Yacc) parser and C++. The lex token parser is hand coded in C, this was done due to memory size constraints in the 16 bit era of Intel. This was in the days when Gates was reputed to say **"Who will ever need more than 640K?"**. The Oracle Call Interface is mostly used as the access interface to the database. There is also some code generation that use ADO.Net and a smattering of proC code generation.

## Tools

CMake is used to build the dbportal utility tools.
There is a [common cmake](cmake/dbportal.cmake) [main](CMakeLists.txt) [pocilib](pocilib/CMakeLists.txt)