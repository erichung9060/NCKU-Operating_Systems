judge:
	@gcc -o 1.out 1_1.c
	@i=1; while [ $$i -le 100 ]; do \
		./1.out; \
		i=$$((i + 1)); \
	done
	@./judge.out
	@rm -f 1.out
	@rm -f 1.txt

diff:
	@gcc -o 1.out 1.c
	@i=1; while [ $$i -le 100 ]; do \
		./1.out; \
		i=$$((i + 1)); \
	done
	@git diff --word-diff  1_ans.txt 1.txt || true
	@rm -f 1.out
	@rm -f 1.txt
