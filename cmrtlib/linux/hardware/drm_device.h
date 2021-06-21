/*
* Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
* Copyright 2000 VA Linux Systems, Inc., Sunnyvale, California.
* Copyright(c) 2019, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files(the "Software"),
*to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
*and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions :
*
* The above copyright notice and this permission notice(including the next
* paragraph) shall be included in all copies or substantial portions of the
* Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL
* PRECISION INSIGHT AND / OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
*ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*/

#ifndef DRM_DEVICE_H_
#define DRM_DEVICE_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <dirent.h>
#include <stddef.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#define stat_t struct stat
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdarg.h>
#ifdef __sun //#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#if defined(__GLIBC__) || defined(__linux__) //#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#include <math.h>
#include <string>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* Not all systems have MAP_FAILED defined */
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

#define DRM_DEV_UID  0
#define DRM_DEV_GID  0
/* Default /dev/dri directory permissions 0755 */
#define DRM_DEV_DIRMODE  (S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH)
#define DRM_DEV_MODE     (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)
#define DRM_DEVICE_GET_PCI_REVISION (1 << 0)

#ifdef __OpenBSD__
#define DRM_DIR_NAME  "/dev"
#define DRM_DEV_NAME  "%s/drm%d"
#define DRM_CONTROL_DEV_NAME  "%s/drmC%d"
#define DRM_RENDER_DEV_NAME  "%s/drmR%d"
#else
#define DRM_DIR_NAME  "/dev/dri"
#define DRM_DEV_NAME  "%s/card%d"
#define DRM_CONTROL_DEV_NAME  "%s/controlD%d"
#define DRM_RENDER_DEV_NAME  "%s/renderD%d"
#define DRM_PROC_NAME "/proc/dri/" /* For backward Linux compatibility */
#endif

#ifdef __OpenBSD__
#define DRM_PRIMARY_MINOR_NAME  "drm"
#define DRM_CONTROL_MINOR_NAME  "drmC"
#define DRM_RENDER_MINOR_NAME   "drmR"
#else
#define DRM_PRIMARY_MINOR_NAME  "card"
#define DRM_CONTROL_MINOR_NAME  "controlD"
#define DRM_RENDER_MINOR_NAME   "renderD"
#endif

#define DRM_ERR_NO_DEVICE  (-1001)
#define DRM_ERR_NO_ACCESS  (-1002)
#define DRM_ERR_NOT_ROOT   (-1003)
#define DRM_ERR_INVALID    (-1004)
#define DRM_ERR_NO_FD      (-1005)

#define DRM_AGP_NO_HANDLE 0

#define DRM_BUS_PCI       0
#define DRM_BUS_USB       1
#define DRM_BUS_PLATFORM  2
#define DRM_BUS_HOST1X    3
#define DRM_BUS_VIRTIO 0x10

#define DRM_NODE_PRIMARY 0
#define DRM_NODE_CONTROL 1
#define DRM_NODE_RENDER  2
#define DRM_NODE_MAX     3

typedef unsigned int  drmSize, *drmSizePtr;         /**< For mapped regions */
typedef void          *drmAddress, **drmAddressPtr; /**< For mapped regions */

#if (__GNUC__ >= 3)
#define DRM_PRINTFLIKE(f, a) __attribute__ ((format(__printf__, f, a)))
#else
#define DRM_PRINTFLIKE(f, a)
#endif

#define MIN2( A, B )   ( (A)<(B) ? (A) : (B) )
#define MAX2( A, B )   ( (A)>(B) ? (A) : (B) )
#define MAX3( A, B, C ) ((A) > (B) ? MAX2(A, C) : MAX2(B, C))

#define __align_mask(value, mask)  (((value) + (mask)) & ~(mask))
#define ALIGN(value, alignment)    __align_mask(value, (__typeof__(value))((alignment) - 1))
#define DRM_PLATFORM_DEVICE_NAME_LEN 512

typedef struct _drmPciBusInfo {
    uint16_t domain;
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
} drmPciBusInfo, *drmPciBusInfoPtr;

typedef struct _drmPciDeviceInfo {
    uint32_t vendor_id;
    uint32_t device_id;
    uint32_t subvendor_id;
    uint32_t subdevice_id;
    uint32_t revision_id;
    char     driverInfo[1024];
    uint64_t videoMem[4] = {};
    uint64_t systemMem[4] = {};
    uint64_t sharedMem[4] = {};
    uint64_t ioportMem[4] = {};
    uint64_t ioMem[4] = {};
} drmPciDeviceInfo, *drmPciDeviceInfoPtr;


