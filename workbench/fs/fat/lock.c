/*
 * fat.handler - FAT12/16/32 filesystem handler
 *
 * Copyright � 2006 Marek Szyprowski
 * Copyright � 2007 The AROS Development Team
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the same terms as AROS itself.
 *
 * $Id$
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include <string.h>

#include "fat_fs.h"
#include "fat_protos.h"

#define sb glob->sb

LONG TryLockObj(struct ExtFileLock *fl, UBYTE *name, LONG namelen, LONG access, BPTR *result) {
    LONG err = ERROR_OBJECT_NOT_FOUND;
    struct DirHandle dh;
    struct DirEntry de;
    ULONG start_cluster;

    if (fl && (fl->attr & ATTR_DIRECTORY) == 0)
        return ERROR_OBJECT_WRONG_TYPE;

    start_cluster = (fl) ? fl->first_cluster : 0;
    
    kprintf("\tSearching for: "); knprints(name, namelen);
    SkipColon(name, namelen);

    InitDirHandle(sb, start_cluster, &dh);
    err = GetDirEntryByPath(&dh, name, namelen, &de);

    if (err == 0) {
        if (FIRST_FILE_CLUSTER(&de) == 0)
            err = LockRoot(access, result);
        else
            err = LockFile(de.index, dh.cur_cluster, access, result);
    }

    ReleaseDirHandle(&dh);

    return err;
}

LONG LockFile(ULONG entry, ULONG cluster, LONG axs, BPTR *res) {
    struct ExtFileLock *fl;

    kprintf("\tLockFile entry %ld cluster %ld\n", entry, cluster);

    if ((fl = FS_AllocMem(sizeof(struct ExtFileLock)))) {
        struct DirHandle dh;
        struct DirEntry de;
        ULONG len;

        InitDirHandle(sb, cluster, &dh);
        GetDirEntry(&dh, entry, &de);

        fl->fl_Access = axs;
        fl->fl_Task = glob->ourport;
        fl->fl_Volume = MKBADDR(sb->doslist);
        fl->fl_Link = sb->doslist->dol_misc.dol_volume.dol_LockList;
        fl->magic = ID_FAT_DISK;

        sb->doslist->dol_misc.dol_volume.dol_LockList = MKBADDR(fl);

        fl->dir_entry = entry;
        fl->dir_cluster = cluster;
        fl->attr = de.e.entry.attr | ATTR_REALENTRY;
        fl->first_cluster = FIRST_FILE_CLUSTER(&de);
        fl->size = AROS_LE2LONG(de.e.entry.file_size);

        GetDirShortName(&de, &(fl->name[1]), &len); fl->name[0] = (UBYTE) len;
        GetDirLongName(&de, &(fl->name[1]), &len); fl->name[0] = (UBYTE) len;

        ReleaseDirHandle(&dh);

        *res = MKBADDR(fl);
        return 0;
    }

    return ERROR_NO_FREE_STORE;
}

LONG LockRoot(LONG axs, BPTR *res) {
    struct ExtFileLock *fl;

    kprintf("\tLockRoot()\n");

    if ((fl = FS_AllocMem(sizeof(struct ExtFileLock)))) {
        fl->fl_Access = axs;
        fl->fl_Task = glob->ourport;
        fl->fl_Volume = MKBADDR(sb->doslist);
        fl->fl_Link = sb->doslist->dol_misc.dol_volume.dol_LockList;
        fl->magic = ID_FAT_DISK;

        sb->doslist->dol_misc.dol_volume.dol_LockList = MKBADDR(fl);

        fl->dir_entry = FAT_ROOTDIR_MARK;
        fl->dir_cluster = FAT_ROOTDIR_MARK;
        fl->attr = ATTR_DIRECTORY | ATTR_ROOTDIR;
        fl->first_cluster = 0;
        fl->size = 0;

        memcpy(fl->name, sb->volume.name, 32);

        *res = MKBADDR(fl);
        return 0;
    }

    return ERROR_NO_FREE_STORE;
}

LONG CopyLock(struct ExtFileLock *src_fl, BPTR *res) {
    struct ExtFileLock *fl;

    if (src_fl->fl_Access == EXCLUSIVE_LOCK)
        return ERROR_OBJECT_IN_USE;

    if ((fl = FS_AllocMem(sizeof(struct ExtFileLock)))) {
        fl->fl_Access = src_fl->fl_Access;
        fl->fl_Task = glob->ourport;
        fl->fl_Volume = MKBADDR(sb->doslist);
        fl->fl_Link = sb->doslist->dol_misc.dol_volume.dol_LockList;
        fl->magic = ID_FAT_DISK;

        sb->doslist->dol_misc.dol_volume.dol_LockList = MKBADDR(fl);

        fl->dir_entry = src_fl->dir_entry;
        fl->dir_cluster = src_fl->dir_cluster;
        fl->attr = src_fl->attr;
        fl->first_cluster = src_fl->first_cluster;
        fl->size = src_fl->size;

        memcpy(fl->name, src_fl->name, 108);

        *res = MKBADDR(fl);
        return 0;
    }

    return ERROR_NO_FREE_STORE;
}

LONG LockParent(struct ExtFileLock *fl, LONG axs, BPTR *res) {
    LONG err;
    struct DirHandle dh;
    struct DirEntry de;
    ULONG parent_cluster;

    if (fl->dir_cluster == 0)
        return LockRoot(axs, res);

    /* get the parent dir */
    InitDirHandle(sb, fl->dir_cluster, &dh);
    if ((err = GetDirEntryByPath(&dh, "/", 1, &de)) != 0) {
        ReleaseDirHandle(&dh);
        return err;
    }

    /* and its cluster */
    parent_cluster = FIRST_FILE_CLUSTER(&de);

    /* then we go through the parent dir, looking for a link back to us. we do
     * this so that we have an entry with the proper name for copying by
     * LockFile() */
    InitDirHandle(sb, parent_cluster, &dh);
    while ((err = GetDirEntry(&dh, dh.cur_index + 1, &de)) == 0) {
        /* don't go past the end */
        if (de.e.entry.name[0] == 0x00) {
            err = ERROR_OBJECT_NOT_FOUND;
            break;
        }

        /* we found it if its not empty, and its not the volume id or a long
         * name, and it is a directory, and it does point to us */
        if (de.e.entry.name[0] != 0xe5 &&
            !(de.e.entry.attr & ATTR_VOLUME_ID) &&
            de.e.entry.attr & ATTR_DIRECTORY &&
            FIRST_FILE_CLUSTER(&de) == fl->dir_cluster) {
            
            err = LockFile(dh.cur_index, parent_cluster, axs, res);
            break;
        }
    }

    ReleaseDirHandle(&dh);
    return err;
}

