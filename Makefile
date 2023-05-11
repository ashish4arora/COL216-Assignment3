CC=ass3.cpp
OUT=cache_simulate
BLOCKSIZE=64
L1_SIZE=1024
L1_ASSOC=2
L2_SIZE=65536
L2_ASSOC=8
TRACE_FILE=memory_trace_files/trace4.txt

all: compile run

compile: $(CC) 
	g++ -o $(OUT) $(CC)

run: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_FILE)