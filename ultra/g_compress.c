// Zlib-based compression and decompression for Eraser routing system
// For KMQ2-specific builds, this hooks into the filesystem's exported funcs instead.

#include "g_local.h"

#ifdef KMQUAKE2_ENGINE_MOD
#define	USE_ENGINE_COMPRESSION
#endif	// KMQUAKE2_ENGINE_MOD

#ifndef USE_ENGINE_COMPRESSION
#include "zlib/unzip.h"
#include "zlib/zip.h"

#define MAX_G_FILEHANDLES 8

typedef struct {
	char		name[MAX_OSPATH+MAX_QPATH];
	fsMode_t	mode;
	FILE		*file;				// Only one of file or
	unzFile		*readZip;			// zip will be used
	zipFile		*writeZip;			// Only used for writing to zip file
} g_fHandle_t;

g_fHandle_t		g_fHandles[MAX_G_FILEHANDLES];
#endif	// USE_ENGINE_COMPRESSION

//=========================================================================

#ifndef USE_ENGINE_COMPRESSION
/*
================
G_HandleForFile

Finds a free g_fHandle_t
================
*/
g_fHandle_t *G_HandleForFile (const char *path, fileHandle_t *f)
{
	int			i;
	g_fHandle_t	*handle;

	handle = g_fHandles;
	for (i = 0; i < MAX_G_FILEHANDLES; i++, handle++)
	{
		if (!handle->readZip && !handle->writeZip)
		{
			Com_strcpy (handle->name, sizeof(handle->name), path);
			*f = i +1;
			return handle;
		}
	}

	gi.error ("G_HandleForFile: none free");
	return 0;
}

/*
================
G_GetFileByHandle

Returns a g_fHandle_t * for the given fileHandle_t
================
*/
g_fHandle_t *G_GetFileByHandle (fileHandle_t f)
{
	if (f <= 0 || f > MAX_G_FILEHANDLES)
		gi.error ("G_GetFileByHandlep: out of range");

	return &g_fHandles[f-1];
}

/*
=================
G_FileLength
=================
*/
int G_FileLength (FILE *f)
{
	int	cur, end;

	cur = ftell(f);
	fseek (f, 0, SEEK_END);
	end = ftell(f);
	fseek (f, cur, SEEK_SET);

	return end;
}

/*
=================
G_OpenFileAppend

Returns file size or -1 on error
=================
*/
int G_OpenFileAppend (g_fHandle_t *handle)
{
	char	path[MAX_OSPATH];

	// Include game path, but check for leading /
	if (handle->name[0] == '/')
		CreatePath (va("%s%s", SavegameDir(), handle->name));
	else
		CreatePath (va("%s/%s", SavegameDir(), handle->name));

	Com_sprintf(path, sizeof(path), "%s/%s", SavegameDir(), handle->name);

	handle->file = fopen(path, "ab");
	if (handle->file)
	{
		gi.dprintf ("G_OpenFileAppend: %s\n", path);
		return G_FileLength(handle->file);
	}

	gi.dprintf ("G_OpenFileAppend: couldn't open %s\n", path);

	return -1;
}

/*
=================
G_OpenFileWrite

Always returns 0 or -1 on error
=================
*/
int G_OpenFileWrite (g_fHandle_t *handle)
{
	char	path[MAX_OSPATH];

	// Include game path, but check for leading /
	if (handle->name[0] == '/')
		CreatePath (va("%s%s", SavegameDir(), handle->name));
	else
		CreatePath (va("%s/%s", SavegameDir(), handle->name));

	Com_sprintf(path, sizeof(path), "%s/%s", SavegameDir(), handle->name);

	handle->file = fopen(path, "wb");
	if (handle->file)
	{
		gi.dprintf ("G_OpenFileWrite: %s\n", path);
		return 0;
	}

	gi.dprintf ("G_OpenFileWrite: couldn't open %s\n", path);

	return -1;
}

