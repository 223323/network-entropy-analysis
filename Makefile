all: entropy merge

entropy: src/process_entropy.cpp src/fsd.cpp src/Entropy.cpp
	g++ $^ -g -o $@ -lpcap -std=c++14 -Wno-address-of-packed-member
	
# merge: src/merge.c
# 	gcc $^ -lpcap -lm -o $@

clean:
	rm -f merge entropy
