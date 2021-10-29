/*
===========================================================================

Return to Castle Wolfenstein single player GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Return to Castle Wolfenstein single player GPL Source Code (RTCW SP Source Code).  

RTCW SP Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RTCW SP Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RTCW SP Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the RTCW SP Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the
terms and conditions of the GNU General Public License which accompanied the RTCW SP
Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms,
you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "ef_local.h"

replacefunc_t *replacefuncs;
int	numfuncs;

#define MAX_TOKEN_LIST  64
tokenList_t tokenList[MAX_TOKEN_LIST];
int tokenListHead = 0;
int verbose = 0;

// the function names
//#define DEFAULT_FUNCBASE "g_func"
static char *func_listfile = "g_func_list.h";
static char *func_decsfile = "g_func_decs.h";

#define TEMP_LIST_NAME		"g_func_list.tmp"
#define TEMP_DECS_NAME		"g_func_decs.tmp"

//===========================================================================

#if 0
/*
=================
WriteWhiteSpace
=================
*/
void WriteWhiteSpace (FILE *fp, script_t *script)
{
	int c;
	//write out the white space
	c = PS_NextWhiteSpaceChar( script );
	while ( c )
	{
		//NOTE: do NOT write out carriage returns (for unix/linux compatibility
		if ( c != 13 ) {
			fputc( c, fp );
		}
		c = PS_NextWhiteSpaceChar( script );
	} // end while
} // end of the function WriteWhiteSpace

/*
=================
WriteString
=================
*/
void WriteString (FILE *fp, script_t *script)
{
	char *ptr;

	ptr = script->endwhitespace_p;
	while ( ptr < script->script_p )
	{
		fputc( *ptr, fp );
		ptr++;
	} // end while
} // end of the function WriteString

/*
=================
ScrewUpFile
=================
*/
void ScrewUpFile (char *oldfile, char *newfile)
{
	FILE *fp;
	script_t *script;
	token_t token;
	replacefunc_t *f;
	char *ptr;

	printf( "screwing up file %s\n", oldfile );
	script = LoadScriptFile( oldfile );
	if ( !script ) {
		Error( "error opening %s\n", oldfile );
	}
	fp = fopen( newfile, "wb" );
	if ( !fp ) {
		Error( "error opening %s\n", newfile );
	}
	//
	while ( PS_ReadToken( script, &token ) )
	{
		WriteWhiteSpace( fp, script );
		if ( token.type == TT_NAME )
		{
			f = FindFunctionName( token.string );
			if ( f ) {
				ptr = f->newname;
			} else { ptr = token.string;}
			while ( *ptr )
			{
				fputc( *ptr, fp );
				ptr++;
			} //end while
		} //end if
		else
		{
			WriteString( fp, script );
		} // end else
	} // end while
	WriteWhiteSpace( fp, script );
	FreeMemory( script );
	fclose( fp );
} // end of the function ScrewUpFile
#endif

/*
=================
Error
=================
*/
void Error (char *error, ...)
{
	va_list argptr;

	va_start( argptr, error );
	vprintf( error, argptr );
	va_end( argptr );

	exit( 1 );
}

