/*
 * Unit tests for OLE storage
 *
 * Copyright (c) 2004 Mike McCormack
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <stdio.h>

#define COBJMACROS

#include <windows.h>
#include "wine/test.h"

#include "ole2.h"
#include "objidl.h"
#include "initguid.h"

DEFINE_GUID( test_stg_cls, 0x88888888, 0x0425, 0x0000, 0,0,0,0,0,0,0,0);

#define ok_ole_success(hr, func) ok(hr == S_OK, func " failed with error 0x%08x\n", hr)

static CHAR filenameA[MAX_PATH];
static WCHAR filename[MAX_PATH];

static const WCHAR file1_name[] = {'c','o','p','y','t','e','s','t','A',0};
static const WCHAR file2_name[] = {'c','o','p','y','t','e','s','t','B',0};
static const WCHAR stgA_name[] = {'S','t','o','r','a','g','e','A',0};
static const WCHAR stgB_name[] = {'S','t','o','r','a','g','e','B',0};
static const WCHAR strmA_name[] = {'S','t','r','e','a','m','A',0};
static const WCHAR strmB_name[] = {'S','t','r','e','a','m','B',0};
static const WCHAR strmC_name[] = {'S','t','r','e','a','m','C',0};

static void test_hglobal_storage_stat(void)
{
    ILockBytes *ilb = NULL;
    IStorage *stg = NULL;
    HRESULT r;
    STATSTG stat;
    DWORD mode, refcount;

    r = CreateILockBytesOnHGlobal( NULL, TRUE, &ilb );
    ok( r == S_OK, "CreateILockBytesOnHGlobal failed\n");

    mode = STGM_CREATE|STGM_SHARE_EXCLUSIVE|STGM_READWRITE;/*0x1012*/
    r = StgCreateDocfileOnILockBytes( ilb, mode, 0,  &stg );
    ok( r == S_OK, "StgCreateDocfileOnILockBytes failed\n");

    r = WriteClassStg( stg, &test_stg_cls );
    ok( r == S_OK, "WriteClassStg failed\n");

    memset( &stat, 0, sizeof stat );
    r = IStorage_Stat( stg, &stat, 0 );

    ok( stat.pwcsName == NULL, "storage name not null\n");
    ok( stat.type == 1, "type is wrong\n");
    ok( stat.grfMode == 0x12, "grf mode is incorrect\n");
    ok( !memcmp(&stat.clsid, &test_stg_cls, sizeof test_stg_cls), "CLSID is wrong\n");

    refcount = IStorage_Release( stg );
    ok( refcount == 0, "IStorage refcount is wrong\n");
    refcount = ILockBytes_Release( ilb );
    ok( refcount == 0, "ILockBytes refcount is wrong\n");
}

