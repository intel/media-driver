/**
 * \file xf86drm.c
 * User-level interface to DRM device
 *
 * \author Rickard E. (Rik) Faith <faith@valinux.com>
 * \author Kevin E. Martin <martin@valinux.com>
 */

/*
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#define stat_t struct stat
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdarg.h>
#ifdef __sun //#ifdef MAJOR_IN_MKDEV
# include <sys/mkdev.h> /* defines major(), minor(), and makedev() on Solaris */
#endif
#if defined(__GLIBC__) || defined(__linux__) //#ifdef MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#endif

/* Not all systems have MAP_FAILED defined */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#include "xf86drm.h"
//#include "xf86drmCSC.h"
#include "libdrm_macros.h"
#include "i915_drm.h"

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
#define DRM_MAJOR 145
#endif

#ifdef __NetBSD__
#define DRM_MAJOR 34
#endif

# ifdef __OpenBSD__
#  define DRM_MAJOR 81
# endif

#ifndef DRM_MAJOR
#define DRM_MAJOR 226        /* Linux */
#endif

/*
 * This definition needs to be changed on some systems if dev_t is a structure.
 * If there is a header file we can get it from, there would be best.
 */
#ifndef makedev
#define makedev(x,y)    ((dev_t)(((x) << 8) | (y)))
#endif

#define DRM_MSG_VERBOSITY 3

#define memclear(s) memset(&s, 0, sizeof(s))

static drmServerInfoPtr drm_server_info;

void drmSetServerInfo(drmServerInfoPtr info)
{
    drm_server_info = info;
}

/**
 * Output a message to stderr.
 *
 * \param format printf() like format string.
 *
 * \internal
 * This function is a wrapper around vfprintf().
 */

static int DRM_PRINTFLIKE(1, 0)
drmDebugPrint(const char *format, va_list ap)
{
    return vfprintf(stderr, format, ap);
}

void
drmMsg(const char *format, ...)
{
    va_list    ap;
    const char *env;
    if (((env = getenv("LIBGL_DEBUG")) && strstr(env, "verbose")) || drm_server_info)
    {
    va_start(ap, format);
    if (drm_server_info) {
      drm_server_info->debug_print(format,ap);
    } else {
      drmDebugPrint(format, ap);
    }
    va_end(ap);
    }
}

static void *drmHashTable = nullptr; /* Context switch callbacks */

void *drmGetHashTable(void)
{
    return drmHashTable;
}

void *drmMalloc(int size)
{
    void *pt;
    if ((pt = malloc(size)))
    memset(pt, 0, size);
    return pt;
}

void drmFree(void *pt)
{
    if (pt)
    free(pt);
}

/**
 * Call ioctl, restarting if it is interupted
 */

#if defined(__cplusplus)
extern "C" {
#endif
drm_export int
mosdrmIoctl(int fd, unsigned long request, void *arg)
{
    int    ret;

    do {
    ret = ioctl(fd, request, arg);
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));
    return ret;
}
drm_export int
drmIoctl(int fd, unsigned long request, void *arg)
{
    return mosdrmIoctl(fd,request,arg);
}
#if defined(__cplusplus)
}
#endif

static unsigned long drmGetKeyFromFd(int fd)
{
    stat_t     st;

    st.st_rdev = 0;
    int state = fstat(fd, &st);
    if (state == -1)
    {
        drmMsg("fstat execute failed!");
    }
    return st.st_rdev;
}

