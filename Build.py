#!/usr/bin/env python3
"""Cross-platform CMake build script for all projects.

Usage:
    python Build.py [p=<project>] [clean] [target=Release|Debug]

Arguments:
    p=<project>       Build a specific project (e.g. p=SalT, p=HttpClientTest)
    clean             Remove build directory before building
    target=<config>   Release or Debug (default: Debug)

When p= is omitted, builds every subfolder that contains a CMakeLists.txt,
skipping library directories (Common, CommonXplat).

Examples:
    python Build.py                          # Build all projects, Debug
    python Build.py p=SalT target=Release    # Build only SalT, Release
    python Build.py clean                    # Clean + rebuild all projects
    python Build.py p=HttpClientTest clean   # Clean + rebuild HttpClientTest
"""

import os
import platform
import shutil
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR_NAME = "build"
SKIP_PREFIXES = ("common",)


def get_generator():
    system = platform.system()
    if system == "Windows":
        return "Visual Studio 17 2022"
    if system == "Darwin":
        return "Xcode"
    return "Unix Makefiles"


def is_multi_config():
    return platform.system() in ("Windows", "Darwin")


def run(cmd, **kwargs):
    print(f">> {' '.join(cmd)}")
    result = subprocess.run(cmd, **kwargs)
    if result.returncode != 0:
        raise subprocess.CalledProcessError(result.returncode, cmd)


def discover_projects():
    projects = []
    for name in sorted(os.listdir(SCRIPT_DIR)):
        if name.lower().startswith(SKIP_PREFIXES):
            continue
        project_dir = os.path.join(SCRIPT_DIR, name)
        if os.path.isdir(project_dir) and os.path.isfile(os.path.join(project_dir, "CMakeLists.txt")):
            projects.append(name)
    return projects


def clean(project_dir):
    build_dir = os.path.join(project_dir, BUILD_DIR_NAME)
    if os.path.isdir(build_dir):
        print(f"Removing {os.path.relpath(build_dir, SCRIPT_DIR)}/")
        shutil.rmtree(build_dir)


def configure(project_dir, build_type):
    build_dir = os.path.join(project_dir, BUILD_DIR_NAME)
    if os.path.isdir(build_dir):
        return
    cmd = [
        "cmake",
        "-S", project_dir,
        "-B", build_dir,
        "-G", get_generator(),
    ]
    if platform.system() == "Windows":
        cmd += ["-A", "x64"]
    if not is_multi_config():
        cmd.append(f"-DCMAKE_BUILD_TYPE={build_type}")
    run(cmd)


def build(project_dir, build_type):
    build_dir = os.path.join(project_dir, BUILD_DIR_NAME)
    run(["cmake", "--build", build_dir, "--config", build_type])


def build_project(name, build_type, do_clean):
    project_dir = os.path.join(SCRIPT_DIR, name)
    print(f"\n{'='*60}")
    print(f"  {name}")
    print(f"{'='*60}")
    if do_clean:
        clean(project_dir)
    configure(project_dir, build_type)
    build(project_dir, build_type)


def main():
    build_type = "Debug"
    do_clean = False
    project = None

    for arg in sys.argv[1:]:
        lower = arg.lower()
        if lower == "clean":
            do_clean = True
        elif lower.startswith("target="):
            value = arg.split("=", 1)[1]
            if value in ("Release", "Debug"):
                build_type = value
            else:
                print(f"Unknown target: {value}  (use Release or Debug)")
                sys.exit(1)
        elif lower.startswith("p="):
            project = arg.split("=", 1)[1]
        else:
            print(f"Unknown argument: {arg}")
            print(__doc__)
            sys.exit(1)

    if project:
        project_dir = os.path.join(SCRIPT_DIR, project)
        if not os.path.isfile(os.path.join(project_dir, "CMakeLists.txt")):
            print(f"No CMakeLists.txt found in {project}/")
            sys.exit(1)
        projects = [project]
    else:
        projects = discover_projects()
        if not projects:
            print("No buildable projects found.")
            sys.exit(1)
        print(f"Projects: {', '.join(projects)}")

    failed = []
    for name in projects:
        try:
            build_project(name, build_type, do_clean)
        except subprocess.CalledProcessError:
            failed.append(name)
            if project:
                sys.exit(1)
            print(f"\n*** {name} FAILED — continuing with remaining projects ***\n")

    print(f"\n{'='*60}")
    if failed:
        print(f"FAILED: {', '.join(failed)}")
        print(f"PASSED: {', '.join(p for p in projects if p not in failed)}")
        sys.exit(1)
    else:
        print(f"All {len(projects)} project(s) built successfully.")


if __name__ == "__main__":
    main()