static void test_create_storage_modes(void)
{
   IStorage *stg = NULL;
   HRESULT r;

   DeleteFileA(filenameA);

   /* test with some invalid parameters */
   r = StgCreateDocfile( NULL, 0, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, 0, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_CREATE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_CREATE | STGM_READWRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, NULL);
   ok(r==STG_E_INVALIDPOINTER, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 1, &stg);
   ok(r==STG_E_INVALIDPARAMETER, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_DENY_WRITE | STGM_READWRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READ, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_PRIORITY, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");

   /* StgCreateDocfile seems to be very particular about the flags it accepts */
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED | STGM_WRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED | 8, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED | 0x80, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED | 0x800, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED | 0x8000, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED | 0x80000, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED | 0x800000, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   ok(stg == NULL, "stg was set\n");

   /* check what happens if the file already exists (which is how it's meant to be used) */
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile failed\n");
   r = IStorage_Release(stg);
   ok(r == 0, "storage not released\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
   ok(r==STG_E_FILEALREADYEXISTS, "StgCreateDocfile wrong error\n"); /* FAILIFTHERE is default */
   r = StgCreateDocfile( filename, STGM_READ, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n"); /* need at least readmode and sharemode */
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_DENY_WRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_SHARE_DENY_NONE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile failed\n");
   r = StgCreateDocfile( filename, STGM_SHARE_DENY_NONE | STGM_TRANSACTED, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile failed\n");
   r = StgCreateDocfile( filename, STGM_SHARE_DENY_NONE | STGM_READWRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile failed\n");
   r = StgCreateDocfile( filename, STGM_SHARE_DENY_NONE | STGM_WRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile failed\n");
   r = StgCreateDocfile( filename, STGM_SHARE_DENY_WRITE | STGM_WRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile failed\n");
   r = StgCreateDocfile( filename, STGM_SHARE_DENY_WRITE | STGM_READ, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile wrong error\n");
   r = StgCreateDocfile( filename, STGM_TRANSACTED | STGM_SHARE_DENY_WRITE | STGM_READ, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile wrong error\n");
   ok(DeleteFileA(filenameA), "failed to delete file\n");

   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile failed\n");
   r = IStorage_Release(stg);
   ok(r == 0, "storage not released\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED |STGM_FAILIFTHERE, 0, &stg);
   ok(r==STG_E_FILEALREADYEXISTS, "StgCreateDocfile wrong error\n");
   r = StgCreateDocfile( filename, STGM_SHARE_EXCLUSIVE | STGM_WRITE, 0, &stg);
   ok(r==STG_E_FILEALREADYEXISTS, "StgCreateDocfile wrong error\n");

   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_DENY_WRITE | STGM_READWRITE, 0, &stg);
   ok(r==STG_E_INVALIDFLAG, "StgCreateDocfile succeeded\n");
   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile failed\n");
   r = IStorage_Release(stg);
   ok(r == 0, "storage not released\n");
   ok(DeleteFileA(filenameA), "failed to delete file\n");

   r = StgCreateDocfile( filename, STGM_CREATE | STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile failed\n");
   r = IStorage_Release(stg);
   ok(r == 0, "storage not released\n");
   ok(DeleteFileA(filenameA), "failed to delete file\n");

   /* test the way excel uses StgCreateDocFile */
   r = StgCreateDocfile( filename, STGM_TRANSACTED|STGM_CREATE|STGM_SHARE_DENY_WRITE|STGM_READWRITE, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile the excel way failed\n");
   if(r == S_OK)
   {
      r = IStorage_Release(stg);
      ok(r == 0, "storage not released\n");
      ok(DeleteFileA(filenameA), "failed to delete file\n");
   }

   /* and the way windows media uses it ... */
   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_DENY_NONE | STGM_READWRITE | STGM_TRANSACTED, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile the windows media way failed\n");
   if (r == S_OK)
   {
      r = IStorage_Release(stg);
      ok(r == 0, "storage not released\n");
      ok(DeleteFileA(filenameA), "failed to delete file\n");
   }

   /* looks like we need STGM_TRANSACTED or STGM_CREATE */
   r = StgCreateDocfile( filename, STGM_TRANSACTED|STGM_SHARE_DENY_WRITE|STGM_READWRITE, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile the excel way failed\n");
   if(r == S_OK)
   {
      r = IStorage_Release(stg);
      ok(r == 0, "storage not released\n");
      ok(DeleteFileA(filenameA), "failed to delete file\n");
   }

   r = StgCreateDocfile( filename, STGM_TRANSACTED|STGM_CREATE|STGM_SHARE_DENY_WRITE|STGM_WRITE, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile the excel way failed\n");
   if(r == S_OK)
   {
      r = IStorage_Release(stg);
      ok(r == 0, "storage not released\n");
      ok(DeleteFileA(filenameA), "failed to delete file\n");
   }

   r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &stg);
   ok(r==S_OK, "StgCreateDocfile the powerpoint way failed\n");
   if(r == S_OK)
   {
      r = IStorage_Release(stg);
      ok(r == 0, "storage not released\n");
      ok(DeleteFileA(filenameA), "failed to delete file\n");
   }

   /* test the way msi uses StgCreateDocfile */
   r = StgCreateDocfile( filename, STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stg);
   ok(r==S_OK, "StgCreateDocFile failed\n");
   r = IStorage_Release(stg);
   ok(r == 0, "storage not released\n");
   ok(DeleteFileA(filenameA), "failed to delete file\n");
}

static void test_storage_stream(void)
{
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR longname[] = {
        'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',
        'a','a','a','a','a','a','a','a','a','a','a','a','a','a','a','a',0
    };
    IStorage *stg = NULL;
    HRESULT r;
    IStream *stm = NULL;
    IStream *stm2 = NULL;
    ULONG count = 0;
    LARGE_INTEGER pos;
    ULARGE_INTEGER p;
    unsigned char buffer[0x100];

    DeleteFileA(filenameA);

    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* try create some invalid streams */
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 1, 0, &stm );
    ok(r==STG_E_INVALIDPARAMETER, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 1, &stm );
    ok(r==STG_E_INVALIDPARAMETER, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, NULL );
    ok(r==STG_E_INVALIDPOINTER, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, NULL, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==STG_E_INVALIDNAME, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, longname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==STG_E_INVALIDNAME || broken(r==S_OK) /* nt4 */,
       "IStorage->CreateStream wrong error, got %d GetLastError()=%d\n", r, GetLastError());
    r = IStorage_CreateStream(stg, stmname, STGM_READWRITE, 0, 0, &stm );
    ok(r==STG_E_INVALIDFLAG, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, stmname, STGM_READ, 0, 0, &stm );
    ok(r==STG_E_INVALIDFLAG, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, stmname, STGM_WRITE, 0, 0, &stm );
    ok(r==STG_E_INVALIDFLAG, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_DENY_NONE | STGM_READWRITE, 0, 0, &stm );
    ok(r==STG_E_INVALIDFLAG, "IStorage->CreateStream wrong error\n");
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_DENY_NONE | STGM_READ, 0, 0, &stm );
    ok(r==STG_E_INVALIDFLAG, "IStorage->CreateStream wrong error\n");

    /* now really create a stream and delete it */
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");
    r = IStream_Release(stm);
    ok(r == 0, "wrong ref count\n");
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==STG_E_FILEALREADYEXISTS, "IStorage->CreateStream failed\n");
    r = IStorage_DestroyElement(stg,stmname);
    ok(r==S_OK, "IStorage->DestroyElement failed\n");

    /* create a stream and write to it */
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStream_Clone(stm, &stm2);
    ok(r==S_OK, "failed to clone stream\n");

    r = IStream_Write(stm, NULL, 0, NULL );
    ok(r==STG_E_INVALIDPOINTER, "IStream->Write wrong error\n");
    r = IStream_Write(stm, "Hello\n", 0, NULL );
    ok(r==S_OK, "failed to write stream\n");
    r = IStream_Write(stm, "Hello\n", 0, &count );
    ok(r==S_OK, "failed to write stream\n");
    r = IStream_Write(stm, "Hello\n", 6, &count );
    ok(r==S_OK, "failed to write stream\n");
    r = IStream_Commit(stm, STGC_DEFAULT );
    ok(r==S_OK, "failed to commit stream\n");
    r = IStream_Commit(stm, STGC_DEFAULT );
    ok(r==S_OK, "failed to commit stream\n");

    /* seek round a bit, reset the stream size */
    pos.QuadPart = 0;
    r = IStream_Seek(stm, pos, 3, &p );
    ok(r==STG_E_INVALIDFUNCTION, "IStream->Seek returned wrong error\n");
    r = IStream_Seek(stm, pos, STREAM_SEEK_SET, NULL);
    ok(r==S_OK, "failed to seek stream\n");
    r = IStream_Seek(stm, pos, STREAM_SEEK_SET, &p );
    ok(r==S_OK, "failed to seek stream\n");
    r = IStream_SetSize(stm,p);
    ok(r==S_OK, "failed to set pos\n");
    pos.QuadPart = 10;
    r = IStream_Seek(stm, pos, STREAM_SEEK_SET, &p );
    ok(r==S_OK, "failed to seek stream\n");
    ok(p.QuadPart == 10, "at wrong place\n");
    pos.QuadPart = 0;
    r = IStream_Seek(stm, pos, STREAM_SEEK_END, &p );
    ok(r==S_OK, "failed to seek stream\n");
    ok(p.QuadPart == 0, "at wrong place\n");
    r = IStream_Read(stm, buffer, sizeof buffer, &count );
    ok(r==S_OK, "failed to set pos\n");
    ok(count == 0, "read bytes from empty stream\n");

    /* wrap up */
    r = IStream_Release(stm2);
    ok(r == 0, "wrong ref count\n");

    /* create a stream and write to it */
    r = IStorage_CreateStream(stg, stmname, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm2 );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStream_Seek(stm, pos, STREAM_SEEK_SET, &p);
    ok(r==STG_E_REVERTED, "overwritten stream should return STG_E_REVERTED instead of 0x%08x\n", r);

    r = IStream_Release(stm2);
    ok(r == 0, "wrong ref count\n");
    r = IStream_Release(stm);
    ok(r == 0, "wrong ref count\n");

    r = IStorage_Release(stg);
    ok(r == 0, "wrong ref count\n");
    r = DeleteFileA(filenameA);
    ok(r, "file should exist\n");
}

static BOOL touch_file(LPCSTR filename)
{
    HANDLE file;

    file = CreateFileA(filename, GENERIC_READ|GENERIC_WRITE, 0, NULL,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file==INVALID_HANDLE_VALUE)
        return FALSE;
    CloseHandle(file);
    return TRUE;
}

static BOOL is_zero_length(LPCSTR filename)
{
    HANDLE file;
    DWORD len;

    file = CreateFileA(filename, GENERIC_READ, 0, NULL,
                OPEN_EXISTING, 0, NULL);
    if (file==INVALID_HANDLE_VALUE)
        return FALSE;
    len = GetFileSize(file, NULL);
    CloseHandle(file);
    return len == 0;
}

static BOOL is_existing_file(LPCSTR filename)
{
    HANDLE file;

    file = CreateFileA(filename, GENERIC_READ, 0, NULL,
                       OPEN_EXISTING, 0, NULL);
    if (file==INVALID_HANDLE_VALUE)
        return FALSE;
    CloseHandle(file);
    return TRUE;
}

static void test_open_storage(void)
{
    static const WCHAR szNonExist[] = { 'n','o','n','e','x','i','s','t',0 };
    IStorage *stg = NULL, *stg2 = NULL;
    HRESULT r;
    DWORD stgm;

    /* try opening a zero length file - it should stay zero length */
    DeleteFileA(filenameA);
    touch_file(filenameA);
    stgm = STGM_NOSCRATCH | STGM_TRANSACTED | STGM_SHARE_DENY_WRITE | STGM_READWRITE;
    r = StgOpenStorage( filename, NULL, stgm, NULL, 0, &stg);
    ok(r==STG_E_FILEALREADYEXISTS, "StgOpenStorage didn't fail\n");

    stgm = STGM_SHARE_EXCLUSIVE | STGM_READWRITE;
    r = StgOpenStorage( filename, NULL, stgm, NULL, 0, &stg);
    ok(r==STG_E_FILEALREADYEXISTS, "StgOpenStorage didn't fail\n");
    ok(is_zero_length(filenameA), "file length changed\n");

    DeleteFileA(filenameA);

    /* try opening a nonexistent file - it should not create it */
    stgm = STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READWRITE;
    r = StgOpenStorage( filename, NULL, stgm, NULL, 0, &stg);
    ok(r!=S_OK, "StgOpenStorage failed: 0x%08x\n", r);
    if (r==S_OK) IStorage_Release(stg);
    ok(!is_existing_file(filenameA), "StgOpenStorage should not create a file\n");
    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");
    IStorage_Release(stg);

    r = StgOpenStorage( filename, NULL, 0, NULL, 0, &stg);
    ok(r==STG_E_INVALIDFLAG, "StgOpenStorage wrong error\n");
    r = StgOpenStorage( NULL, NULL, STGM_SHARE_EXCLUSIVE, NULL, 0, &stg);
    ok(r==STG_E_INVALIDNAME, "StgOpenStorage wrong error\n");
    r = StgOpenStorage( filename, NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 0, NULL);
    ok(r==STG_E_INVALIDPOINTER, "StgOpenStorage wrong error\n");
    r = StgOpenStorage( filename, NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 1, &stg);
    ok(r==STG_E_INVALIDPARAMETER, "StgOpenStorage wrong error\n");
    r = StgOpenStorage( szNonExist, NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 0, &stg);
    ok(r==STG_E_FILENOTFOUND, "StgOpenStorage failed\n");
    r = StgOpenStorage( filename, NULL, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 0, &stg);
    ok(r==STG_E_INVALIDFLAG, "StgOpenStorage failed\n");
    r = StgOpenStorage( filename, NULL, STGM_SHARE_DENY_NONE | STGM_READ, NULL, 0, &stg);
    ok(r==STG_E_INVALIDFLAG, "StgOpenStorage failed\n");
    r = StgOpenStorage( filename, NULL, STGM_SHARE_DENY_READ | STGM_READ, NULL, 0, &stg);
    ok(r==STG_E_INVALIDFLAG, "StgOpenStorage failed\n");
    r = StgOpenStorage( filename, NULL, STGM_SHARE_DENY_WRITE | STGM_READWRITE, NULL, 0, &stg);
    ok(r==STG_E_INVALIDFLAG, "StgOpenStorage failed\n");

    /* open it for real */
    r = StgOpenStorage( filename, NULL, STGM_SHARE_DENY_NONE | STGM_READ | STGM_TRANSACTED, NULL, 0, &stg); /* XLViewer 97/2000 */
    ok(r==S_OK, "StgOpenStorage failed\n");
    if(stg)
    {
        r = IStorage_Release(stg);
        ok(r == 0, "wrong ref count\n");
    }

    r = StgOpenStorage( filename, NULL, STGM_SHARE_DENY_WRITE | STGM_READ, NULL, 0, &stg);
    ok(r==S_OK, "StgOpenStorage failed\n");
    if(stg)
    {
        r = IStorage_Release(stg);
        ok(r == 0, "wrong ref count\n");
    }

    /* test the way word opens its custom dictionary */
    r = StgOpenStorage( filename, NULL, STGM_NOSCRATCH | STGM_TRANSACTED |
                        STGM_SHARE_DENY_WRITE | STGM_READWRITE, NULL, 0, &stg);
    ok(r==S_OK, "StgOpenStorage failed\n");
    if(stg)
    {
        r = IStorage_Release(stg);
        ok(r == 0, "wrong ref count\n");
    }

    r = StgOpenStorage( filename, NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 0, &stg);
    ok(r==S_OK, "StgOpenStorage failed\n");
    r = StgOpenStorage( filename, NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 0, &stg2);
    ok(r==STG_E_SHAREVIOLATION, "StgOpenStorage failed\n");
    if(stg)
    {
        r = IStorage_Release(stg);
        ok(r == 0, "wrong ref count\n");
    }

    /* now try write to a storage file we opened read-only */ 
    r = StgOpenStorage( filename, NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 0, &stg);
    ok(r==S_OK, "StgOpenStorage failed\n");
    if(stg)
    { 
        static const WCHAR stmname[] =  { 'w','i','n','e','t','e','s','t',0};
        IStream *stm = NULL;
        IStorage *stg2 = NULL;

        r = IStorage_CreateStream( stg, stmname, STGM_WRITE | STGM_SHARE_EXCLUSIVE,
                                   0, 0, &stm );
        ok(r == STG_E_ACCESSDENIED, "CreateStream should fail\n");
        r = IStorage_CreateStorage( stg, stmname, STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg2);
        ok(r == STG_E_ACCESSDENIED, "CreateStream should fail\n");

        r = IStorage_Release(stg);
        ok(r == 0, "wrong ref count\n");
    }

    /* open like visio 2003 */
    stg = NULL;
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY | STGM_SHARE_DENY_NONE, NULL, 0, &stg);
    ok(r == S_OK, "should succeed\n");
    if (stg)
        IStorage_Release(stg);

    /* test other sharing modes with STGM_PRIORITY */
    stg = NULL;
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg);
    ok(r == S_OK, "should succeed\n");
    if (stg)
        IStorage_Release(stg);

    stg = NULL;
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY | STGM_SHARE_DENY_WRITE, NULL, 0, &stg);
    ok(r == S_OK, "should succeed\n");
    if (stg)
        IStorage_Release(stg);

    stg = NULL;
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY | STGM_SHARE_DENY_READ, NULL, 0, &stg);
    ok(r == S_OK, "should succeed\n");
    if (stg)
        IStorage_Release(stg);

    /* open like Project 2003 */
    stg = NULL;
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY, NULL, 0, &stg);
    ok(r == S_OK, "should succeed\n");
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY, NULL, 0, &stg2);
    ok(r == S_OK, "should succeed\n");
    if (stg2)
        IStorage_Release(stg2);
    if (stg)
        IStorage_Release(stg);

    stg = NULL;
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY | STGM_READWRITE, NULL, 0, &stg);
    ok(r == STG_E_INVALIDFLAG, "should fail\n");

    r = StgOpenStorage( filename, NULL, STGM_TRANSACTED | STGM_PRIORITY, NULL, 0, &stg);
    ok(r == STG_E_INVALIDFLAG, "should fail\n");

    r = StgOpenStorage( filename, NULL, STGM_SIMPLE | STGM_PRIORITY, NULL, 0, &stg);
    ok(r == STG_E_INVALIDFLAG, "should fail\n");

    r = StgOpenStorage( filename, NULL, STGM_DELETEONRELEASE | STGM_PRIORITY, NULL, 0, &stg);
    ok(r == STG_E_INVALIDFUNCTION, "should fail\n");

    r = StgOpenStorage( filename, NULL, STGM_NOSCRATCH | STGM_PRIORITY, NULL, 0, &stg);
    ok(r == STG_E_INVALIDFLAG, "should fail\n");

    r = StgOpenStorage( filename, NULL, STGM_NOSNAPSHOT | STGM_PRIORITY, NULL, 0, &stg);
    ok(r == STG_E_INVALIDFLAG, "should fail\n");

    r = DeleteFileA(filenameA);
    ok(r, "file didn't exist\n");
}

static void test_storage_suminfo(void)
{
    IStorage *stg = NULL;
    IPropertySetStorage *propset = NULL;
    IPropertyStorage *ps = NULL;
    HRESULT r;

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | 
                            STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    r = IStorage_QueryInterface( stg, &IID_IPropertySetStorage, (LPVOID) &propset );
    ok(r == S_OK, "query interface failed\n");

    /* delete it */
    r = IPropertySetStorage_Delete( propset, &FMTID_SummaryInformation );
    ok(r == STG_E_FILENOTFOUND, "deleted property set storage\n");

    r = IPropertySetStorage_Open( propset, &FMTID_SummaryInformation, 
                                STGM_READ | STGM_SHARE_EXCLUSIVE, &ps );
    ok(r == STG_E_FILENOTFOUND, "opened property set storage\n");

    r = IPropertySetStorage_Create( propset, &FMTID_SummaryInformation, NULL, 0,
                                STGM_READ | STGM_SHARE_EXCLUSIVE, &ps );
    ok(r == STG_E_INVALIDFLAG, "created property set storage\n");

    r = IPropertySetStorage_Create( propset, &FMTID_SummaryInformation, NULL, 0,
                                STGM_READ, &ps );
    ok(r == STG_E_INVALIDFLAG, "created property set storage\n");

    r = IPropertySetStorage_Create( propset, &FMTID_SummaryInformation, NULL, 0, 0, &ps );
    ok(r == STG_E_INVALIDFLAG, "created property set storage\n");

    r = IPropertySetStorage_Create( propset, &FMTID_SummaryInformation, NULL, 0,
                                STGM_WRITE|STGM_SHARE_EXCLUSIVE, &ps );
    ok(r == STG_E_INVALIDFLAG, "created property set storage\n");

    r = IPropertySetStorage_Create( propset, &FMTID_SummaryInformation, NULL, 0,
                                STGM_CREATE|STGM_WRITE|STGM_SHARE_EXCLUSIVE, &ps );
    ok(r == STG_E_INVALIDFLAG, "created property set storage\n");

    /* now try really creating a property set */
    r = IPropertySetStorage_Create( propset, &FMTID_SummaryInformation, NULL, 0,
                                STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &ps );
    ok(r == S_OK, "failed to create property set storage\n");

    if( ps )
        IPropertyStorage_Release(ps);

    /* now try creating the same thing again */
    r = IPropertySetStorage_Create( propset, &FMTID_SummaryInformation, NULL, 0,
                                STGM_CREATE|STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &ps );
    ok(r == S_OK, "failed to create property set storage\n");
    if( ps )
        IPropertyStorage_Release(ps);

    /* should be able to open it */
    r = IPropertySetStorage_Open( propset, &FMTID_SummaryInformation, 
            STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &ps);
    ok(r == S_OK, "open failed\n");
    if(r == S_OK)
        IPropertyStorage_Release(ps);

    /* delete it */
    r = IPropertySetStorage_Delete( propset, &FMTID_SummaryInformation );
    ok(r == S_OK, "failed to delete property set storage\n");

    /* try opening with an invalid FMTID */
    r = IPropertySetStorage_Open( propset, NULL, 
            STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &ps);
    ok(r == E_INVALIDARG, "open succeeded\n");
    if(r == S_OK)
        IPropertyStorage_Release(ps);

    /* try a bad guid */
    r = IPropertySetStorage_Open( propset, &IID_IStorage, 
            STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &ps);
    ok(r == STG_E_FILENOTFOUND, "open succeeded\n");
    if(r == S_OK)
        IPropertyStorage_Release(ps);
    

    /* try some invalid flags */
    r = IPropertySetStorage_Open( propset, &FMTID_SummaryInformation, 
            STGM_CREATE | STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &ps);
    ok(r == STG_E_INVALIDFLAG, "open succeeded\n");
    if(r == S_OK)
        IPropertyStorage_Release(ps);

    /* after deleting it, it should be gone */
    r = IPropertySetStorage_Open( propset, &FMTID_SummaryInformation, 
            STGM_READWRITE|STGM_SHARE_EXCLUSIVE, &ps);
    ok(r == STG_E_FILENOTFOUND, "open failed\n");
    if(r == S_OK)
        IPropertyStorage_Release(ps);

    r = IPropertySetStorage_Release( propset );
    ok(r == 1, "ref count wrong\n");

    r = IStorage_Release(stg);
    ok(r == 0, "ref count wrong\n");

    DeleteFileA(filenameA);
}

static void test_storage_refcount(void)
{
    IStorage *stg = NULL;
    IStorage *stgprio = NULL;
    HRESULT r;
    IStream *stm = NULL;
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    LARGE_INTEGER pos;
    ULARGE_INTEGER upos;
    STATSTG stat;
    char buffer[10];

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | 
                            STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    r = WriteClassStg( stg, &test_stg_cls );
    ok( r == S_OK, "WriteClassStg failed\n");

    r = IStorage_Commit( stg, STGC_DEFAULT );
    ok( r == S_OK, "IStorage_Commit failed\n");

    /* now create a stream */
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStorage_Release( stg );
    ok (r == 0, "storage not released\n");

    pos.QuadPart = 0;
    r = IStream_Seek( stm, pos, 0, &upos );
    ok (r == STG_E_REVERTED, "seek should fail\n");

    r = IStream_Stat( stm, &stat, STATFLAG_DEFAULT );
    ok (r == STG_E_REVERTED, "stat should fail\n");

    r = IStream_Write( stm, "Test string", strlen("Test string"), NULL);
    ok (r == STG_E_REVERTED, "IStream_Write should return STG_E_REVERTED instead of 0x%08x\n", r);

    r = IStream_Read( stm, buffer, sizeof(buffer), NULL);
    ok (r == STG_E_REVERTED, "IStream_Read should return STG_E_REVERTED instead of 0x%08x\n", r);

    r = IStream_Release(stm);
    ok (r == 0, "stream not released\n");

    /* tests that STGM_PRIORITY doesn't prevent readwrite access from other
     * StgOpenStorage calls in transacted mode */
    r = StgOpenStorage( filename, NULL, STGM_PRIORITY, NULL, 0, &stgprio);
    ok(r==S_OK, "StgOpenStorage failed with error 0x%08x\n", r);

    todo_wine {
    /* non-transacted mode read/write fails */
    r = StgOpenStorage( filename, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg);
    ok(r==STG_E_LOCKVIOLATION, "StgOpenStorage should return STG_E_LOCKVIOLATION instead of 0x%08x\n", r);
    }

    /* non-transacted mode read-only succeeds */
    r = StgOpenStorage( filename, NULL, STGM_SHARE_DENY_WRITE|STGM_READ, NULL, 0, &stg);
    ok(r==S_OK, "StgOpenStorage failed with error 0x%08x\n", r);
    IStorage_Release(stg);

    r = StgOpenStorage( filename, NULL, STGM_TRANSACTED|STGM_SHARE_DENY_WRITE|STGM_READWRITE, NULL, 0, &stg);
    ok(r==S_OK, "StgOpenStorage failed with error 0x%08x\n", r);
    if(stg)
    {
        static const WCHAR stgname[] = { ' ',' ',' ','2','9',0 };
        static const WCHAR stgname2[] = { 'C','V','_','i','e','w',0 };
        static const WCHAR stmname2[] = { 'V','a','r','2','D','a','t','a',0 };
        IStorage *stg2;
        IStorage *stg3;
        STATSTG statstg;

        r = IStorage_Stat( stg, &statstg, STATFLAG_NONAME );
        ok(r == S_OK, "Stat should have succeded instead of returning 0x%08x\n", r);
        ok(statstg.type == STGTY_STORAGE, "Statstg type should have been STGTY_STORAGE instead of %d\n", statstg.type);
        ok(U(statstg.cbSize).LowPart == 0, "Statstg cbSize.LowPart should have been 0 instead of %d\n", U(statstg.cbSize).LowPart);
        ok(U(statstg.cbSize).HighPart == 0, "Statstg cbSize.HighPart should have been 0 instead of %d\n", U(statstg.cbSize).HighPart);
        ok(statstg.grfMode == (STGM_TRANSACTED|STGM_SHARE_DENY_WRITE|STGM_READWRITE),
            "Statstg grfMode should have been 0x10022 instead of 0x%x\n", statstg.grfMode);
        ok(statstg.grfLocksSupported == 0, "Statstg grfLocksSupported should have been 0 instead of %d\n", statstg.grfLocksSupported);
        ok(IsEqualCLSID(&statstg.clsid, &test_stg_cls), "Statstg clsid is not test_stg_cls\n");
        ok(statstg.grfStateBits == 0, "Statstg grfStateBits should have been 0 instead of %d\n", statstg.grfStateBits);
        ok(statstg.reserved == 0, "Statstg reserved should have been 0 instead of %d\n", statstg.reserved);

        r = IStorage_CreateStorage( stg, stgname, STGM_SHARE_EXCLUSIVE, 0, 0, &stg2 );
        ok(r == S_OK, "CreateStorage should have succeeded instead of returning 0x%08x\n", r);

        r = IStorage_Stat( stg2, &statstg, STATFLAG_DEFAULT );
        ok(r == S_OK, "Stat should have succeded instead of returning 0x%08x\n", r);
        ok(!memcmp(statstg.pwcsName, stgname, sizeof(stgname)),
            "Statstg pwcsName should have been the name the storage was created with\n");
        ok(statstg.type == STGTY_STORAGE, "Statstg type should have been STGTY_STORAGE instead of %d\n", statstg.type);
        ok(U(statstg.cbSize).LowPart == 0, "Statstg cbSize.LowPart should have been 0 instead of %d\n", U(statstg.cbSize).LowPart);
        ok(U(statstg.cbSize).HighPart == 0, "Statstg cbSize.HighPart should have been 0 instead of %d\n", U(statstg.cbSize).HighPart);
        ok(statstg.grfMode == STGM_SHARE_EXCLUSIVE,
            "Statstg grfMode should have been STGM_SHARE_EXCLUSIVE instead of 0x%x\n", statstg.grfMode);
        ok(statstg.grfLocksSupported == 0, "Statstg grfLocksSupported should have been 0 instead of %d\n", statstg.grfLocksSupported);
        ok(IsEqualCLSID(&statstg.clsid, &CLSID_NULL), "Statstg clsid is not CLSID_NULL\n");
        ok(statstg.grfStateBits == 0, "Statstg grfStateBits should have been 0 instead of %d\n", statstg.grfStateBits);
        ok(statstg.reserved == 0, "Statstg reserved should have been 0 instead of %d\n", statstg.reserved);
        CoTaskMemFree(statstg.pwcsName);

        r = IStorage_CreateStorage( stg2, stgname2, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &stg3 );
        ok(r == STG_E_ACCESSDENIED, "CreateStorage should have returned STG_E_ACCESSDENIED instead of 0x%08x\n", r);

        r = IStorage_CreateStream( stg2, stmname2, STGM_CREATE|STGM_SHARE_EXCLUSIVE, 0, 0, &stm );
        ok(r == STG_E_ACCESSDENIED, "CreateStream should have returned STG_E_ACCESSDENIED instead of 0x%08x\n", r);

        IStorage_Release(stg2);

        r = IStorage_Release(stg);
        ok(r == 0, "wrong ref count\n");
    }
    IStorage_Release(stgprio);

    DeleteFileA(filenameA);
}

static void test_writeclassstg(void)
{
    IStorage *stg = NULL;
    HRESULT r;
    CLSID temp_cls;

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    r = ReadClassStg( NULL, NULL );
    ok(r == E_INVALIDARG, "ReadClassStg should return E_INVALIDARG instead of 0x%08X\n", r);

    r = ReadClassStg( stg, NULL );
    ok(r == E_INVALIDARG, "ReadClassStg should return E_INVALIDARG instead of 0x%08X\n", r);

    temp_cls.Data1 = 0xdeadbeef;
    r = ReadClassStg( stg, &temp_cls );
    ok(r == S_OK, "ReadClassStg failed with 0x%08X\n", r);

    ok(IsEqualCLSID(&temp_cls, &CLSID_NULL), "ReadClassStg returned wrong clsid\n");

    r = WriteClassStg( NULL, NULL );
    ok(r == E_INVALIDARG, "WriteClassStg should return E_INVALIDARG instead of 0x%08X\n", r);

    r = WriteClassStg( stg, NULL );
    ok(r == STG_E_INVALIDPOINTER, "WriteClassStg should return STG_E_INVALIDPOINTER instead of 0x%08X\n", r);

    r = WriteClassStg( stg, &test_stg_cls );
    ok( r == S_OK, "WriteClassStg failed with 0x%08X\n", r);

    r = ReadClassStg( stg, &temp_cls );
    ok( r == S_OK, "ReadClassStg failed with 0x%08X\n", r);
    ok(IsEqualCLSID(&temp_cls, &test_stg_cls), "ReadClassStg returned wrong clsid\n");

    r = IStorage_Release( stg );
    ok (r == 0, "storage not released\n");

    DeleteFileA(filenameA);
}

static void test_streamenum(void)
{
    IStorage *stg = NULL;
    HRESULT r;
    IStream *stm = NULL;
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    STATSTG stat;
    IEnumSTATSTG *ee = NULL;
    ULONG count;

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    r = WriteClassStg( stg, &test_stg_cls );
    ok( r == S_OK, "WriteClassStg failed\n");

    r = IStorage_Commit( stg, STGC_DEFAULT );
    ok( r == S_OK, "IStorage_Commit failed\n");

    /* now create a stream */
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStream_Release(stm);

    /* first enum ... should be 1 stream */
    r = IStorage_EnumElements(stg, 0, NULL, 0, &ee);
    ok(r==S_OK, "IStorage->EnumElements failed\n");

    count = 0xf00;
    r = IEnumSTATSTG_Next(ee, 1, &stat, &count);
    ok(r==S_OK, "IEnumSTATSTG->Next failed\n");
    ok(count == 1, "count wrong\n");

    r = IEnumSTATSTG_Release(ee);

    /* second enum... destroy the stream before reading */
    r = IStorage_EnumElements(stg, 0, NULL, 0, &ee);
    ok(r==S_OK, "IStorage->EnumElements failed\n");

    r = IStorage_DestroyElement(stg, stmname);
    ok(r==S_OK, "IStorage->EnumElements failed\n");

    todo_wine {
    count = 0xf00;
    r = IEnumSTATSTG_Next(ee, 1, &stat, &count);
    ok(r==S_FALSE, "IEnumSTATSTG->Next failed\n");
    ok(count == 0, "count wrong\n");
    }

    /* reset and try again */
    r = IEnumSTATSTG_Reset(ee);
    ok(r==S_OK, "IEnumSTATSTG->Reset failed\n");

    count = 0xf00;
    r = IEnumSTATSTG_Next(ee, 1, &stat, &count);
    ok(r==S_FALSE, "IEnumSTATSTG->Next failed\n");
    ok(count == 0, "count wrong\n");

    r = IEnumSTATSTG_Release(ee);
    ok (r == 0, "enum not released\n");

    r = IStorage_Release( stg );
    ok (r == 0, "storage not released\n");

    DeleteFileA(filenameA);
}

static void test_transact(void)
{
    IStorage *stg = NULL, *stg2 = NULL, *stg3 = NULL;
    HRESULT r;
    IStream *stm = NULL;
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR stmname2[] = { 'F','O','O',0 };
    static const WCHAR stgname[] = { 'P','E','R','M','S','T','G',0 };
    static const WCHAR stgname2[] = { 'T','E','M','P','S','T','G',0 };

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | 
                            STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* commit a new stream and storage */
    r = IStorage_CreateStream(stg, stmname2, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStream_Write(stm, "this is stream 1\n", 16, NULL);
    ok(r==S_OK, "IStream->Write failed\n");

    IStream_Release(stm);

    r = IStorage_CreateStorage(stg, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    if (r == S_OK)
    {
        /* Create two substorages but only commit one */
        r = IStorage_CreateStorage(stg2, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg3);
        ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

        if (r == S_OK)
            IStorage_Release(stg3);

        r = IStorage_Commit(stg, 0);
        ok(r==S_OK, "IStorage->Commit failed\n");

        r = IStorage_CreateStorage(stg2, stgname2, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg3);
        ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

        if (r == S_OK)
            IStorage_Release(stg3);

        IStorage_Release(stg2);
    }

    /* now create a stream and storage, but don't commit them */
    stm = NULL;
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStream_Write(stm, "this is stream 2\n", 16, NULL);
    ok(r==S_OK, "IStream->Write failed\n");

    /* IStream::Commit does nothing for OLE storage streams */
    r = IStream_Commit(stm, STGC_ONLYIFCURRENT | STGC_DANGEROUSLYCOMMITMERELYTODISKCACHE);
    ok(r==S_OK, "IStream->Commit failed\n");

    r = IStorage_CreateStorage(stg, stgname2, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    if (r == S_OK)
        IStorage_Release(stg2);

    IStream_Release(stm);

    IStorage_Release(stg);

    stm = NULL;
    stg = NULL;
    r = StgOpenStorage( filename, NULL, STGM_SHARE_DENY_NONE | STGM_READ | STGM_TRANSACTED, NULL, 0, &stg);
    ok(r==S_OK, "StgOpenStorage failed\n");

    if (!stg)
        return;

    r = IStorage_OpenStream(stg, stmname, NULL, STGM_SHARE_DENY_NONE|STGM_READ, 0, &stm );
    ok(r==STG_E_INVALIDFLAG, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStream(stg, stmname, NULL, STGM_DELETEONRELEASE|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==STG_E_INVALIDFUNCTION, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStream(stg, stmname, NULL, STGM_TRANSACTED|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==STG_E_INVALIDFUNCTION, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStorage(stg, stmname, NULL, STGM_TRANSACTED|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream failed %08x\n", r);

    todo_wine {
    r = IStorage_OpenStream(stg, stmname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream should fail %08x\n", r);
    }
    if (r == S_OK)
        IStream_Release(stm);

    todo_wine {
    r = IStorage_OpenStorage(stg, stgname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStorage should fail %08x\n", r);
    }
    if (r == S_OK)
        IStorage_Release(stg2);

    r = IStorage_OpenStorage(stg, stmname2, NULL, STGM_TRANSACTED|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStream(stg, stmname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==S_OK, "IStorage->OpenStream should succeed %08x\n", r);
    if (r == S_OK)
        IStream_Release(stm);

    r = IStorage_OpenStorage(stg, stgname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==S_OK, "IStorage->OpenStorage should succeed %08x\n", r);
    if (r == S_OK)
    {
        r = IStorage_OpenStorage(stg2, stgname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg3 );
        ok(r==S_OK, "IStorage->OpenStorage should succeed %08x\n", r);
        if (r == S_OK)
            IStorage_Release(stg3);

        r = IStorage_OpenStorage(stg2, stgname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg3 );
        todo_wine ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStorage should fail %08x\n", r);
        if (r == S_OK)
            IStorage_Release(stg3);

        IStorage_Release(stg2);
    }

    IStorage_Release(stg);

    r = DeleteFileA(filenameA);
    ok( r == TRUE, "deleted file\n");
}

static void test_substorage_share(void)
{
    IStorage *stg, *stg2, *stg3;
    IStream *stm, *stm2;
    HRESULT r;
    static const WCHAR stgname[] = { 'P','E','R','M','S','T','G',0 };
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR othername[] = { 'N','E','W','N','A','M','E',0 };

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* create a read/write storage and try to open it again */
    r = IStorage_CreateStorage(stg, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    if (r == S_OK)
    {
        r = IStorage_OpenStorage(stg, stgname, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg3);
        todo_wine ok(r==STG_E_ACCESSDENIED, "IStorage->OpenStorage should fail %08x\n", r);

        if (r == S_OK)
            IStorage_Release(stg3);

        r = IStorage_OpenStorage(stg, stgname, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, 0, &stg3);
        todo_wine ok(r==STG_E_ACCESSDENIED, "IStorage->OpenStorage should fail %08x\n", r);

        if (r == S_OK)
            IStorage_Release(stg3);

        /* cannot rename the storage while it's open */
        r = IStorage_RenameElement(stg, stgname, othername);
        todo_wine ok(r==STG_E_ACCESSDENIED, "IStorage->RenameElement should fail %08x\n", r);
        if (SUCCEEDED(r)) IStorage_RenameElement(stg, othername, stgname);

#if 0
        /* This crashes on Wine. */

        /* destroying an object while it's open invalidates it */
        r = IStorage_DestroyElement(stg, stgname);
        ok(r==S_OK, "IStorage->DestroyElement failed, hr=%08x\n", r);

        r = IStorage_CreateStream(stg2, stmname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stm);
        todo_wine ok(r==STG_E_REVERTED, "IStorage->CreateStream failed, hr=%08x\n", r);

        if (r == S_OK)
            IStorage_Release(stm);
#endif

        IStorage_Release(stg2);
    }

    /* create a read/write stream and try to open it again */
    r = IStorage_CreateStream(stg, stmname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stm);
    ok(r==S_OK, "IStorage->CreateStream failed, hr=%08x\n", r);

    if (r == S_OK)
    {
        r = IStorage_OpenStream(stg, stmname, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stm2);
        ok(r==STG_E_ACCESSDENIED, "IStorage->OpenStream should fail %08x\n", r);

        if (r == S_OK)
            IStorage_Release(stm2);

        r = IStorage_OpenStream(stg, stmname, NULL, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &stm2);
        ok(r==STG_E_ACCESSDENIED, "IStorage->OpenStream should fail %08x\n", r);

        if (r == S_OK)
            IStorage_Release(stm2);

        /* cannot rename the stream while it's open */
        r = IStorage_RenameElement(stg, stmname, othername);
        todo_wine ok(r==STG_E_ACCESSDENIED, "IStorage->RenameElement should fail %08x\n", r);
        if (SUCCEEDED(r)) IStorage_RenameElement(stg, othername, stmname);

        /* destroying an object while it's open invalidates it */
        r = IStorage_DestroyElement(stg, stmname);
        ok(r==S_OK, "IStorage->DestroyElement failed, hr=%08x\n", r);

        r = IStream_Write(stm, "this shouldn't work\n", 20, NULL);
        ok(r==STG_E_REVERTED, "IStream_Write should fail %08x\n", r);

        IStorage_Release(stm);
    }

    IStorage_Release(stg);

    r = DeleteFileA(filenameA);
    ok( r == TRUE, "deleted file\n");
}

static void test_revert(void)
{
    IStorage *stg = NULL, *stg2 = NULL, *stg3 = NULL;
    HRESULT r;
    IStream *stm = NULL, *stm2 = NULL;
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR stmname2[] = { 'F','O','O',0 };
    static const WCHAR stgname[] = { 'P','E','R','M','S','T','G',0 };
    static const WCHAR stgname2[] = { 'T','E','M','P','S','T','G',0 };
    STATSTG statstg;

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* commit a new stream and storage */
    r = IStorage_CreateStream(stg, stmname2, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStream_Write(stm, "this is stream 1\n", 16, NULL);
    ok(r==S_OK, "IStream->Write failed\n");

    r = IStorage_CreateStorage(stg, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    if (r == S_OK)
    {
        /* Create two substorages but only commit one */
        r = IStorage_CreateStorage(stg2, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg3);
        ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

        if (r == S_OK)
            IStorage_Release(stg3);

        r = IStorage_Commit(stg, 0);
        ok(r==S_OK, "IStorage->Commit failed\n");

        r = IStorage_CreateStorage(stg2, stgname2, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg3);
        ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

        if (r == S_OK)
            IStorage_Release(stg3);
    }

    /* now create a stream and storage, then revert */
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm2 );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStream_Write(stm2, "this is stream 2\n", 16, NULL);
    ok(r==S_OK, "IStream->Write failed\n");

    r = IStorage_CreateStorage(stg, stgname2, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg3);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    r = IStorage_Revert(stg);

    /* all open objects become invalid */
    todo_wine {
    r = IStream_Write(stm, "this shouldn't work\n", 20, NULL);
    ok(r==STG_E_REVERTED, "IStream_Write should fail %08x\n", r);

    r = IStream_Write(stm2, "this shouldn't work\n", 20, NULL);
    ok(r==STG_E_REVERTED, "IStream_Write should fail %08x\n", r);

    r = IStorage_Stat(stg2, &statstg, STATFLAG_NONAME);
    ok(r==STG_E_REVERTED, "IStorage_Stat should fail %08x\n", r);

    r = IStorage_Stat(stg3, &statstg, STATFLAG_NONAME);
    ok(r==STG_E_REVERTED, "IStorage_Stat should fail %08x\n", r);
    }

    IStream_Release(stm);
    IStream_Release(stm2);
    IStorage_Release(stg2);
    IStorage_Release(stg3);

    r = IStorage_OpenStream(stg, stmname, NULL, STGM_SHARE_DENY_NONE|STGM_READ, 0, &stm );
    ok(r==STG_E_INVALIDFLAG, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStream(stg, stmname, NULL, STGM_DELETEONRELEASE|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==STG_E_INVALIDFUNCTION, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStream(stg, stmname, NULL, STGM_TRANSACTED|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==STG_E_INVALIDFUNCTION, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStorage(stg, stmname, NULL, STGM_TRANSACTED|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream failed %08x\n", r);

    todo_wine {
    r = IStorage_OpenStream(stg, stmname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream should fail %08x\n", r);
    }
    if (r == S_OK)
        IStream_Release(stm);

    todo_wine {
    r = IStorage_OpenStorage(stg, stgname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStorage should fail %08x\n", r);
    }
    if (r == S_OK)
        IStorage_Release(stg2);

    r = IStorage_OpenStorage(stg, stmname2, NULL, STGM_TRANSACTED|STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream failed %08x\n", r);

    r = IStorage_OpenStream(stg, stmname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
    ok(r==S_OK, "IStorage->OpenStream should succeed %08x\n", r);
    if (r == S_OK)
        IStream_Release(stm);

    r = IStorage_OpenStorage(stg, stgname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg2 );
    ok(r==S_OK, "IStorage->OpenStorage should succeed %08x\n", r);
    if (r == S_OK)
    {
        r = IStorage_OpenStorage(stg2, stgname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg3 );
        ok(r==S_OK, "IStorage->OpenStorage should succeed %08x\n", r);
        if (r == S_OK)
            IStorage_Release(stg3);

        r = IStorage_OpenStorage(stg2, stgname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, NULL, 0, &stg3 );
        todo_wine ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStorage should fail %08x\n", r);
        if (r == S_OK)
            IStorage_Release(stg3);

        IStorage_Release(stg2);
    }

    IStorage_Release(stg);

    r = DeleteFileA(filenameA);
    ok( r == TRUE, "deleted file\n");

    /* Revert only invalidates objects in transacted mode */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    r = IStorage_CreateStream(stg, stmname2, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
    ok(r==S_OK, "IStorage->CreateStream failed\n");

    r = IStorage_Revert(stg);
    todo_wine ok(r==S_OK, "IStorage->Revert failed %08x\n", r);

    r = IStream_Write(stm, "this works\n", 11, NULL);
    ok(r==S_OK, "IStream_Write should succeed %08x\n", r);

    IStream_Release(stm);
    IStream_Release(stg);

    r = DeleteFileA(filenameA);
    ok( r == TRUE, "deleted file\n");
}

static void test_parent_free(void)
{
    IStorage *stg = NULL, *stg2 = NULL;
    HRESULT r;
    IStream *stm = NULL;
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR stgname[] = { 'P','E','R','M','S','T','G',0 };
    ULONG ref;

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* commit a new storage */
    r = IStorage_CreateStorage(stg, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    if (r == S_OK)
    {
        /* now create a stream inside the new storage */
        r = IStorage_CreateStream(stg2, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
        ok(r==S_OK, "IStorage->CreateStream failed\n");

        ref = IStorage_Release(stg2);
        ok(ref == 0, "IStorage still has %u references\n", ref);

        if (r == S_OK)
        {
            r = IStream_Write(stm, "this should fail\n", 17, NULL);
            ok(r==STG_E_REVERTED, "IStream->Write sould fail, hr=%x\n", r);

            IStream_Release(stm);
        }
    }

    IStorage_Release(stg);

    r = DeleteFileA(filenameA);
    ok( r == TRUE, "deleted file\n");
}

static void test_nonroot_transacted(void)
{
    IStorage *stg = NULL, *stg2 = NULL;
    HRESULT r;
    IStream *stm = NULL;
    static const WCHAR stgname[] = { 'P','E','R','M','S','T','G',0 };
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR stmname2[] = { 'F','O','O',0 };

    DeleteFileA(filenameA);

    /* create a transacted file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* create a transacted substorage */
    r = IStorage_CreateStorage(stg, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_TRANSACTED, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    if (r == S_OK)
    {
        /* create and commit stmname */
        r = IStorage_CreateStream(stg2, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
        ok(r==S_OK, "IStorage->CreateStream failed\n");
        if (r == S_OK)
            IStream_Release(stm);

        IStorage_Commit(stg2, 0);

        /* create and revert stmname2 */
        r = IStorage_CreateStream(stg2, stmname2, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
        ok(r==S_OK, "IStorage->CreateStream failed\n");
        if (r == S_OK)
            IStream_Release(stm);

        IStorage_Revert(stg2);

        /* check that Commit and Revert really worked */
        r = IStorage_OpenStream(stg2, stmname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
        ok(r==S_OK, "IStorage->OpenStream should succeed %08x\n", r);
        if (r == S_OK)
            IStream_Release(stm);

        r = IStorage_OpenStream(stg2, stmname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
        todo_wine ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream should fail %08x\n", r);
        if (r == S_OK)
            IStream_Release(stm);

        IStorage_Release(stg2);
    }

    IStream_Release(stg);

    /* create a non-transacted file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* create a transacted substorage */
    r = IStorage_CreateStorage(stg, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_TRANSACTED, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    if (r == S_OK)
    {
        /* create and commit stmname */
        r = IStorage_CreateStream(stg2, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
        ok(r==S_OK, "IStorage->CreateStream failed\n");
        if (r == S_OK)
            IStream_Release(stm);

        IStorage_Commit(stg2, 0);

        /* create and revert stmname2 */
        r = IStorage_CreateStream(stg2, stmname2, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm );
        ok(r==S_OK, "IStorage->CreateStream failed\n");
        if (r == S_OK)
            IStream_Release(stm);

        IStorage_Revert(stg2);

        /* check that Commit and Revert really worked */
        r = IStorage_OpenStream(stg2, stmname, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
        ok(r==S_OK, "IStorage->OpenStream should succeed %08x\n", r);
        if (r == S_OK)
            IStream_Release(stm);

        r = IStorage_OpenStream(stg2, stmname2, NULL, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, &stm );
        todo_wine ok(r==STG_E_FILENOTFOUND, "IStorage->OpenStream should fail %08x\n", r);
        if (r == S_OK)
            IStream_Release(stm);

        IStorage_Release(stg2);
    }

    IStream_Release(stg);

    r = DeleteFileA(filenameA);
    ok( r == TRUE, "deleted file\n");
}

static void test_ReadClassStm(void)
{
    CLSID clsid;
    HRESULT hr;
    IStream *pStream;
    static const LARGE_INTEGER llZero;

    hr = ReadClassStm(NULL, &clsid);
    ok(hr == E_INVALIDARG, "ReadClassStm should have returned E_INVALIDARG instead of 0x%08x\n", hr);

    hr = CreateStreamOnHGlobal(NULL, TRUE, &pStream);
    ok_ole_success(hr, "CreateStreamOnHGlobal");
    hr = WriteClassStm(pStream, &test_stg_cls);
    ok_ole_success(hr, "WriteClassStm");

    hr = ReadClassStm(pStream, NULL);
    ok(hr == E_INVALIDARG, "ReadClassStm should have returned E_INVALIDARG instead of 0x%08x\n", hr);

    /* test not rewound stream */
    hr = ReadClassStm(pStream, &clsid);
    ok(hr == STG_E_READFAULT, "ReadClassStm should have returned STG_E_READFAULT instead of 0x%08x\n", hr);
    ok(IsEqualCLSID(&clsid, &CLSID_NULL), "clsid should have been zeroed\n");

    hr = IStream_Seek(pStream, llZero, STREAM_SEEK_SET, NULL);
    ok_ole_success(hr, "IStream_Seek");
    hr = ReadClassStm(pStream, &clsid);
    ok_ole_success(hr, "ReadClassStm");
    ok(IsEqualCLSID(&clsid, &test_stg_cls), "clsid should have been set to CLSID_WineTest\n");
}

struct access_res
{
    BOOL gothandle;
    DWORD lasterr;
    BOOL ignore;
};

static const struct access_res create[16] =
{
    { TRUE, ERROR_SUCCESS, TRUE },
    { TRUE, ERROR_SUCCESS, TRUE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { TRUE, ERROR_SUCCESS, TRUE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { TRUE, ERROR_SUCCESS, TRUE }
};

static const struct access_res create_commit[16] =
{
    { TRUE, ERROR_SUCCESS, TRUE },
    { TRUE, ERROR_SUCCESS, TRUE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { TRUE, ERROR_SUCCESS, TRUE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { FALSE, ERROR_SHARING_VIOLATION, FALSE },
    { TRUE, ERROR_SUCCESS, TRUE }
};

static const struct access_res create_close[16] =
{
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS, FALSE },
    { TRUE, ERROR_SUCCESS }
};

static void _test_file_access(LPCSTR file, const struct access_res *ares, DWORD line)
{
    DWORD access = 0, share = 0;
    DWORD lasterr;
    HANDLE hfile;
    int i, j, idx = 0;

    for (i = 0; i < 4; i++)
    {
        if (i == 0) access = 0;
        if (i == 1) access = GENERIC_READ;
        if (i == 2) access = GENERIC_WRITE;
        if (i == 3) access = GENERIC_READ | GENERIC_WRITE;

        for (j = 0; j < 4; j++)
        {
            if (ares[idx].ignore)
                continue;

            if (j == 0) share = 0;
            if (j == 1) share = FILE_SHARE_READ;
            if (j == 2) share = FILE_SHARE_WRITE;
            if (j == 3) share = FILE_SHARE_READ | FILE_SHARE_WRITE;

            SetLastError(0xdeadbeef);
            hfile = CreateFileA(file, access, share, NULL, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, 0);
            lasterr = GetLastError();

            ok((hfile != INVALID_HANDLE_VALUE) == ares[idx].gothandle,
               "(%d, handle, %d): Expected %d, got %d\n",
               line, idx, ares[idx].gothandle,
               (hfile != INVALID_HANDLE_VALUE));

            ok(lasterr == ares[idx].lasterr ||
               broken(lasterr == 0xdeadbeef) /* win9x */,
               "(%d, lasterr, %d): Expected %d, got %d\n",
               line, idx, ares[idx].lasterr, lasterr);

            CloseHandle(hfile);
            idx++;
        }
    }
}

#define test_file_access(file, ares) _test_file_access(file, ares, __LINE__)

static void test_access(void)
{
    IStorage *stg;
    HRESULT hr;

    static const WCHAR fileW[] = {'w','i','n','e','t','e','s','t',0};

    /* STGM_TRANSACTED */

    hr = StgCreateDocfile(fileW, STGM_CREATE | STGM_READWRITE |
                          STGM_SHARE_EXCLUSIVE | STGM_TRANSACTED, 0, &stg);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create);

    hr = IStorage_Commit(stg, STGC_DEFAULT);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create_commit);

    IStorage_Release(stg);

    test_file_access("winetest", create_close);

    DeleteFileA("winetest");

    /* STGM_DIRECT */

    hr = StgCreateDocfile(fileW, STGM_CREATE | STGM_READWRITE |
                          STGM_SHARE_EXCLUSIVE | STGM_DIRECT, 0, &stg);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create);

    hr = IStorage_Commit(stg, STGC_DEFAULT);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create_commit);

    IStorage_Release(stg);

    test_file_access("winetest", create_close);

    DeleteFileA("winetest");

    /* STGM_SHARE_DENY_NONE */

    hr = StgCreateDocfile(fileW, STGM_CREATE | STGM_READWRITE |
                          STGM_SHARE_DENY_NONE | STGM_TRANSACTED, 0, &stg);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create);

    hr = IStorage_Commit(stg, STGC_DEFAULT);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create_commit);

    IStorage_Release(stg);

    test_file_access("winetest", create_close);

    DeleteFileA("winetest");

    /* STGM_SHARE_DENY_READ */

    hr = StgCreateDocfile(fileW, STGM_CREATE | STGM_READWRITE |
                          STGM_SHARE_DENY_READ | STGM_TRANSACTED, 0, &stg);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create);

    hr = IStorage_Commit(stg, STGC_DEFAULT);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create_commit);

    IStorage_Release(stg);

    test_file_access("winetest", create_close);

    DeleteFileA("winetest");

    /* STGM_SHARE_DENY_WRITE */

    hr = StgCreateDocfile(fileW, STGM_CREATE | STGM_READWRITE |
                          STGM_SHARE_DENY_WRITE | STGM_TRANSACTED, 0, &stg);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create);

    hr = IStorage_Commit(stg, STGC_DEFAULT);
    ok(hr == S_OK, "Expected S_OK, got %08x\n", hr);

    test_file_access("winetest", create_commit);

    IStorage_Release(stg);

    test_file_access("winetest", create_close);

    DeleteFileA("winetest");
}

static void test_readonly(void)
{
    IStorage *stg, *stg2, *stg3;
    IStream *stream;
    HRESULT hr;
    static const WCHAR fileW[] = {'w','i','n','e','t','e','s','t',0};
    static const WCHAR storageW[] = {'s','t','o','r','a','g','e',0};
    static const WCHAR streamW[] = {'s','t','r','e','a','m',0};

    hr = StgCreateDocfile( fileW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &stg);
    ok(hr == S_OK, "should succeed, res=%x\n", hr);
    if (SUCCEEDED(hr))
    {
        hr = IStorage_CreateStorage( stg, storageW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stg2 );
        ok(hr == S_OK, "should succeed, res=%x\n", hr);
        if (SUCCEEDED(hr))
        {
            hr = IStorage_CreateStream( stg2, streamW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stream );
            ok(hr == S_OK, "should succeed, res=%x\n", hr);
            if (SUCCEEDED(hr))
                IStream_Release(stream);
            IStorage_Release(stg2);
        }
        IStorage_Release(stg);
    }

    /* re-open read only */
    hr = StgOpenStorage( fileW, NULL, STGM_TRANSACTED | STGM_SHARE_DENY_NONE | STGM_READ, NULL, 0, &stg);
    ok(hr == S_OK, "should succeed, res=%x\n", hr);
    if (SUCCEEDED(hr))
    {
        hr = IStorage_OpenStorage( stg, storageW, NULL, STGM_SHARE_EXCLUSIVE | STGM_READ, NULL, 0, &stg2 );
        ok(hr == S_OK, "should succeed, res=%x\n", hr);
        if (SUCCEEDED(hr))
        {
            /* CreateStream on read-only storage, name exists */
            hr = IStorage_CreateStream( stg2, streamW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READ, 0, 0, &stream );
            ok(hr == STG_E_ACCESSDENIED, "should fail, res=%x\n", hr);
            if (SUCCEEDED(hr))
                IStream_Release(stream);

            /* CreateStream on read-only storage, name does not exist */
            hr = IStorage_CreateStream( stg2, storageW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READ, 0, 0, &stream );
            ok(hr == STG_E_ACCESSDENIED, "should fail, res=%x\n", hr);
            if (SUCCEEDED(hr))
                IStream_Release(stream);

            /* CreateStorage on read-only storage, name exists */
            hr = IStorage_CreateStorage( stg2, streamW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READ, 0, 0, &stg3 );
            ok(hr == STG_E_FILEALREADYEXISTS, "should fail, res=%x\n", hr);
            if (SUCCEEDED(hr))
                IStream_Release(stg3);

            /* CreateStorage on read-only storage, name does not exist */
            hr = IStorage_CreateStorage( stg2, storageW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READ, 0, 0, &stg3 );
            ok(hr == STG_E_ACCESSDENIED, "should fail, res=%x\n", hr);
            if (SUCCEEDED(hr))
                IStream_Release(stg3);

            /* DestroyElement on read-only storage, name exists */
            hr = IStorage_DestroyElement( stg2, streamW );
            ok(hr == STG_E_ACCESSDENIED, "should fail, res=%x\n", hr);

            /* DestroyElement on read-only storage, name does not exist */
            hr = IStorage_DestroyElement( stg2, storageW );
            ok(hr == STG_E_ACCESSDENIED, "should fail, res=%x\n", hr);

            IStorage_Release(stg2);
        }

        IStorage_Release(stg);
    }

    DeleteFileA("winetest");
}

static void test_simple(void)
{
    /* Tests for STGM_SIMPLE mode */

    IStorage *stg;
    HRESULT r;
    IStream *stm;
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR stmname2[] = { 'S','m','a','l','l',0 };
    LARGE_INTEGER pos;
    ULARGE_INTEGER upos;
    DWORD count;
    STATSTG stat;

    DeleteFileA(filenameA);

    r = StgCreateDocfile( filename, STGM_SIMPLE | STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &stg);
    ok(r == S_OK, "got %08x\n", r);

    r = IStorage_CreateStream(stg, stmname, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
    ok(r == STG_E_INVALIDFLAG, "got %08x\n", r);
    r = IStorage_CreateStream(stg, stmname, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
    ok(r == S_OK, "got %08x\n", r);

    upos.QuadPart = 6000;
    r = IStream_SetSize(stm, upos);
    ok(r == S_OK, "got %08x\n", r);

    r = IStream_Write(stm, "foo", 3, &count);
    ok(r == S_OK, "got %08x\n", r);
    ok(count == 3, "got %d\n", count);

    pos.QuadPart = 0;
    r = IStream_Seek(stm, pos, STREAM_SEEK_CUR, &upos);
    ok(r == S_OK, "got %08x\n", r);
    ok(upos.QuadPart == 3, "got %d\n", upos.u.LowPart);

    r = IStream_Stat(stm, &stat, STATFLAG_NONAME);
    ok(r == S_OK ||
       broken(r == STG_E_INVALIDFUNCTION), /* NT4 and below */
       "got %08x\n", r);
    if (r == S_OK)
        ok(stat.cbSize.QuadPart == 3, "got %d\n", stat.cbSize.u.LowPart);

    pos.QuadPart = 1;
    r = IStream_Seek(stm, pos, STREAM_SEEK_SET, &upos);
    ok(r == S_OK, "got %08x\n", r);
    ok(upos.QuadPart == 1, "got %d\n", upos.u.LowPart);

    r = IStream_Stat(stm, &stat, STATFLAG_NONAME);
    ok(r == S_OK ||
       broken(r == STG_E_INVALIDFUNCTION), /* NT4 and below */
       "got %08x\n", r);
    if (r == S_OK)
        ok(stat.cbSize.QuadPart == 1, "got %d\n", stat.cbSize.u.LowPart);

    IStream_Release(stm);

    r = IStorage_CreateStream(stg, stmname2, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
    ok(r == S_OK, "got %08x\n", r);

    upos.QuadPart = 100;
    r = IStream_SetSize(stm, upos);
    ok(r == S_OK, "got %08x\n", r);

    r = IStream_Write(stm, "foo", 3, &count);
    ok(r == S_OK, "got %08x\n", r);
    ok(count == 3, "got %d\n", count);

    IStream_Release(stm);

    IStorage_Commit(stg, STGC_DEFAULT);
    IStorage_Release(stg);

    r = StgOpenStorage( filename, NULL, STGM_SIMPLE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, NULL, 0, &stg);
    if (r == STG_E_INVALIDFLAG)
    {
        win_skip("Flag combination is not supported on NT4 and below\n");
        DeleteFileA(filenameA);
        return;
    }
    ok(r == S_OK, "got %08x\n", r);

    r = IStorage_OpenStream(stg, stmname, NULL, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &stm);
    ok(r == S_OK, "got %08x\n", r);

    r = IStream_Stat(stm, &stat, STATFLAG_NONAME);
    ok(r == S_OK, "got %08x\n", r);
    ok(stat.cbSize.QuadPart == 6000, "got %d\n", stat.cbSize.u.LowPart);

    IStream_Release(stm);

    r = IStorage_OpenStream(stg, stmname2, NULL, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &stm);
    ok(r == S_OK, "got %08x\n", r);

    r = IStream_Stat(stm, &stat, STATFLAG_NONAME);
    ok(r == S_OK, "got %08x\n", r);
    ok(stat.cbSize.QuadPart == 4096, "got %d\n", stat.cbSize.u.LowPart);

    IStream_Release(stm);


    IStorage_Release(stg);

    DeleteFileA(filenameA);
}

static void test_fmtusertypestg(void)
{
    IStorage *stg;
    IEnumSTATSTG *stat;
    HRESULT hr;
    static const WCHAR fileW[] = {'f','m','t','t','e','s','t',0};
    static WCHAR userTypeW[] = {'S','t','g','U','s','r','T','y','p','e',0};
    static WCHAR strmNameW[] = {1,'C','o','m','p','O','b','j',0};

    hr = StgCreateDocfile( fileW, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &stg);
    ok(hr == S_OK, "should succeed, res=%x\n", hr);

    if (SUCCEEDED(hr))
    {
        /* try to write the stream */
        hr = WriteFmtUserTypeStg(stg, 0, userTypeW);
        ok(hr == S_OK, "should succeed, res=%x\n", hr);

        /* check that the stream was created */
        hr = IStorage_EnumElements(stg, 0, NULL, 0, &stat);
        ok(hr == S_OK, "should succeed, res=%x\n", hr);
        if (SUCCEEDED(hr))
        {
            BOOL found = FALSE;
            STATSTG statstg;
            DWORD got;
            while ((hr = IEnumSTATSTG_Next(stat, 1, &statstg, &got)) == S_OK && got == 1)
            {
                if (lstrcmpW(statstg.pwcsName, strmNameW) == 0)
                    found = TRUE;
                else
                    ok(0, "found unexpected stream or storage\n");
            }
            ok(found == TRUE, "expected storage to contain stream \\0001CompObj\n");
            IEnumSTATSTG_Release(stat);
        }

        /* re-write the stream */
        hr = WriteFmtUserTypeStg(stg, 0, userTypeW);
        ok(hr == S_OK, "should succeed, res=%x\n", hr);

        /* check that the stream is still there */
        hr = IStorage_EnumElements(stg, 0, NULL, 0, &stat);
        ok(hr == S_OK, "should succeed, res=%x\n", hr);
        if (SUCCEEDED(hr))
        {
            BOOL found = FALSE;
            STATSTG statstg;
            DWORD got;
            while ((hr = IEnumSTATSTG_Next(stat, 1, &statstg, &got)) == S_OK && got == 1)
            {
                if (lstrcmpW(statstg.pwcsName, strmNameW) == 0)
                    found = TRUE;
                else
                    ok(0, "found unexpected stream or storage\n");
            }
            ok(found == TRUE, "expected storage to contain stream \\0001CompObj\n");
            IEnumSTATSTG_Release(stat);
        }

        IStorage_Release(stg);
        DeleteFileW( fileW );
    }
}

static void test_references(void)
{
    IStorage *stg,*stg2;
    HRESULT hr;
    unsigned c1,c2;
    static const WCHAR StorName[] = { 'D','a','t','a','S','p','a','c','e','I','n','f','o',0 };

    DeleteFileA(filenameA);

    hr = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE |STGM_TRANSACTED, 0, &stg);
    ok(hr==S_OK, "StgCreateDocfile failed\n");

    if (SUCCEEDED(hr))
    {
        IStorage_Release(stg);

        hr = StgOpenStorage( filename, NULL, STGM_TRANSACTED | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, NULL, 0, &stg);
        ok(hr==S_OK, "StgOpenStorage failed (result=%x)\n",hr);

        if (SUCCEEDED(hr))
        {
            hr = IStorage_CreateStorage(stg,StorName,STGM_READWRITE | STGM_SHARE_EXCLUSIVE,0,0,&stg2);
            ok(hr == S_OK, "IStorage_CreateStorage failed (result=%x)\n",hr);

            if (SUCCEEDED(hr))
            {
                c1 = IStorage_AddRef(stg);
                ok(c1 == 2, "creating internal storage added references to ancestor\n");
                c1 = IStorage_AddRef(stg);
                IStorage_Release(stg2);
                c2 = IStorage_AddRef(stg) - 1;
                ok(c1 == c2, "releasing internal storage removed references to ancestor\n");
            }
            c1 = IStorage_Release(stg);
            while ( c1 ) c1 = IStorage_Release(stg);
        }
    }

    DeleteFileA(filenameA);
}

/* dest
 *  |-StorageA
 *  |  `StreamA: "StreamA"
 *  |-StorageB
 *  |  `StreamB: "StreamB"
 *  `StreamC: "StreamC"
 */
static HRESULT create_test_file(IStorage *dest)
{
    IStorage *stgA = NULL, *stgB = NULL;
    IStream *strmA = NULL, *strmB = NULL, *strmC = NULL;
    const ULONG strmA_name_size = lstrlenW(strmA_name) * sizeof(WCHAR);
    const ULONG strmB_name_size = lstrlenW(strmB_name) * sizeof(WCHAR);
    const ULONG strmC_name_size = lstrlenW(strmC_name) * sizeof(WCHAR);
    ULONG bytes;
    HRESULT hr;

    hr = IStorage_CreateStorage(dest, stgA_name, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stgA);
    ok(hr == S_OK, "IStorage_CreateStorage failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = IStorage_CreateStream(stgA, strmA_name, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &strmA);
    ok(hr == S_OK, "IStorage_CreateStream failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = IStream_Write(strmA, strmA_name, strmA_name_size, &bytes);
    ok(hr == S_OK && bytes == strmA_name_size, "IStream_Write failed: 0x%08x, %d of %d bytes written\n", hr, bytes, strmA_name_size);

    hr = IStorage_CreateStorage(dest, stgB_name, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stgB);
    ok(hr == S_OK, "IStorage_CreateStorage failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = IStorage_CreateStream(stgB, strmB_name, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &strmB);
    ok(hr == S_OK, "IStorage_CreateStream failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = IStream_Write(strmB, strmB_name, strmB_name_size, &bytes);
    ok(hr == S_OK && bytes == strmB_name_size, "IStream_Write failed: 0x%08x, %d of %d bytes written\n", hr, bytes, strmB_name_size);

    hr = IStorage_CreateStream(dest, strmC_name, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &strmC);
    ok(hr == S_OK, "IStorage_CreateStream failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = IStream_Write(strmC, strmC_name, strmC_name_size, &bytes);
    ok(hr == S_OK && bytes == strmC_name_size, "IStream_Write failed: 0x%08x, %d of %d bytes written\n", hr, bytes, strmC_name_size);

cleanup:
    if(strmC)
        IStream_Release(strmC);
    if(strmB)
        IStream_Release(strmB);
    if(stgB)
        IStorage_Release(stgB);
    if(strmA)
        IStream_Release(strmA);
    if(stgA)
        IStorage_Release(stgA);

    return hr;
}

static void test_copyto(void)
{
    IStorage *file1 = NULL, *file2 = NULL, *stg_tmp;
    IStream *strm_tmp;
    WCHAR buf[64];
    HRESULT hr;

    /* create & populate file1 */
    hr = StgCreateDocfile(file1_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file1);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = create_test_file(file1);
    if(FAILED(hr))
        goto cleanup;

    /* create file2 */
    hr = StgCreateDocfile(file2_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file2);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* copy file1 into file2 */
    hr = IStorage_CopyTo(file1, 0, NULL, NULL, NULL);
    ok(hr == STG_E_INVALIDPOINTER, "CopyTo should give STG_E_INVALIDPONITER, gave: 0x%08x\n", hr);

    hr = IStorage_CopyTo(file1, 0, NULL, NULL, file2);
    ok(hr == S_OK, "CopyTo failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* verify that all of file1 was copied */
    hr = IStorage_OpenStorage(file2, stgA_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == S_OK, "OpenStorage failed: 0x%08x\n", hr);

    if(SUCCEEDED(hr)){
        hr = IStorage_OpenStream(stg_tmp, strmA_name, NULL,
                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
        ok(hr == S_OK, "OpenStream failed: 0x%08x\n", hr);

        if(SUCCEEDED(hr)){
            memset(buf, 0, sizeof(buf));
            hr = IStream_Read(strm_tmp, buf, sizeof(buf), NULL);
            ok(hr == S_OK, "Read failed: 0x%08x\n", hr);
            if(SUCCEEDED(hr))
                ok(lstrcmpW(buf, strmA_name) == 0,
                        "Expected %s to be read, got %s\n", wine_dbgstr_w(strmA_name), wine_dbgstr_w(buf));

            IStream_Release(strm_tmp);
        }

        IStorage_Release(stg_tmp);
    }

    hr = IStorage_OpenStorage(file2, stgB_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == S_OK, "OpenStorage failed: 0x%08x\n", hr);

    if(SUCCEEDED(hr)){
        hr = IStorage_OpenStream(stg_tmp, strmB_name, NULL,
                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
        ok(hr == S_OK, "OpenStream failed: 0x%08x\n", hr);

        if(SUCCEEDED(hr)){
            memset(buf, 0, sizeof(buf));
            hr = IStream_Read(strm_tmp, buf, sizeof(buf), NULL);
            ok(hr == S_OK, "Read failed: 0x%08x\n", hr);
            if(SUCCEEDED(hr))
                ok(lstrcmpW(buf, strmB_name) == 0,
                        "Expected %s to be read, got %s\n", wine_dbgstr_w(strmB_name), wine_dbgstr_w(buf));

            IStream_Release(strm_tmp);
        }

        IStorage_Release(stg_tmp);
    }

    hr = IStorage_OpenStream(file2, strmC_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
    ok(hr == S_OK, "OpenStream failed: 0x%08x\n", hr);

    if(SUCCEEDED(hr)){
        memset(buf, 0, sizeof(buf));
        hr = IStream_Read(strm_tmp, buf, sizeof(buf), NULL);
        ok(hr == S_OK, "Read failed: 0x%08x\n", hr);
        if(SUCCEEDED(hr))
            ok(lstrcmpW(buf, strmC_name) == 0,
                    "Expected %s to be read, got %s\n", wine_dbgstr_w(strmC_name), wine_dbgstr_w(buf));

        IStream_Release(strm_tmp);
    }

cleanup:
    if(file1)
        IStorage_Release(file1);
    if(file2)
        IStorage_Release(file2);

    DeleteFileW(file1_name);
    DeleteFileW(file2_name);
}

static void test_copyto_snbexclusions(void)
{
    static const WCHAR *snb_exclude[] = {stgA_name, strmB_name, strmC_name, 0};

    IStorage *file1 = NULL, *file2 = NULL, *stg_tmp;
    IStream *strm_tmp;
    WCHAR buf[64];
    HRESULT hr;

    /* create & populate file1 */
    hr = StgCreateDocfile(file1_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file1);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = create_test_file(file1);
    if(FAILED(hr))
        goto cleanup;

    /* create file2 */
    hr = StgCreateDocfile(file2_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file2);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* copy file1 to file2 with name exclusions */
    hr = IStorage_CopyTo(file1, 0, NULL, (SNB)snb_exclude, file2);
    ok(hr == S_OK, "CopyTo failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* verify that file1 copied over, respecting exclusions */
    hr = IStorage_OpenStorage(file2, stgA_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStorage should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStorage_Release(stg_tmp);

    hr = IStorage_OpenStream(file2, strmA_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStream should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStream_Release(strm_tmp);

    hr = IStorage_OpenStorage(file2, stgB_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == S_OK, "OpenStorage failed: 0x%08x\n", hr);

    if(SUCCEEDED(hr)){
        hr = IStorage_OpenStream(stg_tmp, strmB_name, NULL,
                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
        ok(hr == S_OK, "OpenStream failed: 0x%08x\n", hr);

        if(SUCCEEDED(hr)){
            memset(buf, 0, sizeof(buf));
            hr = IStream_Read(strm_tmp, buf, sizeof(buf), NULL);
            ok(hr == S_OK, "Read failed: 0x%08x\n", hr);
            if(SUCCEEDED(hr))
                ok(lstrcmpW(buf, strmB_name) == 0,
                        "Expected %s to be read, got %s\n", wine_dbgstr_w(strmB_name), wine_dbgstr_w(buf));

            IStream_Release(strm_tmp);
        }

        IStorage_Release(stg_tmp);
    }

    hr = IStorage_OpenStream(file2, strmC_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStream should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStream_Release(strm_tmp);

cleanup:
    if(file1)
        IStorage_Release(file1);
    if(file2)
        IStorage_Release(file2);

    DeleteFileW(file1_name);
    DeleteFileW(file2_name);
}

static void test_copyto_iidexclusions_storage(void)
{
    IStorage *file1 = NULL, *file2 = NULL, *stg_tmp;
    IStream *strm_tmp;
    WCHAR buf[64];
    HRESULT hr;

    /* create & populate file1 */
    hr = StgCreateDocfile(file1_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file1);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = create_test_file(file1);
    if(FAILED(hr))
        goto cleanup;

    /* create file2 */
    hr = StgCreateDocfile(file2_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file2);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* copy file1 to file2 with iid exclusions */
    hr = IStorage_CopyTo(file1, 1, &IID_IStorage, NULL, file2);
    ok(hr == S_OK, "CopyTo failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* verify that file1 copied over, respecting exclusions */
    hr = IStorage_OpenStorage(file2, stgA_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStorage should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStorage_Release(stg_tmp);

    hr = IStorage_OpenStream(file2, strmA_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStream should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStream_Release(strm_tmp);

    hr = IStorage_OpenStorage(file2, stgB_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStorage should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStorage_Release(stg_tmp);

    hr = IStorage_OpenStream(file2, strmB_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStream should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStream_Release(strm_tmp);

    hr = IStorage_OpenStream(file2, strmC_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
    ok(hr == S_OK, "OpenStream failed: 0x%08x\n", hr);

    if(SUCCEEDED(hr)){
        memset(buf, 0, sizeof(buf));
        hr = IStream_Read(strm_tmp, buf, sizeof(buf), NULL);
        ok(hr == S_OK, "Read failed: 0x%08x\n", hr);
        if(SUCCEEDED(hr))
            ok(lstrcmpW(buf, strmC_name) == 0,
                    "Expected %s to be read, got %s\n", wine_dbgstr_w(strmC_name), wine_dbgstr_w(buf));

        IStream_Release(strm_tmp);
    }

cleanup:
    if(file1)
        IStorage_Release(file1);
    if(file2)
        IStorage_Release(file2);

    DeleteFileW(file1_name);
    DeleteFileW(file2_name);
}

static void test_copyto_iidexclusions_stream(void)
{
    IStorage *file1 = NULL, *file2 = NULL, *stg_tmp;
    IStream *strm_tmp;
    HRESULT hr;

    /* create & populate file1 */
    hr = StgCreateDocfile(file1_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file1);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    hr = create_test_file(file1);
    if(FAILED(hr))
        goto cleanup;

    /* create file2 */
    hr = StgCreateDocfile(file2_name, STGM_CREATE | STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, &file2);
    ok(hr == S_OK, "StgCreateDocfile failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* copy file1 to file2 with iid exclusions */
    hr = IStorage_CopyTo(file1, 1, &IID_IStream, NULL, file2);
    ok(hr == S_OK, "CopyTo failed: 0x%08x\n", hr);
    if(FAILED(hr))
        goto cleanup;

    /* verify that file1 copied over, respecting exclusions */
    hr = IStorage_OpenStorage(file2, stgA_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == S_OK, "OpenStorage failed: 0x%08x\n", hr);

    if(SUCCEEDED(hr)){
        hr = IStorage_OpenStream(stg_tmp, strmA_name, NULL,
                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
        ok(hr == STG_E_FILENOTFOUND, "OpenStream should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
        if(SUCCEEDED(hr))
            IStream_Release(strm_tmp);

        IStorage_Release(stg_tmp);
    }

    hr = IStorage_OpenStorage(file2, stgB_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg_tmp);
    ok(hr == S_OK, "OpenStorage failed: 0x%08x\n", hr);

    if(SUCCEEDED(hr)){
        hr = IStorage_OpenStream(stg_tmp, strmB_name, NULL,
                STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
        ok(hr == STG_E_FILENOTFOUND, "OpenStream should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
        if(SUCCEEDED(hr))
            IStream_Release(strm_tmp);

        IStorage_Release(stg_tmp);
    }

    hr = IStorage_OpenStream(file2, strmC_name, NULL,
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &strm_tmp);
    ok(hr == STG_E_FILENOTFOUND, "OpenStream should give STG_E_FILENOTFOUND, gave: 0x%08x\n", hr);
    if(SUCCEEDED(hr))
        IStream_Release(strm_tmp);

cleanup:
    if(file1)
        IStorage_Release(file1);
    if(file2)
        IStorage_Release(file2);

    DeleteFileW(file1_name);
    DeleteFileW(file2_name);
}

static void test_rename(void)
{
    IStorage *stg, *stg2;
    IStream *stm;
    HRESULT r;
    static const WCHAR stgname[] = { 'P','E','R','M','S','T','G',0 };
    static const WCHAR stgname2[] = { 'S','T','G',0 };
    static const WCHAR stmname[] = { 'C','O','N','T','E','N','T','S',0 };
    static const WCHAR stmname2[] = { 'E','N','T','S',0 };

    DeleteFileA(filenameA);

    /* create the file */
    r = StgCreateDocfile( filename, STGM_CREATE | STGM_SHARE_EXCLUSIVE |
                            STGM_READWRITE, 0, &stg);
    ok(r==S_OK, "StgCreateDocfile failed\n");

    /* create a substorage */
    r = IStorage_CreateStorage(stg, stgname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stg2);
    ok(r==S_OK, "IStorage->CreateStorage failed, hr=%08x\n", r);

    /* create a stream in the substorage */
    r = IStorage_CreateStream(stg2, stmname, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &stm);
    ok(r==S_OK, "IStorage->CreateStream failed, hr=%08x\n", r);
    IStream_Release(stm);

    /* rename the stream */
    r = IStorage_RenameElement(stg2, stmname, stmname2);
    ok(r==S_OK, "IStorage->RenameElement failed, hr=%08x\n", r);

    /* cannot open stream with old name */
    r = IStorage_OpenStream(stg2, stmname, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stm);
    ok(r==STG_E_FILENOTFOUND, "IStorage_OpenStream should fail, hr=%08x\n", r);
    if (SUCCEEDED(r)) IStream_Release(stm);

    /* can open stream with new name */
    r = IStorage_OpenStream(stg2, stmname2, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stm);
    ok(r==S_OK, "IStorage_OpenStream failed, hr=%08x\n", r);
    if (SUCCEEDED(r)) IStream_Release(stm);

    IStorage_Release(stg2);

    /* rename the storage */
    IStorage_RenameElement(stg, stgname, stgname2);

    /* cannot open storage with old name */
    r = IStorage_OpenStorage(stg, stgname, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg2);
    ok(r==STG_E_FILENOTFOUND, "IStorage_OpenStream should fail, hr=%08x\n", r);
    if (SUCCEEDED(r)) IStorage_Release(stg2);

    /* can open storage with new name */
    r = IStorage_OpenStorage(stg, stgname2, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, NULL, 0, &stg2);
    ok(r==S_OK, "IStorage_OpenStream should fail, hr=%08x\n", r);
    if (SUCCEEDED(r))
    {
        /* opened storage still has the stream */
        r = IStorage_OpenStream(stg2, stmname2, NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE, 0, &stm);
        ok(r==S_OK, "IStorage_OpenStream failed, hr=%08x\n", r);
        if (SUCCEEDED(r)) IStream_Release(stm);

        IStorage_Release(stg2);
    }

    IStorage_Release(stg);

    r = DeleteFileA(filenameA);
    ok( r == TRUE, "deleted file\n");
}

START_TEST(storage32)
{
    CHAR temp[MAX_PATH];

    GetTempPathA(MAX_PATH, temp);
    if(!GetTempFileNameA(temp, "stg", 0, filenameA))
    {
        win_skip("Could not create temp file, %u\n", GetLastError());
        return;
    }
    MultiByteToWideChar(CP_ACP, 0, filenameA, -1, filename, MAX_PATH);
    DeleteFileA(filenameA);

    test_hglobal_storage_stat();
    test_create_storage_modes();
    test_storage_stream();
    test_open_storage();
    test_storage_suminfo();
    test_storage_refcount();
    test_streamenum();
    test_transact();
    test_substorage_share();
    test_revert();
    test_parent_free();
    test_nonroot_transacted();
    test_ReadClassStm();
    test_access();
    test_writeclassstg();
    test_readonly();
    test_simple();
    test_fmtusertypestg();
    test_references();
    test_copyto();
    test_copyto_snbexclusions();
    test_copyto_iidexclusions_storage();
    test_copyto_iidexclusions_stream();
    test_rename();
}
