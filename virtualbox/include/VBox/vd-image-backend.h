/** @file
 * VD: Image backend interface.
 */

/*
 * Copyright (C) 2006-2014 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 *
 * The contents of this file may alternatively be used under the terms
 * of the Common Development and Distribution License Version 1.0
 * (CDDL) only, as it comes in the "COPYING.CDDL" file of the
 * VirtualBox OSE distribution, in which case the provisions of the
 * CDDL are applicable instead of those of the GPL.
 *
 * You may elect to license modified versions of this file under the
 * terms and conditions of either the GPL or the CDDL or both.
 */

#ifndef __vd_image_backend_h__
#define __vd_image_backend_h__

#include <VBox/vd.h>
#include <VBox/vd-ifs-internal.h>


/** @name VBox HDD backend write flags
 * @{
 */
/** Do not allocate a new block on this write. This is just an advisory
 * flag. The backend may still decide in some circumstances that it wants
 * to ignore this flag (which may cause extra dynamic image expansion). */
#define VD_WRITE_NO_ALLOC   RT_BIT(1)
/** @}*/

/** @name VBox HDD backend discard flags
 * @{
 */
/** Don't discard block but mark the given range as unused
 * (usually by writing 0's to it).
 * This doesn't require the range to be aligned on a block boundary but
 * the image size might not be decreased. */
#define VD_DISCARD_MARK_UNUSED RT_BIT(0)
/** @}*/

/** @name VBox HDD backend metadata traverse flags
 * @{
 */
/** Include per block metadata while traversing the metadata.
 * This might take much longer instead of traversing just global metadata. */
#define VD_TRAVERSE_METADATA_INCLUDE_PER_BLOCK_METADATA RT_BIT(0)
/** @}*/

/**
 * Image format backend interface used by VBox HDD Container implementation.
 */
