/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *
 * @defgroup nfc_writable_ndef_msg_example_main main.c
 * @{
 * @ingroup nfc_writable_ndef_msg_example
 * @brief The application main file of NFC writable NDEF message example.
 *
 */



#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>
#include <stdbool.h>
#include <nfc_t4t_lib.h>

#include <nfc/ndef/msg.h>
#include <nfc/t4t/ndef_file.h>

#include <dk_buttons_and_leds.h>

#include "ndef_file_m.h"

#include <zephyr/types.h>
#include <zephyr/drivers/sensor.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <stdio.h>

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#define LAB2_SERVICE_UUID BT_UUID_128_ENCODE(0xBDFC9792, 0x8234, 0x405E, 0xAE02, 0x35EF3274B299)

const char msg1[] = "User Check-in";
const char msg2[] = "User Access Info";
const char msg3[] = "User Access Quiz";
const char msg4[] = "User Access Survey";
const char* const bt_notifs[] = { msg1, msg2, msg3, msg4};

bool notify_scan = false;

// Set up the advertisement data.
#define DEVICE_NAME "NFC_Proj_18"
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, LAB2_SERVICE_UUID)
};

static void ccc_changed(const struct bt_gatt_attr *attr,
				       uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	printk("CCC Notifications %s", notif_enabled ? "enabled" : "disabled");
}

BT_GATT_SERVICE_DEFINE(lab2_service,
	BT_GATT_PRIMARY_SERVICE(
		BT_UUID_DECLARE_128(LAB2_SERVICE_UUID)
	),
	BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(0x0001), BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ, NULL, NULL, bt_notifs[0]),
	BT_GATT_CCC(ccc_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);

#define NFC_FIELD_LED		DK_ALL_LEDS_MSK
#define NFC_WRITE_LED		DK_ALL_LEDS_MSK
#define NFC_READ_LED		DK_ALL_LEDS_MSK

#define NDEF_RESTORE_BTN_MSK	DK_BTN1_MSK

static char url_c0[] = "Check-In";
static char url_c1[] = "Info";
static char url_c2[] = "Quiz";
static char url_c3[] = "Survey";

static char* url_cat[] = {url_c0, url_c1, url_c2, url_c3 };
static uint8_t ndef_msg_buf[CONFIG_NDEF_FILE_SIZE]; /**< Buffer for NDEF file. */

enum {
	FLASH_WRITE_FINISHED,
	FLASH_BUF_PREP_STARTED,
	FLASH_BUF_PREP_FINISHED,
	FLASH_WRITE_STARTED,
};
static atomic_t op_flags;
static uint8_t flash_buf[CONFIG_NDEF_FILE_SIZE]; /**< Buffer for flash update. */
static uint8_t flash_buf_len; /**< Length of the flash buffer. */

static void flash_buffer_prepare(size_t data_length)
{
	if (atomic_cas(&op_flags, FLASH_WRITE_FINISHED,
			FLASH_BUF_PREP_STARTED)) {
		flash_buf_len = data_length + NFC_NDEF_FILE_NLEN_FIELD_SIZE;
		memcpy(flash_buf, ndef_msg_buf, sizeof(flash_buf));

		atomic_set(&op_flags, FLASH_BUF_PREP_FINISHED);
	} else {
		printk("Flash update pending. Discarding new data...\n");
	}

}

int url_id = 0;

/**
 * @brief Callback function for handling NFC events.
 */
static void nfc_callback(void *context,
			 nfc_t4t_event_t event,
			 const uint8_t *data,
			 size_t data_length,
			 uint32_t flags)
{
	ARG_UNUSED(context);
	ARG_UNUSED(data);
	ARG_UNUSED(flags);

	switch (event) {
	case NFC_T4T_EVENT_FIELD_ON:
		//dk_set_led_on(NFC_FIELD_LED);
		dk_set_leds( NFC_FIELD_LED );
		break;

	case NFC_T4T_EVENT_FIELD_OFF:
		dk_set_leds( DK_NO_LEDS_MSK );
		dk_set_led_on(url_id==0?DK_LED1:url_id==1?DK_LED2:url_id==2?DK_LED3:DK_LED4);
		break;

	case NFC_T4T_EVENT_NDEF_READ:
		//dk_set_led_on(NFC_READ_LED);
		dk_set_leds(NFC_READ_LED);
		printk( "User accessed '%s' portal\n", url_cat[url_id]);
		notify_scan = true;
		break;

	case NFC_T4T_EVENT_NDEF_UPDATED:
		if (data_length > 0) {
			//dk_set_led_on(NFC_WRITE_LED);
			dk_set_leds(NFC_WRITE_LED);
			flash_buffer_prepare(data_length);
		}
		break;

	default:
		break;
	}
	// printk( "%d\n", event );
	//for (int i=0; i<data_length; i++ )
	//  printk( "%2x ", data[i]);
	//printk( "\n" );
}

