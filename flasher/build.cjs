const fs=require('fs'), crypto=require('crypto'), zlib=require('zlib'), esbuild=require('esbuild');
const dir='../release-assets/';
const hash=(b,algo)=>crypto.createHash(algo).update(b).digest('hex');
const sums=fs.readFileSync(dir+'SHA256SUMS.txt','utf8');
let full; const images={};
for (const [mode,address] of [['full',0],['app',65536]]) {
  const name='civswitch-esp32-s3-n16r8-v1.1.0-'+mode+'.bin', data=fs.readFileSync(dir+name);
  if (!sums.includes(hash(data,'sha256')+'  '+name)) throw Error('Release checksum mismatch');
  if (mode==='full') full=data;
  if (!full.subarray(address,address+data.length).equals(data)) throw Error('Application mismatch');
  images[mode]={address,length:data.length,sha256:hash(data,'sha256'),md5:hash(data,'md5')};
}
images.gzip=zlib.gzipSync(full).toString('base64');
fs.writeFileSync('images.json',JSON.stringify(images));
const js=esbuild.buildSync({entryPoints:['app.js'],bundle:true,minify:true,format:'iife',target:'chrome110',write:false}).outputFiles[0].text;
const html=fs.readFileSync('page.html','utf8').replace('/*BUNDLE*/',()=>js.replace(/<\/script/gi,'<\\/script'));
fs.writeFileSync('CivSwitch-Flasher.html',html);

