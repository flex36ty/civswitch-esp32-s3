"""Local CivSwitch controls; requires pyserial. No payload is executed by this server."""
import argparse
import json
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import urlsplit
import serial
from serial.tools.list_ports import comports

PAGE = '''<!doctype html><meta charset="utf-8"><title>ESP32 CivSwitch</title>
<style>body{font:18px system-ui;background:#121820;color:#edf2f7;max-width:760px;margin:50px auto;padding:24px}button{font:inherit;padding:14px;margin:8px}pre{white-space:pre-wrap}p{line-height:1.6}</style>
<h1>ESP32 CivSwitch</h1><p>87 chat messages from the supplied payload. Connect COM to your computer and USB to the Switch dock. Use English (US) system and USB-keyboard settings, with Caps Lock off.</p>
<p>Open Civilization VII 1.0.0 (Switch 1 cartridge): Multiplayer → Wireless → Create Game → Host Lobby → Chat. Focus the chat box before starting.</p>
<p>Start runs the complete controller-and-keyboard macro after 10 seconds. Do not interact with the console during the run. Without a computer, hold BOOT for 2 seconds to start the same countdown, then release it. Press BOOT during a run to stop. RST only resets the board.</p>
<button id="start" disabled>Start CivSwitch macro</button><button id="test" disabled>Send two-message test</button><button id="stop">Stop / release controls</button><pre id="status">Connecting…</pre>
<a href="/serial-port">Manage COM port for flashing</a>
<p>This sends the session loader only. It does not run the persistent installer or delete saves. Uses repeated USB reports and a 2-second chat-opening wait, verified with the two-message test. The full run still needs verification.</p>
<script>
const statusBox=document.getElementById('status'),start=document.getElementById('start');let pending=false;
async function poll(){try{const r=await fetch('/status');if(!r.ok)throw Error(await r.text());const s=await r.json();start.disabled=pending||s.active||!s.usb;document.getElementById('test').disabled=start.disabled;statusBox.textContent=`${s.message}\nUSB: ${s.usb?'connected':'disconnected'}\nStep ${s.step} / ${s.total}`;}catch(e){start.disabled=true;document.getElementById('test').disabled=true;statusBox.textContent=e.message;}}
async function command(name){pending=true;start.disabled=true;document.getElementById('test').disabled=true;try{const r=await fetch('/'+name,{method:'POST',headers:{'X-CivTyper':'1'}});if(!r.ok)throw Error(await r.text());await poll();}catch(e){statusBox.textContent=e.message;}finally{pending=false;}}
start.onclick=()=>command('start');document.getElementById('test').onclick=()=>command('test');document.getElementById('stop').onclick=()=>command('stop');
async function loop(){await poll();setTimeout(loop,1000);}loop();
</script>'''

PORT_PAGE = '''<!doctype html><meta charset="utf-8"><title>CivSwitch serial port</title>
<style>body{max-width:620px;margin:3em auto;padding:1em;font:18px system-ui}button{padding:.7em}</style>
<h1>CivSwitch serial port</h1><p>Free the controller's serial port before choosing it in the flasher. After flashing, reconnect here to use the controller again.</p>
<button data-action="release">Free COM port</button> <button data-action="reconnect">Reconnect controller</button>
<p id="result" role="status"></p><a href="/">Return to controller</a>
<script>for(const b of document.querySelectorAll('button')) b.onclick=async()=>{
 const buttons=document.querySelectorAll('button');buttons.forEach(x=>x.disabled=true);
 try{const r=await fetch('/'+b.dataset.action,{method:'POST',headers:{'X-CivTyper':'1'}});
 document.getElementById('result').textContent=await r.text();}
 catch(e){document.getElementById('result').textContent=e.message;}
 finally{buttons.forEach(x=>x.disabled=false);}
};</script>'''

