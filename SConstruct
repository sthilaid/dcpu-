
coreenv = Environment(CCFLAGS='-ggdb', CPPPATH=['.'])
env = Environment(CCFLAGS='-ggdb', CPPPATH=['.'], LIBS=['dcpu-core'], LIBPATH=['.'])
hardware_env = Environment(CCFLAGS='-ggdb', CPPPATH=['.', '/usr/include/SDL2'], LIBS=['dcpu-core', 'SDL2'], LIBPATH=['.', '/usr/lib'])
corefiles = ['dcpu.cpp', 'dcpu-codex.cpp', 'dcpu-mem.cpp', 'dcpu-tokenizer.cpp', 'dcpu-sexp.cpp']
asmfiles = ['dcpu-lispasm.cpp']
hardwarefiles = ['dcpu-hardware.cpp'] + Glob('dcpu-hardware-*.cpp')

coreenv.Library('dcpu-core', corefiles);

env.Program('dcpu-compiler', ['dcpu-compiler.cpp'] + asmfiles)
env.Program('dcpu-asm-test', ['dcpu-lispasm-test.cpp'] + asmfiles)
hardware_env.Program('dcpu', ['dcpu-main.cpp'] + hardwarefiles)
hardware_env.Program('dcpu-test', ['dcpu-test.cpp'] + asmfiles+ hardwarefiles)
