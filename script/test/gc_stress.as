set_gc({ step:300000, enable: true, log: true })
const global_s = {
  i: 0
}
const loop = () => {
  const i2 = global_s.i
  global_s.i = i2 + 1
  const alloc = {
    "tasks": [{
            "type": "shell",
            "label": "cpp-build",
            "command": "cmake -S src -B bin  && cmake --build bin",
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "detail": "调试器生成的任务。",
            "group": "build"
        }],
    "version": "2.0.0"
  }
  macro(loop)
}

loop()

