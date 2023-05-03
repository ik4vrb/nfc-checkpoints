.. _writable_ndef_msg:

NFC: Activity checkpoint
##########################

This produces a writable NFC interface which advertises selected URL from 4 available URLs. A phone may be used to scan and/or open the webpage. 4 buttons on the nRF52840 may be used to select a URL. One of the 4 corresponding LEDs would be turned on to indicate which URL is enabled.

When a phone is used to scan the NFC, all 4 LEDs would flash and also send a notification to the central, indicating which URL was accessed.

Requirements
************

The sample supports the following development kits:

.. table-from-sample-yaml::

.. include:: /includes/tfm.txt

The sample also requires a smartphone or tablet with NFC Tools application (or equivalent).
