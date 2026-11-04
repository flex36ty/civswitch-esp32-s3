import {ESPLoader, Transport} from 'esptool-js';
import {ungzip} from 'pako';
import images from './images.json';
const el = id => document.getElementById(id);
const mode = el('mode'), button = el('install'), progress = el('progress'), status = el('status');
function describe() {
  el('effect').textContent = mode.value === 'full' ? 'Replaces all 16 MB of flash on the selected board.' : 'Updates only the application on a board already using the CivSwitch partition layout.';
}
mode.onchange = describe;
describe();
if (!navigator.serial || !window.isSecureContext) {
  button.disabled = true;
  status.textContent = 'Open this file in desktop Chrome or Edge to use the USB serial port.';
}
function log(text) { el('log').append(document.createTextNode(text)); }
button.onclick = async () => {
  const target = images[mode.value];
  button.disabled = mode.disabled = true;
  progress.value = 0;
  el('log').textContent = '';
  let transport;
  let verified = false;
  try {
    const port = await navigator.serial.requestPort();
    status.textContent = 'Connecting and checking board…';
    transport = new Transport(port);
    const loader = new ESPLoader({transport, baudrate:460800, terminal:{clean(){}, write:log, writeLine:s=>log(s+'\n')}});
    await loader.main();
    if (loader.chip.CHIP_NAME !== 'ESP32-S3' || await loader.detectFlashSize() !== '16MB') throw Error('This installer requires an ESP32-S3 with 16 MB flash.');
    const full = ungzip(Uint8Array.from(atob(images.gzip), c=>c.charCodeAt(0)));
    const data = full.subarray(target.address, target.address + target.length);
    const digest = Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256', data)), b=>b.toString(16).padStart(2,'0')).join('');
    if (digest !== target.sha256) throw Error('Firmware checksum mismatch. Download the flasher again.');
    status.textContent = 'Writing firmware. Keep the cable connected through verification.';
    await loader.writeFlash({fileArray:[{data,address:target.address}], flashMode:'keep',flashFreq:'keep',flashSize:'keep',eraseAll:false,compress:true,calculateMD5Hash:()=>target.md5,reportProgress:(_,done,total)=>{progress.value=100*done/total;}});
    verified = true;
    await loader.after('hard_reset');
    progress.value = 100;
    status.textContent = 'Firmware verified and board reset. Connect native USB to the Switch dock.';
  } catch (err) {
    status.textContent = verified ? 'Firmware verified. Tap RST to restart the board.' : err.name === 'NotFoundError' ? 'Port selection canceled.' : 'Could not finish: '+err.message;
  } finally {
    if (transport) { try { await transport.disconnect(); } catch (err) { log(String(err)); } }
    button.disabled = mode.disabled = false;
  }
};
