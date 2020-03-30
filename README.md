#0: convert,disasm,...
evil disasm_x8664 xxx.bin
evil disasm_arm64 xxx.bin 0x1000 0x2000

#1: translate into graph network
evil learn xxx.c

#2: operate on graph network
evil search main
evil modify func@240 ...

#3: have fun with knowledge
evil serve :8080
evil graph func_42