drmHashEntry *drmGetEntry(int fd)
{
    unsigned long key = drmGetKeyFromFd(fd);
    void          *value = nullptr;
    drmHashEntry  *entry;

    if (!drmHashTable)
    drmHashTable = drmHashCreate();

    if (drmHashTable && drmHashLookup(drmHashTable, key, &value)) {
    entry           = (drmHashEntry *)drmMalloc(sizeof(*entry));
    if (!entry)
        return nullptr;
    entry->fd       = fd;
    entry->f        = nullptr;
    entry->tagTable = drmHashCreate();
    if (entry->tagTable) {
        drmHashInsert(drmHashTable, key, entry);
    } else {
        drmFree(entry);
        entry = nullptr;
    }
    } else {
    entry = (drmHashEntry *)value;
    }
    return entry;
}
/**
 * Compare two busid strings
 *
 * \param first
 * \param second
 *
 * \return 1 if matched.
 *
 * \internal
 * This function compares two bus ID strings.  It understands the older
 * PCI:b:d:f format and the newer pci:oooo:bb:dd.f format.  In the format, o is
 * domain, b is bus, d is device, f is function.
 */
static int drmMatchBusID(const char *id1, const char *id2, int pci_domain_ok)
{
    /* First, check if the IDs are exactly the same */
    if (strcasecmp(id1, id2) == 0)
    return 1;

    /* Try to match old/new-style PCI bus IDs. */
    if (strncasecmp(id1, "pci", 3) == 0) {
    unsigned int o1, b1, d1, f1;
    unsigned int o2, b2, d2, f2;
    int ret;

    ret = sscanf(id1, "pci:%04x:%02x:%02x.%u", &o1, &b1, &d1, &f1);
    if (ret != 4) {
        o1 = 0;
        ret = sscanf(id1, "PCI:%u:%u:%u", &b1, &d1, &f1);
        if (ret != 3)
        return 0;
    }

    ret = sscanf(id2, "pci:%04x:%02x:%02x.%u", &o2, &b2, &d2, &f2);
    if (ret != 4) {
        o2 = 0;
        ret = sscanf(id2, "PCI:%u:%u:%u", &b2, &d2, &f2);
        if (ret != 3)
        return 0;
    }

    /* If domains aren't properly supported by the kernel interface,
     * just ignore them, which sucks less than picking a totally random
     * card with "open by name"
     */
    if (!pci_domain_ok)
        o1 = o2 = 0;

    if ((o1 != o2) || (b1 != b2) || (d1 != d2) || (f1 != f2))
        return 0;
    else
        return 1;
    }
    return 0;
}

/**
 * Handles error checking for chown call.
 *
 * \param path to file.
 * \param id of the new owner.
 * \param id of the new group.
 *
 * \return zero if success or -1 if failure.
 *
 * \internal
 * Checks for failure. If failure was caused by signal call chown again.
 * If any other failure happened then it will output error mesage using
 * drmMsg() call.
 */
#if !defined(UDEV)
static int chown_check_return(const char *path, uid_t owner, gid_t group)
{
    int rv;

    do {
        rv = chown(path, owner, group);
    } while (rv != 0 && errno == EINTR);

    if (rv == 0)
        return 0;

    drmMsg("Failed to change owner or group for file %s! %d: %s\n",
            path, errno, strerror(errno));
    return -1;
}
#endif

/**
 * Open the DRM device, creating it if necessary.
 *
 * \param dev major and minor numbers of the device.
 * \param minor minor number of the device.
 *
 * \return a file descriptor on success, or a negative value on error.
 *
 * \internal
 * Assembles the device name from \p minor and opens it, creating the device
 * special file node with the major and minor numbers specified by \p dev and
 * parent directory if necessary and was called by root.
 */
