
env = Environment(CCFLAGS='-ggdb')
env.Program('dcpu', ['dcpu-main.cpp', 'dcpu.cpp', 'dcpu-mem.cpp', 'dcpu-codex.cpp', 'dcpu-hardware.cpp'], CPPPATH='.')
env.Program('dcpu-compiler', ['dcpu-compiler.cpp', 'dcpu-codex.cpp', 'dcpu-lispasm.cpp'], CPPPATH='.')
env.Program('dcpu-asm-test', ['dcpu-lispasm-test.cpp', 'dcpu-lispasm.cpp'], CPPPATH='.')
env.Program('dcpu-test', ['dcpu-test.cpp', 'dcpu.cpp', 'dcpu-codex.cpp', 'dcpu-mem.cpp', 'dcpu-lispasm.cpp', 'dcpu-hardware.cpp'], CPPPATH='.')