#undef sb

LONG FreeLockSB(struct ExtFileLock *fl, struct FSSuper *sb) {
    LONG found = FALSE;

    if (sb == NULL)
        return ERROR_OBJECT_NOT_FOUND;
    if (fl->magic != ID_FAT_DISK)
        return ERROR_OBJECT_WRONG_TYPE;

    if (fl == BADDR(sb->doslist->dol_misc.dol_volume.dol_LockList)) {
        sb->doslist->dol_misc.dol_volume.dol_LockList = fl->fl_Link;
        found = TRUE;
    }
    else {
        struct ExtFileLock *prev = NULL, *ptr = BADDR(sb->doslist->dol_misc.dol_volume.dol_LockList);

        while (ptr != NULL) {
            if (ptr == fl) {
                prev->fl_Link = fl->fl_Link;
                found = TRUE;
                break;
            }
            prev = ptr;
            ptr = BADDR(ptr->fl_Link);
        }
    }

    if (found) {
        kprintf("\tFreeing lock.\n");

        fl->fl_Task = NULL;

        FS_FreeMem(fl);

        return 0;
    }

    return ERROR_OBJECT_NOT_FOUND;
}

void FreeLock(struct ExtFileLock *fl) {
    struct FSSuper *ptr = glob->sblist, *prev=NULL;

    if (FreeLockSB(fl, glob->sb) == 0)
        return;

    while (ptr != NULL) {
        if (FreeLockSB(fl, ptr) == 0)
            break;

        prev = ptr;
        ptr = ptr->next;
    }

    if (ptr) {
        if (ptr->doslist->dol_misc.dol_volume.dol_LockList == NULL) { /* check if the device can be removed */
            kprintf("\tRemoving disk completely\n");

            SendVolumePacket(ptr->doslist, ACTION_VOLUME_REMOVE);

            ptr->doslist = NULL;
            FreeFATSuper(ptr);

            if (prev)
                prev->next = ptr->next;
            else
                glob->sblist = ptr->next;

            FS_FreeMem(ptr);
        }
    }
}