typedef struct _drmUsbBusInfo {
    uint8_t bus;
    uint8_t dev;
} drmUsbBusInfo, *drmUsbBusInfoPtr;

typedef struct _drmUsbDeviceInfo {
    uint16_t vendor;
    uint16_t product;
} drmUsbDeviceInfo, *drmUsbDeviceInfoPtr;



typedef struct _drmPlatformBusInfo {
    char fullname[DRM_PLATFORM_DEVICE_NAME_LEN + 1];
} drmPlatformBusInfo, *drmPlatformBusInfoPtr;

typedef struct _drmPlatformDeviceInfo {
    char **compatible; /* NULL terminated list of compatible strings */
} drmPlatformDeviceInfo, *drmPlatformDeviceInfoPtr;

#define DRM_HOST1X_DEVICE_NAME_LEN 512

typedef struct _drmHost1xBusInfo {
    char fullname[DRM_HOST1X_DEVICE_NAME_LEN + 1];
} drmHost1xBusInfo, *drmHost1xBusInfoPtr;

typedef struct _drmHost1xDeviceInfo {
    char **compatible; /* NULL terminated list of compatible strings */
} drmHost1xDeviceInfo, *drmHost1xDeviceInfoPtr;

typedef struct _drmDevice {
    char **nodes; /* DRM_NODE_MAX sized array */
    int available_nodes; /* DRM_NODE_* bitmask */
    int bustype;
    union {
        drmPciBusInfoPtr pci;
        drmUsbBusInfoPtr usb;
        drmPlatformBusInfoPtr platform;
        drmHost1xBusInfoPtr host1x;
    } businfo;
    union {
        drmPciDeviceInfoPtr pci;
        drmUsbDeviceInfoPtr usb;
        drmPlatformDeviceInfoPtr platform;
        drmHost1xDeviceInfoPtr host1x;
    } deviceinfo;

    unsigned int MaxThread = 0;
    unsigned int EuNumber = 0;
    unsigned int TileNumber = 0;
    unsigned int reserved[16];
} drmDevice, *drmDevicePtr;

/*
 * The kernel drm core has a number of places that assume maximum of
 * 3x64 devices nodes. That's 64 for each of primary, control and
 * render nodes. Rounded it up to 256 for simplicity.
 */
#define MAX_DRM_NODES 256

inline int memcpy_s(void *dst, size_t numberOfElements, const void *src, size_t count)
{
    if ((dst == nullptr) || (src == nullptr))
    {
        return EINVAL;
    }
    if (numberOfElements < count)
    {
        return ERANGE;
    }
    std::memcpy(dst, src, count);
    return 0;
}

 /* Check that the given flags are valid returning 0 on success */
static int
drm_device_validate_flags(uint32_t flags)
{
    return (flags & ~DRM_DEVICE_GET_PCI_REVISION);
}

static int drmGetMaxNodeName(void)
{
    return sizeof(DRM_DIR_NAME) +
        MAX3(sizeof(DRM_PRIMARY_MINOR_NAME),
            sizeof(DRM_CONTROL_MINOR_NAME),
            sizeof(DRM_RENDER_MINOR_NAME)) +
        3 /* length of the node number */;
}

static bool drmNodeIsDRM(int maj, int min)
{
#ifdef __linux__
    char path[64];
    struct stat sbuf;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device/drm",
        maj, min);
    return stat(path, &sbuf) == 0;
#else
    return maj == DRM_MAJOR;
#endif
}

static int drmDevicesEqual(drmDevicePtr a, drmDevicePtr b)
{
    if (a == NULL || b == NULL)
        return 0;

    if (a->bustype != b->bustype)
        return 0;

    switch (a->bustype) {
    case DRM_BUS_PCI:
        return memcmp(a->businfo.pci, b->businfo.pci, sizeof(drmPciBusInfo)) == 0;

    case DRM_BUS_USB:
        return memcmp(a->businfo.usb, b->businfo.usb, sizeof(drmUsbBusInfo)) == 0;

    case DRM_BUS_PLATFORM:
        return memcmp(a->businfo.platform, b->businfo.platform, sizeof(drmPlatformBusInfo)) == 0;

    case DRM_BUS_HOST1X:
        return memcmp(a->businfo.host1x, b->businfo.host1x, sizeof(drmHost1xBusInfo)) == 0;

    default:
        break;
    }
    return 0;
}