/*
=================
DumpReplaceFunctions
=================
*/
void DumpReplaceFunctions (char *typeName)
{
	replacefunc_t	*rf;
	char			path[_MAX_PATH];
	FILE			*f;
	int				len, newlen;
	unsigned char	*buf, *newbuf;
	int				updated;
	// Knightmare added
	size_t			i, j, k, l;
	char			varNameBuf[256];

	updated = 0;

	// dump the function header
//	strncpy (path, ".");
//	strncat (path, PATHSEPERATOR_STR);
//	strncat (path, TEMP_LIST_NAME);
	Q_strncpyz (path, sizeof(path), ".");
	Q_strncatz (path, sizeof(path), PATHSEPERATOR_STR);
	Q_strncatz (path, sizeof(path), TEMP_LIST_NAME);
	Log_Open( path );
	for ( rf = replacefuncs; rf; rf = rf->next )
	{
		if (typeName)
		{
			// TODO: output all elements of an array
			if (rf->isArray)
			{
				for (i = 0; i < rf->arrayDimSizes[0]; i++)
				{
					for (j = 0; j < rf->arrayDimSizes[1]; j++)
					{
						for (k = 0; k < rf->arrayDimSizes[2]; k++)
						{
							for (l = 0; l < rf->arrayDimSizes[3]; l++) {
								Com_sprintf (varNameBuf, sizeof(varNameBuf), "%s[%d][%d][%d][%d]", rf->name, i, j, k, l);
								Log_Print( "{\"%s\", &%s},\n", varNameBuf, varNameBuf );
							}
							if (rf->nArrayDims == 3) {
								Com_sprintf (varNameBuf, sizeof(varNameBuf), "%s[%d][%d][%d]", rf->name, i, j, k);
								Log_Print( "{\"%s\", &%s},\n", varNameBuf, varNameBuf );
							}
						}
						if (rf->nArrayDims == 2) {
							Com_sprintf (varNameBuf, sizeof(varNameBuf), "%s[%d][%d]", rf->name, i, j);
							Log_Print( "{\"%s\", &%s},\n", varNameBuf, varNameBuf );
						}
					}
					if (rf->nArrayDims == 1) {
						Com_sprintf (varNameBuf, sizeof(varNameBuf), "%s[%d]", rf->name, i);
						Log_Print( "{\"%s\", &%s},\n", varNameBuf, varNameBuf );
					}
				}
			}
			else {
				Log_Print( "{\"%s\", &%s},\n", rf->name, rf->name );
			}
		}
		else
			Log_Print( "{\"%s\", (byte *)%s},\n", rf->name, rf->name );
	} // end for
	Log_Print( "{0, 0}\n" );
	Log_Close();

	// if it's different, rename the file over the real header
//	strncpy (path, TEMP_LIST_NAME);
	Q_strncpyz (path, sizeof(path), TEMP_LIST_NAME);
	f = fopen( path, "rb" );
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	buf = (unsigned char *) malloc( len + 1 );
	fseek( f, 0, SEEK_SET );
	fread( buf, len, 1, f );
	buf[len] = 0;
	fclose( f );

//	strncpy (path, func_listfile);
	Q_strncpyz (path, sizeof(path), func_listfile);
	if ( f = fopen( path, "rb" ) )
	{
		fseek( f, 0, SEEK_END );
		newlen = ftell( f );
		newbuf = (unsigned char *) malloc( newlen + 1 );
		fseek( f, 0, SEEK_SET );
		fread( newbuf, newlen, 1, f );
		newbuf[newlen] = 0;
		fclose( f );

		if ( len != newlen || Q_stricmp( buf, newbuf ) )
		{
			char newpath[_MAX_PATH];

			// delete the old file, rename the new one
		//	strncpy (path, func_listfile);
			Q_strncpyz (path, sizeof(path), func_listfile);
			remove( path );

		//	strncpy (newpath, TEMP_LIST_NAME);
			Q_strncpyz (newpath, sizeof(newpath), TEMP_LIST_NAME);
			rename( newpath, path );

#ifdef _WIN32
			// make g_save recompile itself
			remove( "debug\\g_save.obj" );
			remove( "debug\\g_save.sbr" );
			remove( "release\\g_save.obj" );
			remove( "release\\g_save.sbr" );
			remove( "x64\\debug\\g_save.obj" );
			remove( "x64\\debug\\g_save.sbr" );
			remove( "x64\\release\\g_save.obj" );
			remove( "x64\\release\\g_save.sbr" );
#endif

			updated = 1;
		}
		else {
			// delete the old file
		//	strncpy (path, TEMP_LIST_NAME);
			Q_strncpyz (path, sizeof(path), TEMP_LIST_NAME);
			remove( path );
		}
	}
	else {
		rename( TEMP_LIST_NAME, func_listfile );
	}

	free( buf );
	free( newbuf );

	// dump the function declarations
//	strncpy (path, TEMP_DECS_NAME);
	Q_strncpyz (path, sizeof(path), TEMP_DECS_NAME);
	Log_Open( path );
	for ( rf = replacefuncs; rf; rf = rf->next )
	{
		if (typeName)
			Log_Print( "extern %s %s;\n", typeName, rf->dec );
		else
			Log_Print( "extern %s;\n", rf->dec );
	} //end for
	Log_Close();

	// if it's different, rename the file over the real header
//	strncpy (path, TEMP_DECS_NAME);
	Q_strncpyz (path, sizeof(path), TEMP_DECS_NAME);
	f = fopen( path, "rb" );
	fseek( f, 0, SEEK_END );
	len = ftell( f );
	buf = (unsigned char *) malloc( len + 1 );
	fseek( f, 0, SEEK_SET );
	fread( buf, len, 1, f );
	buf[len] = 0;
	fclose( f );

//	strncpy (path, func_decsfile);
	Q_strncpyz (path, sizeof(path), func_decsfile);
	if ( f = fopen( path, "rb" ) )
	{
		fseek( f, 0, SEEK_END );
		newlen = ftell( f );
		newbuf = (unsigned char *) malloc( newlen + 1 );
		fseek( f, 0, SEEK_SET );
		fread( newbuf, newlen, 1, f );
		newbuf[newlen] = 0;
		fclose( f );

		if ( len != newlen || Q_stricmp( buf, newbuf ) )
		{
			char newpath[_MAX_PATH];

			// delete the old file, rename the new one
		//	strncpy (path, func_decsfile);
			Q_strncpyz (path, sizeof(path), func_decsfile);
			remove( path );

		//	strncpy (newpath, TEMP_DECS_NAME);
			Q_strncpyz (newpath, sizeof(newpath), TEMP_DECS_NAME);
			rename( newpath, path );

#ifdef _WIN32
			// make g_save recompile itself
			// NOTE TTimo win32 only? (harmless on *nix anyway)
			remove( "debug\\g_save.obj" );
			remove( "debug\\g_save.sbr" );
			remove( "release\\g_save.obj" );
			remove( "release\\g_save.sbr" );
			remove( "x64\\debug\\g_save.obj" );
			remove( "x64\\debug\\g_save.sbr" );
			remove( "x64\\release\\g_save.obj" );
			remove( "x64\\release\\g_save.sbr" );
#endif

			updated = 1;
		}
		else {
			// delete the old file
		//	strncpy (path, TEMP_DECS_NAME);
			Q_strncpyz (path, sizeof(path), TEMP_DECS_NAME);
			remove( path );
		}
	}
	else {
		rename( TEMP_DECS_NAME, func_decsfile );
	}

	free( buf );
	free( newbuf );

#ifdef _WIN32
	if ( updated )
	{
		if (typeName) {
			printf( "Updated the %s table, recompile required.\n", typeName );
		}
		else {
			printf( "Updated the function table, recompile required.\n" );
		}
	}
#endif
} // end of the function DumpReplaceFunctions

