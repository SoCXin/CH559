## Introduction

Use bootloader to dump flash data. *Actually it is an exploit for built-in bootloader.*

Verified in CH558 with bootloader V231. CH559 should be okay too. Other chips (CH551/2/4) could be referred, but I'm not sure.

Thanks for [MarsTechHAN/ch552tool](https://github.com/MarsTechHAN/ch552tool) to provide this idea.

## Usage

Please install Python 3.x and PyUSB first.

```
pip install pyusb
```

Then, plug your device and make sure it in **download mode**.

For Windows, use Zadig to substitude the driver of "USB Module".

Finally, run *dump.py* and wait until it is done. It would take much time, so please be patient.
