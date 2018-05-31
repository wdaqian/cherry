
## Introduction
Cherry is a c++ multithreading framework using gn as build tools. Task can be posted to a specified thread such as TaskRunner::PostTask(TaskRunner::EVENT, Bind(ThreadHelper, 4)).

## Usage
Cherry depends on google depot_tools(https://chromium.googlesource.com/chromium/tools/depot_tools.git). Add depot_tools to PATH first.
Run gen_debug.bat/gen_debug.sh to generate project files.
```shell
$ chmod u+x linux_init.sh
$ ./linux_init.sh
$ ./gen_debug.sh
$ ninja -C out/debug Example
