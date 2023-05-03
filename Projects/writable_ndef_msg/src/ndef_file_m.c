/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *
 * @ingroup nfc_writable_ndef_msg_example_ndef_file_m ndef_file_m.c
 * @{
 * @ingroup nfc_writable_ndef_msg_example
 * @brief FLASH service for the NFC writable NDEF message example.
 *
 */

#include <zephyr/kernel.h>
#include <soc.h>
#include <zephyr/device.h>
#include <string.h>
#include <zephyr/fs/nvs.h>
#include <nfc/t4t/ndef_file.h>
#include <nfc/ndef/uri_msg.h>
#include <zephyr/storage/flash_map.h>

#include "ndef_file_m.h"

#define FLASH_URL_ADDRESS_ID 1 /**< Address of URL message in FLASH */

//static const uint8_t m_url[] = /**< Default NDEF message: URL "nordicsemi.com". */
//	{'n', 'o', 'r', 'd', 'i', 'c', 's', 'e', 'm', 'i', '.', 'c', 'o', 'm'};

static const char url1[] = "ik-polls-project.herokuapp.com/polls/deepthoughts";
static const char url2[] = "sites.google.com/view/group18finalproject/about-the-project";
static const char url3[] = "forms.gle/2ra6XanJvxNrh9xTA";
static const char url4[] = "forms.gle/Gs5diyMgjs6YHkmNA";
static const char* urls[] = { url1, url2, url3, url4 };

/* Flash partition for NVS */
#define NVS_FLASH_DEVICE FIXED_PARTITION_DEVICE(storage_partition)
/* Flash block size in bytes */
#define NVS_SECTOR_SIZE  (DT_PROP(DT_CHOSEN(zephyr_flash), erase_block_size))
#define NVS_SECTOR_COUNT 2
/* Start address of the filesystem in flash */
#define NVS_STORAGE_OFFSET FIXED_PARTITION_OFFSET(storage_partition)

static struct nvs_fs fs = {
	.sector_size = NVS_SECTOR_SIZE,
	.sector_count = NVS_SECTOR_COUNT,
	.offset = NVS_STORAGE_OFFSET,
};

int ndef_file_setup(void)
{
	int err;

	fs.flash_device = NVS_FLASH_DEVICE;
	if (fs.flash_device == NULL) {
		return -ENODEV;
	}

	err = nvs_mount(&fs);
	if (err < 0) {
		printk("Cannot initialize NVS!\n");
	}

	return err;
}

int ndef_file_update(int index, uint8_t const *buff, uint32_t size)
{
	/* Update FLASH file with new NDEF message. */
	return nvs_write(&fs, FLASH_URL_ADDRESS_ID+index*CONFIG_NDEF_FILE_SIZE, buff, size);
}

/** .. include_startingpoint_ndef_file_rst */
int ndef_file_default_message(int index, uint8_t *buff, uint32_t *size)
{
	int err;
	uint32_t ndef_size = nfc_t4t_ndef_file_msg_size_get(*size);

	/* Encode URI message into buffer. */
	err = nfc_ndef_uri_msg_encode(NFC_URI_HTTPS,
				      urls[index],
				      strlen(urls[index]),
				      nfc_t4t_ndef_file_msg_get(buff),
				      &ndef_size);
	if (err) {
		return err;
	}

	err = nfc_t4t_ndef_file_encode(buff, &ndef_size);
	if (err) {
		return err;
	}

	*size = ndef_size;

	return 0;
}
/** .. include_endpoint_ndef_file_rst */

int ndef_restore_default(int index, uint8_t *buff, uint32_t size)
{
	int err;

	err = ndef_file_default_message(index, buff, &size);
	if (err < 0) {
		printk("Cannot create default message!\n");
		return err;
	}

	/* Save record with default NDEF message. */
	err = ndef_file_update(index, buff, size);
	if (err < 0) {
		printk("Cannot flash NDEF message!\n");
	}
	return err;
}

int ndef_file_load(int index, uint8_t *buff, uint32_t size)
{
	int err;

	/* If there is no record with given ID, create default message and store
	 * in FLASH. FLASH_URL_ADDRESS_ID is used to store an address, lets see
	 * if we can read it from flash, since we don't know the size read the
	 * maximum possible
	 */
	err = nvs_read(&fs, FLASH_URL_ADDRESS_ID+index*CONFIG_NDEF_FILE_SIZE, buff, size);
	if (err > 0) { /* Item was found, show it */
		printk("Found NDEF file record.\n");
	} else {
		printk("NDEF file record not found, creating default NDEF.\n");
		/* Create default NDEF message. */
		err = ndef_restore_default(index, buff, size);
	}

	return err;
}

/** @} */
