SANITIZER_FLAGS="-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr"
FLAGS="-O3"
INCLUDES="-L/mnt/c/Users/Azerty/my_project/my_libs -I../my_libs/ -Iinclude/"

g++ $INCLUDES   src/tree.cpp src/treeverifier.cpp               \
                src/treeoptimize.cpp src/treehelpers.cpp        \
                src/main.cpp src/treelatexdump.cpp              \
                src/treedump.cpp ../my_libs/error_manage.cpp    \
                $FLAGS $SANITIZER_FLAGS                         \
                -o bin/diff