static int drmOpenDevice(dev_t dev, int minor, int type)
{
    stat_t          st;
    const char      *dev_name;
    char            buf[64];
    int             fd;
    mode_t          devmode = DRM_DEV_MODE, serv_mode;
    gid_t           serv_group;
#if !defined(UDEV)
    int             isroot  = !geteuid();
    uid_t           user    = DRM_DEV_UID;
    gid_t           group   = DRM_DEV_GID;
#endif

    switch (type) {
    case DRM_NODE_PRIMARY:
        dev_name = DRM_DEV_NAME;
        break;
    case DRM_NODE_CONTROL:
        dev_name = DRM_CONTROL_DEV_NAME;
        break;
    case DRM_NODE_RENDER:
        dev_name = DRM_RENDER_DEV_NAME;
        break;
    default:
        return -EINVAL;
    };

    snprintf(buf, sizeof(buf), dev_name, DRM_DIR_NAME, minor);
    drmMsg("drmOpenDevice: node name is %s\n", buf);

    if (drm_server_info) {
    drm_server_info->get_perms(&serv_group, &serv_mode);
    devmode  = serv_mode ? serv_mode : DRM_DEV_MODE;
    devmode &= ~(S_IXUSR|S_IXGRP|S_IXOTH);
    }

#if !defined(UDEV)
    if (stat(DRM_DIR_NAME, &st)) {
    if (!isroot)
        return DRM_ERR_NOT_ROOT;
    mkdir(DRM_DIR_NAME, DRM_DEV_DIRMODE);
    chown_check_return(DRM_DIR_NAME, 0, 0); /* root:root */
    chmod(DRM_DIR_NAME, DRM_DEV_DIRMODE);
    }

    /* Check if the device node exists and create it if necessary. */
    if (stat(buf, &st)) {
    if (!isroot)
        return DRM_ERR_NOT_ROOT;
    remove(buf);
    mknod(buf, S_IFCHR | devmode, dev);
    }

    if (drm_server_info) {
    group = serv_group; /*(serv_group >= 0) ? serv_group : DRM_DEV_GID;*/
    chown_check_return(buf, user, group);
    chmod(buf, devmode);
    }
#else
    /* if we modprobed then wait for udev */
    {
    int udev_count = 0;
wait_for_udev:
        if (stat(DRM_DIR_NAME, &st)) {
        usleep(20);
        udev_count++;

        if (udev_count == 50)
            return -1;
        goto wait_for_udev;
    }

        if (stat(buf, &st)) {
        usleep(20);
        udev_count++;

        if (udev_count == 50)
            return -1;
        goto wait_for_udev;
        }
    }
#endif

    fd = open(buf, O_RDWR, 0);
    drmMsg("drmOpenDevice: open result is %d, (%s)\n",
        fd, fd < 0 ? strerror(errno) : "OK");
    if (fd >= 0)
    return fd;

#if !defined(UDEV)
    /* Check if the device node is not what we expect it to be, and recreate it
     * and try again if so.
     */
    if (st.st_rdev != dev) {
    if (!isroot)
        return DRM_ERR_NOT_ROOT;
    remove(buf);
    mknod(buf, S_IFCHR | devmode, dev);
    if (drm_server_info) {
        chown_check_return(buf, user, group);
        chmod(buf, devmode);
    }
    }
    fd = open(buf, O_RDWR, 0);
    drmMsg("drmOpenDevice: open result is %d, (%s)\n",
        fd, fd < 0 ? strerror(errno) : "OK");
    if (fd >= 0)
    return fd;

    drmMsg("drmOpenDevice: Open failed\n");
    remove(buf);
#endif
    return -errno;
}

/**
 * Open the DRM device
 *
 * \param minor device minor number.
 * \param create allow to create the device if set.
 *
 * \return a file descriptor on success, or a negative value on error.
 *
 * \internal
 * Calls drmOpenDevice() if \p create is set, otherwise assembles the device
 * name from \p minor and opens it.
 */
static int drmOpenMinor(int minor, int create, int type)
{
    int  fd;
    char buf[64];
    const char *dev_name;

    if (create)
    return drmOpenDevice(makedev(DRM_MAJOR, minor), minor, type);

    switch (type) {
    case DRM_NODE_PRIMARY:
        dev_name = DRM_DEV_NAME;
        break;
    case DRM_NODE_CONTROL:
        dev_name = DRM_CONTROL_DEV_NAME;
        break;
    case DRM_NODE_RENDER:
        dev_name = DRM_RENDER_DEV_NAME;
        break;
    default:
        return -EINVAL;
    };

    sprintf(buf, dev_name, DRM_DIR_NAME, minor);
    if ((fd = open(buf, O_RDWR, 0)) >= 0)
    return fd;
    return -errno;
}

