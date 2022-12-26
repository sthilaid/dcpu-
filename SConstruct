
env = Environment(CCFLAGS='-ggdb')
env.Program('dcpu', ['dcpu-main.cpp', 'dcpu.cpp', 'mem.cpp', 'decoder.cpp'], CPPPATH='.')
env.Program('dcpu-compiler', ['dcpu-compiler.cpp', 'decoder.cpp', 'dcpu-lispasm.cpp'], CPPPATH='.')
env.Program('dcpu-asm-test', ['dcpu-lispasm-test.cpp', 'dcpu-lispasm.cpp'], CPPPATH='.')