static int board_init(void)
{
	int err;

	err = dk_buttons_init(NULL);
	if (err) {
		printk("Cannot init buttons (err: %d)\n", err);
		return err;
	}

	err = dk_leds_init();
	if (err) {
		printk("Cannot init LEDs (err: %d)\n", err);
	}

	return err;
}

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}
}

/**
 * @brief   Function for application main entry.
 */
int main(void)
{
	printk("Starting Nordic NFC Writable NDEF Message example\n");

	/* Configure LED-pins as outputs. */
	if (board_init() < 0) {
		printk("Cannot initialize board!\n");
		goto fail;
	}
	/* Initialize NVS. */
	if (ndef_file_setup() < 0) {
		printk("Cannot setup NDEF file!\n");
		goto fail;
	}
	/* Load NDEF message from the flash file. */
	if (ndef_file_load(url_id, ndef_msg_buf, sizeof(ndef_msg_buf)) < 0) {
		printk("Cannot load NDEF file!\n");
		goto fail;
	}

	/* Restore default NDEF message if button is pressed. */
	uint32_t button_state;

	dk_read_buttons(&button_state, NULL);
	if (button_state & NDEF_RESTORE_BTN_MSK) {
		if (ndef_restore_default(url_id, ndef_msg_buf,
					 sizeof(ndef_msg_buf)) < 0) {
			printk("Cannot flash NDEF message!\n");
			goto fail;
		}
		printk("Default NDEF message restored!\n");
	}
	/* Set up NFC */
	int err = nfc_t4t_setup(nfc_callback, NULL);

	if (err < 0) {
		printk("Cannot setup t4t library!\n");
		goto fail;
	}
	/* Run Read-Write mode for Type 4 Tag platform */
	if (nfc_t4t_ndef_rwpayload_set(ndef_msg_buf,
				       sizeof(ndef_msg_buf)) < 0) {
		printk("Cannot set payload!\n");
		goto fail;
	}
	/* Start sensing NFC field */
	if (nfc_t4t_emulation_start() < 0) {
		printk("Cannot start emulation!\n");
		goto fail;
	}
	printk("Starting NFC Writable NDEF Message example\n");
	dk_set_led_on(url_id==0?DK_LED1:url_id==1?DK_LED2:url_id==2?DK_LED3:DK_LED4);

	int err_2;

	err_2 = bt_enable(bt_ready);
	if (err_2) {
		printk("Bluetooth init failed (err %d)\n", err_2);
		return;
	}

	while (true) {
		/*if (atomic_cas(&op_flags, FLASH_BUF_PREP_FINISHED,
				FLASH_WRITE_STARTED)) {
			if (ndef_file_update(0, flash_buf, flash_buf_len) < 0) {
				printk("Cannot flash NDEF message!\n");
			} else {
				printk("NDEF message successfully flashed.\n");
			}

			atomic_set(&op_flags, FLASH_WRITE_FINISHED);
		}*/
		bool seturl = false;
		dk_read_buttons(&button_state, NULL);
		if ( url_id != 0 && ( button_state & DK_BTN1_MSK ) )
		{
            url_id = 0;
			seturl = true;
		}
		if ( url_id != 1 && ( button_state & DK_BTN2_MSK ) )
		{
            url_id = 1;
			seturl = true;
		}
		if ( url_id != 2 && ( button_state & DK_BTN3_MSK ) )
		{
            url_id = 2;
			seturl = true;
		}
		if ( url_id != 3 && ( button_state & DK_BTN4_MSK ) )
		{
            url_id = 3;
			seturl = true;
		}
		if ( seturl )
		{
			if (nfc_t4t_emulation_stop() < 0) {
				printk("Cannot start emulation!\n");
				goto fail;
			}
			if (ndef_restore_default(url_id, ndef_msg_buf,
						sizeof(ndef_msg_buf)) < 0) {
				printk("Cannot flash NDEF message!\n");
				goto fail;
			}
			if (nfc_t4t_ndef_rwpayload_set(ndef_msg_buf,
					sizeof(ndef_msg_buf)) < 0) {
				printk("Cannot set payload!\n");
				goto fail;
			}
			if (nfc_t4t_emulation_start() < 0) {
				printk("Cannot start emulation!\n");
				goto fail;
			}
			dk_set_leds( DK_NO_LEDS_MSK );
			dk_set_led_on(url_id==0?DK_LED1:url_id==1?DK_LED2:url_id==2?DK_LED3:DK_LED4);

			printk("Switch URL-%d (%s) done.\n", url_id+1, url_cat[url_id] );			 
		}

        if ( notify_scan )
		{
			bt_gatt_notify(NULL, &lab2_service.attrs[1], (bt_notifs[url_id]), strlen(bt_notifs[url_id]));
		    notify_scan = false;
		}

		__WFE();
	}

fail:
	#if CONFIG_REBOOT
		sys_reboot(SYS_REBOOT_COLD);
	#endif /* CONFIG_REBOOT */
		return -EIO;
}
/** @} */
