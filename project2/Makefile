sce212sim: sce212sim.c util.c parse.c run.c
	gcc -g -O2 $^ -o $@

.PHONY: clean test help
clean:
	rm -rf *~ sce212sim

help:
	@echo "The following options are provided with Make\n\t-make:\t\tbuild simulator\n\t-make clean:\tclean the build\n\t-make test:\ttest your simulator"

test: sce212sim test_1 test_2 test_3 test_4 test_5 test_fact test_leaf

test_1:
	@echo "Testing example01"; \
	./sce212sim -m 0x10000000:0x10000010 -n 50 sample_input/example01.o | diff -Naur sample_output/example01 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_2:
	@echo "Testing example02"; \
	./sce212sim -n 50 sample_input/example02.o | diff -Naur sample_output/example02 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_3:
	@echo "Testing example03"; \
	./sce212sim -n 100 sample_input/example03.o | diff -Naur sample_output/example03 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_4:
	@echo "Testing example04"; \
	./sce212sim -n 100 sample_input/example04.o | diff -Naur sample_output/example04 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_5:
	@echo "Testing example05"; \
	./sce212sim -n 100 sample_input/example05.o | diff -Naur sample_output/example05 - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_fact:
	@echo "Testing fact"; \
	./sce212sim -n 100 sample_input/fact.o | diff -Naur sample_output/fact - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi

test_leaf:
	@echo "Testing leaf_example"; \
	./sce212sim -n 100 sample_input/leaf_example.o | diff -Naur sample_output/leaf_example - ;\
	if [ $$? -eq 0 ]; then echo "\tTest seems correct\n"; else echo "\tResults not identical, check the diff output\n"; fi