static drmDevicePtr drmDeviceAlloc(unsigned int type, const char *node,
    size_t bus_size, size_t device_size,
    char **ptrp)
{
    size_t max_node_length, extra, size;
    drmDevicePtr device;
    unsigned int i;
    char *ptr;

    max_node_length = ALIGN(drmGetMaxNodeName(), sizeof(void *));

    extra = DRM_NODE_MAX * (sizeof(void *) + max_node_length);

     size = sizeof(*device) + extra + bus_size + device_size;

    device = (drmDevicePtr)calloc(1, size);
    if (!device)
        return NULL;

    device->available_nodes = 1 << type;

    ptr = (char *)device + sizeof(*device);
    device->nodes = (char **)ptr;

    ptr += DRM_NODE_MAX * sizeof(void *);

    for (i = 0; i < DRM_NODE_MAX; i++) {
        device->nodes[i] = ptr;
        ptr += max_node_length;
    }

    memcpy(device->nodes[type], node, max_node_length);
    *ptrp = ptr;

    return device;
}

static char * DRM_PRINTFLIKE(2, 3)
sysfs_uevent_get(const char *path, const char *fmt, ...)
{
    char filename[PATH_MAX + 1], *key, *line = NULL, *value = NULL;
    size_t size = 0, len;
    ssize_t num;
    va_list ap;
    FILE *fp;

    va_start(ap, fmt);
    num = vasprintf(&key, fmt, ap);
    va_end(ap);
    len = num;

    snprintf(filename, sizeof(filename), "%s/uevent", path);

    fp = fopen(filename, "r");
    if (!fp) {
        free(key);
        return NULL;
    }

    while ((num = getline(&line, &size, fp)) >= 0) {
        if ((strncmp(line, key, len) == 0) && (line[len] == '=')) {
            char *start = line + len + 1, *end = line + num - 1;

            if (*end != '\n')
                end++;

            value = strndup(start, end - start);
            break;
        }
    }

    free(line);
    fclose(fp);

    free(key);

    return value;
}

static int drmParseHost1xDeviceInfo(int maj,
                                    int min,
                                    drmHost1xDeviceInfoPtr info)
{
    char path[PATH_MAX + 1];
    int err = 0;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);

    char *value = sysfs_uevent_get(path, "OF_COMPATIBLE_N");
    if (!value)
    {
        return -ENOENT;
    }
    unsigned int count = 0;
    int scanned_value_count = sscanf(value, "%u", &count);
    free(value);
    if (scanned_value_count <= 0 || 0 == count)
    {
        return -ENOENT;
    }

    info->compatible = (char**)calloc(count + 1, sizeof(*info->compatible));
    if (!info->compatible)
    {
        return -ENOMEM;
    }

    unsigned int i = 0;
    for (; i < count; i++)
    {
        value = sysfs_uevent_get(path, "OF_COMPATIBLE_%u", i);
        if (!value)
        {
            err = -ENOENT;
            goto free;
        }
        info->compatible[i] = value;
    }
    return 0;

free:
    while (i--)
    {
        free(info->compatible[i]);
    }
    free(info->compatible);
    return err;
}

static int drmParseHost1xBusInfo(int maj, int min, drmHost1xBusInfoPtr info)
{
    char path[PATH_MAX + 1], *name;
    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);
    name = sysfs_uevent_get(path, "OF_FULLNAME");
    if (!name)
        return -ENOENT;
    strncpy(info->fullname, name, DRM_HOST1X_DEVICE_NAME_LEN);
    info->fullname[DRM_HOST1X_DEVICE_NAME_LEN - 1] = '\0';
    free(name);
    return 0;
}