/**
 * Determine whether the DRM kernel driver has been loaded.
 *
 * \return 1 if the DRM driver is loaded, 0 otherwise.
 *
 * \internal
 * Determine the presence of the kernel driver by attempting to open the 0
 * minor and get version information.  For backward compatibility with older
 * Linux implementations, /proc/dri is also checked.
 */
int drmAvailable(void)
{
    drmVersionPtr version;
    int           retval = 0;
    int           fd;

    if ((fd = drmOpenMinor(0, 1, DRM_NODE_PRIMARY)) < 0) {
#ifdef __linux__
    /* Try proc for backward Linux compatibility */
    if (!access("/proc/dri/0", R_OK))
        return 1;
#endif
    return 0;
    }

    if ((version = drmGetVersion(fd))) {
    retval = 1;
    drmFreeVersion(version);
    }
    close(fd);

    return retval;
}

static int drmGetMinorBase(int type)
{
    switch (type) {
    case DRM_NODE_PRIMARY:
        return 0;
    case DRM_NODE_CONTROL:
        return 64;
    case DRM_NODE_RENDER:
        return 128;
    default:
        return -1;
    };
}

static int drmGetMinorType(int minor)
{
    int type = minor >> 6;

    if (minor < 0)
        return -1;

    switch (type) {
    case DRM_NODE_PRIMARY:
    case DRM_NODE_CONTROL:
    case DRM_NODE_RENDER:
        return type;
    default:
        return -1;
    }
}

static const char *drmGetMinorName(int type)
{
    switch (type) {
    case DRM_NODE_PRIMARY:
        return "card";
    case DRM_NODE_CONTROL:
        return "controlD";
    case DRM_NODE_RENDER:
        return "renderD";
    default:
        return nullptr;
    }
}

/**
 * Open the device by bus ID.
 *
 * \param busid bus ID.
 * \param type device node type.
 *
 * \return a file descriptor on success, or a negative value on error.
 *
 * \internal
 * This function attempts to open every possible minor (up to DRM_MAX_MINOR),
 * comparing the device bus ID with the one supplied.
 *
 * \sa drmOpenMinor() and drmGetBusid().
 */
static int drmOpenByBusid(const char *busid, int type)
{
    int        i, pci_domain_ok = 1;
    int        fd;
    const char *buf;
    drmSetVersion sv;
    int        base = drmGetMinorBase(type);

    if (base < 0)
        return -1;

    drmMsg("drmOpenByBusid: Searching for BusID %s\n", busid);
    for (i = base; i < base + DRM_MAX_MINOR; i++) {
        fd = drmOpenMinor(i, 1, type);
        drmMsg("drmOpenByBusid: drmOpenMinor returns %d\n", fd);
        if (fd >= 0) {
            /* We need to try for 1.4 first for proper PCI domain support
             * and if that fails, we know the kernel is busted
             */
            sv.drm_di_major = 1;
            sv.drm_di_minor = 4;
            sv.drm_dd_major = -1;    /* Don't care */
            sv.drm_dd_minor = -1;    /* Don't care */
            if (drmSetInterfaceVersion(fd, &sv)) {
#ifndef __alpha__
                pci_domain_ok = 0;
#endif
                sv.drm_di_major = 1;
                sv.drm_di_minor = 1;
                sv.drm_dd_major = -1;       /* Don't care */
                sv.drm_dd_minor = -1;       /* Don't care */
                drmMsg("drmOpenByBusid: Interface 1.4 failed, trying 1.1\n");
                drmSetInterfaceVersion(fd, &sv);
            }
            buf = drmGetBusid(fd);
            if (buf) {
                drmMsg("drmOpenByBusid: drmGetBusid reports %s\n", buf);
                if (drmMatchBusID(buf, busid, pci_domain_ok)) {
                    drmFreeBusid(buf);
                    return fd;
                }
                drmFreeBusid(buf);
            }
            close(fd);
        }
    }
    return -1;
}

