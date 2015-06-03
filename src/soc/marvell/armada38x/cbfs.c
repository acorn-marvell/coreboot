/*
 * This file is part of the coreboot project.
 *
 * Copyright 2013 Google Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */


#include <cbfs.h>  /* This driver serves as a CBFS media source. */
#include <console/console.h>

static int armada38x_mmaped_cbfs_open(struct cbfs_media *media)
{
	return 0;
}
static int armada38x_mmaped_cbfs_close(struct cbfs_media *media)
{
	return 0;
}

static size_t armada38x_mmaped_cbfs_read(struct cbfs_media *media, void *dest,
				   size_t offset, size_t count)
{
	size_t saved_count = count;
	while (count --)
	{
		*(unsigned char *)dest = * (unsigned char *) (CONFIG_BOOTBLOCK_BASE + offset);
		dest ++;
		offset++;
	}

	return saved_count;

}
static void *armada38x_mmaped_cbfs_map(struct cbfs_media *media, size_t offset,
				 size_t count)
{
	return (void *)(CONFIG_BOOTBLOCK_BASE + offset);
}
static void *armada38x_mmaped_cbfs_unmap(struct cbfs_media *media,
				   const void *address)
{
	return (void *)address;
}



int init_default_cbfs_media(struct cbfs_media *media)
{
	media->open = armada38x_mmaped_cbfs_open;
	media->close = armada38x_mmaped_cbfs_close;
	media->read = armada38x_mmaped_cbfs_read;
	media->map = armada38x_mmaped_cbfs_map;
	media->unmap = armada38x_mmaped_cbfs_unmap;
	return 0;
}