static int drmProcessHost1xDevice(drmDevicePtr *device,
    const char *node, int node_type,
    int maj, int min, bool fetch_deviceinfo,
    uint32_t flags)
{
    drmDevicePtr dev;
    char *ptr;
    int ret;

    dev = drmDeviceAlloc(node_type, node, sizeof(drmHost1xBusInfo),
        sizeof(drmHost1xDeviceInfo), &ptr);
    if (!dev)
        return -ENOMEM;

    dev->bustype = DRM_BUS_HOST1X;

    dev->businfo.host1x = (drmHost1xBusInfoPtr)ptr;

    ret = drmParseHost1xBusInfo(maj, min, dev->businfo.host1x);
    if (ret < 0)
        goto free_device;

    if (fetch_deviceinfo) {
        ptr += sizeof(drmHost1xBusInfo);
        dev->deviceinfo.host1x = (drmHost1xDeviceInfoPtr)ptr;

        ret = drmParseHost1xDeviceInfo(maj, min, dev->deviceinfo.host1x);
        if (ret < 0)
            goto free_device;
    }

    *device = dev;

    return 0;

free_device:
    free(dev);
    return ret;
}

static int drmParsePlatformBusInfo(int maj, int min, drmPlatformBusInfoPtr info)
{
    char path[PATH_MAX + 1], *name;
    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);
    name = sysfs_uevent_get(path, "OF_FULLNAME");
    if (!name)
        return -ENOENT;
    strncpy(info->fullname, name, DRM_PLATFORM_DEVICE_NAME_LEN);
    info->fullname[DRM_PLATFORM_DEVICE_NAME_LEN - 1] = '\0';
    free(name);
    return 0;
}

static int drmParsePlatformDeviceInfo(int maj, int min,
    drmPlatformDeviceInfoPtr info)
{
    char path[PATH_MAX + 1], *value;
    unsigned int count, i;
    int err;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);
    value = sysfs_uevent_get(path, "OF_COMPATIBLE_N");
    if (!value)
        return -ENOENT;
    sscanf(value, "%u", &count);

    free(value);
    if (count <= MAX_DRM_NODES)
    {
        info->compatible = (char**)calloc(count + 1, sizeof(*info->compatible));
    }else
        return -ENOENT;

    if (!info->compatible)
        return -ENOMEM;
    for (i = 0; i < count; i++) {
        value = sysfs_uevent_get(path, "OF_COMPATIBLE_%u", i);
        if (!value) {
            err = -ENOENT;
            goto free;
        }
        info->compatible[i] = value;
    }
    return 0;
free:
    while (i--)
        free(info->compatible[i]);
    free(info->compatible);
    return err;
}

static void
get_pci_path(int maj, int min, char *pci_path)
{
    char path[PATH_MAX + 1], *term;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);
    if (!realpath(path, pci_path)) {
        strcpy(pci_path, path);
        return;
    }

    term = strrchr(pci_path, '/');
    if (term && strncmp(term, "/virtio", 7) == 0)
        *term = 0;
}

static int parse_separate_sysfs_files(int maj, int min,
    drmPciDeviceInfoPtr device,
    bool ignore_revision)
{
    static const char *attrs[] = {
        "revision", /* Older kernels are missing the file, so check for it first */
        "vendor",
        "device",
        "subsystem_vendor",
        "subsystem_device",
    };
    char path[PATH_MAX + 128], pci_path[PATH_MAX];
    char resourcename[PATH_MAX + 64], driverpath[PATH_MAX + 64], drivername[PATH_MAX + 64], irqpath[PATH_MAX + 64];

    unsigned int data[ARRAY_SIZE(attrs)];
    FILE *fp;
    int ret;
    char *dev_path;
    char *driver_name;

    get_pci_path(maj, min, pci_path);
    dev_path = strrchr(pci_path, '/');
    dev_path++;

    snprintf(driverpath, sizeof(driverpath), "%s/driver", pci_path);
    snprintf(irqpath, sizeof(irqpath), "%s/irq", pci_path);
    int fd = open(irqpath, O_RDONLY);

    char buffer[512] ;
    memset(buffer, 0, sizeof(buffer));
    if (fd >= 0)
    {
        int count = 0;
        count = read(fd, buffer, sizeof(buffer));
        close(fd);
    }

    snprintf(resourcename, sizeof(resourcename), "%s/resource", pci_path);

    if (readlink(driverpath, drivername, PATH_MAX) < 0)
        printf("   readlink -errno %d\n", errno);

    driver_name = strrchr(drivername, '/');
    driver_name++;
    snprintf(device->driverInfo, sizeof(device->driverInfo), "Driver Module %s  Bus %s IRQ %s", driver_name, dev_path, buffer);

    FILE*resource = fopen(resourcename, "r");

    if (resource)
    {
        while (!feof(resource))
        {
            unsigned long long start, end, flags;

            start = end = flags = 0;

            if (fscanf(resource, "%llx %llx %llx", &start, &end, &flags) != 3)
                break;

            if (flags & 0x101)
            {
                device->ioportMem[0] = end - start + 1;
            }
            else
                if (flags & 0x100)
                {
                    device->ioMem[0] = end - start + 1;
                }
                else
                    if (flags & 0x200)
                    {
                        int i;
                        for ( i = 0; i < 4; i++)
                        {
                            if (device->videoMem[i] == 0)
                            {
                                device->videoMem[i] = end - start + 1;
                                break;
                            }
                        }

                    }
        }
        fclose(resource);
    }


    for (unsigned i = ignore_revision ? 1 : 0; i < ARRAY_SIZE(attrs); i++) {
        snprintf(path, PATH_MAX + 128, "%s/%s", pci_path, attrs[i]);

        fp = fopen(path, "r");
        if (!fp)
            return -errno;

        ret = fscanf(fp, "%x", &data[i]);
        fclose(fp);
        if (ret != 1)
            return -errno;
    }

    device->revision_id = ignore_revision ? 0xff : data[0] & 0xff;
    device->vendor_id = data[1] & 0xffff;
    device->device_id = data[2] & 0xffff;
    device->subvendor_id = data[3] & 0xffff;
    device->subdevice_id = data[4] & 0xffff;
    return 0;
}

