/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CALL = 258,
     CLOSE = 259,
     DAEMON = 260,
     DEVICE = 261,
     EXECD = 262,
     EXPECT = 263,
     FAIL = 264,
     FORKD = 265,
     FORKC = 266,
     MATCH = 267,
     MODE = 268,
     MONITOR = 269,
     OPEN = 270,
     READ = 271,
     READCHAR = 272,
     RETRY = 273,
     RETURN = 274,
     SHELL = 275,
     SLEEP = 276,
     TIME_OUT = 277,
     TRACE = 278,
     WRITE = 279,
     SLOWWRITE = 280,
     CRC = 281,
     ID = 282,
     NEXT = 283,
     SET = 284,
     TEST = 285,
     SHIFT = 286,
     NUMBER = 287,
     STRING = 288
   };
#endif
/* Tokens.  */
#define CALL 258
#define CLOSE 259
#define DAEMON 260
#define DEVICE 261
#define EXECD 262
#define EXPECT 263
#define FAIL 264
#define FORKD 265
#define FORKC 266
#define MATCH 267
#define MODE 268
#define MONITOR 269
#define OPEN 270
#define READ 271
#define READCHAR 272
#define RETRY 273
#define RETURN 274
#define SHELL 275
#define SLEEP 276
#define TIME_OUT 277
#define TRACE 278
#define WRITE 279
#define SLOWWRITE 280
#define CRC 281
#define ID 282
#define NEXT 283
#define SET 284
#define TEST 285
#define SHIFT 286
#define NUMBER 287
#define STRING 288




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 19 "grammar.y"
{
	Symbol  *ysym;		/* ysymbol table pointer */
	char	*str;		/* string value */
	long	num;		/* integer */
	Op	*oper;		/* operation */
	Pat	*pattern;	/* pattern */
	Vlist	*vlist;		/* variable list */
}
/* Line 1489 of yacc.c.  */
#line 124 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

