/*
 * This file is part of the coreboot project.
 *
 * Copyright 2015 Google Inc.
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
 * Foundation, Inc.
 */

#include <commonlib/region.h>
#include <string.h>

static inline size_t region_end(const struct region *r)
{
	return region_sz(r) + region_offset(r);
}

static int is_subregion(const struct region *p, const struct region *c)
{
	if (region_offset(c) < region_offset(p))
		return 0;

	if (region_sz(c) > region_sz(p))
		return 0;

	if (region_end(c) > region_end(p))
		return 0;

	return 1;
}

static int normalize_and_ok(const struct region *outer, struct region *inner)
{
	inner->offset += region_offset(outer);
	return is_subregion(outer, inner);
}

static const struct region_device *rdev_root(const struct region_device *rdev)
{
	if (rdev->root == NULL)
		return rdev;
	return rdev->root;
}

void *rdev_mmap(const struct region_device *rd, size_t offset, size_t size)
{
	const struct region_device *rdev;
	struct region req = {
		.offset = offset,
		.size = size,
	};

	if (!normalize_and_ok(&rd->region, &req))
		return NULL;

	rdev = rdev_root(rd);

	return rdev->ops->mmap(rdev, req.offset, req.size);
}

int rdev_munmap(const struct region_device *rd, void *mapping)
{
	const struct region_device *rdev;

	rdev = rdev_root(rd);

	return rdev->ops->munmap(rdev, mapping);
}

ssize_t rdev_readat(const struct region_device *rd, void *b, size_t offset,
			size_t size)
{
	const struct region_device *rdev;
	struct region req = {
		.offset = offset,
		.size = size,
	};

	if (!normalize_and_ok(&rd->region, &req))
		return -1;

	rdev = rdev_root(rd);

	return rdev->ops->readat(rdev, b, req.offset, req.size);
}

int rdev_chain(struct region_device *child, const struct region_device *parent,
		size_t offset, size_t size)
{
	struct region req = {
		.offset = offset,
		.size = size,
	};

	if (!normalize_and_ok(&parent->region, &req))
		return -1;

	/* Keep track of root region device. Note the offsets are relative
	 * to the root device. */
	child->root = rdev_root(parent);
	child->ops = NULL;
	child->region.offset = req.offset;
	child->region.size = req.size;

	return 0;
}

void mem_region_device_init(struct mem_region_device *mdev, void *base,
				size_t size)
{
	memset(mdev, 0, sizeof(*mdev));
	mdev->base = base;
	mdev->rdev.ops = &mem_rdev_ops;
	mdev->rdev.region.size = size;
}

static void *mdev_mmap(const struct region_device *rd, size_t offset,
			size_t size)
{
	const struct mem_region_device *mdev;

	mdev = container_of(rd, typeof(*mdev), rdev);

	return &mdev->base[offset];
}

static int mdev_munmap(const struct region_device *rd, void *mapping)
{
	return 0;
}

static ssize_t mdev_readat(const struct region_device *rd, void *b,
				size_t offset, size_t size)
{
	const struct mem_region_device *mdev;

	mdev = container_of(rd, typeof(*mdev), rdev);

	memcpy(b, &mdev->base[offset], size);

	return size;
}

const struct region_device_ops mem_rdev_ops = {
	.mmap = mdev_mmap,
	.munmap = mdev_munmap,
	.readat = mdev_readat,
};

void mmap_helper_device_init(struct mmap_helper_region_device *mdev,
				void *cache, size_t cache_size)
{
	mem_pool_init(&mdev->pool, cache, cache_size);
}

void *mmap_helper_rdev_mmap(const struct region_device *rd, size_t offset,
				size_t size)
{
	struct mmap_helper_region_device *mdev;
	void *mapping;

	mdev = container_of((void *)rd, typeof(*mdev), rdev);

	mapping = mem_pool_alloc(&mdev->pool, size);

	if (mapping == NULL)
		return NULL;

	if (rd->ops->readat(rd, mapping, offset, size) != size) {
		mem_pool_free(&mdev->pool, mapping);
		return NULL;
	}

	return mapping;
}

int mmap_helper_rdev_munmap(const struct region_device *rd, void *mapping)
{
	struct mmap_helper_region_device *mdev;

	mdev = container_of((void *)rd, typeof(*mdev), rdev);

	mem_pool_free(&mdev->pool, mapping);

	return 0;
}