/*
=================
FindFunctionName
=================
*/
replacefunc_t *FindFunctionName (char *funcname)
{
	replacefunc_t *f;

	for (f = replacefuncs; f; f = f->next)
	{
		if ( !strcmp(f->name, funcname) )
		{
			return f;
		}
	} // end for
	return NULL;
} // end of the function FindFunctionName

/*
=================
MayScrewUp
=================
*/
int MayScrewUp (char *funcname)
{
	if ( !strcmp(funcname, "GetBotAPI") )
	{
		return false;
	}
	if ( !strcmp(funcname, "main") )
	{
		return false;
	}
	if ( !strcmp(funcname, "WinMain") )
	{
		return false;
	}
	return true;
} // end of the function MayScrewUp

/*
=================
ConcatDec
=================
*/
void ConcatDec (tokenList_t *list, char *str, size_t strSize, int inc)
{
/*	if (!((list->token.type == TT_NAME) || (list->token.string[0] == '*')))
	{
		if (list->token.string[0] == ')' || list->token.string[0] == '(') {
			if (inc++ >= 2)
				return;
		}
		else {
			return;
		}
	}*/
	if (list->next) {
		ConcatDec (list->next, str, strSize, inc);
	}
//	strncat (str, list->token.string);
//	strncat (str, " " );
	Q_strncatz (str, strSize, list->token.string);
	Q_strncatz (str, strSize, " " );
}