typedef struct VBOXHDDBACKEND
{
    /**
     * The name of the backend (constant string).
     */
    const char *pszBackendName;

    /**
     * The size of the structure.
     */
    uint32_t cbSize;

    /**
     * The capabilities of the backend.
     */
    uint64_t uBackendCaps;

    /**
     * Pointer to a NULL-terminated array, containing the supported
     * file extensions. Note that some backends do not work on files, so this
     * pointer may just contain NULL.
     */
    PCVDFILEEXTENSION paFileExtensions;

    /**
     * Pointer to an array of structs describing each supported config key.
     * Terminated by a NULL config key. Note that some backends do not support
     * the configuration interface, so this pointer may just contain NULL.
     * Mandatory if the backend sets VD_CAP_CONFIG.
     */
    PCVDCONFIGINFO paConfigInfo;

    /**
     * Check if a file is valid for the backend.
     *
     * @returns VBox status code.
     * @param   pszFilename     Name of the image file.
     * @param   pVDIfsDisk      Pointer to the per-disk VD interface list.
     * @param   pVDIfsImage     Pointer to the per-image VD interface list.
     * @param   penmType        Returns the supported device type on success.
     */
    DECLR3CALLBACKMEMBER(int, pfnCheckIfValid, (const char *pszFilename, PVDINTERFACE pVDIfsDisk,
                                                PVDINTERFACE pVDIfsImage, VDTYPE *penmType));

    /**
     * Open a disk image.
     *
     * @returns VBox status code.
     * @param   pszFilename     Name of the image file to open. Guaranteed to be available and
     *                          unchanged during the lifetime of this image.
     * @param   uOpenFlags      Image file open mode, see VD_OPEN_FLAGS_* constants.
     * @param   pVDIfsDisk      Pointer to the per-disk VD interface list.
     * @param   pVDIfsImage     Pointer to the per-image VD interface list.
     * @param   enmType         Requested type of the image.
     * @param   ppBackendData   Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(int, pfnOpen, (const char *pszFilename, unsigned uOpenFlags,
                                        PVDINTERFACE pVDIfsDisk, PVDINTERFACE pVDIfsImage,
                                        VDTYPE enmType, void **ppBackendData));

    /**
     * Create a disk image.
     *
     * @returns VBox status code.
     * @param   pszFilename     Name of the image file to create. Guaranteed to be available and
     *                          unchanged during the lifetime of this image.
     * @param   cbSize          Image size in bytes.
     * @param   uImageFlags     Flags specifying special image features.
     * @param   pszComment      Pointer to image comment. NULL is ok.
     * @param   pPCHSGeometry   Physical drive geometry CHS <= (16383,16,255).
     * @param   pLCHSGeometry   Logical drive geometry CHS <= (1024,255,63).
     * @param   pUuid           New UUID of the image. Not NULL.
     * @param   uOpenFlags      Image file open mode, see VD_OPEN_FLAGS_* constants.
     * @param   uPercentStart   Starting value for progress percentage.
     * @param   uPercentSpan    Span for varying progress percentage.
     * @param   pVDIfsDisk      Pointer to the per-disk VD interface list.
     * @param   pVDIfsImage     Pointer to the per-image VD interface list.
     * @param   pVDIfsOperation Pointer to the per-operation VD interface list.
     * @param   ppBackendData   Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(int, pfnCreate, (const char *pszFilename, uint64_t cbSize,
                                          unsigned uImageFlags, const char *pszComment,
                                          PCVDGEOMETRY pPCHSGeometry,
                                          PCVDGEOMETRY pLCHSGeometry,
                                          PCRTUUID pUuid, unsigned uOpenFlags,
                                          unsigned uPercentStart, unsigned uPercentSpan,
                                          PVDINTERFACE pVDIfsDisk,
                                          PVDINTERFACE pVDIfsImage,
                                          PVDINTERFACE pVDIfsOperation,
                                          void **ppBackendData));

    /**
     * Rename a disk image. Only needs to work as long as the operating
     * system's rename file functionality is usable. If an attempt is made to
     * rename an image to a location on another disk/filesystem, this function
     * may just fail with an appropriate error code (not changing the opened
     * image data at all). Also works only on images which actually refer to
     * regular files. May be NULL.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pszFilename     New name of the image file. Guaranteed to be available and
     *                          unchanged during the lifetime of this image.
     */
    DECLR3CALLBACKMEMBER(int, pfnRename, (void *pBackendData, const char *pszFilename));

    /**
     * Close a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   fDelete         If true, delete the image from the host disk.
     */
    DECLR3CALLBACKMEMBER(int, pfnClose, (void *pBackendData, bool fDelete));

    /**
     * Start a read request.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   uOffset         The offset of the virtual disk to read from.
     * @param   cbRead          How many bytes to read.
     * @param   pIoCtx          I/O context associated with this request.
     * @param   pcbActuallyRead Pointer to returned number of bytes read.
     */
    DECLR3CALLBACKMEMBER(int, pfnRead, (void *pBackendData, uint64_t uOffset, size_t cbRead,
                                        PVDIOCTX pIoCtx, size_t *pcbActuallyRead));

    /**
     * Start a write request.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   uOffset         The offset of the virtual disk to write to.
     * @param   cbWrite         How many bytes to write.
     * @param   pIoCtx          I/O context associated with this request.
     * @param   pcbWriteProcess Pointer to returned number of bytes that could
     *                          be processed. In case the function returned
     *                          VERR_VD_BLOCK_FREE this is the number of bytes
     *                          that could be written in a full block write,
     *                          when prefixed/postfixed by the appropriate
     *                          amount of (previously read) padding data.
     * @param   pcbPreRead      Pointer to the returned amount of data that must
     *                          be prefixed to perform a full block write.
     * @param   pcbPostRead     Pointer to the returned amount of data that must
     *                          be postfixed to perform a full block write.
     * @param   fWrite          Flags which affect write behavior. Combination
     *                          of the VD_WRITE_* flags.
     */
    DECLR3CALLBACKMEMBER(int, pfnWrite, (void *pBackendData, uint64_t uOffset, size_t cbWrite,
                                         PVDIOCTX pIoCtx,
                                         size_t *pcbWriteProcess, size_t *pcbPreRead,
                                         size_t *pcbPostRead, unsigned fWrite));

    /**
     * Flush data to disk.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pIoCtx          I/O context associated with this request.
     */
    DECLR3CALLBACKMEMBER(int, pfnFlush, (void *pBackendData, PVDIOCTX pIoCtx));

    /**
     * Discards the given amount of bytes decreasing the size of the image if possible
     *
     * @returns VBox status code.
     * @retval  VERR_VD_DISCARD_ALIGNMENT_NOT_MET if the range doesn't meet the required alignment
     *          for the discard.
     * @param   pBackendData         Opaque state data for this image.
     * @param   pIoCtx               I/O context associated with this request.
     * @param   uOffset              The offset of the first byte to discard.
     * @param   cbDiscard            How many bytes to discard.
     * @param   pcbPreAllocated      Pointer to the returned amount of bytes that must
     *                               be discarded before the range to perform a full
     *                               block discard.
     * @param   pcbPostAllocated     Pointer to the returned amount of bytes that must
     *                               be discarded after the range to perform a full
     *                               block discard.
     * @param   pcbActuallyDiscarded Pointer to the returned amount of bytes which
     *                               could be actually discarded.
     * @param   ppbmAllocationBitmap Where to store the pointer to the allocation bitmap
     *                               if VERR_VD_DISCARD_ALIGNMENT_NOT_MET is returned or NULL
     *                               if the allocation bitmap should be returned.
     * @param   fDiscard             Flags which affect discard behavior. Combination
     *                               of the VD_DISCARD_* flags.
     */
    DECLR3CALLBACKMEMBER(int, pfnDiscard, (void *pBackendData, PVDIOCTX pIoCtx,
                                           uint64_t uOffset, size_t cbDiscard,
                                           size_t *pcbPreAllocated,
                                           size_t *pcbPostAllocated,
                                           size_t *pcbActuallyDiscarded,
                                           void   **ppbmAllocationBitmap,
                                           unsigned fDiscard));

    /**
     * Get the version of a disk image.
     *
     * @returns version of disk image.
     * @param   pBackendData    Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(unsigned, pfnGetVersion, (void *pBackendData));

    /**
     * Get the sector size of a disk image.
     *
     * @returns size of disk image in bytes.
     * @param   pBackendData    Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(uint32_t, pfnGetSectorSize, (void *pBackendData));

    /**
     * Get the capacity of a disk image.
     *
     * @returns size of disk image in bytes.
     * @param   pBackendData    Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(uint64_t, pfnGetSize, (void *pBackendData));

    /**
     * Get the file size of a disk image.
     *
     * @returns size of disk image in bytes.
     * @param   pBackendData    Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(uint64_t, pfnGetFileSize, (void *pBackendData));

    /**
     * Get virtual disk PCHS geometry stored in a disk image.
     *
     * @returns VBox status code.
     * @returns VERR_VD_GEOMETRY_NOT_SET if no geometry present in the image.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pPCHSGeometry   Where to store the geometry. Not NULL.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetPCHSGeometry, (void *pBackendData, PVDGEOMETRY pPCHSGeometry));

    /**
     * Set virtual disk PCHS geometry stored in a disk image.
     * Only called if geometry is different than before.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pPCHSGeometry   Where to load the geometry from. Not NULL.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetPCHSGeometry, (void *pBackendData, PCVDGEOMETRY pPCHSGeometry));

    /**
     * Get virtual disk LCHS geometry stored in a disk image.
     *
     * @returns VBox status code.
     * @returns VERR_VD_GEOMETRY_NOT_SET if no geometry present in the image.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pLCHSGeometry   Where to store the geometry. Not NULL.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetLCHSGeometry, (void *pBackendData,  PVDGEOMETRY pLCHSGeometry));

    /**
     * Set virtual disk LCHS geometry stored in a disk image.
     * Only called if geometry is different than before.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pLCHSGeometry   Where to load the geometry from. Not NULL.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetLCHSGeometry, (void *pBackendData,  PCVDGEOMETRY pLCHSGeometry));

    /**
     * Get the image flags of a disk image.
     *
     * @returns image flags of disk image.
     * @param   pBackendData    Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(unsigned, pfnGetImageFlags, (void *pBackendData));

    /**
     * Get the open flags of a disk image.
     *
     * @returns open flags of disk image.
     * @param   pBackendData    Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(unsigned, pfnGetOpenFlags, (void *pBackendData));

    /**
     * Set the open flags of a disk image. May cause the image to be locked
     * in a different mode or be reopened (which can fail).
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   uOpenFlags      New open flags for this image.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetOpenFlags, (void *pBackendData, unsigned uOpenFlags));

    /**
     * Get comment of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pszComment      Where to store the comment.
     * @param   cbComment       Size of the comment buffer.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetComment, (void *pBackendData, char *pszComment, size_t cbComment));

    /**
     * Set comment of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pszComment      Where to get the comment from. NULL resets comment.
     *                          The comment is silently truncated if the image format
     *                          limit is exceeded.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetComment, (void *pBackendData, const char *pszComment));

    /**
     * Get UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to store the image UUID.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetUuid, (void *pBackendData, PRTUUID pUuid));

    /**
     * Set UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to get the image UUID from.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetUuid, (void *pBackendData, PCRTUUID pUuid));

    /**
     * Get last modification UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to store the image modification UUID.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetModificationUuid, (void *pBackendData, PRTUUID pUuid));

    /**
     * Set last modification UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to get the image modification UUID from.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetModificationUuid, (void *pBackendData, PCRTUUID pUuid));

    /**
     * Get parent UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to store the parent image UUID.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetParentUuid, (void *pBackendData, PRTUUID pUuid));

    /**
     * Set parent UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to get the parent image UUID from.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetParentUuid, (void *pBackendData, PCRTUUID pUuid));

    /**
     * Get parent modification UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to store the parent image modification UUID.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetParentModificationUuid, (void *pBackendData, PRTUUID pUuid));

    /**
     * Set parent modification UUID of a disk image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pUuid           Where to get the parent image modification UUID from.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetParentModificationUuid, (void *pBackendData, PCRTUUID pUuid));

    /**
     * Dump information about a disk image.
     *
     * @param   pBackendData    Opaque state data for this image.
     */
    DECLR3CALLBACKMEMBER(void, pfnDump, (void *pBackendData));

    /**
     * Get a time stamp of a disk image. May be NULL.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pTimeStamp      Where to store the time stamp.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetTimeStamp, (void *pBackendData, PRTTIMESPEC pTimeStamp));

    /**
     * Get the parent time stamp of a disk image. May be NULL.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pTimeStamp      Where to store the time stamp.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetParentTimeStamp, (void *pBackendData, PRTTIMESPEC pTimeStamp));

    /**
     * Set the parent time stamp of a disk image. May be NULL.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   pTimeStamp      Where to get the time stamp from.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetParentTimeStamp, (void *pBackendData, PCRTTIMESPEC pTimeStamp));

    /**
     * Get the relative path to parent image. May be NULL.
     *
     * @returns VBox status code.
     * @param   pBackendData      Opaque state data for this image.
     * @param   pszParentFilename Where to store the path.
     */
    DECLR3CALLBACKMEMBER(int, pfnGetParentFilename, (void *pBackendData, char **ppszParentFilename));

    /**
     * Set the relative path to parent image. May be NULL.
     *
     * @returns VBox status code.
     * @param   pBackendData      Opaque state data for this image.
     * @param   pszParentFilename Where to get the path from.
     */
    DECLR3CALLBACKMEMBER(int, pfnSetParentFilename, (void *pBackendData, const char *pszParentFilename));

    /** Returns a human readable hard disk location string given a
     *  set of hard disk configuration keys. The returned string is an
     *  equivalent of the full file path for image-based hard disks.
     *  Mandatory for backends with no VD_CAP_FILE and NULL otherwise. */
    DECLR3CALLBACKMEMBER(int, pfnComposeLocation, (PVDINTERFACE pConfig, char **pszLocation));

    /** Returns a human readable hard disk name string given a
     *  set of hard disk configuration keys. The returned string is an
     *  equivalent of the file name part in the full file path for
     *  image-based hard disks. Mandatory for backends with no
     *  VD_CAP_FILE and NULL otherwise. */
    DECLR3CALLBACKMEMBER(int, pfnComposeName, (PVDINTERFACE pConfig, char **pszName));

    /**
     * Compact the image. The pointer may be NULL, indicating that this
     * isn't supported yet (for file-based images) or not necessary.
     *
     * @returns VBox status code.
     * @returns VERR_NOT_SUPPORTED if this image cannot be compacted yet.
     * @param   pBackendData    Opaque state data for this image.
     * @param   uPercentStart   Starting value for progress percentage.
     * @param   uPercentSpan    Span for varying progress percentage.
     * @param   pVDIfsDisk      Pointer to the per-disk VD interface list.
     * @param   pVDIfsImage     Pointer to the per-image VD interface list.
     * @param   pVDIfsOperation Pointer to the per-operation VD interface list.
     */
    DECLR3CALLBACKMEMBER(int, pfnCompact, (void *pBackendData,
                                           unsigned uPercentStart, unsigned uPercentSpan,
                                           PVDINTERFACE pVDIfsDisk,
                                           PVDINTERFACE pVDIfsImage,
                                           PVDINTERFACE pVDIfsOperation));

    /**
     * Resize the image. The pointer may be NULL, indicating that this
     * isn't supported yet (for file-based images) or not necessary.
     *
     * @returns VBox status code.
     * @returns VERR_NOT_SUPPORTED if this image cannot be resized yet.
     * @param   pBackendData    Opaque state data for this image.
     * @param   cbSize          New size of the image.
     * @param   pPCHSGeometry   Pointer to the new physical disk geometry <= (16383,16,63). Not NULL.
     * @param   pLCHSGeometry   Pointer to the new logical disk geometry <= (x,255,63). Not NULL.
     * @param   uPercentStart   Starting value for progress percentage.
     * @param   uPercentSpan    Span for varying progress percentage.
     * @param   pVDIfsDisk      Pointer to the per-disk VD interface list.
     * @param   pVDIfsImage     Pointer to the per-image VD interface list.
     * @param   pVDIfsOperation Pointer to the per-operation VD interface list.
     */
    DECLR3CALLBACKMEMBER(int, pfnResize, (void *pBackendData,
                                          uint64_t cbSize,
                                          PCVDGEOMETRY pPCHSGeometry,
                                          PCVDGEOMETRY pLCHSGeometry,
                                          unsigned uPercentStart, unsigned uPercentSpan,
                                          PVDINTERFACE pVDIfsDisk,
                                          PVDINTERFACE pVDIfsImage,
                                          PVDINTERFACE pVDIfsOperation));

    /**
     * Try to repair the given image.
     *
     * @returns VBox status code.
     * @param   pszFilename     Name of the image file.
     * @param   pVDIfsDisk      Pointer to the per-disk VD interface list.
     * @param   pVDIfsImage     Pointer to the per-image VD interface list.
     * @param   fFlags          Combination of the VD_REPAIR_* flags.
     */
    DECLR3CALLBACKMEMBER(int, pfnRepair, (const char *pszFilename, PVDINTERFACE pVDIfsDisk,
                                          PVDINTERFACE pVDIfsImage, uint32_t fFlags));

    /**
     * Traverse all metadata of the opened image.
     *
     * @returns VBox status code.
     * @param   pBackendData    Opaque state data for this image.
     * @param   fFlags          Traverse flags, combination of VD_TRAVERSE_METDATA_* defines.
     * @param   pVDIfsDisk      Pointer to the per-disk VD interface list.
     * @param   pVDIfsImage     Pointer to the per-image VD interface list.
     * @param   pVDIfsOperation Pointer to the per-operation VD interface list.
     */
    DECLR3CALLBACKMEMBER(int, pfnTraverseMetadata, (void *pBackendData, uint32_t fFlags,
                                                    PVDINTERFACE pVDIfsDisk,
                                                    PVDINTERFACE pVDIfsImage,
                                                    PVDINTERFACE pVDIfsOperation));

} VBOXHDDBACKEND;

/** Pointer to VD backend. */
typedef VBOXHDDBACKEND *PVBOXHDDBACKEND;
/** Constant pointer to VD backend. */
typedef const VBOXHDDBACKEND *PCVBOXHDDBACKEND;

/** @copydoc VBOXHDDBACKEND::pfnComposeLocation */
DECLINLINE(int) genericFileComposeLocation(PVDINTERFACE pConfig, char **pszLocation)
{
    *pszLocation = NULL;
    return VINF_SUCCESS;
}
/** @copydoc VBOXHDDBACKEND::pfnComposeName */
DECLINLINE(int) genericFileComposeName(PVDINTERFACE pConfig, char **pszName)
{
    *pszName = NULL;
    return VINF_SUCCESS;
}

#endif
