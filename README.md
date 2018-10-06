
GLFW_DIR=/home/henry/Downloads/computer/GLFW
SRC=$1

GLFW_SRC="$GLFW_DIR/glfw-3.2.1/src"
GLFW_INCLUDE="${GLFW_DIR}/glfw-3.2.1/include"
GLFW_DEPS="${GLFW_DIR}/glfw-3.2.1/deps"

MY_SOURCES="${GLFW_DIR}/../C/Graphics/src/*.c"
MY_INCLUDES="${GLFW_DIR}/../C/Graphics/include"

COMMAND="gcc $SRC $MY_SOURCES $GLFW_DEPS/*.c -I $MY_INCLUDES -I $GLFW_DEPS -I $GLFW_INCLUDE -L $GLFW_SRC -lglfw3 -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lXinerama -lXcursor -lXxf86vm -lm"

$COMMAND

# echo Compiling $SRC with command: $COMMAND

# if [[ -z $OUTPUT ]]
# then
# 	echo hi
# fi

# 
# OUTPUT=`$COMMAND 1>/dev/null`
# 
# if [[ $OUTPUT -eq "" ]]
# then
	# echo $OUTPUT
	# echo
	# echo Compiling $SRC with command: $COMMAND
# fi
