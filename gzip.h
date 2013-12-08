/* Author: Rui Pedro Paiva
Teoria da Informação, LEI, 2007/2008*/

#include <stdio.h>

// Gzip header variables
typedef struct header
{
	//elementos fixos
	unsigned char ID1, ID2, CM, XFL, OS;
	unsigned long MTIME;
	unsigned char FLG_FTEXT, FLG_FHCRC, FLG_FEXTRA, FLG_FNAME, FLG_FCOMMENT;   //bits 0, 1, 2, 3 e 4, respectivamente (restantes 3: reservados)
	
	// FLG_FTEXT --> ignorado deliberadamente (tipicamente igual a 0)
	//se FLG_FEXTRA == 1
	unsigned char xlen; 
	unsigned char *extraField;
	
	//se FLG_FNAME == 1
	char *fName;  //terminada por um byte a 0
	
	//se FLG_FCOMMENT == 1
	char *fComment; //terminada por um byte a 0
		
	//se FLG_HCRC == 1
	unsigned char *HCRC;
} gzipHeader;

long getOrigFileSize(FILE *gzFile);
int getHeader(FILE *gzFile, gzipHeader *gzh);
int isDynamicHuffman(unsigned char rb);
void bits2String(char *strBits, int len, int byte);

//------------------------------------------------------------------------------------------------------------------------------------------------- Funções criadas
void getBlockFormat(unsigned int *rb, char *availBits, FILE *gzFile, char *hlit, char *hdist, char *hclen);
int getCodeLength(unsigned int *rb, char *availBits, char hclen, int *codeLen, FILE *gzFile);
void printArray(int *array, int arrayLen);
void getOrderedSymbols(int *symbols, int *codeLen, int len);
void getBlCount(int *codeLen, int *bl_count, int len);
void getByte(unsigned int *rb, char *availBits, char needBits, FILE *gzFile);
void getNumericalValue(int *bl_count, int *next_code, int maxBits);
void createHuffCode(struct tree *huffCode, int *symbols, int *codeLen, int *next_code, int len);
void createHuffTree(struct tree *huffCode, HuffmanTree* huffTree, int *symbols, int len);
char getLiteralLength(char hlit, unsigned int *rb, char *availBits, FILE *gzFile, HuffmanTree* huffTree, int *huffLiteral);
int getDistance(char hdist, unsigned int *rb, char *availBits, FILE *gzFile, HuffmanTree* huffTree, int *huffDistance);
int decodeLen(int index, HuffmanTree* huffTreeLiteral, unsigned int *rb, char *availBits, FILE *gzFile);
int decodeDist(HuffmanTree* huffTreeDistance, unsigned int *rb, char *availBits, FILE *gzFile);
void decodeDataBytes(HuffmanTree* huffTreeLiteral, HuffmanTree* huffTreeDistance, char *ficheiro, int *frontFile, unsigned int *rb, char *availBits, FILE *gzFile);
//------------------------------------------------------------------------------------------------------------------------------------------------- Funções criadas
