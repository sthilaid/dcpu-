
env = Environment(CCFLAGS='-ggdb')
env.Program('dcpu', ['dcpu-main.cpp', 'dcpu.cpp', 'mem.cpp', 'decoder.cpp'], CPPPATH='.')
env.Program('dcpu-compiler', ['dcpu-compiler.cpp', 'decoder.cpp'], CPPPATH='.')
