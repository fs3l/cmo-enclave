import subprocess
import sys
import os

def DirectoryOfThisScript():
    return os.path.dirname(os.path.abspath( __file__ ))

def MakeRelativePathsInFlagsAbsolute(flags, working_directory):
    if not working_directory:
        return list(flags)
    new_flags = []
    make_next_absolute = False
    path_flags = ["-isystem", "-I", "-iquote", "--sysroot="]
    for flag in flags:
        new_flag = flag

        if make_next_absolute:
            make_next_absolute = False
            if not flag.startswith('/'):
                new_flag = os.path.join(working_directory, flag)

        for path_flag in path_flags:
            if flag == path_flag:
                make_next_absolute = True
                break

            if flag.startswith(path_flag):
                path = flag[len(path_flag):]
                new_flag = path_flag + os.path.join(working_directory, path)
                break

        if new_flag:
            new_flags.append( new_flag )
    return new_flags

def CXX():
    return "c++"

def SystemIncludePathasFlags():
    cmd = "{} -E -x c++ - -v </dev/null 2>&1".format(CXX())
    out = subprocess.check_output(cmd, shell=True).decode("utf-8")
    out = out.split("\n")
    out = out[out.index("#include <...> search starts here:") + 1:-1]
    out = out[0:out.index("End of search list.")]
    out = [p.strip() for p in out if not p.endswith("(framework directory)")]
    flags = [["-isystem", p] for p in out]
    return [f for ff in flags for f in ff]

def FlagsForFile(filename, **kwargs):
    flags = [
            "-Wall",
            "-Wextra",
            "-Werror",
            "-Wno-long-long",
            "-Wno-variadic-macros",
            "-fexceptions",
            "-DNDEBUG",
            "-std=c++11",
            "-x",
            "c++",
            "-Iinclude/",
            ]
    flags += SystemIncludePathasFlags()
    flags = MakeRelativePathsInFlagsAbsolute(flags, DirectoryOfThisScript())
    return { "flags": flags, "do_cache": True }