class Link:
    def __init__(self, port):
        self.port, self.lock = port, threading.RLock()
        self.released = False

    def manage(self, action):
        with self.lock:
            if action == 'release':
                if not self.released and self.request('STATUS')['active']:
                    raise ValueError('A macro is running. Stop it on the controller page before releasing the port.')
                self.port.close()
                self.released = True
                return 'Port released. Return to the flasher and choose your port.'
            if not self.port.is_open:
                self.port.open()
            self.released = False
            return 'Port connected. Return to the controller.'
    def request(self, command):
        with self.lock:
            if self.released:
                raise ValueError('COM port released for flashing. Use Manage COM port to reconnect.')
            try:
                if not self.port.is_open: self.port.open()
                self.port.reset_input_buffer()
                self.port.write(command.encode('ascii') + b'\n')
                deadline = time.monotonic() + 3
                while time.monotonic() < deadline:
                    try: result = json.loads(self.port.readline())
                    except (ValueError, UnicodeError): continue
                    if not isinstance(result, dict): continue
                    if 'error' in result: raise ValueError(result['error'])
                    if 'step' not in result:
                        raise ValueError('Install CivTyper firmware first; ordinary typing firmware is connected.')
                    return result
                raise TimeoutError('No board reply. Check COM cable and firmware.')
            except OSError:
                self.port.close()
                raise

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port')
    parser.add_argument('--released', action='store_true', help='Start the web page without opening the serial port')
    parser.add_argument('--http-port',type=int,default=8768)
    args=parser.parse_args()
    port=args.port
    if not port:
        candidates=[p.device for p in comports() if p.vid in (0x1a86,0x10c4,0x403)]
        if len(candidates)!=1: parser.error('Specify --port COM7 or --port /dev/ttyACM0')
        port=candidates[0]
    uart=serial.Serial(port=None,baudrate=115200,timeout=.25,write_timeout=3)
    uart.dtr=False;uart.rts=False;uart.port=port
    link=Link(uart)
    link.released=args.released
    if not link.released:
        uart.open()
    class Handler(BaseHTTPRequestHandler):
        def reply(self,code,body,kind='text/plain'):
            body=body.encode();self.send_response(code)
            self.send_header('Content-Type',kind+'; charset=utf-8')
            self.send_header('Content-Length',str(len(body)));self.send_header('Cache-Control','no-store')
            self.end_headers();self.wfile.write(body)
        def trusted(self):
            host=self.headers.get('Host','')
            return host in (f'127.0.0.1:{args.http_port}',f'localhost:{args.http_port}') and self.headers.get('Origin', 'http://'+host)=='http://'+host
        def do_GET(self):
            if not self.trusted():return self.reply(403,'Local page only')
            if self.path=='/':return self.reply(200,PAGE,'text/html')
            if self.path=='/serial-port':return self.reply(200,PORT_PAGE,'text/html')
            if self.path!='/status':return self.reply(404,'Not found')
            self.run_command('STATUS')
        def do_POST(self):
            if not self.trusted() or self.headers.get('X-CivTyper')!='1':return self.reply(403,'Use the local page')
            if self.path in ('/release','/reconnect'):
                try:result=link.manage(self.path[1:])
                except (OSError,ValueError) as error:return self.reply(409,str(error))
                return self.reply(200,result)
            command={'/start':'START','/test':'TEST','/stop':'STOP'}.get(self.path)
            if not command:return self.reply(404,'Not found')
            self.run_command(command)
        def run_command(self,command):
            try:self.reply(200,json.dumps(link.request(command)),'application/json')
            except (OSError,ValueError) as error:self.reply(503,str(error))
        def log_message(self,*args):pass
    try:
        print('Open http://127.0.0.1:'+str(args.http_port),flush=True)
        with ThreadingHTTPServer(('127.0.0.1',args.http_port),Handler) as server:server.serve_forever()
    except KeyboardInterrupt:pass
    finally:
        try:link.request('STOP')
        except Exception:pass
        uart.close()
if __name__=='__main__':main()