/*
=================
AddFunctionName
=================
*/
void AddFunctionName (char *funcname, char *filename, tokenList_t *head)
{
	replacefunc_t	*f;
	tokenList_t     *list;

	if ( FindFunctionName(funcname) ) {
		return;
	}

#if defined( __linux__ ) || defined (__FreeBSD__)
	// the bad thing is, this doesn't preprocess .. on __linux__ this
	// function is not implemented (q_math.c)
	if ( !Q_stricmp( funcname, "BoxOnPlaneSide" ) ) {
		return;
	}
#endif

	// NERVE - SMF - workaround for Graeme's predifined MACOSX functions
	// TTimo - looks like linux version needs to escape those too
#if defined( _WIN32 ) || defined( __linux__ ) || defined (__FreeBSD__)
	if ( !Q_stricmp(funcname, "qmax") ) {
		return;
	}
	else if ( !Q_stricmp(funcname, "qmin") ) {
		return;
	}
#endif
	// -NERVE - SMF

	f = (replacefunc_t *) GetMemory(sizeof(replacefunc_t) + (int)strlen(funcname) + 1 + 6 + (int)strlen(filename) + 1);
	f->name = (char *)f + sizeof(replacefunc_t);
//	strncpy (f->name, funcname);
	Q_strncpyz (f->name, strlen(funcname) + 1, funcname);
	f->newname = (char *)f + sizeof(replacefunc_t) + strlen(funcname) + 1;
//	sprintf (f->newname, "F%d", numfuncs++);
	Com_sprintf (f->newname, 6, "F%d", numfuncs++);
	f->filename = (char *)f + sizeof(replacefunc_t) + strlen(funcname) + 1 + strlen(f->newname) + 1;
//	strncpy (f->filename, filename);
	Q_strncpyz (f->filename, strlen(filename) + 1, filename);
	f->next = replacefuncs;
	replacefuncs = f;

	// construct the declaration
	list = head;
	f->dec[0] = '\0';
	ConcatDec (list, f->dec, sizeof(f->dec), 0);
} // end of the function AddFunctionName

/*
=================
ConcatVarDec

Knightmare- this builds out var declaration
=================
*/
void ConcatVarDec (tokenList_t *list, char *str, size_t strSize, int nDims, size_t DimSizes[])
{
	int		i;
	char	buf[32];	

	if (list->next) {
		ConcatVarDec (list->next, str, strSize, nDims, DimSizes);
	}
	Q_strncatz (str, strSize, list->token.string);
	for (i = 0; i < nDims; i++) {
		Com_sprintf (buf, sizeof(buf), "[%d]", DimSizes[i]);
		Q_strncatz (str, strSize, buf );
	}
	Q_strncatz (str, strSize, " " );
}

/*
=================
AddVarName

Knightmare- this adds structs / vars of a given type
=================
*/
void AddVarName (char *funcname, int nDims, size_t DimSizes[], char *filename, tokenList_t *head)
{
	int				i;
	replacefunc_t	*f;
	tokenList_t     *list;

	if ( FindFunctionName(funcname) ) {
		return;
	}

	f = (replacefunc_t *) GetMemory(sizeof(replacefunc_t) + (int)strlen(funcname) + 1 + 6 + (int)strlen(filename) + 1);
	f->name = (char *)f + sizeof(replacefunc_t);
	Q_strncpyz (f->name, strlen(funcname) + 1, funcname);
	f->newname = (char *)f + sizeof(replacefunc_t) + strlen(funcname) + 1;
	Com_sprintf (f->newname, 6, "F%d", numfuncs++);
	f->filename = (char *)f + sizeof(replacefunc_t) + strlen(funcname) + 1 + strlen(f->newname) + 1;
	Q_strncpyz (f->filename, strlen(filename) + 1, filename);
	if (nDims > 0)
	{
		f->isArray = 1;
		f->nArrayDims = min(nDims, MAX_VAR_ARRAY_DIMENSIONS);
		for (i = 0; i < MAX_VAR_ARRAY_DIMENSIONS; i++) {
			f->arrayDimSizes[i] = DimSizes[i];
		}
	/*	printf ("AddVarName: var %s has array size(s) of: ", funcname);
		for (i = 0; i < f->nArrayDims; i++) {
			printf ("[%d]", f->arrayDimSizes[i]);
		}
		printf ("\n"); */
	}
	else {
		f->isArray = 0;
		f->nArrayDims = 0;
		memset (f->arrayDimSizes, 0, sizeof(f->arrayDimSizes));
	}
	f->next = replacefuncs;
	replacefuncs = f;

	// construct the declaration
	list = head;
	f->dec[0] = '\0';
	ConcatVarDec (list, f->dec, sizeof(f->dec), f->nArrayDims, f->arrayDimSizes);
}