/**
 * Open the device by name.
 *
 * \param name driver name.
 * \param type the device node type.
 *
 * \return a file descriptor on success, or a negative value on error.
 *
 * \internal
 * This function opens the first minor number that matches the driver name and
 * isn't already in use.  If it's in use it then it will already have a bus ID
 * assigned.
 *
 * \sa drmOpenMinor(), drmGetVersion() and drmGetBusid().
 */
static int drmOpenByName(const char *name, int type)
{
    int           i;
    int           fd;
    drmVersionPtr version;
    char *        id;
    int           base = drmGetMinorBase(type);

    if (base < 0)
        return -1;

    /*
     * Open the first minor number that matches the driver name and isn't
     * already in use.  If it's in use it will have a busid assigned already.
     */
    for (i = base; i < base + DRM_MAX_MINOR; i++) {
    if ((fd = drmOpenMinor(i, 1, type)) >= 0) {
        if ((version = drmGetVersion(fd))) {
        if (!strcmp(version->name, name)) {
            drmFreeVersion(version);
            id = drmGetBusid(fd);
            drmMsg("drmGetBusid returned '%s'\n", id ? id : "NULL");
            if (!id || !*id) {
            if (id)
                drmFreeBusid(id);
            return fd;
            } else {
            drmFreeBusid(id);
            }
        } else {
            drmFreeVersion(version);
        }
        }
        close(fd);
    }
    }

#ifdef __linux__
    /* Backward-compatibility /proc support */
    for (i = 0; i < 8; i++) {
    char proc_name[64], buf[512];
    char *driver, *pt, *devstring;
    int  retcode;

    sprintf(proc_name, "/proc/dri/%d/name", i);
    if ((fd = open(proc_name, 0, 0)) >= 0) {
        retcode = read(fd, buf, sizeof(buf)-1);
        close(fd);
        if (retcode > 0) {
        buf[retcode-1] = '\0';
        for (driver = pt = buf; *pt && *pt != ' '; ++pt)
            ;
        if (*pt) { /* Device is next */
            *pt = '\0';
            if (!strcmp(driver, name)) { /* Match */
            for (devstring = ++pt; *pt && *pt != ' '; ++pt)
                ;
            if (*pt) { /* Found busid */
                return drmOpenByBusid(++pt, type);
            } else { /* No busid */
                return drmOpenDevice(strtol(devstring, nullptr, 0),i, type);
            }
            }
        }
        }
    }
    }
#endif

    return -1;
}

/**
 * Open the DRM device.
 *
 * Looks up the specified name and bus ID, and opens the device found.  The
 * entry in /dev/dri is created if necessary and if called by root.
 *
 * \param name driver name. Not referenced if bus ID is supplied.
 * \param busid bus ID. Zero if not known.
 *
 * \return a file descriptor on success, or a negative value on error.
 *
 * \internal
 * It calls drmOpenByBusid() if \p busid is specified or drmOpenByName()
 * otherwise.
 */
int drmOpen(const char *name, const char *busid)
{
    return drmOpenWithType(name, busid, DRM_NODE_PRIMARY);
}

/**
 * Open the DRM device with specified type.
 *
 * Looks up the specified name and bus ID, and opens the device found.  The
 * entry in /dev/dri is created if necessary and if called by root.
 *
 * \param name driver name. Not referenced if bus ID is supplied.
 * \param busid bus ID. Zero if not known.
 * \param type the device node type to open, PRIMARY, CONTROL or RENDER
 *
 * \return a file descriptor on success, or a negative value on error.
 *
 * \internal
 * It calls drmOpenByBusid() if \p busid is specified or drmOpenByName()
 * otherwise.
 */
