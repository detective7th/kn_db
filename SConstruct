#! /usr/bin/python3
# -*- coding:utf-8 -*-

import os
import pathlib
import subprocess
import re

release = False

root_dir = os.getcwd()

install_dir = os.path.join(root_dir, "debug")
cflags_debug = '-Wall -std=c++1z -finline-functions -Wno-deprecated -g'
cflags_release = '-Wall -std=c++1z -finline-functions -Wno-deprecated -O3'
global_cppdefines = ['DEBUG_']

protoc_bin_path = ""
fast_type_gen_bin_path = ""

config_service_ws_srv_compress = False
config_service_config_file_force_update = False

lib_path = ["/usr/local/lib", install_dir]

env = Environment(CCFLAGS = cflags_debug, LIBPATH = lib_path, CPPPATH = [root_dir]
                  , CPPDEFINES = global_cppdefines)

for key, value in ARGLIST:
    if 'lib_path' == key:
        env.AppendUnique(LIBPATH = [value])
        env.AppendUnique(RPATH = [value])
    elif 'release' == key:
        if 0 != value:
            env.Replace(CCFLAGS = cflags_release)
            install_dir = os.path.join(root_dir, "release")
            del global_cppdefines[0]
            env.Replace(CPPDEFINES = global_cppdefines)
            del lib_path[1]
            lib_path.append(install_dir)
            env.Replace(LIBPATH = lib_path)
    elif 'include_path' == key:
        env.AppendUnique(CPPPATH = value)

env.Decider('MD5-timestamp')

if not os.path.exists(install_dir): os.makedirs(install_dir)

class Version(object):
    def __init__(self, version_string):
        version_pattern = re.compile(r"\.")
        main_ver = 0
        minor_ver = 0
        maintain_ver = 0
        try:
            main_ver, minor_ver, maintain_ver = re.split(version_pattern, version_string, maxsplit = 3)
        except Exception, e:
            pass

        self.main = int(main_ver)
        self.minor = int(minor_ver)
        self.maintain = int(maintain_ver)

    def __repr__ (self):
        return ("%d.%d.%d" % (self.main, self.minor, self.maintain))

    def __str__ (self):
        return ("%d.%d.%d" % (self.main, self.minor, self.maintain))

    def __lt__(self, other):
        if self.main < other.main: return True
        elif self.minor < other.minor: return True
        elif self.maintain < other.maintain: return True
        return False

    def VersionStr(self):
        return ("%d.%d.%d" % (self.main, self.minor, self.maintain))

class SharedLibVer(Version):
    def __init__(self, name, version_string):
        Version.__init__(self, version_string)
        if (name.startswith('lib')): self.name = name
        else: self.name = 'lib' + name;

    def SoName(self):
        return ('%s.so.%d' %(self.name, self.main))

    def BuildName(self):
        return self.name + '.so'

    def FullName(self):
        return ("%s.so.%d.%d.%d" % (self.name, self.main, self.minor, self.matain))

db_core_version = SharedLibVer('db_core', '0.0.1')
db_service_version = Version('0.0.1')

Export('db_core_version', 'db_service_version')

def AddProjectSource():
    sources = []
    AddSource(os.getcwd(), sources)
    return sources

def AddSource(root, sources):
    list = os.listdir(root)
    for line in list:
        line = os.path.join(root, line)
        if os.path.isdir(line):
            AddSource(line, sources)
        elif os.path.isfile(line):
            if line.endswith('.cc') or line.endswith('.cpp') or line.endswith('.c') :
                sources.append(line)

def ExcuteCmds(cmds):
    for cmd in cmds:
        print cmd
        handle = subprocess.Popen(cmd, shell = True, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        handle.wait()
        print handle.communicate()

def CreateLink(target, source=None, env=None):
    for t in target:
        if libname_pattern.match(t.name): #and not os.path.exists(os.path.join(install_dir, t.name)):
            dir_name = t.name[3:t.name.find(".")]
            abs_file_path_src = str(pathlib.Path(os.path.join(dir_name, t.name)).resolve())
            abs_file_path_dst = os.path.join(install_dir, t.name)
            abs_file_soname_path = os.path.join(install_dir, t.name[0:t.name.find('.so.') + 5])
            abs_file_buildname_path = os.path.join(install_dir, t.name[0:t.name.find(".so") + 3])

            cmds = [
                "cp -f " + abs_file_path_src + " " + abs_file_path_dst
                , "rm -f " + abs_file_soname_path
                , "cp -s " + abs_file_path_dst + " " + abs_file_soname_path
                , "rm -f " + abs_file_buildname_path
                , "cp -s " + abs_file_soname_path + " " + abs_file_buildname_path
            ]

            ExcuteCmds(cmds)

libname_pattern = re.compile(r"lib.+\.so\.\d+\.\d+\.\d+")

bldr = Builder(action = CreateLink)

Export('CreateLink', 'Version', 'env', 'install_dir', 'AddProjectSource')

db_core = env.SConscript('db_core/SConscript')
db_service = env.SConscript('db_service/SConscript')

env.Depends(db_service, db_core)
env.Requires(db_service, db_core)

env.AddPostAction(db_core, Action(CreateLink))

env.Install(install_dir, [db_core, db_service])
env.Alias('install', install_dir)

Clean(db_core, Glob(os.path.join(install_dir, db_core_version.name) + '*'))
