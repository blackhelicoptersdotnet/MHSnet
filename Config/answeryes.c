/*
**	MHSnet Version 1
**
**	Copyright 2005 Piers Lauder, Personis P/L.
**
**	All rights reserved.
**
**	Use of this software is subject to a licence agreement.
**
**	SCCSID	%W% %E%
*/

/*
**	answer(yes|no)|get(def)*ans
*/

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>

typedef enum { yes, no, ans, defans } req;

char		EmptyString[]	= "";

/* extern char *strrchr();	*/
/* extern int	strcmp();	*/

int
main(argc, argv)
	int	argc;
	char *	argv[];
{
	req	typ;
	char *	cp;
	int	i;
	int	minargs;
	int	maxargs;
	int	esc = 0;
	char	buf[1024];

	if ( (cp = strrchr(argv[0], '/')) != (char *)0 )
		cp++;
	else
		cp = argv[0];

	if ( argc > 1 && strcmp(argv[1], "-n") == 0 )
	{
		argv++;
		argc--;
		esc = 1;
	}

	if ( strcmp(cp, "answeryes") == 0 )
	{
		typ = yes;
		minargs = 2;
		maxargs = 2;
	}
	else
	if ( strcmp(cp, "answerno") == 0 )
	{
		typ = no;
		minargs = 2;
		maxargs = 2;
	}
	else
	if ( strcmp(cp, "getans") == 0 )
	{
		typ = ans;
		minargs = 2;
		maxargs = 3;
	}
	else
	if ( strcmp(cp, "getdefans") == 0 )
	{
		typ = defans;
		minargs = 3;
		maxargs = 4;
		if ( argc == maxargs )
			esc = 1;
	}
	else
	{
		(void)fprintf(stderr, "%s is an unrecognised question\n", cp);
		exit(2);
	}

	if ( argc < minargs )
	{
		(void)fprintf(stderr, "%s needs more args\n", cp);
		exit(2);
	}

	if ( argc > maxargs )
	{
		(void)fprintf(stderr, "%s expects fewer args\n", cp);
		exit(2);
	}

	for ( cp = argv[1] ;; )
	{
		(void)fprintf(stderr, "%s%s", cp, (cp[strlen(cp)-1]==' ')?EmptyString:" ");

		if ( typ == defans )
			(void)fprintf(stderr, "[default: %s%s] ", argv[2], esc?"; <CTL-D> for none":EmptyString);

		(void)clearerr(stdin);

		if ( fgets(buf, sizeof(buf), stdin) == NULL )
			buf[0] = '\0';

		if ( (i = strlen(buf)-1) >= 0 && buf[i] == '\n' )
			buf[i] = '\0';
		else
			fputc('\n', stderr);

		switch ( typ )
		{
		case yes:
		case no:
			if ( (buf[0]|040) == 'y' )
				exit(typ==no);
			else
			if ( buf[0] == '\0' || (buf[0]|040) == 'n')
				exit(typ==yes);
			(void)fprintf(stderr, "\n\007Please answer yes or no\n");
			continue;

		case ans:
			if ( buf[0] != '\0' || argc < 3 )
				break;
			(void)fprintf(stderr, "%s\n", argv[2]);
			continue;

		case defans:
			if ( buf[0] == '\0' )
			{
				if ( i == 0 )
					strcpy(buf, argv[2]);
				else
				if ( !esc )
					continue;
			}
		}

		if ( buf[0] != '\0' )
			(void)fprintf(stdout, "%s\n", buf);

		exit(0);
		return 0;
	}
}