/*
=================
AddTokenToList
=================
*/
void AddTokenToList (tokenList_t **head, token_t *token)
{
	tokenList_t *newhead;

	newhead = &tokenList[tokenListHead++]; //GetMemory( sizeof( tokenList_t ) );
	if ( tokenListHead == MAX_TOKEN_LIST ) {
		tokenListHead = 0;
	}

	newhead->next = *head;
	newhead->token = *token;

	*head = newhead;
}

#if 0
/*
=================
KillTokenList
=================
*/
void KillTokenList (tokenList_t *head)
{
	if (head->next) {
		KillTokenList( head->next );
		FreeMemory( head->next );
		head->next = NULL;
	}
}
#endif

/*
=================
StripTokenList
=================
*/
void StripTokenList (tokenList_t *head)
{
	tokenList_t *trav, *lastTrav;

	trav = head;

	// now go back to the start of the declaration
	lastTrav = trav;
	trav = trav->next;  // should be on the function name now
	while ( ( trav->token.type == TT_NAME ) || ( trav->token.string[0] == '*' ) )
	{
		lastTrav = trav;
		trav = trav->next;
		if ( !trav ) {
			return;
		}
	}
	// now kill everything after lastTrav
//	KillTokenList (lastTrav);
	lastTrav->next = NULL;
}

/*
=================
GetVarNamesFromFile

Knightmare- this gets structs / vars of a given type
=================
*/
void GetVarNamesFromFile (char *filename, char *typeName)
{
	source_t	*source;
	token_t		token, lastToken, varToken;
	int			indent = 0;
	int			isStatic = 0;
	int			isExtern = 0;
	int			isArray = 0;
	int			sqBracketLevel = 0;
	int			nArrayDims = 0;
	size_t		arrayDimSizes[MAX_VAR_ARRAY_DIMENSIONS] = {0};
	tokenList_t	*listHead;

	listHead = NULL;
	source = LoadSourceFile (filename);
	if ( !source ) {
		Error ("error opening %s", filename);
		return;
	}

	while ( 1 )
	{
		if ( !PC_ReadToken(source, &token) ) {
			break;
		}
		if ( token.type == TT_PUNCTUATION )
		{
			switch ( token.string[0] )
			{
			case ';':
				isStatic = 0;
				isExtern = 0;
				isArray = 0;
				nArrayDims = 0;
				memset(arrayDimSizes, 0, sizeof(arrayDimSizes));
				break;
			case '{':
				indent++;
				break;
			case '}':
				indent--;
				if ( indent < 0 )
					indent = 0;
				break;
			}
		}
		if ( token.type == TT_NAME )
		{	// type declarations for pointer table must be non-static, non-extern, and global in scope
			if ( token.string[0] == 's' && !strcmp( token.string, "static" ) ) {
				isStatic = 1;
			}
			if ( token.string[0] == 'e' && !strcmp( token.string, "extern" ) ) {
				isExtern = 1;
			}
			if ( !isStatic && !isExtern && (indent == 0) && !strcmp(token.string, typeName) )	// type name matches
			{
				if ( PC_ReadToken(source, &token) )		// next token should be var name
				{
					if ( token.type == TT_NAME )
					{
						memcpy( &varToken, &token, sizeof(token_t) );	// save var name

						// look for array sizes after var name
						while ( PC_ReadToken(source, &token) )
						{
							if ( token.string[0] == ';' ) {	// end of declaration
								break;
							}
							// look for array dimensions ( e.g. var[x]... )
							if ( token.type == TT_PUNCTUATION )
							{
								switch ( token.string[0] )
								{
								case '[':
									isArray = 1;
									sqBracketLevel++;
									break;
								case ']':
									sqBracketLevel--;
									if ( sqBracketLevel < 0 )
										sqBracketLevel = 0;
									break;
								}
							}
							// set array sizes
							if ( isArray && (token.type == TT_NUMBER) && (sqBracketLevel == 1) )
							{
								if (nArrayDims < MAX_VAR_ARRAY_DIMENSIONS) {
									arrayDimSizes[nArrayDims] = token.intvalue;							
									nArrayDims++;
								}
								else {
									printf ("Too many array dimensions for var %s in file %s!\n", varToken.string, filename);
								}
							}
						}

						if (listHead)
							listHead->next = NULL;
						listHead = NULL;
						AddTokenToList (&listHead, &varToken);
						AddVarName (varToken.string, nArrayDims, arrayDimSizes, filename, listHead);
					//	AddTokenToList (&listHead, &token);
					//	AddVarName (token.string, nArrayDims, arrayDimSizes, filename, listHead);

						// reset array sizes
						isArray = 0;
						nArrayDims = 0;
						memset(arrayDimSizes, 0, sizeof(arrayDimSizes));
					}
				}
			}
		}
		memcpy( &lastToken, &token, sizeof(token_t) );
	}
	FreeSource (source);
}