int drmOpenWithType(const char *name, const char *busid, int type)
{
    if (!drmAvailable() && name != nullptr && drm_server_info) {
    /* try to load the kernel module */
    if (!drm_server_info->load_module(name)) {
        drmMsg("[drm] failed to load kernel module \"%s\"\n", name);
        return -1;
    }
    }

    if (busid) {
    int fd = drmOpenByBusid(busid, type);
    if (fd >= 0)
        return fd;
    }

    if (name)
    return drmOpenByName(name, type);

    return -1;
}

int drmOpenControl(int minor)
{
    return drmOpenMinor(minor, 0, DRM_NODE_CONTROL);
}

int drmOpenRender(int minor)
{
    return drmOpenMinor(minor, 0, DRM_NODE_RENDER);
}

/**
 * Free the version information returned by drmGetVersion().
 *
 * \param v pointer to the version information.
 *
 * \internal
 * It frees the memory pointed by \p %v as well as all the non-null strings
 * pointers in it.
 */
void drmFreeVersion(drmVersionPtr v)
{
    if (!v)
    return;
    drmFree(v->name);
    drmFree(v->date);
    drmFree(v->desc);
    drmFree(v);
}

/**
 * Free the non-public version information returned by the kernel.
 *
 * \param v pointer to the version information.
 *
 * \internal
 * Used by drmGetVersion() to free the memory pointed by \p %v as well as all
 * the non-null strings pointers in it.
 */
static void drmFreeKernelVersion(drm_version_t *v)
{
    if (!v)
    return;
    drmFree(v->name);
    drmFree(v->date);
    drmFree(v->desc);
    drmFree(v);
}

/**
 * Copy version information.
 *
 * \param d destination pointer.
 * \param s source pointer.
 *
 * \internal
 * Used by drmGetVersion() to translate the information returned by the ioctl
 * interface in a private structure into the public structure counterpart.
 */
static void drmCopyVersion(drmVersionPtr d, const drm_version_t *s)
{
    d->version_major      = s->version_major;
    d->version_minor      = s->version_minor;
    d->version_patchlevel = s->version_patchlevel;
    d->name_len           = s->name_len;
    d->name               = strdup(s->name);
    d->date_len           = s->date_len;
    d->date               = strdup(s->date);
    d->desc_len           = s->desc_len;
    d->desc               = strdup(s->desc);
}

/**
 * Query the driver version information.
 *
 * \param fd file descriptor.
 *
 * \return pointer to a drmVersion structure which should be freed with
 * drmFreeVersion().
 *
 * \note Similar information is available via /proc/dri.
 *
 * \internal
 * It gets the version information via successive DRM_IOCTL_VERSION ioctls,
 * first with zeros to get the string lengths, and then the actually strings.
 * It also null-terminates them since they might not be already.
 */
drmVersionPtr drmGetVersion(int fd)
{
    drmVersionPtr retval;
    drm_version_t *version = (drm_version_t *)drmMalloc(sizeof(*version));
    if (!version)
    return nullptr;

    memclear(*version);

    if (drmIoctl(fd, DRM_IOCTL_VERSION, version)) {
    drmFreeKernelVersion(version);
    return nullptr;
    }

    if (version->name_len)
    version->name    = (char *)drmMalloc(version->name_len + 1);
    if (version->date_len)
    version->date    = (char *)drmMalloc(version->date_len + 1);
    if (version->desc_len)
    version->desc    = (char *)drmMalloc(version->desc_len + 1);

    if (drmIoctl(fd, DRM_IOCTL_VERSION, version)) {
    drmMsg("DRM_IOCTL_VERSION: %s\n", strerror(errno));
    drmFreeKernelVersion(version);
    return nullptr;
    }

    /* The results might not be null-terminated strings, so terminate them. */
    if (version->name_len) version->name[version->name_len] = '\0';
    if (version->date_len) version->date[version->date_len] = '\0';
    if (version->desc_len) version->desc[version->desc_len] = '\0';

    retval = (drmVersionPtr)drmMalloc(sizeof(*retval));
    if (retval)
        drmCopyVersion(retval, version);

    drmFreeKernelVersion(version);
    return retval;
}

