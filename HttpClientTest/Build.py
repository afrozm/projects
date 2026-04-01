#!/usr/bin/env python3
"""Cross-platform CMake build script.

Usage:
    python Build.py [clean] [target=Release|Debug]

Examples:
    python Build.py                  # Debug build
    python Build.py target=Release   # Release build
    python Build.py clean            # Remove build directory
    python Build.py clean target=Release  # Clean then Release build
"""

import os
import platform
import shutil
import subprocess
import sys

BUILD_DIR = "build"


def get_generator():
    system = platform.system()
    if system == "Windows":
        return "Visual Studio 17 2022"
    if system == "Darwin":
        return "Xcode"
    return "Unix Makefiles"


def run(cmd, **kwargs):
    print(f">> {' '.join(cmd)}")
    result = subprocess.run(cmd, **kwargs)
    if result.returncode != 0:
        sys.exit(result.returncode)


def clean():
    if os.path.isdir(BUILD_DIR):
        print(f"Removing {BUILD_DIR}/")
        shutil.rmtree(BUILD_DIR)


def is_multi_config():
    return platform.system() in ("Windows", "Darwin")


def configure(build_type):
    if os.path.isdir(BUILD_DIR):
        return
    cmd = [
        "cmake", "-B", BUILD_DIR,
        "-G", get_generator(),
    ]
    if platform.system() == "Windows":
        cmd += ["-A", "x64"]
    if not is_multi_config():
        cmd.append(f"-DCMAKE_BUILD_TYPE={build_type}")
    run(cmd)


def build(build_type):
    run(["cmake", "--build", BUILD_DIR, "--config", build_type])


def main():
    build_type = "Debug"
    do_clean = False

    for arg in sys.argv[1:]:
        if arg.lower() == "clean":
            do_clean = True
        elif arg.lower().startswith("target="):
            value = arg.split("=", 1)[1]
            if value in ("Release", "Debug"):
                build_type = value
            else:
                print(f"Unknown target: {value}  (use Release or Debug)")
                sys.exit(1)
        else:
            print(f"Unknown argument: {arg}")
            print(__doc__)
            sys.exit(1)

    os.chdir(os.path.dirname(os.path.abspath(__file__)))

    if do_clean:
        clean()

    configure(build_type)
    build(build_type)


if __name__ == "__main__":
    main()
