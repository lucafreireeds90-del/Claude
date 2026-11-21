document.documentElement.classList.add('js');
const nav=document.getElementById('menu'),b=document.querySelector('.burger');
b.addEventListener('click',()=>{const o=nav.classList.toggle('open');b.setAttribute('aria-expanded',o);b.setAttribute('aria-label',o?'Fechar menu':'Abrir menu')});
nav.addEventListener('click',e=>{if(e.target.closest('a')){nav.classList.remove('open');b.setAttribute('aria-expanded',false)}});
document.querySelectorAll('[role=tab]').forEach(t=>t.addEventListener('click',()=>{
  document.querySelectorAll('[role=tab]').forEach(x=>x.setAttribute('aria-selected',x===t));
  document.querySelectorAll('.panel').forEach(p=>{p.hidden=p.id!=='panel-'+t.dataset.cat;
    if(!p.hidden)p.querySelectorAll('.reveal').forEach(r=>r.classList.add('in'))});
}));
const io='IntersectionObserver'in window?new IntersectionObserver(es=>es.forEach(e=>{if(e.isIntersecting){e.target.classList.add('in');io.unobserve(e.target)}}),{threshold:.12}):null;
document.querySelectorAll('.reveal').forEach(el=>io?io.observe(el):el.classList.add('in'));
document.getElementById('ano').textContent=new Date().getFullYear();
