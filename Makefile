CC=ass3.cpp
OUT=cache_simulate
BLOCKSIZE=64
L1_SIZE=1024
L1_ASSOC=2
L2_SIZE=65536
L2_ASSOC=8
TRACE_1=memory_trace_files/trace1.txt
TRACE_2=memory_trace_files/trace2.txt
TRACE_3=memory_trace_files/trace3.txt
TRACE_4=memory_trace_files/trace4.txt
TRACE_5=memory_trace_files/trace5.txt
TRACE_6=memory_trace_files/trace6.txt
TRACE_7=memory_trace_files/trace7.txt
TRACE_8=memory_trace_files/trace8.txt

all: compile run

compile: $(CC) 
	g++ -o $(OUT) $(CC)

run: run1 run2 run3 run4 run5 run6 run7 run8

run1: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_1) 

run2: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_2)

run3: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_3)

run4: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_4)

run5: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_5)

run6: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_6)

run7: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_7)

run8: $(OUT)
	./$(OUT) $(BLOCKSIZE) $(L1_SIZE) $(L1_ASSOC) $(L2_SIZE) $(L2_ASSOC) $(TRACE_8)