/*
=================
GetFunctionNamesFromFile
=================
*/
void GetFunctionNamesFromFile (char *filename)
{
	source_t	*source;
	token_t		token, lasttoken;
	int			indent = 0, brace;
	int			isStatic = 0;
	tokenList_t	*listHead;

	// filter some files out
	if ( !Q_stricmp( filename, "bg_lib.c" ) ) {
		return;
	}

	listHead = NULL;
	source = LoadSourceFile (filename);
	if ( !source ) {
		Error ("error opening %s", filename);
		return;
	}

//	printf("loaded %s\n", filename);
//	if (!PC_ReadToken(source, &lasttoken))
//	{
//		FreeSource(source);
//		return;
//	} //end if
	while ( 1 )
	{
		if ( !PC_ReadToken(source, &token) ) {
			break;
		}
		AddTokenToList (&listHead, &token);
		if ( token.type == TT_PUNCTUATION )
		{
			switch ( token.string[0] )
			{
			case ';':
				isStatic = 0;
				break;
			case '{':
				indent++;
				break;
			case '}':
				indent--;
				if ( indent < 0 )
					indent = 0;
				break;
			case '(':
				if ( indent <= 0 && lasttoken.type == TT_NAME )
				{
					StripTokenList (listHead);

					brace = 1;
					while ( PC_ReadToken(source, &token) )
					{
						AddTokenToList (&listHead, &token);
						if ( token.string[0] == '(' ) {
							brace++;
						}
						else if ( token.string[0] == ')' )
						{
							brace--;
							if ( brace <= 0 )
							{
								if ( !PC_ReadToken(source, &token) ) {
									break;
								}
								if ( token.string[0] == '{' ) {
									indent++;
									if ( !isStatic && MayScrewUp(lasttoken.string) ) {
										AddFunctionName (lasttoken.string, filename, listHead);
									}
								}	// end if
								break;
							}	// end if
						}	// end if
					}	// end while
				}	// end if
				break;
			}	// end switch
		}	// end if
		if ( token.type == TT_NAME )
		{
			if ( token.string[0] == 's' && !strcmp( token.string, "static" ) ) {
				isStatic = 1;
			}
		}
		memcpy( &lasttoken, &token, sizeof(token_t) );
	} // end while	
	FreeSource (source);
} // end of the function GetFunctionNamesFromFile

/*
=================
Usage
=================
*/
void Usage (void)
{
//	Error( "USAGE SCREWUP: extractfuncs <file filter>\n" );
	Error( "USAGE SCREWUP: extractfuncs <file1> [<file2> ..] [-t <structname>] [-o <func_list> <func_decs>] [-d <define>]\n"
		   "no -o defaults to g_func_list.h g_func_decs.h or g_<structname>_list.h g_<structname>_decs.h\n" );
}

