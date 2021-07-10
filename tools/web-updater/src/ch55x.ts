import { WebSerial } from "./webSerial";

const PACKET_HEADER: number[] = [0x57, 0xab];
const DETECT_CMD: number[] = [
  0xa1,
  0x12,
  0x00,
  0x59,
  0x11,
  0x4d,
  0x43,
  0x55,
  0x20,
  0x49,
  0x53,
  0x50,
  0x20,
  0x26,
  0x20,
  0x57,
  0x43,
  0x48,
  0x2e,
  0x43,
  0x4e,
];
const RESET_CMD: number[] = [0xa2, 0x01, 0x00, 0x01];
const KEY_CMD_HEAD: number[] = [0xa3, 0x22, 0x00]; // 22 * [0x00]
const ERASE_CMD: number[] = [0xa4, 0x01, 0x00, 0x00];
const WRITE_CMD: number[] = [0xa5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00];
const VERIFY_CMD: number[] = [0xa6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00];
const READ_CFG_CMD: number[] = [0xa7, 0x02, 0x00, 0x1f, 0x00];

const PAGE_MAX: number = 60;

type OpType = "write" | "verify";
class Ch559Bootloader {
  private comReceive: number[] = [];
  private serial: WebSerial;

  private dumpHex(arr: number[]): string {
    return arr
      .map((v) => {
        return v.toString(16);
      })
      .join(" ");
  }

  private sleep(ms: number) {
    return new Promise((resolve: any) => setTimeout(resolve, ms));
  }

  private splitToChunk(arr: Uint8Array, size: number): number[][] {
    let ret = [];
    let i: number;
    let j: number;
    for (i = 0, j = arr.length; i < j; i += size) {
      ret.push(arr.slice(i, i + size));
    }

    return ret;
  }

  private receiveResponse(arr: Uint8Array) {
    this.comReceive = this.comReceive.concat(Array.from(arr));
  }

  private async readResponse(size: number, timeout: number): Promise<number[]> {
    let cnt = 0;
    while (this.comReceive.length < size && cnt < timeout) {
      await this.sleep(1);
      cnt += 1;
    }

    if (cnt >= timeout) {
      return [];
    }

    let ret = this.comReceive.slice(0, size);
    this.comReceive = this.comReceive.slice(size);

    return ret;
  }

  private convertPayload(
    payload: number[],
    chksum: number,
    devid: number
  ): number[] {
    return payload.map((v, index) => {
      if (index % 8 == 0x07) {
        return (v ^ ((chksum + devid) & 0xff)) & 0xff;
      } else {
        return (v ^ chksum) & 0xff;
      }
    });
  }

  private appendHeader(packet: number[]): number[] {
    return PACKET_HEADER.concat(packet);
  }

  private appendCheckSum(packet: number[]): number[] {
    return packet.concat(
      (0x55 +
        0xab +
        packet.reduce((acc, cur) => {
          return acc + cur;
        })) &
        0xff
    );
  }

  private makeFlashCmd(
    cmdheader: number[],
    address: number,
    remain: number,
    payload: number[],
    chksum: number,
    devid: number
  ) {
    let cmd: number[] = Array.from(cmdheader);
    cmd[1] = payload.length + 5;
    cmd[3] = address & 0xff;
    cmd[4] = (address >> 8) & 0xff;
    cmd[7] = remain & 0xff;

    cmd = cmd.concat(Array.from(this.convertPayload(payload, chksum, devid)));

    cmd = this.appendCheckSum(cmd);
    return this.appendHeader(cmd);
  }

  private makeFlashWriteCmd(
    address: number,
    remain: number,
    payload: number[],
    chksum: number,
    devid: number
  ) {
    return this.makeFlashCmd(
      WRITE_CMD,
      address,
      remain,
      payload,
      chksum,
      devid
    );
  }

  private makeVerifyCmd(
    address: number,
    remain: number,
    payload: number[],
    chksum: number,
    devid: number
  ) {
    return this.makeFlashCmd(
      VERIFY_CMD,
      address,
      remain,
      payload,
      chksum,
      devid
    );
  }

  private makeEraseCmd(len: number): number[] {
    let cmd = ERASE_CMD;
    let page_len = Math.floor((len + 1024 - 1) / 1024);

    if (page_len > PAGE_MAX) {
      throw new Error("Erase size is too large");
    }

    cmd[3] = page_len;
    cmd = this.appendCheckSum(cmd);
    cmd = this.appendHeader(cmd);

    return cmd;
  }

  private makeDetectCmd(): number[] {
    let cmd = DETECT_CMD;
    cmd = this.appendCheckSum(cmd);
    cmd = this.appendHeader(cmd);

    return cmd;
  }

  private makeReadCfgCmd(): number[] {
    let cmd = READ_CFG_CMD;
    cmd = this.appendCheckSum(cmd);
    cmd = this.appendHeader(cmd);

    return cmd;
  }

