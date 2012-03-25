EnsurePythonVersion(2, 6)
EnsureSConsVersion(2, 0)

def pch(target, source, env, for_signature):
    includes = ''
    for path in env['CPPPATH']:
       includes += ' -I' + path
    return 'g++ -o '+source[0].name+'.gch -c -std=c++0x -g -ggdb -DBOOST_PP_VARIADICS_MSVC=0 -DDEBUG_BUILD=1 '+includes+' '+source[0].name

    # return ' '.join(['g++ -o ', source[0].name+'.gch', '-c -std=c++0x', includes, source[0].name])

bld = Builder(generator = pch)
env = Environment(BUILDERS = {'PrecompileHeader' : bld})

env.Append(CPPPATH = ['./', '/opt/boost/include/', '/usr/include/', ])
env.Append(LIBPATH = [])

env.Append(CPPDEFINES = {'DEBUG_BUILD':1,
                         'BOOST_PP_VARIADICS_MSVC':0,
                        })

env.Append(CPPFLAGS = Split('-std=c++0x -g -ggdb'))
env.Append(CPPFLAGS = Split('-W -Wall -Wextra -pedantic -Woverloaded-virtual -Wnon-virtual-dtor -Wpointer-arith'))
env.Append(CPPFLAGS = Split('-Wno-long-long -Wundef -Wcast-align -Wcomment -Winit-self -Wchar-subscripts'))

env.Append(LIBS = Split('SDL SDL_mixer SDL_image GL GLU'))

files = Glob("*.cpp")
for directory in ['editor', 'entity', 'gui', 'math', 'video']:
    files = files + Glob(directory + "/*.cpp")

env.PrecompileHeader('A-make-gch-first', 'StdAfx.h')
env.Program("game", files)