/*
=================
main
=================
*/
#ifdef _WIN32
void main (int argc, char *argv[])
{
	WIN32_FIND_DATA	filedata;
	HWND			handle;
	int				i, firstParm, done;
	char			typeName[128];
	qboolean		typeExtract = false;
	qboolean		firstParmSet = false;

//	if ( argc < 2 )
//		Usage ();

	// Knightmare- check for command line switches
	for (i=0; i<argc; i++)
	{
		if (!firstParmSet && argv[i][0] == '-')
		{
			firstParm = i;
			firstParmSet = true;
		}

		if ( !Q_stricmp(argv[i], "-t") && (i < argc-1) && (argv[i+1][0] != '-') )
		{
			char	buf[_MAX_PATH];
			Com_sprintf (typeName, sizeof(typeName), argv[i+1]);
			printf("Data type to extract: %s\n", typeName);
			Com_sprintf (buf, sizeof(buf), "g_%s_list.h", typeName);
			func_listfile = _strdup(buf);
			Com_sprintf (buf, sizeof(buf), "g_%s_decs.h", typeName);
			func_decsfile = _strdup(buf);
			typeExtract = true;
		}
		else if ( !Q_stricmp(argv[i], "-o") && (i < argc-1) && (argv[i+1][0] != '-') )
		{
			func_listfile = argv[i+1];
			printf("Pointer table list file: %s\n", func_listfile);
			if ( (i < argc-2) && (argv[i+2][0] != '-') )
			{
				func_decsfile = argv[i+2];
				printf("Pointer list decs file: %s\n", func_decsfile);
			}
		}
		else if (!Q_stricmp(argv[i], "-d")) {
			printf("Define option not yet implemented.\n");
		}
	}

	if ( (argc < 1) || (firstParmSet && firstParm < 1) )
		Usage ();
	// end Knightmare

	handle = FindFirstFile (argv[1], &filedata);
	done = (handle == INVALID_HANDLE_VALUE);
	while (!done)
	{
		if ( !(filedata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			 if (typeExtract)
				GetVarNamesFromFile (filedata.cFileName, typeName);
			 else
				GetFunctionNamesFromFile (filedata.cFileName);
		}
		// find the next file
		done = !FindNextFile (handle, &filedata);
	}
	if (typeExtract)
		DumpReplaceFunctions (typeName);
	else
		DumpReplaceFunctions (NULL);
}
#else
/*
*nix version, let the shell do the pattern matching
(that's what shells are for :-))
*/
int main (int argc, char *argv[])
{
	int			i, firstParm;
	int			argbase = 1;
	char		typeName[128];
	qboolean	typeExtract = false;
	qboolean	firstParmSet = false;

/*	if ( argc < 2 )
		Usage();

	if ( !Q_stricmp(argv[1],"-o") )
	{
		if ( argc < 5 ) {
			Usage ();
		}
		func_listfile = argv[2];
		func_decsfile = argv[3];
		argbase = 4;
	}
*/
	// Knightmare- check for command line switches
	for (i=0; i<argc; i++)
	{
		if (!firstParmSet && argv[i][0] == '-')
		{
			firstParm = i;
			firstParmSet = true;
		}

		if ( !Q_stricmp(argv[i], "-t") && (i < argc-1) && (argv[i+1][0] != '-') )
		{
			char	buf[_MAX_PATH];
			Com_sprintf (typeName, sizeof(typeName), argv[i+1]);
			printf("Data type to extract: %s\n", typeName);
			Com_sprintf (buf, sizeof(buf), "g_%s_list.h", typeName);
			func_listfile = strdup(buf);
			Com_sprintf (buf, sizeof(buf), "g_%s_decs.h", typeName);
			func_decsfile = strdup(buf);
			typeExtract = true;
		}
		else if ( !Q_stricmp(argv[i], "-o") && (i < argc-1) && (argv[i+1][0] != '-') )
		{
			func_listfile = argv[i+1];
			printf("Pointer table list file: %s\n", func_listfile);
			if ( (i < argc-2) && (argv[i+2][0] != '-') )
			{
				func_decsfile = argv[i+2];
				printf("Pointer list decs file: %s\n", func_decsfile);
			}
		}
		else if (!Q_stricmp(argv[i], "-d")) {
			printf("Define option not yet implemented.\n");
		}
	}

	if (argc < 1 || (firstParmSet && firstParm < 1))
		Usage ();
	// end Knightmare

//	for ( i = argbase; i < argc; i++ )
	for ( i = argbase; i < firstParm; i++ )
	{
	//	printf( "%d: %s\n", i, argv[i] );
		if (typeExtract)
			GetVarNamesFromFile (argv[i], typeName);
		else
			GetFunctionNamesFromFile (argv[i]);
	}
	if (typeExtract)
		DumpReplaceFunctions (typeName);
	else
		DumpReplaceFunctions (NULL);
}

#endif
