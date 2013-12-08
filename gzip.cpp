/*Authors: Luís Pedro Oliveira Neves & Tomás Luís Fonseca Francisco
 *Based on: Rui Pedro Paiva
 *Teoria da Informação, LEI, 2007/2008*/

#include <cstdlib>
#include <math.h>
#include <unistd.h>

#include "huffman.h"
#include "gzip.h"

#define SYMB 19

 struct tree
{
	int Len;
	char Code[11];
};

//função principal, a qual gere todo o processo de descompactação
int main(int argc, char** argv)
{
	//--- Gzip file management variables
	FILE *gzFile;  //ponteiro para o ficheiro a abrir
	long fileSize;
	long origFileSize;
	int numBlocks = 0;	
	gzipHeader gzh;
	unsigned char byte;  //variável temporária para armazenar um byte lido directamente do ficheiro
	unsigned int rb = 0;  //último byte lido (poderá ter mais que 8 bits, se tiverem sobrado alguns de leituras anteriores)
	char needBits = 0, availBits = 0;	


	//-------------------- Declarações -------------------
	char hlit, hdist, hclen;
	struct tree* huffCode = (tree*) malloc(SYMB * sizeof(tree));
	HuffmanTree* huffTree = createHFTree();


	//--- obter ficheiro a descompactar
	//char fileName[] = "FAQ.txt.gz";

	//--- Recebendo o caminho do ficheiro por argumento ------
	if (argc != 2)
	{
		printf("Linha de comando inválida!!!");
		return -1;
	}
	char * fileName = argv[1];

	//--- processar ficheiro
	gzFile = fopen(fileName, "r");
	fseek(gzFile, 0L, SEEK_END);
	fileSize = ftell(gzFile);
	fseek(gzFile, 0L, SEEK_SET);

	//ler tamanho do ficheiro original (acrescentar: e definir Vector com símbolos
	origFileSize = getOrigFileSize(gzFile);
	//------------------------------------------------------------------------------------------------------------------- Debugging -------
	//printf("ORIGFILESIZE: %ld\n", origFileSize);
	char ficheiro[origFileSize];
	int frontFile = 0;

	//--- ler cabeçalho
	int erro = getHeader(gzFile, &gzh);
	if (erro != 0)
	{
		printf ("Formato inválido!!!");
		return -1;
	}

	//--- Para todos os blocos encontrados
	char BFINAL;	
	
	do
	{				
		//--- ler o block header: primeiro byte depois do cabeçalho do ficheiro
		needBits = 3;
		getByte(&rb, &availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
		//------ Original ------
		/*if (availBits < needBits)
		{
			fread(&byte, 1, 1, gzFile);
			rb = (byte << availBits) | rb;
			availBits += 8;
		}*/ 		
		//----- Original -------

		//obter BFINAL
		//ver se é o último bloco
		BFINAL = rb & 0x01; //primeiro bit é o menos significativo
		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("BFINAL = %d\n", BFINAL);
		rb = rb >> 1; //descartar o bit correspondente ao BFINAL
		availBits -=1;
						
		//analisar block header e ver se é huffman dinâmico					
		if (!isDynamicHuffman(rb))  //ignorar bloco se não for Huffman dinâmico
			continue;				
		rb = rb >> 2; //descartar os 2 bits correspondentes ao BTYPE
		availBits -= 2;
								
		//--- Se chegou aqui --> compactado com Huffman dinâmico --> descompactar
		//**************************************************
		//****** ADICIONAR PROGRAMA... *********************
		//------------------------------------------------------------------------------------------------------------------------------------- Exercicio 1 ------	
			
		getBlockFormat(&rb, &availBits, gzFile, &hlit, &hdist, &hclen);

		//------------------------------------------------------------------------------------------------------------------------------------- Exercicio 2 ------

		int codeLen[SYMB] = {0};
		int maxBits = getCodeLength(&rb, &availBits, hclen, codeLen, gzFile);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("%d\n", maxBits);	//Efeitos de teste
		//printArray(codeLen, SYMB);
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		//------------------------------------------------------------------------------------------------------------------------------------- Exercicio 3 ------

		int symbols[SYMB] = {0, 1 ,2 ,3 ,4 ,5 ,6 ,7 ,8 ,9 ,10 ,11 ,12 ,13 ,14 ,15 ,16 ,17 ,18};	//Simbolos do alfabeto
		getOrderedSymbols(symbols, codeLen, SYMB);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nSYMBOLS: \n");
		//printArray(symbols, SYMB); //Efeitos de teste
		//printf("\nCODELEN: \n");
		//printArray(codeLen,SYMB);	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		int bl_count[SYMB] = {0};
		getBlCount(codeLen, bl_count, SYMB);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nBL_COUNT: \n");
		//printArray(bl_count,SYMB);	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		int next_code[maxBits];
		getNumericalValue(bl_count, next_code, maxBits);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nNEXT_CODE: \n");	
		//printArray(next_code, maxBits + 1); //maxBits + 1 para que o print possa imprimir até ao indice 5 	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		createHuffCode(huffCode, symbols, codeLen, next_code, SYMB);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		/*for(int i = 0; i < SYMB; i++)	//Efeitos de teste
		{
			printf("[%d] - LEN: %d, CODE: %s\n", i, huffCode[i].Len, huffCode[i].Code);	
		}*/		
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		createHuffTree(huffCode, huffTree, symbols, SYMB);

		//------------------------------------------------------------------------------------------------------------------------------------ Exercicio 4 ------

		int huffLiteral[hlit + 257];
		int maxBitsLiteral = getLiteralLength(hlit, &rb, &availBits, gzFile, huffTree, huffLiteral);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printArray(huffLiteral, hlit + 257);
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		int huffDistance[hdist + 1];
		int maxBitsDistance = getDistance(hdist, &rb, &availBits, gzFile, huffTree, huffDistance);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printArray(huffDistance, hdist + 1);
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		//------------------------------------------------------------------------------------------------------------------------------------ Exercicio 5 ------

		//------------------------------------------------------------------------------------------------------------------------------------ Literal ----------

		int symbolsLiteral[hlit + 257];
		for(int i = 0; i < hlit + 257; i++)
		{
			symbolsLiteral[i] = i;	//Cria o array de simbolos do alfabeto
		}

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printArray(huffLiteral, (hlit + 257));	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		getOrderedSymbols(symbolsLiteral, huffLiteral, (hlit + 257));

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nSYMBOLS: \n");
		//printArray(symbolsLiteral, (hlit + 257)); //Efeitos de teste
		//printf("\nCODELEN: \n");
		//printArray(huffLiteral, (hlit + 257));	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		int bl_countLiteral[hlit + 257];
		for(int i = 0; i < hlit + 257; i++)
		{
			bl_countLiteral[i] = 0;
		}
		getBlCount(huffLiteral, bl_countLiteral, (hlit + 257));

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nBL_COUNT: \n");
		//printArray(bl_countLiteral, (hlit + 257));	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		int next_codeLiteral[maxBitsLiteral];
		getNumericalValue(bl_countLiteral, next_codeLiteral, maxBitsLiteral);

		//------------------------------------------------------------------------------------------------------------------- Debugging -------	
		//printf("\nNEXT_CODE: \n");	
		//printArray(next_codeLiteral, (maxBitsLiteral + 1)); //maxBits + 1 para que o print possa imprimir até ao indice 5 	//Efeitos de teste
		//printf("MAXBITSLITERAL: %d\n", maxBitsLiteral);
		//------------------------------------------------------------------------------------------------------------------- Debugging -------		

		struct tree* huffCodeLiteral = (tree*) malloc((hlit + 257) * sizeof(tree));	
		createHuffCode(huffCodeLiteral, symbolsLiteral, huffLiteral, next_codeLiteral, (hlit + 257));

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		/*for(int i = 0; i < hlit + 257; i++)	//Efeitos de teste
		{
			printf("[%d] - LEN: %d, CODE: %s\n", i, huffCodeLiteral[i].Len, huffCodeLiteral[i].Code);	
		}*/
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		HuffmanTree* huffTreeLiteral = createHFTree();
		createHuffTree(huffCodeLiteral, huffTreeLiteral, symbolsLiteral, (hlit + 257));

		//------------------------------------------------------------------------------------------------------------------------------------ Distance ---------

		int symbolsDistance[hdist + 1];
		for(int i = 0; i < hdist + 1; i++)
		{
			symbolsDistance[i] = i;	//Cria o array de simbolos do alfabeto
		}

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printArray(huffLiteral, (hlit + 257));	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		getOrderedSymbols(symbolsDistance, huffDistance, (hdist + 1));

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nSYMBOLS: \n");
		//printArray(symbolsLiteral, (hlit + 257)); //Efeitos de teste
		//printf("\nCODELEN: \n");
		//printArray(huffLiteral, (hlit + 257));	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		int bl_countDistance[hdist + 1];
		for(int i = 0; i < hdist + 1; i++)
		{
			bl_countDistance[i] = 0;
		}
		
		getBlCount(huffDistance, bl_countDistance, (hdist + 1));

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nBL_COUNT: \n");
		//printArray(bl_countLiteral, (hlit + 257));	//Efeitos de teste
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		int next_codeDistance[maxBitsDistance];
		getNumericalValue(bl_countDistance, next_codeDistance, maxBitsDistance);	

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		//printf("\nNEXT_CODE: \n");	
		//printArray(next_codeLiteral, (maxBitsLiteral + 1)); //maxBits + 1 para que o print possa imprimir até ao indice 5 	//Efeitos de teste
		//printf("MAXBITSLITERAL: %d\n", maxBitsLiteral);
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		struct tree* huffCodeDistance = (tree*) malloc((hdist + 1) * sizeof(tree));	
		createHuffCode(huffCodeDistance, symbolsDistance, huffDistance, next_codeDistance, (hdist + 1));

		//------------------------------------------------------------------------------------------------------------------- Debugging -------
		/*for(int i = 0; i < SYMB; i++)	//Efeitos de teste
		{
			printf("[%d] - LEN: %d, CODE: %s\n", i, huffCodeLiteral[i].Len, huffCodeLiteral[i].Code);	
		}*/
		//------------------------------------------------------------------------------------------------------------------- Debugging -------

		HuffmanTree* huffTreeDistance = createHFTree();
		createHuffTree(huffCodeDistance, huffTreeDistance, symbolsDistance, (hdist + 1));		
		decodeDataBytes(huffTreeLiteral, huffTreeDistance, ficheiro, &frontFile, &rb, &availBits, gzFile);

		//Libertação dos ponteiros
		free(huffCodeLiteral);
		free(huffTreeLiteral);
		free(huffCodeDistance);
		free(huffTreeDistance);
		//**************************************************																																			
		//actualizar número de blocos analisados
		numBlocks++;
	}while(BFINAL == 0);

	char fn[1024];
	sprintf(fn, "%s", gzh.fName);
	FILE* fp = fopen(fn, "w+");

	for(int i = 0; i < sizeof(ficheiro); i++)
	{
		fprintf(fp, "%c", ficheiro[i]);
	}
	//terminações			
	fclose(gzFile);
	fclose(fp);
	printf("End: %d bloco(s) analisado(s).\n", numBlocks);

    /*//teste da função bits2String: RETIRAR antes de criar o executável final
	char str[9];
	bits2String(str, 0x03);
	printf("%s\n", str);*/

    //RETIRAR antes de criar o executável final
    //system("PAUSE");
    return EXIT_SUCCESS;
}
//---------------------------------------------------------------
//Lê o cabeçalho do ficheiro gzip: devolve erro (-1) se o formato for inválidodevolve, ou 0 se ok
int getHeader(FILE *gzFile, gzipHeader *gzh) //obtém cabeçalho
{
	unsigned char byte;

	//Identicação 1 e 2: valores fixos
	fread(&byte, 1, 1, gzFile);
	(*gzh).ID1 = byte;
	if ((*gzh).ID1 != 0x1f) return -1; //erro no cabeçalho
	
	fread(&byte, 1, 1, gzFile);
	(*gzh).ID2 = byte;
	if ((*gzh).ID2 != 0x8b) return -1; //erro no cabeçalho
	
	//Método de compressão (deve ser 8 para denotar o deflate)
	fread(&byte, 1, 1, gzFile);
	(*gzh).CM = byte;
	if ((*gzh).CM != 0x08) return -1; //erro no cabeçalho
				
	//Flags
	fread(&byte, 1, 1, gzFile);
	unsigned char FLG = byte;
	
	//MTIME
	char lenMTIME = 4;	
	fread(&byte, 1, 1, gzFile);
	(*gzh).MTIME = byte;
	for (int i = 1; i <= lenMTIME - 1; i++)
	{
		fread(&byte, 1, 1, gzFile);
		(*gzh).MTIME = (byte << 8) + (*gzh).MTIME;				
	}
					
	//XFL (not processed...)
	fread(&byte, 1, 1, gzFile);
	(*gzh).XFL = byte;
	
	//OS (not processed...)
	fread(&byte, 1, 1, gzFile);
	(*gzh).OS = byte;
	
	//--- Check Flags
	(*gzh).FLG_FTEXT = (char)(FLG & 0x01);
	(*gzh).FLG_FHCRC = (char)((FLG & 0x02) >> 1);
	(*gzh).FLG_FEXTRA = (char)((FLG & 0x04) >> 2);
	(*gzh).FLG_FNAME = (char)((FLG & 0x08) >> 3);
	(*gzh).FLG_FCOMMENT = (char)((FLG & 0x10) >> 4);
				
	//FLG_EXTRA
	if ((*gzh).FLG_FEXTRA == 1)
	{
		//ler 2 bytes XLEN + XLEN bytes de extra field
		//1º byte: LSB, 2º: MSB
		char lenXLEN = 2;

		fread(&byte, 1, 1, gzFile);
		(*gzh).xlen = byte;
		fread(&byte, 1, 1, gzFile);
		(*gzh).xlen = (byte << 8) + (*gzh).xlen;
		
		(*gzh).extraField = new unsigned char[(*gzh).xlen];
		
		//ler extra field (deixado como está, i.e., não processado...)
		for (int i = 0; i <= (*gzh).xlen - 1; i++)
		{
			fread(&byte, 1, 1, gzFile);
			(*gzh).extraField[i] = byte;
		}
	}
	else
	{
		(*gzh).xlen = 0;
		(*gzh).extraField = 0;
	}
	
	//FLG_FNAME: ler nome original	
	if ((*gzh).FLG_FNAME == 1)
	{			
		(*gzh).fName = new char[1024];
		unsigned int i = 0;
		do
		{
			fread(&byte, 1, 1, gzFile);
			if (i <= 1023)  //guarda no máximo 1024 caracteres no array
				(*gzh).fName[i] = byte;
			i++;
		}while(byte != 0);
		if (i > 1023)
			(*gzh).fName[1023] = 0;  //apesar de nome incompleto, garantir que o array termina em 0
	}
	else
		(*gzh).fName = 0;
	
	//FLG_FCOMMENT: ler comentário
	if ((*gzh).FLG_FCOMMENT == 1)
	{			
		(*gzh).fComment = new char[1024];
		unsigned int i = 0;
		do
		{
			fread(&byte, 1, 1, gzFile);
			if (i <= 1023)  //guarda no máximo 1024 caracteres no array
				(*gzh).fComment[i] = byte;
			i++;
		}while(byte != 0);
		if (i > 1023)
			(*gzh).fComment[1023] = 0;  //apesar de comentário incompleto, garantir que o array termina em 0
	}
	else
		(*gzh).fComment = 0;

	
	//FLG_FHCRC (not processed...)
	if ((*gzh).FLG_FHCRC == 1)
	{			
		(*gzh).HCRC = new unsigned char[2];
		fread(&byte, 1, 1, gzFile);
		(*gzh).HCRC[0] = byte;
		fread(&byte, 1, 1, gzFile);
		(*gzh).HCRC[1] = byte;		
	}
	else
		(*gzh).HCRC = 0;
	
	return 0;
}
//---------------------------------------------------------------
//Analisa block header e vê se é huffman dinâmico
int isDynamicHuffman(unsigned char rb)
{				
	unsigned char BTYPE = rb & 0x03;
					
	if (BTYPE == 0) //--> sem compressão
	{
		printf("Ignorando bloco: sem compactação!!!\n");
		return 0;
	}
	else if (BTYPE == 1)
	{
		printf("Ignorando bloco: compactado com Huffman fixo!!!\n");
		return 0;
	}
	else if (BTYPE == 3)
	{
		printf("Ignorando bloco: BTYPE = reservado!!!\n");
		return 0;
	}
	else
		return 1;		
}
//---------------------------------------------------------------
//Obtém tamanho do ficheiro original
long getOrigFileSize(FILE * gzFile)
{
	//salvaguarda posição actual do ficheiro
	long fp = ftell(gzFile);
	
	//últimos 4 bytes = ISIZE;
	fseek(gzFile, -4, SEEK_END);
	
	//determina ISIZE (só correcto se cabe em 32 bits)
	unsigned long sz = 0;
	unsigned char byte;
	fread(&byte, 1, 1, gzFile);
	sz = byte;
	for (int i = 0; i <= 2; i++)
	{
		fread(&byte, 1, 1, gzFile);
		sz = (byte << 8*(i+1)) + sz;
	}

	
	//restaura file pointer
	fseek(gzFile, fp, SEEK_SET);
	
	return sz;		
}
//---------------------------------------------------------------
void bits2String(char *strBits, int len, int byte)
{
	int mask = 0x01;  //get LSbit

	int Len = (int)len;

	//strBits[8] = 0;
	for (int bit, i = len - 1; i >= 0; i--)
	{		
		bit = byte & mask;
		strBits[i] = bit + 48; //converter valor numérico para o caracter alfanumérico correspondente
		byte = byte >> 1;
	}	
}
//---------------------------------------------------------------
//----------- Get de um byte de informação adicional ------------
void getByte(unsigned int *rb, char *availBits, char needBits, FILE *gzFile)
{
	unsigned char byte;
	while ((*availBits) < needBits)
	{
		fread(&byte, 1, 1, gzFile);
		*rb = (byte << (*availBits)) | *rb;
		(*availBits) += 8;
	}
}
//---------------------------------------------------------------
//------------------ Get do HLIT, HDIST, e do HCLEN -------------
void getBlockFormat(unsigned int *rb, char *availBits, FILE *gzFile, char *hlit, char *hdist, char *hclen)
{
	//Leitura do HLIT
	char needBits = 5;
	getByte(rb, availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
	*hlit = *rb & 0x1F;								//00000011111
	*rb = *rb >> 5;									//Descarta o HLIT acabado de ler
	(*availBits) -= 5;								//Reduz o número de bits disponiveis para analizar

	//Leitura do HDIST
	needBits = 5;
	getByte(rb, availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
	*hdist = *rb & 0x1F;	//5bits
	*rb = *rb >> 5;
	(*availBits) -= 5;

	//Leitura do HCLEN
	needBits = 4;
	getByte(rb, availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
	*hclen = *rb & 0x0F; //4 bits
	*rb = *rb >> 4;
	(*availBits) -= 4;

	//printf("HLIT: %d\n", *hlit);
	//printf("HDIST: %d\n", *hdist);
	//printf("HCLEN: %d\n", *hclen);
}
//-------------------------------------------------------------
//-------------- Get do comprimento dos códigos ---------------
int getCodeLength(unsigned int *rb, char *availBits, char hclen, int *codeLen, FILE *gzFile)
{
	int positions[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
	int maxBits = 0;
	char needBits = 3;

	for(int i = 0; i < (hclen + 4); i++)
	{
		getByte(rb, availBits, needBits, gzFile); 	//Get de mais informação caso o necessário não esteja ainda no rb
		codeLen[positions[i]] = *rb & 0x07;				//00000111 
		if(codeLen[positions[i]] > maxBits)
		{
			maxBits = codeLen[positions[i]];	//Guarda o maior comprimento de bits
		}           
        *rb = *rb >> 3;
        *availBits-=3;
	}
	return maxBits;
}
//-------------------------------------------------------------
//----- Get do número de ocorrências dos comprimentos ---------
void getBlCount(int *codeLen, int *bl_count, int len)
{
	for(int i = 0; i < len; i++)
	{

		bl_count[codeLen[i]] += 1;			//Conta o número de ocorrências de cada comprimento
	}
}
//-------------------------------------------------------------
//----- Ordena o vector de comprimento dos simbolos -----------
void getOrderedSymbols(int *symbols, int *codeLen, int len)
{
	int temp1, temp2, trocou;

	for(int i = 0; (i < len - 1) && trocou; i++)	//Recorrendo a um algoritmo BubbleSort
	{
		trocou = 0;
		for(int j = 0; j < len - i - 1; j++)
		{
			if(codeLen[j] > codeLen[j + 1])		//Troca se o código for maior do que o próximo
			{
				trocou = 1;

				temp1 = codeLen[j];
				codeLen[j] = codeLen[j + 1];
				codeLen[j + 1] = temp1;

				temp2 = symbols[j];
				symbols[j] = symbols[j + 1];
				symbols[j + 1] = temp2;
			}
			else if((symbols[j] > symbols[j + 1]) && (codeLen[j] == codeLen[j + 1]))	//Troca se o código for igual mas o simbolo for maior em termos lexicográficos
			{
				trocou = 1;

				temp1 = codeLen[j];
				codeLen[j] = codeLen[j + 1];
				codeLen[j + 1] = temp1;

				temp2 = symbols[j];
				symbols[j] = symbols[j + 1];
				symbols[j + 1] = temp2;
			}
		}
	}
}
//-------------------------------------------------------------
//Encontra o valor numerico do código mais pequeno para cada comprimento
void getNumericalValue(int *bl_count, int *next_code, int maxBits)
{
	int code = 0;
	bl_count[0]=0;
	next_code[0]=0;
	for(int bits = 1; bits <= maxBits; bits++)
	{
		code = (code + bl_count[bits - 1]) << 1;
		next_code[bits] = code;
	}
}
//-------------------------------------------------------------
//Cria os códigos huffman através de array de simbolos, e dos seus comprimentos respectivos
void createHuffCode(struct tree *huffCode, int *symbols, int *codeLen, int *next_code, int len)
{
	for(int i = 0; i < len; i++)
	{
		int len = codeLen[i];

		huffCode[i].Len = len;

		if(len > 0)
		{
			bits2String(huffCode[i].Code, huffCode[i].Len, next_code[len]);
			next_code[len]++;
		}
	}
}
//-------------------------------------------------------------
//Cria a árvore de huffman
void createHuffTree(struct tree *huffCode, HuffmanTree* huffTree, int *symbols, int len)
{
	for(int i = 0; i < len; i++)
	{
		if(huffCode[i].Len != 0)
		{
			addNode(huffTree, huffCode[i].Code, symbols[i], 0);
			//printf("HUFFCODE: %s, INDEX: %d\n", huffCode[i].Code, symbols[i]);
		}
	}
}
//-------------------------------------------------------------
//Lê os códigos de comprimentos/literais
char getLiteralLength(char hlit, unsigned int *rb, char *availBits, FILE *gzFile, HuffmanTree* huffTree, int *huffLiteral)
{	
	unsigned char bit;
	unsigned char bits;
	int index;
	char needBits;
	int bitsToRead;
	int maxBits = 0;


	for(int i = 0; i < (hlit + 257); i++)
	{
		resetCurNode(huffTree);

		index = -1;
		while(index < 0)
		{
			needBits = 1;
			getByte(rb, availBits, needBits, gzFile);
			bit = (*rb & 0x01) + 48;	//máscara 00000001 + 48 (equivalente a ASCII)
			*rb = *rb >> 1;
			(*availBits) -= 1;

			index = nextNode(huffTree, bit);
			//printf("INDEX por %d: %d\n", (bit - 48), index);
			
			if(index > -1)
			{
				if(index <= 15)
				{
					huffLiteral[i] = index;
					if(index > maxBits)
					{
						maxBits = index;
						//------------------------------------------------------------------------------------------------------------------- Debugging -------
						//printf("maxBits = %d\n", maxBits);
					}
				}
				else if(index == 16)
				{
					needBits = 2;
					getByte(rb, availBits, needBits, gzFile);
					bits = *rb & 0x03;	//máscara 0000 0011
					*rb = *rb >> 2;
					(*availBits) -= 2;

					bitsToRead = 3 + bits;
					for(int j = 0; j < bitsToRead; j++)
					{
						huffLiteral[i + j] = huffLiteral[i - 1];
					}

					i += (bitsToRead - 1);
				}
				else if(index == 17)
				{
					needBits = 3;
					getByte(rb, availBits, needBits, gzFile);
					bits = *rb & 0x07;	//máscara 0000 0111
					*rb = *rb >> 3;
					(*availBits) -= 3;

					bitsToRead = 3 + bits;
					//printf("BITS2READ: %d\n", bitsToRead);
					for(int j = 0; j < bitsToRead; j++)
					{
						huffLiteral[i + j] = 0;
					}

					i += (bitsToRead - 1);
				}
				else if(index == 18)
				{
					needBits = 7;
					getByte(rb, availBits, needBits, gzFile);
					bits = *rb & 0x7F;	//máscara 0111 1111
					*rb = *rb >> 7;
					(*availBits) -= 7;

					bitsToRead = 11 + bits;
					for(int j = 0; j < bitsToRead; j++)
					{
						huffLiteral[i + j] = 0;
					}

					i += (bitsToRead - 1);
				}
			}
		}		
	}
	return maxBits;
}
//-------------------------------------------------------------
//Lê os códigos de distância
int getDistance(char hdist, unsigned int *rb, char *availBits, FILE *gzFile, HuffmanTree* huffTree, int *huffDistance)
{
	unsigned char bit;
	unsigned char bits;
	int index;
	char needBits;
	int bitsToRead;
	int maxBits = 0;

	for(int i = 0; i < (hdist + 1); i++)
	{
		resetCurNode(huffTree);

		index = -1;
		while(index < 0)
		{
			needBits = 1;
			getByte(rb, availBits, needBits, gzFile);
			bit = (*rb & 0x01) + 48;	//máscara 00000001 + 48 (equivalente a ASCII)
			*rb = *rb >> 1;
			(*availBits) -= 1;

			index = nextNode(huffTree, bit);
			
			if(index > -1)
			{
				if(index <= 15)
				{
					huffDistance[i] = index;
				}
				else if(index == 16)
				{
					needBits = 2;
					getByte(rb, availBits, needBits, gzFile);
					bits = *rb & 0x03;	//máscara 0000 0011
					*rb = *rb >> 2;
					(*availBits) -= 2;

					bitsToRead = 3 + bits;
					for(int j = 0; j < bitsToRead; j++)
					{
						huffDistance[i + j] = huffDistance[i - 1];
					}

					i += (bitsToRead - 1);
				}
				else if(index == 17)
				{
					needBits = 3;
					getByte(rb, availBits, needBits, gzFile);
					bits = *rb & 0x07;	//máscara 0000 0111
					*rb = *rb >> 3;
					(*availBits) -= 3;

					bitsToRead = 3 + bits;
					for(int j = 0; j < bitsToRead; j++)
					{
						huffDistance[i + j] = 0;
					}

					i += (bitsToRead - 1);
				}
				else if(index == 18)
				{
					needBits = 7;
					getByte(rb, availBits, needBits, gzFile);
					bits = *rb & 0x7F;	//máscara 0111 1111
					*rb = *rb >> 7;
					(*availBits) -= 7;

					bitsToRead = 11 + bits;
					for(int j = 0; j < bitsToRead; j++)
					{
						huffDistance[i + j] = 0;
					}

					i += (bitsToRead - 1);
				}
				if(index > maxBits)
				{
					maxBits = index;
				}
			}
		}
	}
}
//-------------------------------------------------------------
//--------- Descodifica a informação ----------------
void decodeDataBytes(HuffmanTree* huffTreeLiteral, HuffmanTree* huffTreeDistance, char *ficheiro, int *frontFile, unsigned int *rb, char *availBits, FILE *gzFile)
{
	int index;
	int copy;
	int dist;
	int recua;
	char needBits;
	unsigned char bits;

	index = -1;
	while(index != 256)
	{
		resetCurNode(huffTreeLiteral);

		index = -1;
		while(index < 0)
		{
			needBits = 1;
			getByte(rb, availBits, needBits, gzFile);
			bits = (*rb & 0x01) + 48;	//máscara 00000001 + 48 (equivalente a ASCII)
			*rb = *rb >> 1;
			(*availBits) -= 1;

			index = nextNode(huffTreeLiteral, bits);


			if((index >= 0) && (index <= 255))
			{
				//------------------------------------------------------------------------------------------------------------------- Debugging -------
				//printf("INDEX: %d\n", index);
				//printf("FRONTFILE: %d\n", *frontFile);
				ficheiro[*frontFile] = index;
				//------------------------------------------------------------------------------------------------------------------- Debugging -------
				//printf("%d    ", *frontFile);
				//printf("%c", ficheiro[*frontFile]);
				(*frontFile)++;
				//------------------------------------------------------------------------------------------------------------------- Debugging -------				
				//printf("ficheiro[0] = %c\n", ficheiro[0]);
				//exit(0);
			}
			else
			{
				if(index == 256)
				{
					//------------------------------------------------------------------------------------------------------------------- Debugging -------
					//printf("BREAK\n");
					break;
				}
				else if(index >= 257 && index <= 285)
				{
					copy = decodeLen(index, huffTreeLiteral, rb, availBits, gzFile);
					dist = decodeDist(huffTreeDistance, rb, availBits, gzFile);

					recua = *frontFile - dist;
					for(int i = 0; i < copy; i++)
					{
						ficheiro[*frontFile] = ficheiro[recua + i];
						//------------------------------------------------------------------------------------------------------------------- Debugging -------
						//printf("%d   ", *frontFile);
						//printf("%c", ficheiro[*frontFile]);
						(*frontFile)++;
					}
				}
			}
		}
	}
}
//-------------------------------------------------------------
//--------- Calcula a distância a copiar ----------------
int decodeLen(int index, HuffmanTree* huffTreeLiteral, unsigned int *rb, char *availBits, FILE *gzFile)
{
	int len[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
	int comp;
	char needBits;
	unsigned char bits;

	if(index >= 257 && index <= 264)
	{
		comp = len[index - 257];
	}
	else if(index >= 265 && index <= 284)
	{
		needBits = ((index - 265) / 4) + 1;
		getByte(rb, availBits, needBits, gzFile);
		bits = (*rb << (8 - needBits));	//coloca uma máscara nos bits que são precisos para ler
		bits = bits >> (8 - needBits);
		*rb = *rb >> needBits;
		(*availBits) -= needBits;

		comp = len[index - 257] + bits;
	}
	else if(index == 285)
	{
		comp = len[index - 257];
	}
	return comp;	
}
//-------------------------------------------------------------
//--------- Calcula a distância ----------------
int decodeDist(HuffmanTree* huffTreeDistance, unsigned int *rb, char *availBits, FILE *gzFile)
{
	int dist[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
	int recua;
	char needBits;
	unsigned int bits;
	int index;

	resetCurNode(huffTreeDistance);

	index = -1;
	while(index < 0)
	{
		needBits = 1;
		getByte(rb, availBits, needBits, gzFile);
		bits = (*rb & 0x01) + 48;	//máscara 00000001 + 48 (equivalente a ASCII)
		*rb = *rb >> 1;
		(*availBits) -= 1;

		index = nextNode(huffTreeDistance, bits);

		if(index >= 0 && index <= 3)
		{
			recua = dist[index];
		}
		else if(index >= 4 && index <= 29)
		{
			needBits = ((index - 4) / 2) + 1; //ATENÇÃO BITS = 13
			getByte(rb, availBits, needBits, gzFile);
			bits = (*rb << ((4 * 8) - needBits));	//coloca uma máscara nos bits que são precisos para ler
			bits = bits >> ((4 * 8) - needBits);
			*rb = *rb >> needBits;
			(*availBits) -= needBits;

			recua = dist[index] + bits;
			//------------------------------------------------------------------------------------------------------------------- Debugging -------
			/*if(index >= 18)
						printf("index: %d needBits: %d recua: %d\n", index, needBits, recua);*/
		}
	}
	return recua;
}

//-------------------------------------------------------------
//--------- Print do array de tamanho definido ----------------
void printArray(int *array, int arrayLen)
{
	for(int i = 0; i < arrayLen; i++)
	{
		printf("[%d] - %d\n", i, array[i]);
	}
}