clang log_test.c -g -fdeclspec -fPIC -o ../build/log_test.out -Isrc -I../src/ -L../build -lmschne -Wl,-rpath,.
clang wind_test.c -g -fdeclspec -fPIC -o ../build/wind_test.out -Isrc -I../src/ -L../build -lmschne -Wl,-rpath,.
clang wind_test_correct.c -g -fdeclspec -fPIC -o ../build/wind_test_correct.out -Isrc -I../src/ -L../build -lmschne -Wl,-rpath,.