/**
 * Free the bus ID information.
 *
 * \param busid bus ID information string as given by drmGetBusid().
 *
 * \internal
 * This function is just frees the memory pointed by \p busid.
 */
void drmFreeBusid(const char *busid)
{
    drmFree((void *)busid);
}

/**
 * Get the bus ID of the device.
 *
 * \param fd file descriptor.
 *
 * \return bus ID string.
 *
 * \internal
 * This function gets the bus ID via successive DRM_IOCTL_GET_UNIQUE ioctls to
 * get the string length and data, passing the arguments in a drm_unique
 * structure.
 */
char *drmGetBusid(int fd)
{
    drm_unique_t u;

    memclear(u);

    if (drmIoctl(fd, DRM_IOCTL_GET_UNIQUE, &u))
    return nullptr;
    u.unique = (char *)drmMalloc(u.unique_len + 1);
    if (drmIoctl(fd, DRM_IOCTL_GET_UNIQUE, &u))
    return nullptr;
    u.unique[u.unique_len] = '\0';

    return u.unique;
}

/**
 * Close the device.
 *
 * \param fd file descriptor.
 *
 * \internal
 * This function closes the file descriptor.
 */
int drmClose(int fd)
{
    unsigned long key    = drmGetKeyFromFd(fd);
    drmHashEntry  *entry = drmGetEntry(fd);
    if(!entry)
    return -ENOMEM;

    drmHashDestroy(entry->tagTable);
    entry->fd       = 0;
    entry->f        = nullptr;
    entry->tagTable = nullptr;

    drmHashDelete(drmHashTable, key);
    drmFree(entry);

    return close(fd);
}

/**
 * Issue a set-version ioctl.
 *
 * \param fd file descriptor.
 * \param drmCommandIndex command index
 * \param data source pointer of the data to be read and written.
 * \param size size of the data to be read and written.
 *
 * \return zero on success, or a negative value on failure.
 *
 * \internal
 * It issues a read-write ioctl given by
 * \code DRM_COMMAND_BASE + drmCommandIndex \endcode.
 */
int drmSetInterfaceVersion(int fd, drmSetVersion *version)
{
    int retcode = 0;
    drm_set_version_t sv;

    memclear(sv);
    sv.drm_di_major = version->drm_di_major;
    sv.drm_di_minor = version->drm_di_minor;
    sv.drm_dd_major = version->drm_dd_major;
    sv.drm_dd_minor = version->drm_dd_minor;

    if (drmIoctl(fd, DRM_IOCTL_SET_VERSION, &sv)) {
    retcode = -errno;
    }

    version->drm_di_major = sv.drm_di_major;
    version->drm_di_minor = sv.drm_di_minor;
    version->drm_dd_major = sv.drm_dd_major;
    version->drm_dd_minor = sv.drm_dd_minor;

    return retcode;
}

#define DRM_MAX_FDS 16
static struct {
    char *BusID;
    int fd;
    int refcount;
    int type;
} connection[DRM_MAX_FDS];

static int nr_fds = 0;

int drmPrimeHandleToFD(int fd, uint32_t handle, uint32_t flags, int *prime_fd)
{
    struct drm_prime_handle args;
    int ret;

    args.handle = handle;
    args.flags = flags;
    ret = drmIoctl(fd, DRM_IOCTL_PRIME_HANDLE_TO_FD, &args);
    if (ret)
        return ret;

    *prime_fd = args.fd;
    return 0;
}

int drmPrimeFDToHandle(int fd, int prime_fd, uint32_t *handle)
{
    struct drm_prime_handle args;
    int ret;

    args.fd = prime_fd;
    args.flags = 0;
    ret = drmIoctl(fd, DRM_IOCTL_PRIME_FD_TO_HANDLE, &args);
    if (ret)
        return ret;

    *handle = args.handle;
    return 0;
}
