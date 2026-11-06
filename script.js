const $=id=>document.getElementById(id);
const esc=s=>s.replace(/[&<>]/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;'}[c]));
const asc=s=>[...s].reduce((a,c)=>a+c.charCodeAt(0),0);
const isP=n=>{if(n<2)return false;for(let i=2;i*i<=n;i++)if(n%i==0)return false;return true};
const nextP=n=>{while(!isP(n))n++;return n};
const wait=ms=>new Promise(r=>setTimeout(r,ms));

/* ---------- Module 1 ---------- */
let cap=7,tab=Array(7).fill(null),ch=Array.from({length:7},()=>[]),busy=false,flash=-1,okc=-1;
const cnt=()=>tab.filter(x=>x!==null).length+ch.reduce((a,c)=>a+c.length,0);
function draw(){
 $('m1g').innerHTML=tab.map((v,i)=>{
  const items=ch[i].length?ch[i].join(' → '):(v===null?'':v);
  const cls=(i==flash?' col':'')+(i==okc?' ok':'')+((v!==null||ch[i].length)?' full':'');
  return `<div class="cell${cls}"><b>${i}</b>${items?esc(items):'[empty]'}</div>`}).join('');
 $('m1l').textContent=`Table size: ${cap}   Items: ${cnt()}   Load factor: ${(cnt()/cap).toFixed(2)}`;
}
const L=(t,c)=>{$('m1o').innerHTML+=`<span class="${c||''}">${esc(t)}</span>\n`;$('m1o').scrollTop=1e9};
const idx=(h,p,m)=>m==2?(h+p*p)%cap:(h+p)%cap;
function place(items,c,m){const t=Array(c).fill(null),q=Array.from({length:c},()=>[]);
 for(const w of items){const h=asc(w)%c;
  if(m==3){q[h].push(w);continue}
  let ok=false;for(let p=0;p<c;p++){const i=m==2?(h+p*p)%c:(h+p)%c;if(t[i]===null){t[i]=w;ok=true;break}}
  if(!ok)return null}
 return [t,q]}
function rehash(extra){const m=+$('m1s').value,items=[...tab.filter(x=>x!==null),...ch.flat()];if(extra)items.push(extra);
 let c=nextP(cap*2),r=place(items,c,m);while(!r){c=nextP(c+1);r=place(items,c,m)}
 cap=c;[tab,ch]=r;L(`Rehash done. New size = ${cap} (next prime of double).`,'g')}
async function ins(){
 if(busy)return;const w=$('m1i').value.trim();if(!w){return}
 const m=+$('m1s').value;$('m1o').innerHTML='';
 if(tab.includes(w)||ch.some(c=>c.includes(w))){L('This word already exists.','y');return}
 busy=true;
 if((cnt()+1)/cap>=0.7){L(`Load factor would reach ${((cnt()+1)/cap).toFixed(2)} (≥ 0.70). Growing table...`,'y');rehash();draw();await wait(500)}
 const h=asc(w)%cap;
 L(`ASCII sum of "${w}" = ${asc(w)}`,'c');L(`Index = ${asc(w)} mod ${cap} = ${h}`,'c');
 if(m==3){if(ch[h].length){L(`Collision at box ${h}. Added to the chain.`,'r')}else L(`Placed in box ${h}.`,'g');ch[h].push(w);flash=-1;okc=h;draw()}
 else{let done=false;
  for(let p=0;p<cap;p++){const i=idx(h,p,m);
   if(tab[i]===null){tab[i]=w;flash=-1;okc=i;draw();L(`Box ${i} is empty. Placed "${w}" in box ${i}.`,'g');done=true;break}
   flash=i;draw();L(`Collision at box ${i} (holds "${tab[i]}"). ${m==2?'Quadratic':'Linear'} probe, next try...`,'r');await wait(650)}
  if(!done){L('No free spot on the probe path. Growing the table...','y');rehash(w);okc=tab.indexOf(w);flash=-1}}
 busy=false;$('m1i').value='';draw();
}
function find(){
 const w=$('m1i').value.trim();if(!w||busy)return;$('m1o').innerHTML='';flash=-1;okc=-1;const m=+$('m1s').value,h=asc(w)%cap;
 L(`Index = ${asc(w)} mod ${cap} = ${h}`,'c');
 if(m==3){if(ch[h].includes(w)){okc=h;L(`Found in the chain at box ${h}.`,'g')}else L('Not found.','r')}
 else{let f=false;for(let p=0;p<cap;p++){const i=idx(h,p,m);if(tab[i]===null)break;if(tab[i]===w){okc=i;L(`Found at box ${i} after ${p+1} step(s).`,'g');f=true;break}}if(!f)L('Not found.','r')}
 draw();
}
function reset1(){if(busy)return;cap=7;tab=Array(7).fill(null);ch=Array.from({length:7},()=>[]);flash=okc=-1;$('m1o').textContent='Output appears here.';draw()}
$('m1b').onclick=ins;$('m1f').onclick=find;$('m1r').onclick=reset1;$('m1s').onchange=()=>{reset1();$('m1o').textContent='Method changed, so the table was cleared.'};$('m1i').onkeydown=e=>{if(e.key=='Enter')ins()};draw();

/* ---------- Module 2 ---------- */
function fnv(s){let h=14695981039346656037n;const M=(1n<<64n)-1n;
 for(const ch of new TextEncoder().encode(s)){h^=BigInt(ch);h=(h*1099511628211n)&M;h^=(h>>13n)}return h}
function avalanche(){
 const a=fnv($('p1').value),b=fnv($('p2').value);
 const A=a.toString(2).padStart(64,'0'),B=b.toString(2).padStart(64,'0');
 let n=0,hb='';for(let i=0;i<64;i++){if(A[i]!=B[i]){n++;hb+=`<span class="diff">${B[i]}</span>`}else hb+=B[i]}
 $('h1').innerHTML=`Hash 1: 0x${a.toString(16).padStart(16,'0')}<br>${A}`;
 $('h2').innerHTML=`Hash 2: 0x${b.toString(16).padStart(16,'0')}<br>${hb}`;
 $('m2o').innerHTML=`<span class="${$('p1').value===$('p2').value?'y':'g'}">Changed bits: ${n} of 64 (${(n/64*100).toFixed(0)}%)</span>\n${$('p1').value===$('p2').value?'Same input gives the same hash.':'Passwords differ slightly, hashes look totally different.'}`;
}
$('p1').oninput=$('p2').oninput=avalanche;avalanche();

/* ---------- Module 3 ---------- */
function scan(){
 const a=$('ta').value,b=$('tb').value,k=Math.max(2,+$('k').value||5),B=257n,M=1000000007n;
 const wins=s=>{const r=[];if(s.length<k)return r;let h=0n,p=1n;
  for(let i=0;i<k;i++){h=(h*B+BigInt(s.charCodeAt(i)+1))%M;if(i)p=p*B%M}r.push(h);
  for(let i=k;i<s.length;i++){h=((h+M-BigInt(s.charCodeAt(i-k)+1)*p%M)%M*B+BigInt(s.charCodeAt(i)+1))%M;r.push(h)}return r};
 const wa=wins(a),wb=wins(b),set=new Map();
 wb.forEach((h,i)=>{if(!set.has(h))set.set(h,[]);set.get(h).push(i)});
 const mark=Array(a.length).fill(false);
 wa.forEach((h,i)=>{(set.get(h)||[]).forEach(j=>{if(a.substr(i,k)===b.substr(j,k))for(let t=0;t<k;t++)mark[i+t]=true})});
 const n=mark.filter(Boolean).length,pc=a.length?n/a.length*100:0;
 $('pc').textContent=pc.toFixed(1)+'%';$('pb').style.width=pc+'%';
 let o='';for(let i=0;i<a.length;i++)o+=mark[i]?`<span class="hl">${esc(a[i])}</span>`:esc(a[i]);
 $('m3o').innerHTML=o||'Type some text.';
}
['ta','tb','k'].forEach(i=>$(i).oninput=scan);$('m3b').onclick=scan;scan();

/* ---------- Module 4 ---------- */
let cache=[];
function drawC(ev){
 let h='';for(let i=0;i<3;i++){const v=cache[i];h+=`<div class="slot${v?' full':''}">${v?esc(v):'empty'}</div>`}
 if(ev)h+=`<div class="slot ev">${esc(ev)}<br>evicted</div>`;$('cs').innerHTML=h;
}
function req(){
 const w=$('c1').value.trim();if(!w)return;let ev=null,msg;
 const i=cache.indexOf(w);
 if(i>=0){cache.splice(i,1);cache.unshift(w);msg=`<span class="g">HIT: "${esc(w)}" found, moved to the newest position.</span>`}
 else{if(cache.length==3){ev=cache.pop();msg=`<span class="r">MISS: cache full. Oldest item "${esc(ev)}" evicted.</span>\n<span class="c">"${esc(w)}" added as newest.</span>`}
  else msg=`<span class="y">MISS: "${esc(w)}" loaded into an empty slot.</span>`;cache.unshift(w)}
 $('m4o').innerHTML=msg;drawC(ev);$('c1').value='';
}
$('c1b').onclick=req;$('c1').onkeydown=e=>{if(e.key=='Enter')req()};$('c1r').onclick=()=>{cache=[];$('m4o').textContent='Cache cleared.';drawC()};drawC();

/* ---------- Module 5 ---------- */
let gSize,gTgt,gTry;
$('gb').onclick=()=>{gSize=Math.max(2,Math.floor(+$('gs').value)||0);gTgt=+$('gt').value;
 if(!(gTgt>=0&&gTgt<gSize)){$('m5o').innerHTML=`<span class="r">Target must be between 0 and ${gSize-1}.</span>`;return}
 gTry=3;$('gi').disabled=$('gg').disabled=false;$('gi').value='';$('gi').focus();
 $('m5o').innerHTML=`<span class="c">Find a word with (ASCII sum mod ${gSize}) = ${gTgt}. Attempts left: 3</span>`};
function guess(){
 if(gTry<=0)return;const w=$('gi').value.trim();if(!w)return;gTry--;const r=asc(w)%gSize;
 let t=`"${esc(w)}": ${asc(w)} mod ${gSize} = ${r}\n`;
 if(r===gTgt){t+=`<span class="g">You broke it! Collision at index ${gTgt}.</span>`;gTry=0}
 else if(gTry>0)t+=`<span class="r">Wrong box (${r}). Attempts left: ${gTry}</span>`;
 else t+=`<span class="y">Game over. Many words share the same index, that is why collisions are unavoidable.</span>`;
 $('m5o').innerHTML+=`\n${t}`;$('m5o').scrollTop=1e9;$('gi').value='';
 if(gTry<=0)$('gi').disabled=$('gg').disabled=true;
}
$('gg').onclick=guess;$('gi').onkeydown=e=>{if(e.key=='Enter')guess()};

/* ---------- 3D hero ---------- */
const hc=Array(7).fill(0);let pv=-1;const world=$('w');
world.innerHTML=[...Array(7)].map((_,i)=>`<div class="bar" style="left:${i*66}px"><div class="t">${i}</div><div class="f"></div><div class="r"></div></div>`).join('');
const bars=[...world.children];
function hr(){bars.forEach((b,i)=>{const n=hc[i]+(pv==i?1:0);b.style.setProperty('--h',(14+n*30)+'px');
 b.style.setProperty('--c',pv==i?(hc[i]?'#e5484d':'#00a6c8'):hc[i]>1?'#e5484d':hc[i]?'#19a974':'#aab1ee')})}
function hp(){const w=$('hi').value.trim();pv=w?asc(w)%7:-1;
 $('ho').innerHTML=w?`"${esc(w)}" has ASCII sum ${asc(w)}<br>${asc(w)} mod 7 = <b>${pv}</b>`+(hc[pv]?' <b style="color:#e5484d">Collision! That box is taken.</b>':''):'Type a word to see which box it goes to.';hr()}
function ha(){const w=$('hi').value.trim();if(!w)return;const i=asc(w)%7;hc[i]++;$('hi').value='';pv=-1;
 $('ho').textContent=`Stored in box ${i}. Now try an anagram of that word (same letters): same sum, same box, so a collision.`;hr()}
$('hi').oninput=hp;$('hb').onclick=ha;$('hi').onkeydown=e=>{if(e.key=='Enter')ha()};hr();
