CC	=	g++
OBJS	=	huffman.o gzip.o
PROG	=	deflate
# GENERIC

${PROG}:	
all:

clean:
	rm ${OBJS} *~ ${PROG}
${PROG}:	${OBJS}
	${CC} ${OBJS} -o $@
.c.o:
	${CC} $< -c -o $@



#############################
huffman.o:	huffman.cpp huffman.h
gzip.o:	gzip.cpp gzip.h huffman.h huffman.cpp