static int parse_config_sysfs_file(int maj, int min,
    drmPciDeviceInfoPtr device)
{
    char path[PATH_MAX + 128], pci_path[PATH_MAX + 1];
    unsigned char config[64];
    int fd, ret;

    get_pci_path(maj, min, pci_path);

    snprintf(path, PATH_MAX + 128, "%s/config", pci_path);

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return -errno;

    ret = read(fd, config, sizeof(config));
    close(fd);
    if (ret < 0)
        return -errno;

    device->vendor_id = config[0] | (config[1] << 8);
    device->device_id = config[2] | (config[3] << 8);
    device->revision_id = config[8];
    device->subvendor_id = config[44] | (config[45] << 8);
    device->subdevice_id = config[46] | (config[47] << 8);

    return 0;
}

static int drmParsePciDeviceInfo(int maj, int min,
    drmPciDeviceInfoPtr device,
    uint32_t flags)
{
//#ifdef __linux__
    if (!(flags & DRM_DEVICE_GET_PCI_REVISION))
        return parse_separate_sysfs_files(maj, min, device, true);

    if (parse_separate_sysfs_files(maj, min, device, false))
        return parse_config_sysfs_file(maj, min, device);
    return 0;
/*
#elif defined(__OpenBSD__) || defined(__DragonFly__)
    struct drm_pciinfo pinfo;
    int fd, type;

    type = drmGetMinorType(min);
    if (type == -1)
        return -ENODEV;

    fd = drmOpenMinor(min, 0, type);
    if (fd < 0)
        return -errno;

    if (drmIoctl(fd, DRM_IOCTL_GET_PCIINFO, &pinfo)) {
        close(fd);
        return -errno;
    }
    close(fd);

    device->vendor_id = pinfo.vendor_id;
    device->device_id = pinfo.device_id;
    device->revision_id = pinfo.revision_id;
    device->subvendor_id = pinfo.subvendor_id;
    device->subdevice_id = pinfo.subdevice_id;

    return 0;
#else
    #warning "Missing implementation of drmParsePciDeviceInfo"
        return -EINVAL;
#endif
*/
}

static int drmParsePciBusInfo(int maj, int min, drmPciBusInfoPtr info)
{
    unsigned int domain, bus, dev, func;
    char pci_path[PATH_MAX + 1], *value;
    int num;
    get_pci_path(maj, min, pci_path);
    value = sysfs_uevent_get(pci_path, "PCI_SLOT_NAME");
    if (!value)
        return -ENOENT;

    num = sscanf(value, "%04x:%02x:%02x.%1u", &domain, &bus, &dev, &func);
    free(value);
    if (num != 4)
        return -EINVAL;
    info->domain = domain;
    info->bus = bus;
    info->dev = dev;
    info->func = func;
    return 0;
}

