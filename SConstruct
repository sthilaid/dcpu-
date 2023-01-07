
env = Environment(CCFLAGS='-ggdb', CPPPATH=['.', '/usr/include/SDL2'], LIBS=['SDL2'], LIBPATH=['/usr/lib'])
corefiles = ['dcpu.cpp', 'dcpu-codex.cpp', 'dcpu-mem.cpp', 'dcpu-tokenizer.cpp', 'dcpu-sexp.cpp']
asmfiles = ['dcpu-lispasm.cpp']
hardwarefiles = ['dcpu-hardware.cpp'] + Glob('dcpu-hardware-*.cpp')

env.Program('dcpu', ['dcpu-main.cpp'] + corefiles + hardwarefiles)
env.Program('dcpu-compiler', ['dcpu-compiler.cpp'] + corefiles + asmfiles)
env.Program('dcpu-asm-test', ['dcpu-lispasm-test.cpp'] + corefiles + asmfiles)
env.Program('dcpu-test', ['dcpu-test.cpp'] + corefiles + asmfiles+ hardwarefiles)
