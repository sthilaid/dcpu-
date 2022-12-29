
env = Environment(CCFLAGS='-ggdb')
env.Program('dcpu', ['dcpu-main.cpp', 'dcpu.cpp', 'mem.cpp', 'decoder.cpp', 'dcpu-hardware.cpp'], CPPPATH='.')
env.Program('dcpu-compiler', ['dcpu-compiler.cpp', 'decoder.cpp', 'dcpu-lispasm.cpp'], CPPPATH='.')
env.Program('dcpu-asm-test', ['dcpu-lispasm-test.cpp', 'dcpu-lispasm.cpp'], CPPPATH='.')
env.Program('dcpu-test', ['dcpu-test.cpp', 'dcpu.cpp', 'decoder.cpp', 'mem.cpp', 'dcpu-lispasm.cpp', 'dcpu-hardware.cpp'], CPPPATH='.')