static int drmProcessPlatformDevice(drmDevicePtr *device,
    const char *node, int node_type,
    int maj, int min, bool fetch_deviceinfo,
    uint32_t flags)
{
    drmDevicePtr dev;
    char *ptr;
    int ret;

    dev = drmDeviceAlloc(node_type, node, sizeof(drmPlatformBusInfo),
        sizeof(drmPlatformDeviceInfo), &ptr);
    if (!dev)
        return -ENOMEM;

    dev->bustype = DRM_BUS_PLATFORM;

    dev->businfo.platform = (drmPlatformBusInfoPtr)ptr;

    ret = drmParsePlatformBusInfo(maj, min, dev->businfo.platform);
    if (ret < 0)
        goto free_device;

    if (fetch_deviceinfo) {
        ptr += sizeof(drmPlatformBusInfo);
        dev->deviceinfo.platform = (drmPlatformDeviceInfoPtr)ptr;

        ret = drmParsePlatformDeviceInfo(maj, min, dev->deviceinfo.platform);
        if (ret < 0)
            goto free_device;
    }

    *device = dev;

    return 0;

free_device:
    free(dev);
    return ret;
}

static int drmProcessPciDevice(drmDevicePtr *device,
    const char *node, int node_type,
    int maj, int min, bool fetch_deviceinfo,
    uint32_t flags)
{
    drmDevicePtr dev;
    char *addr;
    int ret;

    dev = drmDeviceAlloc(node_type, node, sizeof(drmPciBusInfo),
        sizeof(drmPciDeviceInfo), &addr);

    if (!dev)
        return -ENOMEM;

    dev->bustype = DRM_BUS_PCI;

    dev->businfo.pci = (drmPciBusInfoPtr)addr;

    ret = drmParsePciBusInfo(maj, min, dev->businfo.pci);
    if (ret)
        goto free_device;

    // Fetch the device info if the user has requested it
    if (fetch_deviceinfo) {
        addr += sizeof(drmPciBusInfo);
        dev->deviceinfo.pci = (drmPciDeviceInfoPtr)addr;
        ret = drmParsePciDeviceInfo(maj, min, dev->deviceinfo.pci, flags);

        if (ret)
            goto free_device;
    }

    *device = dev;

    return 0;

free_device:
    free(dev);
    return ret;
}

static int drmParseUsbBusInfo(int maj, int min, drmUsbBusInfoPtr info)
{
    char path[PATH_MAX + 1], *value;
    unsigned int bus, dev;
    int ret;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);
    value = sysfs_uevent_get(path, "BUSNUM");
    if (!value)
        return -ENOENT;

    ret = sscanf(value, "%03u", &bus);
    free(value);

    if (ret <= 0)
        return -errno;

    value = sysfs_uevent_get(path, "DEVNUM");
    if (!value)
        return -ENOENT;

    ret = sscanf(value, "%03u", &dev);
    free(value);

    if (ret <= 0)
        return -errno;

    info->bus = bus;
    info->dev = dev;
    return 0;
}

static int drmParseUsbDeviceInfo(int maj, int min, drmUsbDeviceInfoPtr info)
{
    char path[PATH_MAX + 1], *value;
    unsigned int vendor, product;
    int ret;

    snprintf(path, sizeof(path), "/sys/dev/char/%d:%d/device", maj, min);
    value = sysfs_uevent_get(path, "PRODUCT");
    if (!value)
        return -ENOENT;

    ret = sscanf(value, "%x/%x", &vendor, &product);
    free(value);

    if (ret <= 0)
        return -errno;

    info->vendor = vendor;
    info->product = product;
    return 0;
}

static int drmProcessUsbDevice(drmDevicePtr *device, const char *node,
    int node_type, int maj, int min,
    bool fetch_deviceinfo, uint32_t flags)
{
    drmDevicePtr dev;
    char *ptr;
    int ret;

    dev = drmDeviceAlloc(node_type, node, sizeof(drmUsbBusInfo),
        sizeof(drmUsbDeviceInfo), &ptr);
    if (!dev)
        return -ENOMEM;

    dev->bustype = DRM_BUS_USB;

    dev->businfo.usb = (drmUsbBusInfoPtr)ptr;

    ret = drmParseUsbBusInfo(maj, min, dev->businfo.usb);
    if (ret < 0)
        goto free_device;

    if (fetch_deviceinfo) {
        ptr += sizeof(drmUsbBusInfo);
        dev->deviceinfo.usb = (drmUsbDeviceInfoPtr)ptr;

        ret = drmParseUsbDeviceInfo(maj, min, dev->deviceinfo.usb);
        if (ret < 0)
            goto free_device;
    }

    *device = dev;

    return 0;

free_device:
    free(dev);
    return ret;
}