  private makeSendKeyCmd(): number[] {
    let cmd = Array.from(KEY_CMD_HEAD).concat(Array(0x22).fill(0));
    cmd = this.appendCheckSum(cmd);
    cmd = this.appendHeader(cmd);

    return cmd;
  }

  private async sendCommand(cmd: number[], rcv_len: number): Promise<number[]> {
    await this.serial.write(Uint8Array.from(cmd));

    let ret = await this.readResponse(rcv_len, 1000);

    console.log(`Receive: ${this.dumpHex(ret)}`);

    if (ret.length < rcv_len || ret[5] != 0) {
      throw new Error("Command failed");
    }

    return ret;
  }

  private async detect() {
    let cmd = this.makeDetectCmd();

    console.log(`Send detect: ${this.dumpHex(cmd)}`);

    await this.sendCommand(cmd, 9);
  }

  private async getCfg(): Promise<{ version: string; checksum: number }> {
    let cmd = this.makeReadCfgCmd();

    console.log(`Send cfg: ${this.dumpHex(cmd)}`);

    let ret = await this.sendCommand(cmd, 33);

    let version = `ver${ret[21]}.${ret[22]}.${ret[23]}`;
    let checksum =
      ret.slice(24, 28).reduce((acc, cur) => {
        return acc + cur;
      }) & 0xff;

    return { version, checksum };
  }

  private async eraseChip(size: number) {
    let cmd = this.makeEraseCmd(size);

    console.log(`Send erase: ${this.dumpHex(cmd)}`);

    await this.sendCommand(cmd, 9);
  }

  private async sendKey() {
    let cmd = this.makeSendKeyCmd();

    console.log(`Send key: ${this.dumpHex(cmd)}`);

    await this.sendCommand(cmd, 9);
  }

  private async opBinFile(
    bin: Uint8Array,
    op: OpType,
    checksum: number,
    devid: number,
    onProgress: ((addr: number, remains: number) => void) | null = null
  ) {
    let chunks = this.splitToChunk(bin, 56);
    let addr = 0;
    let remains = bin.length;

    for (let chunk of chunks) {
      if (chunk.length < 56) {
        chunk = Array.from(chunk).concat(Array(56 - chunk.length).fill(0xff));
      }

      let cmd: number[];
      if (op === "write") {
        cmd = this.makeFlashWriteCmd(addr, remains, chunk, checksum, devid);
      } else if (op === "verify") {
        cmd = this.makeVerifyCmd(addr, remains, chunk, checksum, devid);
      } else {
        throw new Error(`Unknown operation :${op}`);
      }

      addr += chunk.length;
      remains = remains - chunk.length;

      console.log(`Send ${op}: ${this.dumpHex(cmd)}`);

      // Receive response
      let ret = await this.sendCommand(cmd, 9);

      if (ret[6] != 0) {
        throw new Error(`${op} failed address 0x${addr.toString(16)}`);
      }

      if (onProgress != null) {
        onProgress(addr, remains);
      }
    }
  }

  private async openPort() {
    this.serial = new WebSerial(128, 5);
    this.serial.setReceiveCallback(this.receiveResponse.bind(this));
    await this.serial.open(null, 57600);
    this.serial.startReadLoop();
  }

  private async initBootloader(): Promise<{
    version: string;
    checksum: number;
  }> {
    await this.detect();
    let { version, checksum } = await this.getCfg();
    console.log(`Ch55x version ${version}, checksum ${checksum}`);

    return { version: version, checksum: checksum };
  }

  async flash(bin: Uint8Array, progress: (msg: string) => void = () => {}) {
    await this.openPort();

    try {
      let cfg = await this.initBootloader();

      await this.eraseChip(bin.length);

      await this.sendKey();

      progress("write start.\n");

      await this.opBinFile(bin, "write", cfg.checksum, 0x59, (_a, _b) => {
        progress(".");
      });

      progress("write complete.\n");

      await this.sendKey();

      progress("verify start.\n");

      await this.opBinFile(bin, "verify", cfg.checksum, 0x59, (_a, _b) => {
        progress(".");
      });

      progress("verify complete.\n");
    } catch (e) {
      progress("\n" + e.toString() + "\n");
      progress("Flash failed.\n");
    } finally {
      await this.serial.close();
    }
  }

  async verify(bin: Uint8Array, progress: (msg: string) => void = () => {}) {
    await this.openPort();

    try {
      let cfg = await this.initBootloader();

      await this.sendKey();

      progress("verify start.\n");

      await this.opBinFile(bin, "verify", cfg.checksum, 0x59, (_a, _b) => {
        progress(".");
      });

      progress("verify complete.\n");
    } catch (e) {
      progress("\n" + e.toString() + "\n");
      progress("Verify failed.\n");
    } finally {
      await this.serial.close();
    }
  }
}

export { Ch559Bootloader };
