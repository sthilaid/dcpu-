
env = Environment(CCFLAGS='-ggdb')
corefiles = ['dcpu.cpp', 'dcpu-codex.cpp', 'dcpu-mem.cpp']
asmfiles = ['dcpu-lispasm.cpp']
hardwarefiles = ['dcpu-hardware.cpp'] + Glob('dcpu-hardware-*.cpp')

env.Program('dcpu', ['dcpu-main.cpp'] + corefiles + hardwarefiles,
            CPPPATH='.')
env.Program('dcpu-compiler', ['dcpu-compiler.cpp'] + corefiles + asmfiles, CPPPATH='.')
env.Program('dcpu-asm-test', ['dcpu-lispasm-test.cpp'] + asmfiles, CPPPATH='.')
env.Program('dcpu-test', ['dcpu-test.cpp'] + corefiles + asmfiles+ hardwarefiles,
            CPPPATH='.')
