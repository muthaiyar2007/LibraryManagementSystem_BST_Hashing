const $ = id => document.getElementById(id);

function showMessage(text, ok=true){
  $("message").innerHTML = `<div class="${ok ? "ok":"err"}">${text}</div>`;
  setTimeout(()=>{$("message").innerHTML=""},3000);
}

async function loadBooks(){
  const res = await fetch("/api/books");
  const books = await res.json();
  renderBooks(books);
}

function renderBooks(books){
  $("total").textContent = books.length;
  $("available").textContent = books.filter(b=>!b.issued).length;
  $("issued").textContent = books.filter(b=>b.issued).length;

  $("bookRows").innerHTML = books.length ? books.map(b => `
    <tr>
      <td><b>${b.id}</b></td>
      <td>${escapeHtml(b.title)}</td>
      <td>${escapeHtml(b.author)}</td>
      <td>${escapeHtml(b.category || "General")}</td>
      <td><span class="badge ${b.issued?"issued":"available"}">${b.issued?"Issued":"Available"}</span></td>
      <td class="actions">
        ${b.issued
          ? `<button class="return" onclick="returnBook(${b.id})">Return</button>`
          : `<button onclick="issueBook(${b.id})">Issue</button>`}
        <button class="danger" onclick="deleteBook(${b.id})">Delete</button>
      </td>
    </tr>`).join("")
    : `<tr><td colspan="6" style="text-align:center;padding:30px;color:#777">No books found. Add your first book.</td></tr>`;
}

function escapeHtml(s){
  return String(s).replace(/[&<>"']/g,m=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#039;"}[m]));
}

function openAdd(){ $("modal").style.display="flex"; }
function closeModal(){ $("modal").style.display="none"; }

$("bookForm").addEventListener("submit", async e=>{
  e.preventDefault();
  const body = new URLSearchParams(new FormData(e.target));
  const res = await fetch("/api/books",{method:"POST",body});
  const data = await res.json();
  showMessage(data.message,data.success);
  if(data.success){ e.target.reset(); closeModal(); loadBooks(); }
});

async function searchBook(){
  const id = $("search").value.trim();
  if(!id){loadBooks();return}
  const res = await fetch("/api/books/"+id);
  const data = await res.json();
  if(data.success===false){ renderBooks([]); showMessage(data.message,false); return; }
  renderBooks([data]);
  showMessage("Found using Hash Table.");
}

async function deleteBook(id){
  if(!confirm("Delete Book ID "+id+"?")) return;
  const res = await fetch("/api/books/"+id,{method:"DELETE"});
  const data = await res.json();
  showMessage(data.message,data.success);
  loadBooks();
}

async function issueBook(id){
  const memberId = prompt("Enter Member ID:");
  if(!memberId) return;
  const body = new URLSearchParams({memberId});
  const res = await fetch("/api/issue/"+id,{method:"POST",body});
  const data = await res.json();
  showMessage(data.message,data.success);
  loadBooks();
}

async function returnBook(id){
  const res = await fetch("/api/return/"+id,{method:"POST"});
  const data = await res.json();
  showMessage(data.message,data.success);
  loadBooks();
}

loadBooks();
