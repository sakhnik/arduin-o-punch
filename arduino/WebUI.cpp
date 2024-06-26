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
var app=function(){"use strict";function t(){}function n(t){return t()}function e(){return Object.create(null)}function o(t){t.forEach(n)}function r(t){return"function"==typeof t}function c(t,n){return t!=t?n==n:t!==n||t&&"object"==typeof t||"function"==typeof t}function u(t,n,e,o){return t[1]&&o?function(t,n){for(const e in n)t[e]=n[e];return t}(e.ctx.slice(),t[1](o(n))):e.ctx}function i(t,n){t.appendChild(n)}function l(t,n,e){t.insertBefore(n,e||null)}function s(t){t.parentNode&&t.parentNode.removeChild(t)}function a(t,n){for(let e=0;e<t.length;e+=1)t[e]&&t[e].d(n)}function d(t){return document.createElement(t)}function f(t){return document.createTextNode(t)}function p(){return f(" ")}function m(t,n,e,o){return t.addEventListener(n,e,o),()=>t.removeEventListener(n,e,o)}function h(t){return function(n){return n.preventDefault(),t.call(this,n)}}function $(t,n,e){null==e?t.removeAttribute(n):t.getAttribute(n)!==e&&t.setAttribute(n,e)}function g(t){return""===t?null:+t}function y(t,n){n=""+n,t.data!==n&&(t.data=n)}function v(t,n){t.value=null==n?"":n}function b(t,n,e,o){null==e?t.style.removeProperty(n):t.style.setProperty(n,e,o?"important":"")}function x(t,n,e){for(let e=0;e<t.options.length;e+=1){const o=t.options[e];if(o.__value===n)return void(o.selected=!0)}e&&void 0===n||(t.selectedIndex=-1)}function w(t,n,e){t.classList[e?"add":"remove"](n)}let k;function _(t){k=t}function O(){if(!k)throw new Error("Function called outside component initialization");return k}function M(t){O().$$.on_mount.push(t)}const C=[],T=[];let L=[];const E=[],S=Promise.resolve();let H=!1;function P(t){L.push(t)}function A(t){E.push(t)}const N=new Set;let I=0;function z(){if(0!==I)return;const t=k;do{try{for(;I<C.length;){const t=C[I];I++,_(t),D(t.$$)}}catch(t){throw C.length=0,I=0,t}for(_(null),C.length=0,I=0;T.length;)T.pop()();for(let t=0;t<L.length;t+=1){const n=L[t];N.has(n)||(N.add(n),n())}L.length=0}while(C.length);for(;E.length;)E.pop()();H=!1,N.clear(),_(t)}function D(t){if(null!==t.fragment){t.update(),o(t.before_update);const n=t.dirty;t.dirty=[-1],t.fragment&&t.fragment.p(t.ctx,n),t.after_update.forEach(P)}}const B=new Set;let j;function q(t,n){t&&t.i&&(B.delete(t),t.i(n))}function U(t,n,e,o){if(t&&t.o){if(B.has(t))return;B.add(t),j.c.push((()=>{B.delete(t),o&&(e&&t.d(1),o())})),t.o(n)}else o&&o()}function F(t,n,e){const o=t.$$.props[n];void 0!==o&&(t.$$.bound[o]=e,e(t.$$.ctx[o]))}function R(t){t&&t.c()}function X(t,e,c,u){const{fragment:i,after_update:l}=t.$$;i&&i.m(e,c),u||P((()=>{const e=t.$$.on_mount.map(n).filter(r);t.$$.on_destroy?t.$$.on_destroy.push(...e):o(e),t.$$.on_mount=[]})),l.forEach(P)}function G(t,n){const e=t.$$;null!==e.fragment&&(!function(t){const n=[],e=[];L.forEach((o=>-1===t.indexOf(o)?n.push(o):e.push(o))),e.forEach((t=>t())),L=n}(e.after_update),o(e.on_destroy),e.fragment&&e.fragment.d(n),e.on_destroy=e.fragment=null,e.ctx=[])}function J(t,n){-1===t.$$.dirty[0]&&(C.push(t),H||(H=!0,S.then(z)),t.$$.dirty.fill(0)),t.$$.dirty[n/31|0]|=1<<n%31}function K(n,r,c,u,i,l,a,d=[-1]){const f=k;_(n);const p=n.$$={fragment:null,ctx:[],props:l,update:t,not_equal:i,bound:e(),on_mount:[],on_destroy:[],on_disconnect:[],before_update:[],after_update:[],context:new Map(r.context||(f?f.$$.context:[])),callbacks:e(),dirty:d,skip_bound:!1,root:r.target||f.$$.root};a&&a(p.root);let m=!1;if(p.ctx=c?c(n,r.props||{},((t,e,...o)=>{const r=o.length?o[0]:e;return p.ctx&&i(p.ctx[t],p.ctx[t]=r)&&(!p.skip_bound&&p.bound[t]&&p.bound[t](r),m&&J(n,t)),e})):[],p.update(),m=!0,o(p.before_update),p.fragment=!!u&&u(p.ctx),r.target){if(r.hydrate){const t=function(t){return Array.from(t.childNodes)}(r.target);p.fragment&&p.fragment.l(t),t.forEach(s)}else p.fragment&&p.fragment.c();r.intro&&q(n.$$.fragment),X(n,r.target,r.anchor,r.customElement),z()}_(f)}class Q{$destroy(){G(this,1),this.$destroy=t}$on(n,e){if(!r(e))return t;const o=this.$$.callbacks[n]||(this.$$.callbacks[n]=[]);return o.push(e),()=>{const t=o.indexOf(e);-1!==t&&o.splice(t,1)}}$set(t){var n;this.$$set&&(n=t,0!==Object.keys(n).length)&&(this.$$.skip_bound=!0,this.$$set(t),this.$$.skip_bound=!1)}}function V(n){let e,r,c,u,a,g,v;return{c(){e=d("span"),r=d("input"),c=p(),u=d("button"),a=f(n[3]),$(r,"type",n[4]),r.value=n[0],$(r,"id",n[1]),$(r,"name","key"),$(u,"class","toggle-key")},m(t,o){l(t,e,o),i(e,r),i(e,c),i(e,u),i(u,a),g||(v=[m(r,"input",n[5]),m(u,"click",h(n[6]))],g=!0)},p(t,[n]){16&n&&$(r,"type",t[4]),1&n&&r.value!==t[0]&&(r.value=t[0]),2&n&&$(r,"id",t[1]),8&n&&y(a,t[3])},i:t,o:t,d(t){t&&s(e),g=!1,o(v)}}}function W(t,n,e){let o,r,{id:c="key"}=n,{value:u="112233445566"}=n,i=!1;return t.$$set=t=>{"id"in t&&e(1,c=t.id),"value"in t&&e(0,u=t.value)},t.$$.update=()=>{4&t.$$.dirty&&e(4,o=i?"text":"password"),4&t.$$.dirty&&e(3,r=i?"••":"👁️")},[u,c,i,r,o,function(t){e(0,u=t.target.value)},()=>e(2,i=!i)]}class Y extends Q{constructor(t){super(),K(this,t,W,V,c,{id:1,value:0})}}function Z(t,n,e){const o=t.slice();return o[11]=n[e],o}function tt(n){let e,o,r,c=n[11].text+"";return{c(){e=d("option"),o=f(c),r=p(),e.__value=n[11].id,e.value=e.__value},m(t,n){l(t,e,n),i(e,o),i(e,r)},p:t,d(t){t&&s(e)}}}function nt(t){let n,e,r,c,u,f,h,y,b,w,k,_,O,M,C,L,E,S,H,N,I,z,D,B,j,J,K,Q,V,W,nt,et,ot,rt,ct,ut,it,lt,st,at;function dt(n){t[7](n)}let ft={id:"key"};void 0!==t[1]&&(ft.value=t[1]),L=new Y({props:ft}),T.push((()=>F(L,"value",dt)));let pt=t[5],mt=[];for(let n=0;n<pt.length;n+=1)mt[n]=tt(Z(t,pt,n));return{c(){n=d("div"),e=d("h3"),e.textContent="Налаштування",r=p(),c=d("form"),u=d("table"),f=d("tr"),h=d("td"),h.innerHTML='<label for="id">Номер:</label>',y=p(),b=d("td"),w=d("input"),k=p(),_=d("tr"),O=d("td"),O.innerHTML='<label for="key">Ключ:</label>',M=p(),C=d("td"),R(L.$$.fragment),S=p(),H=d("tr"),N=d("td"),N.innerHTML='<label for="record-size">Записів у журналі:</label>',I=p(),z=d("td"),D=d("input"),B=p(),j=d("tr"),J=d("td"),J.innerHTML='<label for="record-bits">Відміток на запис:</label>',K=p(),Q=d("td"),V=d("select");for(let t=0;t<mt.length;t+=1)mt[t].c();W=p(),nt=d("tr"),et=d("td"),et.innerHTML='<label for="record-days">Період журналу (днів):</label>',ot=p(),rt=d("td"),ct=d("input"),ut=p(),it=d("tr"),it.innerHTML='<td></td> \n        <td><input type="submit" value="Застосувати"/></td>',$(w,"type","number"),$(w,"id","id"),$(w,"name","id"),$(w,"min","1"),$(w,"max","255"),$(D,"type","number"),$(D,"id","record-size"),$(D,"name","rec-size"),$(V,"id","record-bits"),$(V,"name","rec-bits"),void 0===t[3]&&P((()=>t[9].call(V))),$(ct,"type","number"),$(ct,"id","record-days"),$(ct,"name","rec-days"),$(c,"method","POST"),$(c,"action","/settings"),$(c,"id","settings-form"),$(c,"target","_self")},m(o,s){l(o,n,s),i(n,e),i(n,r),i(n,c),i(c,u),i(u,f),i(f,h),i(f,y),i(f,b),i(b,w),v(w,t[0]),i(u,k),i(u,_),i(_,O),i(_,M),i(_,C),X(L,C,null),i(u,S),i(u,H),i(H,N),i(H,I),i(H,z),i(z,D),v(D,t[2]),i(u,B),i(u,j),i(j,J),i(j,K),i(j,Q),i(Q,V);for(let t=0;t<mt.length;t+=1)mt[t]&&mt[t].m(V,null);x(V,t[3],!0),i(u,W),i(u,nt),i(nt,et),i(nt,ot),i(nt,rt),i(rt,ct),v(ct,t[4]),i(u,ut),i(u,it),lt=!0,st||(at=[m(w,"input",t[6]),m(D,"input",t[8]),m(V,"change",t[9]),m(ct,"input",t[10])],st=!0)},p(t,[n]){1&n&&g(w.value)!==t[0]&&v(w,t[0]);const e={};if(!E&&2&n&&(E=!0,e.value=t[1],A((()=>E=!1))),L.$set(e),4&n&&g(D.value)!==t[2]&&v(D,t[2]),32&n){let e;for(pt=t[5],e=0;e<pt.length;e+=1){const o=Z(t,pt,e);mt[e]?mt[e].p(o,n):(mt[e]=tt(o),mt[e].c(),mt[e].m(V,null))}for(;e<mt.length;e+=1)mt[e].d(1);mt.length=pt.length}40&n&&x(V,t[3]),16&n&&g(ct.value)!==t[4]&&v(ct,t[4])},i(t){lt||(q(L.$$.fragment,t),lt=!0)},o(t){U(L.$$.fragment,t),lt=!1},d(t){t&&s(n),G(L),a(mt,t),st=!1,o(at)}}}function et(t,n,e){let o=19,r="112233445566",c=512,u=2,i=1,l=[{id:1,text:"0–1"},{id:2,text:"0–3"},{id:4,text:"0–15"},{id:8,text:"0–255"}];return M((async()=>{const t=await fetch("/settings");(await t.text()).trim().split("\n").forEach((t=>{const[n,l]=t.split("=");"id"==n?(e(0,o=Number(l)),document.title="AOP "+l):"key"==n?e(1,r=l):"rec-size"==n?e(2,c=Number(l)):"rec-bits"==n?e(3,u=Number(l)):"rec-days"==n&&e(4,i=Number(l))}))})),[o,r,c,u,i,l,function(){o=g(this.value),e(0,o)},function(t){r=t,e(1,r)},function(){c=g(this.value),e(2,c)},function(){u=function(t){const n=t.querySelector(":checked");return n&&n.__value}(this),e(3,u),e(5,l)},function(){i=g(this.value),e(4,i)}]}class ot extends Q{constructor(t){super(),K(this,t,et,nt,c,{})}}function rt(n){let e,o,r,c,u,a,f,g,y,v;return{c(){e=d("div"),o=d("h3"),o.textContent="Оновлення",r=p(),c=d("form"),c.innerHTML='<input type="file" id="file" name="update" accept=".esp32.bin"/> \n    <input type="submit" value="Завантажити"/>',u=p(),a=d("br"),f=p(),g=d("div"),g.textContent="0%",$(c,"method","POST"),$(c,"enctype","multipart/form-data"),$(c,"id","upload-form"),$(g,"id","prg"),b(g,"width","0"),b(g,"color","white"),b(g,"text-align","center"),$(e,"class","container-upgrade")},m(t,n){l(t,e,n),i(e,o),i(e,r),i(e,c),i(e,u),i(e,a),i(e,f),i(e,g),y||(v=m(c,"submit",h(ct)),y=!0)},p:t,i:t,o:t,d(t){t&&s(e),y=!1,v()}}}function ct(){let t=document.getElementById("upload-form"),n=document.getElementById("prg");n.style.backgroundColor="blue";var e=new FormData(t),o=new XMLHttpRequest,r=document.getElementById("file").files[0].size;o.open("POST","/update?size="+r),o.upload.addEventListener("progress",(t=>{console.log("progress "+t.loaded+" "+t.total);let e=Math.round(t.loaded/t.total*100)+"%";t.lengthComputable&&(n.innerHTML=e,n.style.width=e),"100%"==e&&(n.style.backgroundColor="black")})),o.send(e)}class ut extends Q{constructor(t){super(),K(this,t,null,rt,c,{})}}function it(n){let e,r,c,a,f,h,g;const y=n[2].default,v=function(t,n,e,o){if(t){const r=u(t,n,e,o);return t[0](r)}}(y,n,n[1],null),b=v||function(n){let e;return{c(){e=d("p"),e.textContent="Popup content"},m(t,n){l(t,e,n)},p:t,d(t){t&&s(e)}}}();return{c(){e=d("div"),r=d("div"),b&&b.c(),c=p(),a=d("button"),a.textContent="Закрити",$(a,"id","closeBtn"),$(a,"class","svelte-12tp13r"),$(r,"class","popup-content svelte-12tp13r"),$(e,"id","popup"),$(e,"class","popup svelte-12tp13r"),w(e,"show",n[0])},m(t,o){l(t,e,o),i(e,r),b&&b.m(r,null),i(r,c),i(r,a),f=!0,h||(g=[m(window,"keydown",n[3]),m(a,"click",n[4])],h=!0)},p(t,[n]){v&&v.p&&(!f||2&n)&&function(t,n,e,o,r,c){if(r){const i=u(n,e,o,c);t.p(i,r)}}(v,y,t,t[1],f?function(t,n,e,o){if(t[2]&&o){const r=t[2](o(e));if(void 0===n.dirty)return r;if("object"==typeof r){const t=[],e=Math.max(n.dirty.length,r.length);for(let o=0;o<e;o+=1)t[o]=n.dirty[o]|r[o];return t}return n.dirty|r}return n.dirty}(y,t[1],n,null):function(t){if(t.ctx.length>32){const n=[],e=t.ctx.length/32;for(let t=0;t<e;t++)n[t]=-1;return n}return-1}(t[1]),null),(!f||1&n)&&w(e,"show",t[0])},i(t){f||(q(b,t),f=!0)},o(t){U(b,t),f=!1},d(t){t&&s(e),b&&b.d(t),h=!1,o(g)}}}function lt(t,n,e){let{$$slots:o={},$$scope:r}=n,{isOpen:c=!1}=n;return t.$$set=t=>{"isOpen"in t&&e(0,c=t.isOpen),"$$scope"in t&&e(1,r=t.$$scope)},[c,r,o,()=>e(0,c=!1),()=>e(0,c=!1)]}class st extends Q{constructor(t){super(),K(this,t,lt,it,c,{isOpen:0})}}function at(t,n,e){const o=t.slice();return o[5]=n[e],o}function dt(t){let n,e,o,r,c=t[2],u=[];for(let n=0;n<c.length;n+=1)u[n]=ht(at(t,c,n));return{c(){n=d("table"),e=d("thead"),e.innerHTML='<tr><th class="svelte-wdax5m">Картка</th><th class="svelte-wdax5m">Кількість</th></tr>',o=p(),r=d("tbody");for(let t=0;t<u.length;t+=1)u[t].c();$(n,"class","svelte-wdax5m")},m(t,c){l(t,n,c),i(n,e),i(n,o),i(n,r);for(let t=0;t<u.length;t+=1)u[t]&&u[t].m(r,null)},p(t,n){if(4&n){let e;for(c=t[2],e=0;e<c.length;e+=1){const o=at(t,c,e);u[e]?u[e].p(o,n):(u[e]=ht(o),u[e].c(),u[e].m(r,null))}for(;e<u.length;e+=1)u[e].d(1);u.length=c.length}},d(t){t&&s(n),a(u,t)}}}function ft(n){let e;return{c(){e=d("p"),e.textContent="Відміток ще не було"},m(t,n){l(t,e,n)},p:t,d(t){t&&s(e)}}}function pt(t){let n,e,o;return{c(){n=d("p"),e=f("Помилка: "),o=f(t[1])},m(t,r){l(t,n,r),i(n,e),i(n,o)},p(t,n){2&n&&y(o,t[1])},d(t){t&&s(n)}}}function mt(n){let e;return{c(){e=d("p"),e.textContent="Завантаження…"},m(t,n){l(t,e,n)},p:t,d(t){t&&s(e)}}}function ht(t){let n,e,o,r,c,u,a,m=t[5].card+"",h=t[5].count+"";return{c(){n=d("tr"),e=d("td"),o=f(m),r=p(),c=d("td"),u=f(h),a=p(),$(e,"class","svelte-wdax5m"),$(c,"class","svelte-wdax5m")},m(t,s){l(t,n,s),i(n,e),i(e,o),i(n,r),i(n,c),i(c,u),i(n,a)},p(t,n){4&n&&m!==(m=t[5].card+"")&&y(o,m),4&n&&h!==(h=t[5].count+"")&&y(u,h)},d(t){t&&s(n)}}}function $t(n){let e,o,r;function c(t,n){return t[0]?mt:t[1]?pt:0===t[2].length?ft:dt}let u=c(n),a=u(n);return{c(){e=d("div"),o=d("p"),o.textContent="Журнал відмітки. Натисність ESC щоб закрити.",r=p(),a.c(),$(e,"class","container svelte-wdax5m")},m(t,n){l(t,e,n),i(e,o),i(e,r),a.m(e,null)},p(t,[n]){u===(u=c(t))&&a?a.p(t,n):(a.d(1),a=u(t),a&&(a.c(),a.m(e,null)))},i:t,o:t,d(t){t&&s(e),a.d()}}}function gt(t,n,e){let{isOpen:o=!1}=n,r=!0,c=null,u={};const i=async()=>{try{const n=await fetch("/record");if(!n.ok)throw new Error("Не вдалося отримати дані");e(2,(t=await n.text(),u=t?t.split(" ").map((t=>{const[n,e]=t.split(":");return{card:parseInt(n,10),count:parseInt(e,10)}})):[]))}catch(t){e(1,c=t.message)}finally{e(0,r=!1)}var t};return t.$$set=t=>{"isOpen"in t&&e(3,o=t.isOpen)},t.$$.update=()=>{8&t.$$.dirty&&o&&i()},[r,c,u,o]}class yt extends Q{constructor(t){super(),K(this,t,gt,$t,c,{isOpen:3})}}function vt(t){let n,e,o,r=t[2].toLocaleDateString("uk-UA")+"",c=(t[1]>t[3]||t[1]<-t[3])&&xt(t);return{c(){n=f(r),e=p(),c&&c.c(),o=f("")},m(t,r){l(t,n,r),l(t,e,r),c&&c.m(t,r),l(t,o,r)},p(t,e){4&e&&r!==(r=t[2].toLocaleDateString("uk-UA")+"")&&y(n,r),t[1]>t[3]||t[1]<-t[3]?c?c.p(t,e):(c=xt(t),c.c(),c.m(o.parentNode,o)):c&&(c.d(1),c=null)},d(t){t&&s(n),t&&s(e),c&&c.d(t),t&&s(o)}}}function bt(n){let e;return{c(){e=f("…")},m(t,n){l(t,e,n)},p:t,d(t){t&&s(e)}}}function xt(n){let e,o,r,c=n[6]()+"";return{c(){e=f("("),o=f(c),r=f(" днів)")},m(t,n){l(t,e,n),l(t,o,n),l(t,r,n)},p:t,d(t){t&&s(e),t&&s(o),t&&s(r)}}}function wt(t){let n,e,o,r,c,u,i=t[2].toLocaleTimeString("uk-UA")+"",a=t[5]()+"",d=Math.floor(t[0])+"";return{c(){n=f(i),e=f(" ("),o=f(a),r=f(" ±"),c=f(d),u=f(" мс)")},m(t,i){l(t,n,i),l(t,e,i),l(t,o,i),l(t,r,i),l(t,c,i),l(t,u,i)},p(t,e){4&e&&i!==(i=t[2].toLocaleTimeString("uk-UA")+"")&&y(n,i),1&e&&d!==(d=Math.floor(t[0])+"")&&y(c,d)},d(t){t&&s(n),t&&s(e),t&&s(o),t&&s(r),t&&s(c),t&&s(u)}}}function kt(n){let e;return{c(){e=f("…")},m(t,n){l(t,e,n)},p:t,d(t){t&&s(e)}}}function _t(n){let e,r,c,u,a,h,$,g,y,v,b,x,w,k;function _(t,n){return t[1]&&t[2]?vt:bt}let O=_(n),M=O(n);function C(t,n){return t[1]&&t[2]?wt:kt}let T=C(n),L=T(n);return{c(){e=d("div"),r=d("h3"),r.textContent="Годинник",c=p(),u=d("p"),a=f("Дата:\n"),M.c(),h=p(),$=d("p"),g=f("Час:\n"),L.c(),y=p(),v=d("button"),v.textContent="Звірити",b=p(),x=d("button"),x.textContent="Підвести"},m(t,o){l(t,e,o),i(e,r),i(e,c),i(e,u),i(u,a),M.m(u,null),i(e,h),i(e,$),i($,g),L.m($,null),i(e,y),i(e,v),i(e,b),i(e,x),w||(k=[m(v,"click",n[4]),m(x,"click",n[7])],w=!0)},p(t,[n]){O===(O=_(t))&&M?M.p(t,n):(M.d(1),M=O(t),M&&(M.c(),M.m(u,null))),T===(T=C(t))&&L?L.p(t,n):(L.d(1),L=T(t),L&&(L.c(),L.m($,null)))},i:t,o:t,d(t){t&&s(e),M.d(),L.d(),w=!1,o(k)}}}function Ot(t){return t.getTime()-6e4*t.getTimezoneOffset()}function Mt(t){let n=t%1e3;if(!(t=Math.round(t/1e3)))return"0."+n+" с";let e=t%60;t=Math.round(t/60);let o=Math.round(n/100);if(!t)return e+"."+o+" с";let r=t%60,c=Math.round(t/60);return e<10&&(e="0"+e),c?(r<10&&(r="0"+r),c+":"+r+":"+e):r+":"+e}function Ct(t){return new Promise((n=>setTimeout(n,t)))}function Tt(t,n,e){const o=864e5;let r,c,u,i;function l(){null!==c&&e(2,u=new Date(Date.now()+c))}var s;M((async()=>(i=setInterval(l,1e3),d(),()=>{clearInterval(i)}))),s=()=>{clearInterval(i)},O().$$.on_destroy.push(s);const a=async function(){const t=await fetch("/clock"),n=await t.text();return Number(n.trim())},d=async function(){e(1,c=null),e(0,r=null);let t=await a();for(;;){let n=performance.now(),o=await a(),u=performance.now();if(o!=t){e(0,r=.5*(u-n)),o=1e3*o+r;let t=Ot(new Date);e(1,c=Math.round(o-t));break}}};return[r,c,u,o,d,function(){const t=c%o;return t>0?"+"+Mt(t):"−"+Mt(-t)},function(){const t=Math.round(c/o);return t>0?"+"+t:t},async function(){let t=r;e(1,c=null),e(0,r=null);let n=Ot(new Date),o=Math.floor(n/1e3)+1,u=1e3-n%1e3;u<t&&(u+=1e3,o+=1);let i=performance.now(),l=u-t,s=l-50;for(s>0&&await Ct(s);performance.now()-i<l;);await fetch("/clock",{method:"POST",headers:{"Content-Type":"text/plain"},body:String(o)}),await Ct(750),await d()}]}class Lt extends Q{constructor(t){super(),K(this,t,Tt,_t,c,{})}}function Et(t){let n,e,o;function r(n){t[1](n)}let c={};return void 0!==t[0]&&(c.isOpen=t[0]),n=new yt({props:c}),T.push((()=>F(n,"isOpen",r))),{c(){R(n.$$.fragment)},m(t,e){X(n,t,e),o=!0},p(t,o){const r={};!e&&1&o&&(e=!0,r.isOpen=t[0],A((()=>e=!1))),n.$set(r)},i(t){o||(q(n.$$.fragment,t),o=!0)},o(t){U(n.$$.fragment,t),o=!1},d(t){G(n,t)}}}function St(t){let n,e,o,r,c,u,a,f,h,g,y,v,b,x,w,k,_,O,M,C,L;function E(n){t[2](n)}u=new ot({}),f=new Lt({});let S={$$slots:{default:[Et]},$$scope:{ctx:t}};return void 0!==t[0]&&(S.isOpen=t[0]),g=new st({props:S}),T.push((()=>F(g,"isOpen",E))),O=new ut({}),{c(){n=d("div"),e=d("h1"),e.textContent="Станція Arduin-O-Punch",o=p(),r=d("p"),r.innerHTML="Версія <span>)html" PROJECT_VERSION "-" GIT_REVISION R"html(</span>",c=p(),R(u.$$.fragment),a=p(),R(f.$$.fragment),h=p(),R(g.$$.fragment),v=p(),b=d("div"),x=d("h3"),x.textContent="Різне",w=p(),k=d("button"),k.textContent="Журнал",_=p(),R(O.$$.fragment),$(n,"class","container svelte-al7qx6")},m(s,d){l(s,n,d),i(n,e),i(n,o),i(n,r),i(n,c),X(u,n,null),i(n,a),X(f,n,null),i(n,h),X(g,n,null),i(n,v),i(n,b),i(b,x),i(b,w),i(b,k),i(n,_),X(O,n,null),M=!0,C||(L=m(k,"click",t[3]),C=!0)},p(t,[n]){const e={};17&n&&(e.$$scope={dirty:n,ctx:t}),!y&&1&n&&(y=!0,e.isOpen=t[0],A((()=>y=!1))),g.$set(e)},i(t){M||(q(u.$$.fragment,t),q(f.$$.fragment,t),q(g.$$.fragment,t),q(O.$$.fragment,t),M=!0)},o(t){U(u.$$.fragment,t),U(f.$$.fragment,t),U(g.$$.fragment,t),U(O.$$.fragment,t),M=!1},d(t){t&&s(n),G(u),G(f),G(g),G(O),C=!1,L()}}}function Ht(t,n,e){let o=!1;return[o,function(t){o=t,e(0,o)},function(t){o=t,e(0,o)},()=>e(0,o=!0)]}return new class extends Q{constructor(t){super(),K(this,t,Ht,St,c,{})}}({target:document.body,props:{name:"world"}})}();
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
