#include "7z_dec.h"

#include <stdio.h>
#include <string.h>
#include <memory>

#include "7z.h"
#include "7zAlloc.h"
#include "7zCrc.h"
#include "7zFile.h"

#pragma comment(lib, "7z.lib")

int Extract7zFile(HANDLE h, const wchar_t* ExtractPath)
{
	CFileInStream archiveStream;
	archiveStream.file = *(CSzFile*)&h;
	FileInStream_CreateVTable(&archiveStream);

	CLookToRead* lookStream = (CLookToRead*)SzAlloc(NULL, sizeof(CLookToRead));
	LookToRead_CreateVTable(lookStream, False);

	lookStream->realStream = &archiveStream.s;
	LookToRead_Init(lookStream);

	CrcGenerateTable();

	CSzArEx db;
	SzArEx_Init(&db);

	ISzAlloc allocImp;
	allocImp.Alloc = SzAlloc;
	allocImp.Free = SzFree;

	ISzAlloc allocTempImp;
	allocTempImp.Alloc = SzAllocTemp;
	allocTempImp.Free = SzFreeTemp;

	SRes res = SzArEx_Open(&db, &lookStream->s, &allocImp, &allocTempImp);
	if (res == SZ_OK)
	{
		wchar_t* temp = NULL;
		wchar_t* destPath = NULL;
		size_t path_len = wcslen(ExtractPath);

		/*
		if you need cache, use these 3 variables.
		if you use external function, you can make these variable as static.
		*/
		UInt32 blockIndex = 0; /* it can have any value before first call (if outBuffer = 0) */
		Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
		size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */
		size_t tempSize = 0;
		for (size_t i = 0; i < db.NumFiles; i++)
		{
			size_t offset = 0;
			size_t outSizeProcessed = 0;

			if (SzArEx_IsDir(&db, i))
				continue;

			size_t len = SzArEx_GetFileNameUtf16(&db, i, NULL);
			if (len == 0)
				break;

			if (len > tempSize)
			{
				SzFree(NULL, temp);
				tempSize = len;
				temp = (wchar_t*)SzAlloc(NULL, tempSize * sizeof(wchar_t));
			}

			SzArEx_GetFileNameUtf16(&db, i, (UInt16*)temp);

			res = SzArEx_Extract(&db, &lookStream->s, i,
				&blockIndex, &outBuffer, &outBufferSize,
				&offset, &outSizeProcessed,
				&allocImp, &allocTempImp);

			if (res != SZ_OK)
				break;

			size_t processedSize;
			wchar_t* name = temp;

			for (size_t j = 0; name[j] != 0; j++)
			{
				if (name[j] == '/')
				{
					name[j] = 0;
					CreateDirectoryW(name, NULL);
					name[j] = CHAR_PATH_SEPARATOR;
				}
			}

			CSzFile outFile;

			size_t name_len = wcslen(name);				

			SzFree(NULL, destPath);
			destPath = (wchar_t*)SzAlloc(NULL, (path_len + name_len + 1) * sizeof(wchar_t));
				
			memcpy(destPath, ExtractPath, path_len * sizeof(wchar_t));
			memcpy(destPath + path_len, name, (name_len + 1) * sizeof(wchar_t));

			if (OutFile_OpenW(&outFile, destPath))
			{
				res = SZ_ERROR_FAIL;
				break;
			}			

			processedSize = outSizeProcessed;

			if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed)
			{
				res = SZ_ERROR_FAIL;
				break;
			}

			if (File_Close(&outFile))
			{
				res = SZ_ERROR_FAIL;
				break;
			}
		}
		IAlloc_Free(&allocImp, outBuffer);

		SzFree(NULL, temp);
		SzFree(NULL, destPath);
	}

	SzArEx_Free(&db, &allocImp);

	SzFree(NULL, lookStream);

	return res;
}