static int drmParseSubsystemType(int maj, int min)
{
#ifdef __linux__
    char path[PATH_MAX + 1];
    char link[PATH_MAX + 1] = "";
    char *name;
    struct {
        const char *name;
        int bus_type;
    } bus_types[] = {
        { "/pci", DRM_BUS_PCI },
    { "/usb", DRM_BUS_USB },
    { "/platform", DRM_BUS_PLATFORM },
    { "/spi", DRM_BUS_PLATFORM },
    { "/host1x", DRM_BUS_HOST1X },
    { "/virtio", DRM_BUS_VIRTIO },
    };

    snprintf(path, PATH_MAX, "/sys/dev/char/%d:%d/device/subsystem",
        maj, min);

    if (readlink(path, link, PATH_MAX) < 0)
        return -errno;

    name = strrchr(link, '/');
    if (!name)
        return -EINVAL;

    for (unsigned i = 0; i < ARRAY_SIZE(bus_types); i++) {
        if (strncmp(name, bus_types[i].name, strlen(bus_types[i].name)) == 0)
            return bus_types[i].bus_type;
    }

    return -EINVAL;
#elif defined(__OpenBSD__) || defined(__DragonFly__)
    return DRM_BUS_PCI;
#else
    #warning "Missing implementation of drmParseSubsystemType"
        return -EINVAL;
#endif
}

static void drmFreePlatformDevice(drmDevicePtr device)
{
    if (device->deviceinfo.platform) {
        if (device->deviceinfo.platform->compatible) {
            char **compatible = device->deviceinfo.platform->compatible;

            while (*compatible) {
                free(*compatible);
                compatible++;
            }

            free(device->deviceinfo.platform->compatible);
        }
    }
}

static void drmFreeHost1xDevice(drmDevicePtr device)
{
    if (device->deviceinfo.host1x) {
        if (device->deviceinfo.host1x->compatible) {
            char **compatible = device->deviceinfo.host1x->compatible;

            while (*compatible) {
                free(*compatible);
                compatible++;
            }

            free(device->deviceinfo.host1x->compatible);
        }
    }
}

static int drmGetNodeType(const char *name)
{
    if (strncmp(name, DRM_PRIMARY_MINOR_NAME,
        sizeof(DRM_PRIMARY_MINOR_NAME) - 1) == 0)
        return DRM_NODE_PRIMARY;

    if (strncmp(name, DRM_CONTROL_MINOR_NAME,
        sizeof(DRM_CONTROL_MINOR_NAME) - 1) == 0)
        return DRM_NODE_CONTROL;

    if (strncmp(name, DRM_RENDER_MINOR_NAME,
        sizeof(DRM_RENDER_MINOR_NAME) - 1) == 0)
        return DRM_NODE_RENDER;

    return -EINVAL;
}

void drmFreeDevice(drmDevicePtr *device)
{
    if (device == NULL)
        return;

    if (*device) {
        switch ((*device)->bustype) {
        case DRM_BUS_PLATFORM:
            drmFreePlatformDevice(*device);
            break;

        case DRM_BUS_HOST1X:
            drmFreeHost1xDevice(*device);
            break;
        }
    }

    free(*device);
    *device = NULL;
}

static int
process_device(drmDevicePtr *device, const char *d_name,
    int req_subsystem_type,
    bool fetch_deviceinfo, uint32_t flags)
{
    struct stat sbuf;
    char node[PATH_MAX + 1];
    int node_type, subsystem_type;
    unsigned int maj, min;

    node_type = drmGetNodeType(d_name);
    if (node_type < 0)
        return -1;

    snprintf(node, PATH_MAX, "%s/%s", DRM_DIR_NAME, d_name);

    if (stat(node, &sbuf))
        return -1;

    maj = major(sbuf.st_rdev);
    min = minor(sbuf.st_rdev);

    if (!drmNodeIsDRM(maj, min) || !S_ISCHR(sbuf.st_mode))
    {
        return -1;
    }

    subsystem_type = drmParseSubsystemType(maj, min);

    if (req_subsystem_type != -1 && req_subsystem_type != subsystem_type)
        return -1;

    switch (subsystem_type) {
    case DRM_BUS_PCI:
    case DRM_BUS_VIRTIO:
        return drmProcessPciDevice(device, node, node_type, maj, min,
            fetch_deviceinfo, flags);
    case DRM_BUS_USB:
        return drmProcessUsbDevice(device, node, node_type, maj, min,
            fetch_deviceinfo, flags);
    case DRM_BUS_PLATFORM:
        return drmProcessPlatformDevice(device, node, node_type, maj, min,
            fetch_deviceinfo, flags);
    case DRM_BUS_HOST1X:
        return drmProcessHost1xDevice(device, node, node_type, maj, min,
            fetch_deviceinfo, flags);
    default:
        return -1;
    }
}

