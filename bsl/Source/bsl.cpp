/*
	Copyright 1996, 1997, 1998, 2000
	        Hekkelman Programmatuur B.V.  All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	1. Redistributions of source code must retain the above copyright notice,
	   this list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.
	3. All advertising materials mentioning features or use of this software
	   must display the following acknowledgement:
	   
	    This product includes software developed by Hekkelman Programmatuur B.V.
	
	4. The name of Hekkelman Programmatuur B.V. may not be used to endorse or
	   promote products derived from this software without specific prior
	   written permission.
	
	THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
	INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
	FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
	AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "res.h"
#include <string.h>
#include <stdlib.h>

void usage();
void getopt_();
void *build(long *size);

int resID = 0, trunc = 0;
char *resName = NULL;
char *header = NULL;
char *out = NULL;
FILE *hf = NULL;

void usage()
{
	puts("usage: bsl \n\t[-id resID]\n\t[-name resName]\n\t"
		"[-o dest]\n\t[-h header]\n\t[-nomerge]\n\t[source]");
	exit(1);
}

void getopt_(int argc, char *argv[])
{
	int i = 0;
	char *in = NULL;

	while (++i < argc)
	{
		if (argv[i][0] == '-')
		{
			if (strcmp(argv[i], "-id") == 0)
				resID = atoi(argv[++i]);
			else if (strcmp(argv[i], "-name") == 0)
				resName = argv[++i];
			else if (strcmp(argv[i], "-o") == 0)
				out = argv[++i];
			else if (strcmp(argv[i], "-h") == 0)
				header = argv[++i];
			else if (strcmp(argv[i], "-nomerge") == 0)
				trunc = 1;
			else
				usage();
			continue;
		}
		
		if (!in)
		{
			in = argv[i];
			freopen(argv[i], "r", stdin);
		}
		else
			usage();
	}
	if (!header && !out)
		usage();
	
	if (header)
	{
		hf = fopen(header, "w");
		if (!hf) { perror("error opening header file"); exit(1);}
		fprintf(hf, "/*\n\tHeader generated by bsl\n*/\n");
	}
} /* getopt_ */

main(int argc, char **argv)
{
	void *data;
	long size;

	getopt_(argc, argv);

	data = build(&size);

	if (out)
		WriteResource(out, trunc, 'StrL', resID, data, size, resName);
	
	return 0;
}

void* build(long *length)
{
	FILE *out;
	char s[256];
	void *result;
	int count = 0;

	out = tmpfile();
	if (!out) { perror("Error opening swapfile"); exit(1); }
	
	while (!feof(stdin) && fgets(s, 255, stdin))
	{
		short slen;
		char *def, *str, *e;

		def = s;
		str = strstr(s, " = ");
		if (!str)
		{
			if (hf)
			{
				fprintf(stderr,"syntax error at %s\n", def);
				exit(1);
			}
			else
				str = s;
		}
		else
		{
			*str = 0;
			str += 3;
		}
		
		if (hf)
		{
			fprintf(hf, "#define %s %*d\n", def, 40 - strlen(def),
				count++);
		}
		
		e = strrchr(str, '\n');
		if (e) *e = 0;
		
		slen = strlen(str) + 1;
		fwrite(&slen, sizeof(short), 1, out);
		fwrite(str, slen, 1, out);
	}
	fputc(0, out);
	
	*length = ftell(out);
	result = malloc(*length);
	if (!result) {perror("Insufficient memory");exit(1);}
	fseek(out, 0, SEEK_SET);
	if (!fread(result, 1, *length, out)) {perror("read error");exit(1);} 
	
	fclose(out);
	if (hf) fclose(hf);

	return result;
}
