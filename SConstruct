
core_env = Environment(CCFLAGS='-ggdb', CPPPATH=['.'], LIBS=[], LIBPATH=[])
corefiles = ['dcpu.cpp', 'dcpu-codex.cpp', 'dcpu-mem.cpp',
             'dcpu-tokenizer.cpp', 'dcpu-sexp.cpp', 'dcpu-lispasm.cpp', 'dcpu-lisp.cpp']

compiler_env = core_env.Clone()
compiler_env['LIBS'] += ['dcpu-core']
compiler_env['LIBPATH'] += ['.']

hardware_env = compiler_env.Clone()
hardware_env['CPPPATH'] +=['/usr/include/SDL2']
hardware_env['LIBS'] += ['SDL2']
hardware_env['LIBPATH'] += ['/usr/lib']
hardwarefiles = ['dcpu-hardware.cpp'] + Glob('dcpu-hardware-*.cpp')

core_env.Library('dcpu-core', corefiles);
core_env.Library('dcpu-lispasm', corefiles);

compiler_env.Program('dcpu-compiler', ['dcpu-compiler.cpp'])
compiler_env.Program('dcpu-decoder', ['dcpu-decoder.cpp'])
compiler_env.Program('dcpu-asm-test', ['dcpu-lispasm-test.cpp'])
hardware_env.Program('dcpu', ['dcpu-main.cpp'] + hardwarefiles)
hardware_env.Program('dcpu-test', ['dcpu-test.cpp'] + hardwarefiles)

