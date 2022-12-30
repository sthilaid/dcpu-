# DCPU-16 implementation in c++

DCPU-16 implemented following the 1.7 specifications, such as found
https://github.com/lucaspiller/dcpu-specifications/blob/master/dcpu16.txt.

Contains several executables:

- dcpu-compiler <lasm-source> <bin-outputfile>: Will compile a lisp assembly
  file. These file use a lisp-like syntax to represent the dcpu instructions,
  and provide access to labels, for example:

```
  (set push 10)
  (jsr sqr)
  (add 5 peek 1)
  (set pc hlt)
  
  (label sqr)
  (add 5 (ref peek 1))
  (set pc pop)

  (label hlt)
```

- dcpu <bin-file>: Will run the dcpu emulator on the binary source file (loaded
  at address 0x0) and then outputs the cpu state and the bottom of the stack.

- dcpu-asm <lasm-file>: Tests parsing lisp assembly and outputs the read
  instructions AST.

- dcpu-test [test-name]: Will run all the implemented unit tests on dcpu
  emulator. If a test name is provided, it will only run that specific test.

Currently implements some harware as well:
  
- TestDevice: Simple hardware device that is used to unit test some hardware
  special instruction, like "int".

- Clock: Genric clock implementation following the specificiation
  https://github.com/lucaspiller/dcpu-specifications/blob/master/clock.txt