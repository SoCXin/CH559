import { WebSerial } from "./webSerial";
import { Ch559Bootloader } from "./ch55x";

const ch55x = new Ch559Bootloader();

let progress = document.getElementById("progress");

if (!navigator.serial) {
  progress.innerHTML =
    "Web serial is unavailable.\nPlease use Google Chrome or Chromium Edge\n";
  console.error("Web serial is unavailable");
}

async function verifyFirmware() {
  const version = document.getElementById("version").value;
  const filePath = `./CH559USB_${version}.bin`;
  console.log(`Load ${filePath}`);

  let bin = await fetch(filePath).then((r) => {
    console.log(r.headers);
    if (r.ok) {
      return r.arrayBuffer();
    } else {
      progress.innerHTML = "File not found";
      throw new Error("File not found\n");
    }
  });
  let u8array = new Uint8Array(bin);

  progress.innerHTML =
    "Connect Keyboard Quantizer with bootloader keymap, and select serial port appreared.\n";

  await ch55x.verify(u8array, (str) => {
    progress.innerHTML += str;
  });
}

async function flashFirmware() {
  let bin = await fetch("./CH559USB.bin").then((r) => {
    return r.arrayBuffer();
  });
  let u8array = new Uint8Array(bin);

  progress.innerHTML =
    "Connect Keyboard Quantizer with bootloader keymap, and select serial port appreared.\n";

  await ch55x.flash(u8array, (str) => {
    progress.innerHTML += str;
  });
}

document.getElementById("verify").onclick = verifyFirmware;
document.getElementById("flash").onclick = flashFirmware;
document.getElementById(
  "revision"
).innerText = `Revision:${process.env.REVISION}`;
