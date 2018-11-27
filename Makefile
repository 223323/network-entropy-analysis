all: entropy merge

entropy: src/process_entropy.cpp src/Entropy.cpp
	g++ $^ -g -o $@ -lpcap
	
merge: src/merge.c
	gcc $^ -lpcap -lm -o $@

clean:
	rm -f merge entropy