/* Consider devices located on the same bus as duplicate and fold the respective
* entries into a single one.
*
* Note: this leaves "gaps" in the array, while preserving the length.
*/
static void drmFoldDuplicatedDevices(drmDevicePtr local_devices[], int count)
{
    int node_type, i, j;

    for (i = 0; i < count; i++) {
        for (j = i + 1; j < count; j++) {
            if (drmDevicesEqual(local_devices[i], local_devices[j])) {
                local_devices[i]->available_nodes |= local_devices[j]->available_nodes;
                node_type = log2(local_devices[j]->available_nodes);
                memcpy(local_devices[i]->nodes[node_type],
                    local_devices[j]->nodes[node_type], drmGetMaxNodeName());
                drmFreeDevice(&local_devices[j]);
            }
        }
    }
}

/**
 * Get drm devices on the system
 *
 * \param flags feature/behaviour bitmask
 * \param devices the array of devices with drmDevicePtr elements
 *                can be NULL to get the device number first
 * \param max_devices the maximum number of devices for the array
 *
 * \return on error - negative error code,
 *         if devices is NULL - total number of devices available on the system,
 *         alternatively the number of devices stored in devices[], which is
 *         capped by the max_devices.
 *
 * \note Unlike drmGetDevices it does not retrieve the pci device revision field
 * unless the DRM_DEVICE_GET_PCI_REVISION \p flag is set.
 */
int drmGetDevices2(uint32_t flags, drmDevicePtr devices[],
                              int max_devices)
{
    drmDevicePtr local_devices[MAX_DRM_NODES];
    drmDevicePtr device;
    DIR *sysdir;
    struct dirent *dent;
    int ret, i, node_count, device_count;

    if (drm_device_validate_flags(flags))
        return -EINVAL;

    sysdir = opendir(DRM_DIR_NAME);
    if (!sysdir)
        return -errno;

    i = 0;
    while ((dent = readdir(sysdir))) {
        ret = process_device(&device, dent->d_name, -1, devices != NULL, flags);
        if (ret)
            continue;

        if (i >= MAX_DRM_NODES) {
            fprintf(stderr, "More than %d drm nodes detected. "
                    "Please report  - that should not happen.\n"
                    "Skipping extra nodes\n", MAX_DRM_NODES);
            break;
        }
        local_devices[i] = device;
        i++;
    }
    node_count = i;
    drmFoldDuplicatedDevices(local_devices, node_count);

    device_count = 0;
    for (i = 0; i < node_count; i++) {
        if (!local_devices[i])
            continue;

        if ((devices != NULL) && (device_count < max_devices))
            devices[device_count] = local_devices[i];
        else
            drmFreeDevice(&local_devices[i]);

        device_count++;
    }

    closedir(sysdir);
    return device_count;
}

/**
 * Get drm devices on the system
 *
 * \param devices the array of devices with drmDevicePtr elements
 *                can be NULL to get the device number first
 * \param max_devices the maximum number of devices for the array
 *
 * \return on error - negative error code,
 *         if devices is NULL - total number of devices available on the system,
 *         alternatively the number of devices stored in devices[], which is
 *         capped by the max_devices.
 */
int drmGetDevices(drmDevicePtr devices[], int max_devices)
{
    return drmGetDevices2(DRM_DEVICE_GET_PCI_REVISION, devices, max_devices);
}


static int32_t GetRendererFileDescriptor(char * drm_node)
{
    int32_t driFileDescriptor = -1;

    driFileDescriptor = open(drm_node, O_RDWR);
    return driFileDescriptor;
}

#endif  // #ifndef DRM_DEVICE_H_
