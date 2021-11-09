# 目标文件：先决条件（也需要包含.h文件，因为这个文件也可能被修改）
#	命令
# 在这里test是终极目标，而test.o和cJSON.o是中间文件。
test:test.o cJSON.o
	gcc test.o cJSON.o -o test
test.o:test.c cJSON.h
	gcc -c test.c -o test.o
cJSON.o:cJSON.c cJSON.h
	gcc -c cJSON.c -o cJSON.o
.PHONY:clean
clean:
	rm -rf *.o test