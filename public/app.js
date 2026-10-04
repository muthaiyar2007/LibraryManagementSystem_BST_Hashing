const $ = id => document.getElementById(id);

function showMessage(text, ok = true) {
    $("message").innerHTML =
        `<div class="${ok ? "ok" : "err"}">${text}</div>`;

    setTimeout(() => {
        $("message").innerHTML = "";
    }, 3000);
}

// ==================== LOAD ALL BOOKS ====================

async function loadBooks() {
    try {
        const res = await fetch("/api/books");
        const books = await res.json();
        renderBooks(books);
    } catch (error) {
        console.error(error);
        showMessage("Failed to load books.", false);
    }
}

// ==================== RENDER BOOKS ====================

function renderBooks(books) {
    $("total").textContent = books.length;

    $("available").textContent =
        books.filter(b => !b.issued).length;

    $("issued").textContent =
        books.filter(b => b.issued).length;

    $("bookRows").innerHTML = books.length
        ? books.map(b => `
            <tr>
                <td><b>${b.id}</b></td>

                <td>${escapeHtml(b.title)}</td>

                <td>${escapeHtml(b.author)}</td>

                <td>${escapeHtml(b.category || "General")}</td>

                <td>
                    <span class="badge ${b.issued ? "issued" : "available"}">
                        ${b.issued ? "Issued" : "Available"}
                    </span>
                </td>

                <td class="actions">
                    ${
                        b.issued
                        ? `<button class="return"
                            onclick="returnBook(${b.id})">
                            Return
                           </button>`
                        : `<button
                            onclick="issueBook(${b.id})">
                            Issue
                           </button>`
                    }

                    <button class="danger"
                        onclick="deleteBook(${b.id})">
                        Delete
                    </button>
                </td>
            </tr>
        `).join("")
        : `
            <tr>
                <td colspan="6"
                    style="text-align:center;padding:30px;color:#777">
                    No books found. Add your first book.
                </td>
            </tr>
        `;
}

// ==================== HTML ESCAPE ====================

function escapeHtml(s) {
    return String(s).replace(
        /[&<>"']/g,
        m => ({
            "&": "&amp;",
            "<": "&lt;",
            ">": "&gt;",
            '"': "&quot;",
            "'": "&#039;"
        }[m])
    );
}

// ==================== ADD BOOK MODAL ====================

function openAdd() {
    $("modal").style.display = "flex";
}

function closeModal() {
    $("modal").style.display = "none";
}

// ==================== ADD BOOK ====================

$("bookForm").addEventListener("submit", async e => {
    e.preventDefault();

    try {
        const body = new URLSearchParams(
            new FormData(e.target)
        );

        // IMPORTANT:
        // C++ server expects /api/add
        const res = await fetch("/api/add", {
            method: "POST",
            headers: {
                "Content-Type":
                    "application/x-www-form-urlencoded"
            },
            body: body
        });

        const data = await res.json();

        showMessage(data.message, data.success);

        if (data.success) {
            e.target.reset();
            closeModal();
            await loadBooks();
        }

    } catch (error) {
        console.error(error);
        showMessage("Add Book failed.", false);
    }
});

// ==================== SEARCH BOOK ====================

async function searchBook() {
    const id = $("search").value.trim();

    if (!id) {
        loadBooks();
        return;
    }

    try {
        // IMPORTANT:
        // C++ server expects /api/search?id=...
        const res = await fetch(
            "/api/search?id=" + encodeURIComponent(id)
        );

        const data = await res.json();

        if (!data.success) {
            renderBooks([]);
            showMessage(data.message || "Book not found.", false);
            return;
        }

        renderBooks([data.book || data]);

        showMessage("Found using Hash Table.");

    } catch (error) {
        console.error(error);
        showMessage("Search failed.", false);
    }
}

// ==================== DELETE BOOK ====================

async function deleteBook(id) {
    if (!confirm("Delete Book ID " + id + "?"))
        return;

    try {
        // IMPORTANT:
        // C++ server expects POST /api/delete
        const body = new URLSearchParams({
            id: id
        });

        const res = await fetch("/api/delete", {
            method: "POST",
            headers: {
                "Content-Type":
                    "application/x-www-form-urlencoded"
            },
            body: body
        });

        const data = await res.json();

        showMessage(data.message, data.success);

        if (data.success) {
            await loadBooks();
        }

    } catch (error) {
        console.error(error);
        showMessage("Delete failed.", false);
    }
}

// ==================== ISSUE BOOK ====================

async function issueBook(id) {
    const memberId = prompt("Enter Member ID:");

    if (!memberId)
        return;

    try {
        // C++ expects:
        // id + memberId
        const body = new URLSearchParams({
            id: id,
            memberId: memberId
        });

        const res = await fetch("/api/issue", {
            method: "POST",
            headers: {
                "Content-Type":
                    "application/x-www-form-urlencoded"
            },
            body: body
        });

        const data = await res.json();

        showMessage(data.message, data.success);

        if (data.success) {
            await loadBooks();
        }

    } catch (error) {
        console.error(error);
        showMessage("Issue failed.", false);
    }
}

// ==================== RETURN BOOK ====================

async function returnBook(id) {
    try {
        // C++ expects POST /api/return
        const body = new URLSearchParams({
            id: id
        });

        const res = await fetch("/api/return", {
            method: "POST",
            headers: {
                "Content-Type":
                    "application/x-www-form-urlencoded"
            },
            body: body
        });

        const data = await res.json();

        showMessage(data.message, data.success);

        if (data.success) {
            await loadBooks();
        }

    } catch (error) {
        console.error(error);
        showMessage("Return failed.", false);
    }
}

// ==================== START ====================

loadBooks();