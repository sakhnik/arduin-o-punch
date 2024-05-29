// Autogenerated file, do not edit manually. See arduin-o-punch-web-ui/build.sh

#ifdef ESP32

#include <Arduino.h>
#include <WebServer.h>

// *INDENT-OFF*

const char *index_html PROGMEM = R"html(
<!DOCTYPE html>
<html lang="en">
<head>
	<meta charset='utf-8'>
	<meta name='viewport' content='width=device-width,initial-scale=1'>

	<title>Arduin-O-Punch</title>

	<link rel='icon' type='image/png' href='/favicon.png'>
	<link rel='stylesheet' href='/global.css'>
	<link rel='stylesheet' href='/build/bundle.css'>

</head>

<body>
</body>
	<script src='/build/bundle.js'></script>
</html>
)html";
const char *global_css PROGMEM = R"html(
html, body {
	position: relative;
	width: 100%;
	height: 100%;
}

body {
	color: #333;
	margin: 0;
	padding: 8px;
	box-sizing: border-box;
	font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen-Sans, Ubuntu, Cantarell, "Helvetica Neue", sans-serif;
}

a {
	color: rgb(0,100,200);
	text-decoration: none;
}

a:hover {
	text-decoration: underline;
}

a:visited {
	color: rgb(0,80,160);
}

label {
	display: block;
}

input, button, select, textarea {
	font-family: inherit;
	font-size: inherit;
	-webkit-padding: 0.4em 0;
	padding: 0.4em;
	margin: 0 0 0.5em 0;
	box-sizing: border-box;
	border: 1px solid #ccc;
	border-radius: 2px;
}

input:disabled {
	color: #ccc;
}

button {
	color: #333;
	background-color: #f4f4f4;
	outline: none;
}

button:disabled {
	color: #999;
}

button:not(:disabled):active {
	background-color: #ddd;
}

