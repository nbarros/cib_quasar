source=$1
binary=`basename -s .cpp $1`
echo "g++ -std=c++11 -Wno-literal-suffix -Wno-write-strings -I./ open62541.c ${source} -o ${binary}"
g++ -std=c++11 -Wno-literal-suffix -Wno-write-strings -I./ open62541.c ${source} -o ${binary}