/*
=================
G_OpenFileRead

Returns file size or -1 if not found.
Only opens separate, uncompressed files.
=================
*/
int G_OpenFileRead (g_fHandle_t *handle)
{
	char			path[MAX_OSPATH];
	char			savegameDir[MAX_OSPATH];
	char			gameDir[MAX_OSPATH];

	Com_strcpy (savegameDir, sizeof(savegameDir), SavegameDir());
	Com_strcpy (gameDir, sizeof(gameDir), GameDir());
	SavegameDirRelativePath (handle->name, path, sizeof(path));
	handle->file = fopen(path, "rb");
	if ( !handle->file && (Q_stricmp(savegameDir, gameDir) != 0) ) {	// try gamedir path instead if different from savegame dir
	//	gi.dprintf ("G_OpenFileRead: can't open %s, falling back from SavegameDir (%s) to GameDir (%s)\n", path, savegameDir, gameDir);
		GameDirRelativePath (handle->name, path, sizeof(path));
		handle->file = fopen(path, "rb");
	}
	if (handle->file) {
		// Found it!
	//	gi.dprintf ("G_OpenFileRead: %s (found in %s)\n", handle->name, path);
		return G_FileLength (handle->file);
	}

	// Not found!
//	gi.dprintf ("G_OpenFileRead: couldn't find %s\n", handle->name);

	return -1;
}
#endif	// USE_ENGINE_COMPRESSION

/*
=================
G_OpenFile

Opens a file for "mode".
Returns file size or -1 if an error occurs/not found.
Only opens separate, uncompressed files.
=================
*/
int G_OpenFile (const char *name, fileHandle_t *f, fsMode_t mode)
{
#ifdef USE_ENGINE_COMPRESSION
	return gi.OpenFile(name, f, mode);
#else	// USE_ENGINE_COMPRESSION

	g_fHandle_t	*handle;
	int			size;

	handle = G_HandleForFile(name, f);

	Com_strcpy (handle->name, sizeof(handle->name), name);
	handle->mode = mode;

	switch (mode)
	{
	case FS_READ:
		size = G_OpenFileRead (handle);
		break;
	case FS_WRITE:
		size = G_OpenFileWrite (handle);
		break;
	case FS_APPEND:
		size = G_OpenFileAppend (handle);
		break;
	default:
		gi.error ("G_FOpenFile: bad mode (%i)", mode);
	}

	if (size != -1) {
		return size;
	}

	// Couldn't open, so free the handle
	memset(handle, 0, sizeof(*handle));

	*f = 0;
	return -1;
#endif	// USE_ENGINE_COMPRESSION
}

#ifndef USE_ENGINE_COMPRESSION
/*
================
G_FOpenCompressedWrite

Always returns 0 or -1 on error
Opens files directly from inside a specified zip file,
looking only in the current gamedir.
================
*/
int G_OpenCompressedFileWrite (g_fHandle_t *handle, const char *zipName, const char *fileName, qboolean add)
{
	char	path[MAX_OSPATH];
	int		append;

	// Include game path, but check for leading /
	if (*zipName == '/')
		CreatePath (va("%s%s", SavegameDir(), zipName));
	else
		CreatePath (va("%s/%s", SavegameDir(), zipName));

	Com_sprintf(path, sizeof(path), "%s/%s", SavegameDir(), zipName);

	append = add ? (LocalFileExists((char *)zipName) ? 2 : 0) : 0;
	handle->writeZip = (void **)zipOpen(path, append);
	if (handle->writeZip)
	{
		if ( zipOpenNewFileInZip(handle->writeZip, fileName, NULL, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION) == ZIP_OK ) {
		//	gi.dprintf ("G_OpenCompressedFileWrite: %s/%s\n", path, fileName);
			return 0;
		}
		zipClose(handle->writeZip, NULL);
	}

	gi.dprintf ("G_OpenCompressedFileWrite: couldn't open %s/%s\n", path, fileName);

	return -1;
}

