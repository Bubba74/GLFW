
ASSIMP_INCLUDE=~/Downloads/computer/C/assimp/include
ASSIMP_LIB=~/Downloads/computer/C/assimp/lib

glfw combined.c -I$ASSIMP_INCLUDE -L$ASSIMP_LIB -lassimp


if [ "x$LD_LIBRARY_PATH" == "x" ]; then
	echo You should probably tell the linker where the assimp lib is located with:
	echo "	export LD_LIBRARY_PATH=../assimp/lib"
fi
