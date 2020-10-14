#### 1. build.sh is the build and run script
- hack emcc to delete some objects in libc.a
- build tf-lite with emcc compiler
- build iwasm with pthread enable and include libiary under libc-emcc
- run benchmark model with iwasm
     - set --max-secs 300: means the max training time cost is 5 minutes, you can adjust by yourself