/*
================
G_OpenCompressedFileRead

Returns file size or -1 if not found.
Opens files directly from inside a specified zip file.
================
*/
int G_OpenCompressedFileRead (g_fHandle_t *handle, const char *zipName, const char *fileName)
{
	char			path[MAX_OSPATH];
	char			savegameDir[MAX_OSPATH];
	char			gameDir[MAX_OSPATH];
	unz_file_info	info;

	Com_strcpy (savegameDir, sizeof(savegameDir), SavegameDir());
	Com_strcpy (gameDir, sizeof(gameDir), GameDir());
	SavegameDirRelativePath (zipName, path, sizeof(path));
	handle->readZip = (void **)unzOpen(path);
	if ( !handle->readZip && (Q_stricmp(savegameDir, gameDir) != 0) ) {	// try gamedir path instead if different from savegame dir
	//	gi.dprintf ("G_OpenCompressedFileRead: can't open %s, falling back from SavegameDir (%s) to GameDir (%s)\n", path, savegameDir, gameDir);
		GameDirRelativePath (zipName, path, sizeof(path));
		handle->readZip = (void **)unzOpen(path);
	}
	if (handle->readZip)
	{
	//	gi.dprintf ("G_OpenCompressedFileRead: searching for %s in %s\n", fileName, zipName);

		if ( unzLocateFile(handle->readZip, fileName, 2) == UNZ_OK )
		{
			if ( unzOpenCurrentFile(handle->readZip) == UNZ_OK ) {
				// Found it!
			//	gi.dprintf ("G_OpenCompressedFileRead: %s (found in %s)\n", fileName, path);
				unzGetCurrentFileInfo(handle->readZip, &info, NULL, 0, NULL, 0, NULL, 0);
				return info.uncompressed_size;
			}
		}
		unzClose(handle->readZip);
	}

	// Not found!
//	gi.dprintf ("G_OpenCompressedFileRead: couldn't find %s\n", handle->name);

	return -1;
}
#endif	// USE_ENGINE_COMPRESSION

/*
================
G_OpenCompressedFile

Opens a zip file for "mode".
Returns file size or -1 if an error occurs/not found.
Opens files directly from inside a specified zip file.
================
*/
int G_OpenCompressedFile (const char *zipName, const char *fileName, fileHandle_t *f, fsMode_t mode)
{
#ifdef USE_ENGINE_COMPRESSION
	return gi.OpenCompressedFile (zipName, fileName, f, mode);
#else	// USE_ENGINE_COMPRESSION
	g_fHandle_t	*handle = NULL;
	char		name[MAX_OSPATH+MAX_QPATH];
	int			size = -1;

	Com_sprintf(name, sizeof(name), "%s/%s", zipName, fileName);
	handle = G_HandleForFile (name, f);
	if (!handle)
	{
		*f = 0;
		return -1;
	}

	Com_strcpy (handle->name, sizeof(handle->name), name);
	handle->mode = mode;

	switch (mode)
	{
	case FS_READ:
		size = G_OpenCompressedFileRead(handle, zipName, fileName);
		break;
	case FS_WRITE:
		size = G_OpenCompressedFileWrite(handle, zipName, fileName, false);
		break;
	case FS_APPEND:
		size = G_OpenCompressedFileWrite(handle, zipName, fileName, true);
		break;
	default:
		gi.error ("G_OpenCompressedFile: bad mode (%d)", mode);
	}

	if (size != -1) {
		return size;
	}

	// Couldn't open, so free the handle
	memset (handle, 0, sizeof(*handle));

	*f = 0;
	return -1;
#endif	// USE_ENGINE_COMPRESSION
}

/*
================
G_CloseFile

Closes a zip file.
================
*/
void G_CloseFile (fileHandle_t f)
{
#ifdef USE_ENGINE_COMPRESSION
	gi.CloseFile (f);
#else	// USE_ENGINE_COMPRESSION
	g_fHandle_t	*handle;

	handle = G_GetFileByHandle(f);

	if (handle->file) {
		fclose(handle->file);
	}
	if (handle->readZip) {
		unzCloseCurrentFile(handle->readZip);
		unzClose(handle->readZip);
	}
	else if (handle->writeZip) {
		zipCloseFileInZip(handle->writeZip);
		zipClose(handle->writeZip, NULL);
	}

	memset (handle, 0, sizeof(*handle));
#endif	// USE_ENGINE_COMPRESSION
}