button:focus {
	border-color: #666;
}
)html";
const char *build_bundle_css PROGMEM = R"html(
.popup.svelte-12tp13r{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background-color:rgba(0, 0, 0, 0.8);justify-content:center;align-items:center}.show.svelte-12tp13r{display:flex}.popup-content.svelte-12tp13r{background-color:white;padding:20px;border-radius:8px;text-align:center}#closeBtn.svelte-12tp13r{margin-top:20px}
.container.svelte-wdax5m{width:100%;max-height:400px;overflow-y:scroll;border:1px solid #ccc}table.svelte-wdax5m{width:100%;border-collapse:collapse}th.svelte-wdax5m,td.svelte-wdax5m{border:1px solid #ccc;padding:8px;text-align:left}th.svelte-wdax5m{background-color:#f4f4f4}
.container.svelte-al7qx6{font-family:sans-serif}
)html";
const char *build_bundle_js PROGMEM = R"html(
var app=function(){"use strict";function t(){}function e(t){return t()}function n(){return Object.create(null)}function o(t){t.forEach(e)}function r(t){return"function"==typeof t}function i(t,e){return t!=t?e==e:t!==e||t&&"object"==typeof t||"function"==typeof t}function c(t,e,n,o){return t[1]&&o?function(t,e){for(const n in e)t[n]=e[n];return t}(n.ctx.slice(),t[1](o(e))):n.ctx}function u(t,e){t.appendChild(e)}function l(t,e,n){t.insertBefore(e,n||null)}function s(t){t.parentNode&&t.parentNode.removeChild(t)}function a(t,e){for(let n=0;n<t.length;n+=1)t[n]&&t[n].d(e)}function d(t){return document.createElement(t)}function f(t){return document.createTextNode(t)}function p(){return f(" ")}function m(t,e,n,o){return t.addEventListener(e,n,o),()=>t.removeEventListener(e,n,o)}function $(t){return function(e){return e.preventDefault(),t.call(this,e)}}function h(t,e,n){null==n?t.removeAttribute(e):t.getAttribute(e)!==n&&t.setAttribute(e,n)}function g(t){return""===t?null:+t}function y(t,e){e=""+e,t.data!==e&&(t.data=e)}function v(t,e){t.value=null==e?"":e}function b(t,e,n,o){null==n?t.style.removeProperty(e):t.style.setProperty(e,n,o?"important":"")}function x(t,e,n){for(let n=0;n<t.options.length;n+=1){const o=t.options[n];if(o.__value===e)return void(o.selected=!0)}n&&void 0===e||(t.selectedIndex=-1)}function w(t,e,n){t.classList[n?"add":"remove"](e)}let _;function k(t){_=t}function O(t){(function(){if(!_)throw new Error("Function called outside component initialization");return _})().$$.on_mount.push(t)}const E=[],C=[];let L=[];const M=[],T=Promise.resolve();let H=!1;function P(t){L.push(t)}function N(t){M.push(t)}const z=new Set;let S=0;function A(){if(0!==S)return;const t=_;do{try{for(;S<E.length;){const t=E[S];S++,k(t),I(t.$$)}}catch(t){throw E.length=0,S=0,t}for(k(null),E.length=0,S=0;C.length;)C.pop()();for(let t=0;t<L.length;t+=1){const e=L[t];z.has(e)||(z.add(e),e())}L.length=0}while(E.length);for(;M.length;)M.pop()();H=!1,z.clear(),k(t)}function I(t){if(null!==t.fragment){t.update(),o(t.before_update);const e=t.dirty;t.dirty=[-1],t.fragment&&t.fragment.p(t.ctx,e),t.after_update.forEach(P)}}const B=new Set;let j;function q(t,e){t&&t.i&&(B.delete(t),t.i(e))}function D(t,e,n,o){if(t&&t.o){if(B.has(t))return;B.add(t),j.c.push((()=>{B.delete(t),o&&(n&&t.d(1),o())})),t.o(e)}else o&&o()}function F(t,e,n){const o=t.$$.props[e];void 0!==o&&(t.$$.bound[o]=n,n(t.$$.ctx[o]))}function R(t){t&&t.c()}function X(t,n,i,c){const{fragment:u,after_update:l}=t.$$;u&&u.m(n,i),c||P((()=>{const n=t.$$.on_mount.map(e).filter(r);t.$$.on_destroy?t.$$.on_destroy.push(...n):o(n),t.$$.on_mount=[]})),l.forEach(P)}function G(t,e){const n=t.$$;null!==n.fragment&&(!function(t){const e=[],n=[];L.forEach((o=>-1===t.indexOf(o)?e.push(o):n.push(o))),n.forEach((t=>t())),L=e}(n.after_update),o(n.on_destroy),n.fragment&&n.fragment.d(e),n.on_destroy=n.fragment=null,n.ctx=[])}function J(t,e){-1===t.$$.dirty[0]&&(E.push(t),H||(H=!0,T.then(A)),t.$$.dirty.fill(0)),t.$$.dirty[e/31|0]|=1<<e%31}function K(e,r,i,c,u,l,a,d=[-1]){const f=_;k(e);const p=e.$$={fragment:null,ctx:[],props:l,update:t,not_equal:u,bound:n(),on_mount:[],on_destroy:[],on_disconnect:[],before_update:[],after_update:[],context:new Map(r.context||(f?f.$$.context:[])),callbacks:n(),dirty:d,skip_bound:!1,root:r.target||f.$$.root};a&&a(p.root);let m=!1;if(p.ctx=i?i(e,r.props||{},((t,n,...o)=>{const r=o.length?o[0]:n;return p.ctx&&u(p.ctx[t],p.ctx[t]=r)&&(!p.skip_bound&&p.bound[t]&&p.bound[t](r),m&&J(e,t)),n})):[],p.update(),m=!0,o(p.before_update),p.fragment=!!c&&c(p.ctx),r.target){if(r.hydrate){const t=function(t){return Array.from(t.childNodes)}(r.target);p.fragment&&p.fragment.l(t),t.forEach(s)}else p.fragment&&p.fragment.c();r.intro&&q(e.$$.fragment),X(e,r.target,r.anchor,r.customElement),A()}k(f)}class Q{$destroy(){G(this,1),this.$destroy=t}$on(e,n){if(!r(n))return t;const o=this.$$.callbacks[e]||(this.$$.callbacks[e]=[]);return o.push(n),()=>{const t=o.indexOf(n);-1!==t&&o.splice(t,1)}}$set(t){var e;this.$$set&&(e=t,0!==Object.keys(e).length)&&(this.$$.skip_bound=!0,this.$$set(t),this.$$.skip_bound=!1)}}function U(e){let n,r,i,c,a,g,v;return{c(){n=d("span"),r=d("input"),i=p(),c=d("button"),a=f(e[3]),h(r,"type",e[4]),r.value=e[0],h(r,"id",e[1]),h(r,"name","key"),h(c,"class","toggle-key")},m(t,o){l(t,n,o),u(n,r),u(n,i),u(n,c),u(c,a),g||(v=[m(r,"input",e[5]),m(c,"click",$(e[6]))],g=!0)},p(t,[e]){16&e&&h(r,"type",t[4]),1&e&&r.value!==t[0]&&(r.value=t[0]),2&e&&h(r,"id",t[1]),8&e&&y(a,t[3])},i:t,o:t,d(t){t&&s(n),g=!1,o(v)}}}function V(t,e,n){let o,r,{id:i="key"}=e,{value:c="112233445566"}=e,u=!1;return t.$$set=t=>{"id"in t&&n(1,i=t.id),"value"in t&&n(0,c=t.value)},t.$$.update=()=>{4&t.$$.dirty&&n(4,o=u?"text":"password"),4&t.$$.dirty&&n(3,r=u?"••":"👁️")},[c,i,u,r,o,function(t){n(0,c=t.target.value)},()=>n(2,u=!u)]}class W extends Q{constructor(t){super(),K(this,t,V,U,i,{id:1,value:0})}}function Y(t,e,n){const o=t.slice();return o[11]=e[n],o}function Z(e){let n,o,r,i=e[11].text+"";return{c(){n=d("option"),o=f(i),r=p(),n.__value=e[11].id,n.value=n.__value},m(t,e){l(t,n,e),u(n,o),u(n,r)},p:t,d(t){t&&s(n)}}}function tt(t){let e,n,r,i,c,f,$,y,b,w,_,k,O,E,L,M,T,H,z,S,A,I,B,j,J,K,Q,U,V,tt,et,nt,ot,rt,it,ct,ut,lt,st,at;function dt(e){t[7](e)}let ft={id:"key"};void 0!==t[1]&&(ft.value=t[1]),M=new W({props:ft}),C.push((()=>F(M,"value",dt)));let pt=t[5],mt=[];for(let e=0;e<pt.length;e+=1)mt[e]=Z(Y(t,pt,e));return{c(){e=d("div"),n=d("h3"),n.textContent="Налаштування",r=p(),i=d("form"),c=d("table"),f=d("tr"),$=d("td"),$.innerHTML='<label for="id">Номер:</label>',y=p(),b=d("td"),w=d("input"),_=p(),k=d("tr"),O=d("td"),O.innerHTML='<label for="key">Ключ:</label>',E=p(),L=d("td"),R(M.$$.fragment),H=p(),z=d("tr"),S=d("td"),S.innerHTML='<label for="record-size">Записів у журналі:</label>',A=p(),I=d("td"),B=d("input"),j=p(),J=d("tr"),K=d("td"),K.innerHTML='<label for="record-bits">Відміток на запис:</label>',Q=p(),U=d("td"),V=d("select");for(let t=0;t<mt.length;t+=1)mt[t].c();tt=p(),et=d("tr"),nt=d("td"),nt.innerHTML='<label for="record-days">Період журналу (днів):</label>',ot=p(),rt=d("td"),it=d("input"),ct=p(),ut=d("tr"),ut.innerHTML='<td></td> \n        <td><input type="submit" value="Застосувати"/></td>',h(w,"type","number"),h(w,"id","id"),h(w,"name","id"),h(w,"min","1"),h(w,"max","255"),h(B,"type","number"),h(B,"id","record-size"),h(B,"name","rec-size"),h(V,"id","record-bits"),h(V,"name","rec-bits"),void 0===t[3]&&P((()=>t[9].call(V))),h(it,"type","number"),h(it,"id","record-days"),h(it,"name","rec-days"),h(i,"method","POST"),h(i,"action","/settings"),h(i,"id","settings-form"),h(i,"target","_self"),h(e,"class","container-settings")},m(o,s){l(o,e,s),u(e,n),u(e,r),u(e,i),u(i,c),u(c,f),u(f,$),u(f,y),u(f,b),u(b,w),v(w,t[0]),u(c,_),u(c,k),u(k,O),u(k,E),u(k,L),X(M,L,null),u(c,H),u(c,z),u(z,S),u(z,A),u(z,I),u(I,B),v(B,t[2]),u(c,j),u(c,J),u(J,K),u(J,Q),u(J,U),u(U,V);for(let t=0;t<mt.length;t+=1)mt[t]&&mt[t].m(V,null);x(V,t[3],!0),u(c,tt),u(c,et),u(et,nt),u(et,ot),u(et,rt),u(rt,it),v(it,t[4]),u(c,ct),u(c,ut),lt=!0,st||(at=[m(w,"input",t[6]),m(B,"input",t[8]),m(V,"change",t[9]),m(it,"input",t[10])],st=!0)},p(t,[e]){1&e&&g(w.value)!==t[0]&&v(w,t[0]);const n={};if(!T&&2&e&&(T=!0,n.value=t[1],N((()=>T=!1))),M.$set(n),4&e&&g(B.value)!==t[2]&&v(B,t[2]),32&e){let n;for(pt=t[5],n=0;n<pt.length;n+=1){const o=Y(t,pt,n);mt[n]?mt[n].p(o,e):(mt[n]=Z(o),mt[n].c(),mt[n].m(V,null))}for(;n<mt.length;n+=1)mt[n].d(1);mt.length=pt.length}40&e&&x(V,t[3]),16&e&&g(it.value)!==t[4]&&v(it,t[4])},i(t){lt||(q(M.$$.fragment,t),lt=!0)},o(t){D(M.$$.fragment,t),lt=!1},d(t){t&&s(e),G(M),a(mt,t),st=!1,o(at)}}}function et(t,e,n){let o=19,r="112233445566",i=512,c=2,u=1,l=[{id:1,text:"0–1"},{id:2,text:"0–3"},{id:4,text:"0–15"},{id:8,text:"0–255"}];return O((async()=>{const t=await fetch("/settings");(await t.text()).trim().split("\n").forEach((t=>{const[e,l]=t.split("=");"id"==e?(n(0,o=Number(l)),document.title="AOP "+l):"key"==e?n(1,r=l):"rec-size"==e?n(2,i=Number(l)):"rec-bits"==e?n(3,c=Number(l)):"rec-days"==e&&n(4,u=Number(l))}))})),[o,r,i,c,u,l,function(){o=g(this.value),n(0,o)},function(t){r=t,n(1,r)},function(){i=g(this.value),n(2,i)},function(){c=function(t){const e=t.querySelector(":checked");return e&&e.__value}(this),n(3,c),n(5,l)},function(){u=g(this.value),n(4,u)}]}class nt extends Q{constructor(t){super(),K(this,t,et,tt,i,{})}}function ot(e){let n,o,r,i,c,a,f,g,y,v;return{c(){n=d("div"),o=d("h3"),o.textContent="Оновлення",r=p(),i=d("form"),i.innerHTML='<input type="file" id="file" name="update" accept=".esp32.bin"/> \n    <input type="submit" value="Завантажити"/>',c=p(),a=d("br"),f=p(),g=d("div"),g.textContent="0%",h(i,"method","POST"),h(i,"enctype","multipart/form-data"),h(i,"id","upload-form"),h(g,"id","prg"),b(g,"width","0"),b(g,"color","white"),b(g,"text-align","center"),h(n,"class","container-upgrade")},m(t,e){l(t,n,e),u(n,o),u(n,r),u(n,i),u(n,c),u(n,a),u(n,f),u(n,g),y||(v=m(i,"submit",$(rt)),y=!0)},p:t,i:t,o:t,d(t){t&&s(n),y=!1,v()}}}function rt(){let t=document.getElementById("upload-form"),e=document.getElementById("prg");e.style.backgroundColor="blue";var n=new FormData(t),o=new XMLHttpRequest,r=document.getElementById("file").files[0].size;o.open("POST","/update?size="+r),o.upload.addEventListener("progress",(t=>{console.log("progress "+t.loaded+" "+t.total);let n=Math.round(t.loaded/t.total*100)+"%";t.lengthComputable&&(e.innerHTML=n,e.style.width=n),"100%"==n&&(e.style.backgroundColor="black")})),o.send(n)}class it extends Q{constructor(t){super(),K(this,t,null,ot,i,{})}}function ct(e){let n,r,i,a,f,$,g;const y=e[2].default,v=function(t,e,n,o){if(t){const r=c(t,e,n,o);return t[0](r)}}(y,e,e[1],null),b=v||function(e){let n;return{c(){n=d("p"),n.textContent="Popup content"},m(t,e){l(t,n,e)},p:t,d(t){t&&s(n)}}}();return{c(){n=d("div"),r=d("div"),b&&b.c(),i=p(),a=d("button"),a.textContent="Закрити",h(a,"id","closeBtn"),h(a,"class","svelte-12tp13r"),h(r,"class","popup-content svelte-12tp13r"),h(n,"id","popup"),h(n,"class","popup svelte-12tp13r"),w(n,"show",e[0])},m(t,o){l(t,n,o),u(n,r),b&&b.m(r,null),u(r,i),u(r,a),f=!0,$||(g=[m(window,"keydown",e[3]),m(a,"click",e[4])],$=!0)},p(t,[e]){v&&v.p&&(!f||2&e)&&function(t,e,n,o,r,i){if(r){const u=c(e,n,o,i);t.p(u,r)}}(v,y,t,t[1],f?function(t,e,n,o){if(t[2]&&o){const r=t[2](o(n));if(void 0===e.dirty)return r;if("object"==typeof r){const t=[],n=Math.max(e.dirty.length,r.length);for(let o=0;o<n;o+=1)t[o]=e.dirty[o]|r[o];return t}return e.dirty|r}return e.dirty}(y,t[1],e,null):function(t){if(t.ctx.length>32){const e=[],n=t.ctx.length/32;for(let t=0;t<n;t++)e[t]=-1;return e}return-1}(t[1]),null),(!f||1&e)&&w(n,"show",t[0])},i(t){f||(q(b,t),f=!0)},o(t){D(b,t),f=!1},d(t){t&&s(n),b&&b.d(t),$=!1,o(g)}}}function ut(t,e,n){let{$$slots:o={},$$scope:r}=e,{isOpen:i=!1}=e;return t.$$set=t=>{"isOpen"in t&&n(0,i=t.isOpen),"$$scope"in t&&n(1,r=t.$$scope)},[i,r,o,()=>n(0,i=!1),()=>n(0,i=!1)]}class lt extends Q{constructor(t){super(),K(this,t,ut,ct,i,{isOpen:0})}}function st(t,e,n){const o=t.slice();return o[5]=e[n],o}function at(t){let e,n,o,r,i=t[2],c=[];for(let e=0;e<i.length;e+=1)c[e]=pt(st(t,i,e));return{c(){e=d("table"),n=d("thead"),n.innerHTML='<tr><th class="svelte-wdax5m">Картка</th><th class="svelte-wdax5m">Кількість</th></tr>',o=p(),r=d("tbody");for(let t=0;t<c.length;t+=1)c[t].c();h(e,"class","svelte-wdax5m")},m(t,i){l(t,e,i),u(e,n),u(e,o),u(e,r);for(let t=0;t<c.length;t+=1)c[t]&&c[t].m(r,null)},p(t,e){if(4&e){let n;for(i=t[2],n=0;n<i.length;n+=1){const o=st(t,i,n);c[n]?c[n].p(o,e):(c[n]=pt(o),c[n].c(),c[n].m(r,null))}for(;n<c.length;n+=1)c[n].d(1);c.length=i.length}},d(t){t&&s(e),a(c,t)}}}function dt(t){let e,n,o;return{c(){e=d("p"),n=f("Помилка: "),o=f(t[1])},m(t,r){l(t,e,r),u(e,n),u(e,o)},p(t,e){2&e&&y(o,t[1])},d(t){t&&s(e)}}}function ft(e){let n;return{c(){n=d("p"),n.textContent="Завантаження…"},m(t,e){l(t,n,e)},p:t,d(t){t&&s(n)}}}function pt(t){let e,n,o,r,i,c,a,m=t[5].card+"",$=t[5].count+"";return{c(){e=d("tr"),n=d("td"),o=f(m),r=p(),i=d("td"),c=f($),a=p(),h(n,"class","svelte-wdax5m"),h(i,"class","svelte-wdax5m")},m(t,s){l(t,e,s),u(e,n),u(n,o),u(e,r),u(e,i),u(i,c),u(e,a)},p(t,e){4&e&&m!==(m=t[5].card+"")&&y(o,m),4&e&&$!==($=t[5].count+"")&&y(c,$)},d(t){t&&s(e)}}}function mt(e){let n,o,r;function i(t,e){return t[0]?ft:t[1]?dt:at}let c=i(e),a=c(e);return{c(){n=d("div"),o=d("p"),o.textContent="Журнал відмітки. Натисність ESC щоб закрити.",r=p(),a.c(),h(n,"class","container svelte-wdax5m")},m(t,e){l(t,n,e),u(n,o),u(n,r),a.m(n,null)},p(t,[e]){c===(c=i(t))&&a?a.p(t,e):(a.d(1),a=c(t),a&&(a.c(),a.m(n,null)))},i:t,o:t,d(t){t&&s(n),a.d()}}}function $t(t,e,n){let{isOpen:o=!1}=e,r=!0,i=null,c={};const u=async()=>{try{const e=await fetch("/record");if(!e.ok)throw new Error("Не вдалося отримати дані");n(2,(t=await e.text(),c=t.split(" ").map((t=>{const[e,n]=t.split(":");return{card:parseInt(e,10),count:parseInt(n,10)}}))))}catch(t){n(1,i=t.message)}finally{n(0,r=!1)}var t};return t.$$set=t=>{"isOpen"in t&&n(3,o=t.isOpen)},t.$$.update=()=>{8&t.$$.dirty&&o&&u()},[r,i,c,o]}class ht extends Q{constructor(t){super(),K(this,t,$t,mt,i,{isOpen:3})}}function gt(t){let e,n,o;function r(e){t[1](e)}let i={};return void 0!==t[0]&&(i.isOpen=t[0]),e=new ht({props:i}),C.push((()=>F(e,"isOpen",r))),{c(){R(e.$$.fragment)},m(t,n){X(e,t,n),o=!0},p(t,o){const r={};!n&&1&o&&(n=!0,r.isOpen=t[0],N((()=>n=!1))),e.$set(r)},i(t){o||(q(e.$$.fragment,t),o=!0)},o(t){D(e.$$.fragment,t),o=!1},d(t){G(e,t)}}}function yt(t){let e,n,o,r,i,c,a,f,$,g,y,v,b,x,w,_;function k(e){t[2](e)}c=new nt({});let O={$$slots:{default:[gt]},$$scope:{ctx:t}};return void 0!==t[0]&&(O.isOpen=t[0]),f=new lt({props:O}),C.push((()=>F(f,"isOpen",k))),b=new it({}),{c(){e=d("div"),n=d("h1"),n.textContent="Станція Arduin-O-Punch",o=p(),r=d("p"),r.innerHTML="Версія <span>)html" PROJECT_VERSION "-" GIT_REVISION R"html(</span>",i=p(),R(c.$$.fragment),a=p(),R(f.$$.fragment),g=p(),y=d("button"),y.textContent="Журнал",v=p(),R(b.$$.fragment),h(e,"class","container svelte-al7qx6")},m(s,d){l(s,e,d),u(e,n),u(e,o),u(e,r),u(e,i),X(c,e,null),u(e,a),X(f,e,null),u(e,g),u(e,y),u(e,v),X(b,e,null),x=!0,w||(_=m(y,"click",t[3]),w=!0)},p(t,[e]){const n={};17&e&&(n.$$scope={dirty:e,ctx:t}),!$&&1&e&&($=!0,n.isOpen=t[0],N((()=>$=!1))),f.$set(n)},i(t){x||(q(c.$$.fragment,t),q(f.$$.fragment,t),q(b.$$.fragment,t),x=!0)},o(t){D(c.$$.fragment,t),D(f.$$.fragment,t),D(b.$$.fragment,t),x=!1},d(t){t&&s(e),G(c),G(f),G(b),w=!1,_()}}}function vt(t,e,n){let o=!1;return[o,function(t){o=t,n(0,o)},function(t){o=t,n(0,o)},()=>n(0,o=!0)]}return new class extends Q{constructor(t){super(),K(this,t,vt,yt,i,{})}}({target:document.body,props:{name:"world"}})}();
//# sourceMappingURL=bundle.js.map
)html";
unsigned char favicon_png[] PROGMEM = {
  0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
  0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80,
  0x08, 0x06, 0x00, 0x00, 0x00, 0xc3, 0x3e, 0x61, 0xcb, 0x00, 0x00, 0x00,
  0x06, 0x62, 0x4b, 0x47, 0x44, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0xa0,
  0xbd, 0xa7, 0x93, 0x00, 0x00, 0x07, 0x0f, 0x49, 0x44, 0x41, 0x54, 0x78,
  0x9c, 0xed, 0xdd, 0x7f, 0x6c, 0x94, 0xf5, 0x1d, 0xc0, 0xf1, 0xf7, 0x63,
  0x7f, 0x48, 0x49, 0x8b, 0x09, 0xcc, 0xca, 0x24, 0xc1, 0x00, 0x61, 0x4a,
  0xe2, 0xfe, 0xc0, 0x34, 0xb1, 0x04, 0xb6, 0x91, 0x98, 0x19, 0x98, 0xb3,
  0x99, 0x51, 0x21, 0x5a, 0x53, 0xa3, 0x2e, 0x63, 0xc6, 0x18, 0x05, 0x32,
  0xec, 0x12, 0x96, 0x40, 0x41, 0x06, 0xc8, 0x8a, 0x63, 0x2e, 0x73, 0xc6,
  0x99, 0x8d, 0xcc, 0x2c, 0xd4, 0x18, 0x8c, 0x4a, 0xfc, 0x81, 0x45, 0xa3,
  0x13, 0xaa, 0x2d, 0x05, 0x03, 0xda, 0xc9, 0x36, 0x7e, 0x54, 0xc0, 0x3b,
  0x90, 0x4a, 0x7f, 0xde, 0xf7, 0xb9, 0x7b, 0x9e, 0xe7, 0xbb, 0x3f, 0xda,
  0xde, 0x4e, 0xf9, 0xd9, 0xf6, 0x9e, 0xef, 0x73, 0x77, 0xdf, 0xcf, 0x2b,
  0x21, 0x39, 0x7a, 0xbd, 0xef, 0xf7, 0xcb, 0xf3, 0xbc, 0x7b, 0xbd, 0xbb,
  0xe7, 0x39, 0xce, 0x21, 0x5a, 0xd7, 0x01, 0x0f, 0x00, 0xb7, 0x02, 0x33,
  0x80, 0xef, 0x00, 0x57, 0x44, 0xba, 0xa2, 0xf0, 0x05, 0xc0, 0x69, 0xe0,
  0x30, 0xf0, 0x36, 0xf0, 0x02, 0xd0, 0x19, 0xe9, 0x8a, 0x22, 0x30, 0x0e,
  0x78, 0x1a, 0x48, 0x02, 0xda, 0xf2, 0x3f, 0x2e, 0xd0, 0x08, 0x5c, 0x39,
  0xa6, 0x2d, 0x3a, 0x4a, 0x4e, 0x04, 0x73, 0x56, 0x00, 0x3b, 0x81, 0x9b,
  0x01, 0xa6, 0x4c, 0x99, 0x42, 0x5d, 0x5d, 0x1d, 0x73, 0xe6, 0xcc, 0xa1,
  0xb2, 0xb2, 0x92, 0xe2, 0xe2, 0xe2, 0x08, 0x96, 0x64, 0x8e, 0xe7, 0x79,
  0xc4, 0xe3, 0x71, 0x5a, 0x5a, 0x5a, 0xd8, 0xba, 0x75, 0x2b, 0x27, 0x4e,
  0x9c, 0x18, 0xbe, 0xaa, 0x85, 0xc1, 0x7b, 0xc2, 0xde, 0xe8, 0x56, 0x67,
  0xc6, 0x0e, 0x86, 0xea, 0x5f, 0xb6, 0x6c, 0x99, 0x56, 0x4a, 0x69, 0x5b,
  0x29, 0xa5, 0xf4, 0xd2, 0xa5, 0x4b, 0x33, 0xef, 0x0d, 0x5e, 0x8f, 0x74,
  0xcf, 0x18, 0x70, 0x37, 0x43, 0xff, 0xd8, 0xfa, 0xfa, 0xfa, 0xa8, 0xb7,
  0x7f, 0xce, 0x58, 0xb1, 0x62, 0x45, 0x66, 0x04, 0x77, 0x46, 0xba, 0x87,
  0x42, 0xd6, 0x0c, 0xe8, 0xe9, 0xd3, 0xa7, 0x6b, 0xd7, 0x75, 0xa3, 0xde,
  0xee, 0x39, 0xc3, 0x75, 0x5d, 0x3d, 0x6d, 0xda, 0xb4, 0xe1, 0x00, 0x76,
  0x9a, 0xdc, 0x21, 0x26, 0x1f, 0x71, 0x17, 0x01, 0x3f, 0x04, 0x58, 0xbc,
  0x78, 0x31, 0xa5, 0xa5, 0xa5, 0x06, 0xa7, 0xce, 0x6d, 0xa5, 0xa5, 0xa5,
  0x2c, 0x5a, 0xb4, 0x68, 0xf8, 0xaf, 0x3f, 0xc2, 0xe0, 0x7e, 0x31, 0x19,
  0x40, 0x25, 0x50, 0x0c, 0x30, 0x63, 0xc6, 0x0c, 0x83, 0xd3, 0xe6, 0x87,
  0x99, 0x33, 0x67, 0x0e, 0x5f, 0x2c, 0x01, 0xae, 0x31, 0x35, 0xaf, 0xc9,
  0x00, 0xd2, 0x3f, 0xf2, 0xf2, 0xd3, 0x7f, 0xae, 0x6f, 0x6d, 0x13, 0x63,
  0x1b, 0xa8, 0xd0, 0x5f, 0x74, 0x11, 0x97, 0x20, 0x01, 0x58, 0xae, 0x18,
  0x98, 0x6a, 0x68, 0xae, 0x6b, 0x0d, 0xcd, 0x53, 0x08, 0xae, 0x65, 0xf0,
  0x19, 0x41, 0xe8, 0x8a, 0x81, 0x63, 0x26, 0x26, 0x12, 0x23, 0xb2, 0xdb,
  0xd4, 0x44, 0xf2, 0x2b, 0xc0, 0x72, 0xe9, 0x17, 0xde, 0x1b, 0x1b, 0x1b,
  0x59, 0xb2, 0x64, 0x49, 0x68, 0x13, 0x75, 0x76, 0x76, 0x32, 0x6b, 0xd6,
  0xac, 0xd0, 0xc6, 0x2f, 0x24, 0x1d, 0x1d, 0x1d, 0x4c, 0x9d, 0x1a, 0xd2,
  0x6f, 0x66, 0xdf, 0xa3, 0xfd, 0x37, 0x77, 0xf1, 0x83, 0xdf, 0x0f, 0xbe,
  0xde, 0x94, 0x0e, 0xa0, 0xa4, 0xa4, 0x84, 0xf1, 0xe3, 0xc7, 0x87, 0x33,
  0x29, 0x50, 0x56, 0x56, 0x16, 0xda, 0xd8, 0x85, 0xa6, 0xac, 0xac, 0x2c,
  0x94, 0x7d, 0xa1, 0xdd, 0x7e, 0xd4, 0xe6, 0x7b, 0xb9, 0x3e, 0xfe, 0x71,
  0xfa, 0x6b, 0x85, 0x7d, 0xe8, 0x4d, 0xa4, 0xe9, 0xbe, 0x2e, 0xd4, 0xba,
  0x1a, 0xfc, 0xcf, 0x77, 0x43, 0xe9, 0x84, 0xf4, 0xd7, 0x25, 0x00, 0x0b,
  0xe8, 0xd3, 0xc7, 0x48, 0x34, 0x2c, 0x20, 0x38, 0xf9, 0xf9, 0x39, 0xd7,
  0x49, 0x00, 0x05, 0x2e, 0xf8, 0xe2, 0x53, 0x12, 0x6b, 0x16, 0xa2, 0xcf,
  0x1c, 0x3f, 0xef, 0xf5, 0x12, 0x40, 0x01, 0xf3, 0x0f, 0xb5, 0xa0, 0xd6,
  0xdd, 0x8e, 0xee, 0x3d, 0x73, 0xc1, 0xef, 0x91, 0x00, 0x0a, 0x94, 0xd7,
  0xfa, 0x2a, 0xaa, 0xf1, 0x1e, 0x48, 0x26, 0x2e, 0xfa, 0x7d, 0x12, 0x40,
  0x01, 0x4a, 0xbd, 0xfb, 0x57, 0xdc, 0x3f, 0xfd, 0x02, 0x7c, 0xef, 0xbc,
  0xd7, 0x67, 0xbe, 0xc4, 0x28, 0x2f, 0x04, 0x15, 0x98, 0xe4, 0xf6, 0x0d,
  0xb8, 0x7f, 0x7c, 0xe8, 0x82, 0x3b, 0xff, 0xdb, 0xe4, 0x1e, 0xa0, 0x50,
  0x68, 0x8d, 0xfb, 0xb7, 0x5f, 0x91, 0x7a, 0xad, 0x71, 0x44, 0x37, 0x93,
  0x00, 0x0a, 0x81, 0x97, 0x44, 0x6d, 0xb9, 0x1f, 0xef, 0xc3, 0x6d, 0x23,
  0xbe, 0xa9, 0x04, 0x90, 0xe7, 0xb4, 0xea, 0x43, 0x6d, 0xbc, 0x0b, 0xff,
  0x93, 0xb7, 0x47, 0x75, 0x7b, 0x09, 0x20, 0x8f, 0xe9, 0xb3, 0x71, 0x12,
  0x4f, 0xde, 0x46, 0x70, 0xb8, 0x7d, 0xd4, 0x63, 0x48, 0x00, 0x79, 0x2a,
  0x38, 0x75, 0x04, 0xd5, 0xb0, 0x80, 0xe0, 0xcb, 0x7f, 0x8f, 0x69, 0x1c,
  0x09, 0x20, 0x0f, 0x05, 0xc7, 0x0e, 0x90, 0x58, 0xbb, 0x10, 0xdd, 0x75,
  0x72, 0x74, 0x03, 0x38, 0xff, 0x7f, 0x43, 0x98, 0x04, 0x90, 0x67, 0xfc,
  0x83, 0xef, 0xa2, 0xd6, 0xdf, 0x81, 0x4e, 0xf4, 0x64, 0x65, 0x3c, 0x09,
  0x20, 0x8f, 0x78, 0x1f, 0x6d, 0x47, 0x6d, 0xae, 0x85, 0x94, 0xca, 0xda,
  0x98, 0x12, 0x40, 0x9e, 0x48, 0xbd, 0xf1, 0x0c, 0xee, 0x5f, 0x1e, 0x07,
  0x1d, 0x64, 0x75, 0x5c, 0x09, 0x20, 0xd7, 0x69, 0x4d, 0xb2, 0x69, 0x35,
  0xc9, 0xa6, 0x86, 0x50, 0x86, 0x97, 0x00, 0x72, 0x59, 0xe0, 0xe3, 0x3e,
  0xf7, 0x08, 0xa9, 0x9d, 0xcf, 0x85, 0x36, 0x85, 0x04, 0x90, 0xab, 0x52,
  0x2e, 0xea, 0xe9, 0xfb, 0xf0, 0x5a, 0x5e, 0x0e, 0x61, 0x70, 0x79, 0x16,
  0x90, 0xd3, 0x74, 0x7f, 0x37, 0x89, 0xd5, 0x75, 0xf8, 0x1d, 0x1f, 0x84,
  0x3e, 0x97, 0x04, 0x90, 0x83, 0xd4, 0xe6, 0x7b, 0xf0, 0xbb, 0x3a, 0x8c,
  0xcc, 0x25, 0x87, 0x83, 0x73, 0x50, 0x70, 0xdc, 0xcc, 0xce, 0x07, 0x09,
  0xc0, 0x7a, 0x12, 0x80, 0xe5, 0x24, 0x00, 0xcb, 0x49, 0x00, 0x16, 0x92,
  0x73, 0x02, 0x45, 0x9a, 0x04, 0x60, 0x39, 0x09, 0xc0, 0x72, 0x12, 0x80,
  0xe5, 0x24, 0x00, 0xcb, 0x49, 0x00, 0x36, 0xca, 0x38, 0x25, 0x4c, 0x02,
  0xb0, 0x9c, 0x04, 0x60, 0x39, 0x09, 0xc0, 0x72, 0x12, 0x80, 0xe5, 0x24,
  0x00, 0xcb, 0x49, 0x00, 0x96, 0x93, 0x00, 0x2c, 0xa4, 0x33, 0x8e, 0x06,
  0x49, 0x00, 0x96, 0x93, 0x00, 0x2c, 0x27, 0x01, 0x58, 0x4e, 0x02, 0xb0,
  0x9c, 0x04, 0x60, 0x39, 0x09, 0xc0, 0x72, 0x12, 0x80, 0x8d, 0xe4, 0x68,
  0xa0, 0x18, 0x26, 0x01, 0x58, 0x4e, 0x02, 0xb0, 0x9c, 0x04, 0x60, 0x39,
  0x09, 0xc0, 0x72, 0x12, 0x80, 0xe5, 0x24, 0x00, 0xcb, 0x49, 0x00, 0x96,
  0x93, 0x00, 0x2c, 0x27, 0x01, 0x58, 0x4e, 0x02, 0xb0, 0x9c, 0x04, 0x60,
  0x39, 0x09, 0xc0, 0x4a, 0x72, 0x30, 0x48, 0x0c, 0x91, 0x00, 0x2c, 0x27,
  0x01, 0x58, 0x4e, 0x02, 0xb0, 0x9c, 0x04, 0x60, 0x39, 0x09, 0xc0, 0x46,
  0x72, 0x4a, 0x98, 0x18, 0x26, 0x01, 0x58, 0x4e, 0x02, 0xb0, 0x9c, 0x04,
  0x60, 0x39, 0x09, 0xc0, 0x72, 0x12, 0x80, 0xe5, 0x24, 0x00, 0x0b, 0xc9,
  0xff, 0x16, 0x2e, 0xd2, 0x24, 0x00, 0xcb, 0x49, 0x00, 0x96, 0x93, 0x00,
  0x2c, 0x27, 0x01, 0x58, 0xce, 0x58, 0x00, 0xe5, 0xe5, 0xe5, 0xe9, 0xcb,
  0x7d, 0x7d, 0x7d, 0xa6, 0xa6, 0xcd, 0x1b, 0xbd, 0xbd, 0xbd, 0xe9, 0xcb,
  0xe5, 0xa1, 0x7f, 0x9c, 0x67, 0x04, 0x07, 0x83, 0x26, 0x4e, 0x9c, 0x48,
  0x45, 0x45, 0x05, 0x00, 0xfb, 0xf7, 0xef, 0x37, 0x35, 0x6d, 0xde, 0xd8,
  0xb7, 0x6f, 0x1f, 0x00, 0x13, 0x4a, 0x60, 0x62, 0xa9, 0xb9, 0x79, 0x8d,
  0x05, 0xe0, 0x38, 0x0e, 0x35, 0x35, 0x35, 0x00, 0x6c, 0xdb, 0xb6, 0x8d,
  0x78, 0x3c, 0x6e, 0x6a, 0xea, 0x9c, 0x17, 0x8b, 0xc5, 0x68, 0x6a, 0x6a,
  0x02, 0xe0, 0xb6, 0xef, 0x9a, 0x9d, 0xdb, 0xe8, 0x63, 0x80, 0xe5, 0xcb,
  0x97, 0x53, 0x54, 0x54, 0x44, 0x77, 0x77, 0x37, 0xb5, 0xb5, 0xb5, 0xb8,
  0xae, 0x6b, 0x72, 0xfa, 0x9c, 0xe4, 0xba, 0x2e, 0xb5, 0xb5, 0xb5, 0xf4,
  0xf4, 0xf4, 0x50, 0xec, 0xc0, 0xa3, 0x33, 0x9d, 0x4b, 0xdf, 0x28, 0x8b,
  0x8c, 0x06, 0x30, 0x7b, 0xf6, 0x6c, 0x56, 0xae, 0x5c, 0x09, 0x40, 0x73,
  0x73, 0x33, 0xf3, 0xe6, 0xcd, 0xa3, 0xbd, 0xbd, 0xdd, 0xe4, 0x12, 0x72,
  0xca, 0xde, 0xbd, 0x7b, 0x99, 0x3b, 0x77, 0x2e, 0xbb, 0x76, 0xed, 0x02,
  0xe0, 0x89, 0x1b, 0x1c, 0xbe, 0x7f, 0x95, 0xd9, 0x35, 0x18, 0xff, 0xf4,
  0xf0, 0x55, 0xab, 0x56, 0x31, 0x30, 0x30, 0xc0, 0xa6, 0x4d, 0x9b, 0x68,
  0x6b, 0x6b, 0xa3, 0xaa, 0xaa, 0x8a, 0xaa, 0xaa, 0x2a, 0xaa, 0xab, 0xab,
  0x99, 0x3c, 0x79, 0x32, 0x45, 0x45, 0x45, 0xa6, 0x97, 0x64, 0x94, 0xef,
  0xfb, 0xc4, 0x62, 0x31, 0xf6, 0xec, 0xd9, 0x43, 0x5b, 0x5b, 0x1b, 0x30,
  0xf8, 0x90, 0xec, 0xb1, 0xef, 0x41, 0xfd, 0x2c, 0xf3, 0xeb, 0x89, 0xe4,
  0xe3, 0xe3, 0x37, 0x6e, 0xdc, 0xc8, 0xfc, 0xf9, 0xf3, 0x79, 0xe2, 0x91,
  0x9f, 0x73, 0xf0, 0xe8, 0x97, 0xb4, 0xb6, 0xb6, 0xd2, 0xda, 0xda, 0x1a,
  0xc5, 0x52, 0x22, 0x77, 0xe3, 0x55, 0xd0, 0x70, 0xa3, 0xc3, 0x8f, 0xaf,
  0x89, 0x66, 0xfe, 0x48, 0x02, 0x40, 0x6b, 0x6e, 0xf9, 0xea, 0x3d, 0x76,
  0xdf, 0x14, 0xe3, 0xa3, 0xeb, 0x1c, 0xde, 0x89, 0x6b, 0x8e, 0xf6, 0xc3,
  0x69, 0xf7, 0x9b, 0x07, 0x2a, 0x0a, 0x91, 0x03, 0x5c, 0x7d, 0x25, 0x4c,
  0x2b, 0x87, 0x5b, 0x2a, 0x1d, 0x6e, 0x9e, 0x94, 0xf9, 0xa4, 0xcc, 0x8c,
  0xcc, 0x6d, 0x6c, 0x3e, 0x00, 0x2f, 0x89, 0xfa, 0xc3, 0x03, 0x78, 0xff,
  0xfc, 0x07, 0x0e, 0x50, 0x3d, 0x09, 0xaa, 0x27, 0x99, 0xde, 0x04, 0x62,
  0x98, 0xd1, 0x00, 0xb4, 0xdb, 0x8f, 0x7a, 0xea, 0x6e, 0xfc, 0x7d, 0x6f,
  0x9a, 0x9c, 0x56, 0x5c, 0x84, 0xb1, 0x00, 0x74, 0xef, 0x19, 0xd4, 0xba,
  0xdb, 0xf1, 0x0f, 0xb5, 0x98, 0x9a, 0x52, 0x5c, 0x06, 0x23, 0x01, 0x04,
  0xa7, 0x8e, 0xa2, 0xd6, 0x2c, 0x20, 0x38, 0x79, 0xc8, 0xc4, 0x74, 0x62,
  0x04, 0x42, 0x0f, 0x20, 0xe8, 0x3c, 0x48, 0x62, 0xcd, 0x42, 0x74, 0xd7,
  0x89, 0xb0, 0xa7, 0x12, 0xa3, 0x10, 0x6a, 0x00, 0xfe, 0xa7, 0xef, 0xa1,
  0xd6, 0xdf, 0x81, 0x1e, 0xe8, 0x0e, 0x73, 0x1a, 0x31, 0x06, 0xa1, 0x05,
  0xe0, 0x7d, 0xfc, 0x0a, 0xaa, 0xf1, 0x5e, 0x48, 0xa9, 0xb0, 0xa6, 0x10,
  0xa3, 0x95, 0xf1, 0xd6, 0xb0, 0x50, 0x02, 0x48, 0xed, 0x7a, 0x01, 0xf7,
  0xd9, 0x5f, 0x82, 0xef, 0x85, 0x31, 0xbc, 0xc8, 0xa2, 0xac, 0x07, 0x90,
  0xdc, 0xbe, 0x81, 0xe4, 0xdf, 0x7f, 0x9d, 0xed, 0x61, 0x45, 0x48, 0xb2,
  0x17, 0x40, 0xe0, 0xe3, 0x3e, 0xff, 0x28, 0xa9, 0xb7, 0x9e, 0xcd, 0xda,
  0x90, 0x22, 0x7c, 0xd9, 0x09, 0x20, 0xe5, 0xa2, 0xb6, 0xd4, 0xe1, 0xed,
  0x7e, 0x29, 0x2b, 0xc3, 0x09, 0x73, 0xc6, 0x1c, 0x80, 0xee, 0x3f, 0x8b,
  0x5a, 0xff, 0x33, 0xfc, 0xcf, 0xde, 0xcf, 0xc6, 0x7a, 0x84, 0x61, 0x63,
  0x0a, 0x40, 0x9f, 0x8d, 0x91, 0x58, 0xfb, 0x13, 0x82, 0x23, 0x72, 0x8a,
  0x57, 0x3e, 0xc9, 0xfc, 0xe8, 0xd8, 0x51, 0x07, 0x10, 0xc4, 0x0f, 0xa3,
  0x1a, 0x16, 0x10, 0xc4, 0xfe, 0x93, 0x8d, 0x35, 0x89, 0x88, 0x8c, 0x2a,
  0x80, 0xe0, 0xbf, 0x6d, 0x24, 0x9e, 0xfc, 0x29, 0xba, 0xfb, 0x54, 0xb6,
  0xd7, 0x23, 0x0c, 0x1b, 0x71, 0x00, 0xfe, 0x81, 0x66, 0xd4, 0x86, 0x3b,
  0xd1, 0x89, 0x9e, 0x30, 0xd6, 0x23, 0x0c, 0x1b, 0x51, 0x00, 0xde, 0xfb,
  0x2f, 0xa2, 0x9e, 0x79, 0x10, 0xfc, 0x54, 0x58, 0xeb, 0x11, 0x86, 0x5d,
  0xf6, 0x49, 0xa1, 0xa9, 0x1d, 0x5b, 0x50, 0x5b, 0xee, 0x97, 0x9d, 0x5f,
  0x60, 0x2e, 0x7d, 0x0f, 0xa0, 0x35, 0xc9, 0xa6, 0xd5, 0x24, 0x9b, 0x1a,
  0x0c, 0x2c, 0x47, 0x98, 0x76, 0xf1, 0x00, 0x02, 0x1f, 0xf7, 0xcf, 0x0f,
  0x93, 0x7a, 0xe7, 0x79, 0x43, 0xcb, 0x11, 0x46, 0x5c, 0xd6, 0xc1, 0x20,
  0x77, 0x00, 0xf5, 0xbb, 0xc5, 0x78, 0x7b, 0x77, 0x98, 0x58, 0x92, 0x88,
  0xc8, 0x79, 0x03, 0xd0, 0x7d, 0x5f, 0xa3, 0x7e, 0x5b, 0x83, 0xff, 0xaf,
  0x0f, 0x4d, 0xaf, 0x47, 0x18, 0x76, 0x4e, 0x00, 0xba, 0xeb, 0x24, 0x89,
  0xb5, 0x0b, 0x09, 0x8e, 0x1d, 0x88, 0x62, 0x3d, 0xc2, 0xb0, 0x6f, 0x04,
  0x10, 0x1c, 0xff, 0x6c, 0xf0, 0xf4, 0xad, 0xaf, 0xbe, 0x88, 0x6a, 0x3d,
  0xc2, 0xb0, 0x74, 0x00, 0xc5, 0x47, 0xda, 0xe8, 0xab, 0x5f, 0xc9, 0x15,
  0xf2, 0x02, 0xcf, 0xc8, 0x38, 0x0e, 0xc1, 0xb8, 0x8a, 0xd0, 0xa7, 0xd1,
  0xe3, 0x2a, 0xc0, 0xc9, 0xce, 0x5b, 0x39, 0xbf, 0x4e, 0x96, 0xa4, 0x2f,
  0x3b, 0x0c, 0xbd, 0x51, 0xe4, 0x86, 0x09, 0x50, 0x36, 0x34, 0xbe, 0x0a,
  0x20, 0xe1, 0x67, 0x65, 0xae, 0x0b, 0x4a, 0x05, 0xd0, 0x1f, 0xf2, 0x09,
  0x43, 0xbe, 0x86, 0x5e, 0x39, 0x29, 0xe9, 0xa2, 0xd2, 0x01, 0x08, 0x3b,
  0xfd, 0x0f, 0x2f, 0x58, 0x5c, 0x7d, 0x41, 0x39, 0xdd, 0xd2, 0x00, 0x00,
  0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};
unsigned int favicon_png_len = 1882;

// *INDENT-ON*

extern WebServer webServer;

void RegisterWebUI()
{
    webServer.on("/", [] {
        webServer.send(200, "text/html", index_html);
    });
    webServer.onNotFound([]() {
        webServer.send(200, "text/html", index_html);
    });
    webServer.on("/favicon.png", [] {
        webServer.send_P(200, PSTR("image/png"), reinterpret_cast<const char *>(favicon_png), favicon_png_len);
    });
    webServer.on("/global.css", [] {
        webServer.send(200, "text/css", global_css);
    });
    webServer.on("/build/bundle.css", [] {
        webServer.send(200, "text/css", build_bundle_css);
    });
    webServer.on("/build/bundle.js", [] {
        webServer.send(200, "text/javascript", build_bundle_js);
    });
}

#endif //ESP32