/*
================
G_FRead

Handles partial reads from zip files
================
*/
int G_FRead (void *buffer, int size, fileHandle_t f)
{
#ifdef USE_ENGINE_COMPRESSION
	return gi.FRead (buffer, size, f);
#else	// USE_ENGINE_COMPRESSION
	g_fHandle_t	*handle;
	int			remaining, r;
	byte		*buf;
	qboolean	tried = false;

	handle = G_GetFileByHandle(f);
	remaining = size;
	buf = (byte *)buffer;

	while (remaining)
	{
		if (handle->file)
			r = (int)fread(buf, 1, remaining, handle->file);
		else if (handle->readZip)
			r = unzReadCurrentFile(handle->readZip, buf, remaining);
		else
			return 0;

		if (r == 0)
		{
			if (!tried) {	// Might be trying to read from a CD
				tried = true;
			}
			else {	// Already tried once
			//	gi.dprintf ("G_FRead: 0 bytes read from %s\n", handle->name);
				return size - remaining;
			}
		}
		else if (r == -1)
			gi.error ("G_FRead: -1 bytes read from %s", handle->name);

		remaining -= r;
		buf += r;
	}

	return size;
#endif	// USE_ENGINE_COMPRESSION
}

/*
================
G_FWrite

Handles partial writes to zip files
================
*/
int G_FWrite (const void *buffer, int size, fileHandle_t f)
{
#ifdef USE_ENGINE_COMPRESSION
	return gi.FWrite (buffer, size, f);
#else	// USE_ENGINE_COMPRESSION
	g_fHandle_t	*handle;
	int			remaining, w = 0;
	byte		*buf;
	
	handle = G_GetFileByHandle(f);
	remaining = size;
	buf = (byte *)buffer;

	while (remaining)
	{
		if (handle->file)
			w = (int)fwrite(buf, 1, remaining, handle->file);
		else if (handle->writeZip) {
			if ( zipWriteInFileInZip(handle->writeZip, buf, remaining) == ZIP_OK )
				w = remaining;
		}
		else if (handle->readZip)
			gi.error ("G_FWrite: can't write to zip file %s", handle->name);
		else
			return 0;

		if (w == 0) {
		//	gi.dprintf ("G_FWrite: 0 bytes written to %s\n", handle->name);
			return size - remaining;
		}
		else if (w == -1)
			gi.error ("G_FWrite: -1 bytes written to %s", handle->name);

		remaining -= w;
		buf += w;
	}

	return size;
#endif	// USE_ENGINE_COMPRESSION
}

/*
================
G_CompressFile
================
*/
int G_CompressFile (const char *fileName, const char *zipName, const char *internalName, qboolean appendToZip)
{
	int				size, partSize;
	fileHandle_t	f;
	FILE			*fp;
	byte			buf[8192];
	fsMode_t		mode;

//	gi.dprintf ("G_CompressFile: attempting to compress %s into %s with internal name %s.\n", fileName, zipName, internalName);

	fp = fopen (fileName, "rb");
	if (!fp)
		return -1;

	if ( appendToZip ) {
		mode = LocalFileExists((char*)zipName) ? FS_APPEND : FS_WRITE;
	}
	else {
		mode = FS_WRITE;
	}
	size = G_OpenCompressedFile(zipName, internalName, &f, mode);
	if (size == -1) {
		fclose (fp);
		return -1;
	}

	do {
		partSize = (int)fread (&buf, 1, sizeof(buf), fp);
		if (partSize > 0)
			G_FWrite (&buf, partSize, f);
	} while (partSize  > 0);

	G_CloseFile (f);
	fclose (fp);

//	gi.dprintf ("G_CompressFile: sucessfully compressed %s into %s.\n", fileName, zipName);

	return size;
}

/*
================
G_DecompressFile
================
*/
int G_DecompressFile (const char *fileName, const char *zipName, const char *internalName)
{
	int				size, partSize;
	fileHandle_t	f;
	FILE			*fp;
	byte			buf[8192];

//	gi.dprintf ("G_DecompressFile: attempting to decompress %s from %s with internal name %s.\n", fileName, zipName, internalName);

	size = G_OpenCompressedFile (zipName, internalName, &f, FS_READ);
	if (size == -1)
		return -1;

	fp = fopen (fileName, "wb");
	if (!fp) {
		G_CloseFile (f);
		return -1;
	}

	do {
		partSize = G_FRead (&buf, sizeof(buf), f);
		if (partSize > 0)
			fwrite (&buf, 1, partSize, fp);
	} while (partSize > 0);

	fclose (fp);
	G_CloseFile (f);

//	gi.dprintf ("G_DecompressFile: sucessfully decompressed %s from %s.\n", fileName, zipName);

	return size;
}
