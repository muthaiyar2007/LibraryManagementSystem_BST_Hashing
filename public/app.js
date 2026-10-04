javascript
const $ = id => document.getElementById(id);

// ============================================================
// MESSAGE
// ============================================================

function showMessage(text, ok = true) {
    $("message").innerHTML =
        `<div class="${ok ? "ok" : "err"}">${escapeHtml(text)}</div>`;

    setTimeout(() => {
        $("message").innerHTML = "";
    }, 3000);
}

// ============================================================
// LOAD ALL BOOKS
// ============================================================

async function loadBooks() {

    try {

        const res =
            await fetch("/api/books");

        const books =
            await res.json();

        renderBooks(books);

    }
    catch (error) {

        console.error(error);

        showMessage(
            "Unable to load books",
            false
        );
    }
}

// ============================================================
// RENDER BOOKS
// ============================================================

function renderBooks(books) {

    $("total").textContent =
        books.length;

    $("available").textContent =
        books.filter(
            b => !b.issued
        ).length;

    $("issued").textContent =
        books.filter(
            b => b.issued
        ).length;

    $("bookRows").innerHTML =
        books.length

        ? books.map(b => `

            <tr>

                <td>
                    <b>${b.id}</b>
                </td>

                <td>
                    ${escapeHtml(b.title)}
                </td>

                <td>
                    ${escapeHtml(b.author)}
                </td>

                <td>
                    ${escapeHtml(
                        b.category || "General"
                    )}
                </td>

                <td>

                    <span class="badge ${
                        b.issued
                            ? "issued"
                            : "available"
                    }">

                        ${
                            b.issued
                                ? "Issued"
                                : "Available"
                        }

                    </span>

                    ${
                        b.issued && b.memberId
                            ? `<small>Member: ${escapeHtml(b.memberId)}</small>`
                            : ""
                    }

                </td>

                <td class="actions">

                    ${
                        b.issued

                        ? `
                            <button
                                class="return"
                                onclick="returnBook(${b.id})"
                            >
                                Return
                            </button>
                          `

                        : `
                            <button
                                onclick="issueBook(${b.id})"
                            >
                                Issue
                            </button>
                          `
                    }

                    <button
                        class="danger"
                        onclick="deleteBook(${b.id})"
                    >
                        Delete
                    </button>

                </td>

            </tr>

        `).join("")

        : `
            <tr>

                <td
                    colspan="6"
                    style="
                        text-align:center;
                        padding:30px;
                        color:#777
                    "
                >
                    No books found.
                    Add your first book.
                </td>

            </tr>
          `;
}

// ============================================================
// ESCAPE HTML
// ============================================================

function escapeHtml(value) {

    return String(value ?? "")
        .replace(
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

// ============================================================
// MODAL
// ============================================================

function openAdd() {

    $("modal").style.display =
        "flex";
}

function closeModal() {

    $("modal").style.display =
        "none";
}

// ============================================================
// ADD BOOK
// POST /api/books
// ============================================================

$("bookForm").addEventListener(
    "submit",
    async e => {

        e.preventDefault();

        try {

            const body =
                new URLSearchParams(
                    new FormData(e.target)
                );

            const res =
                await fetch(
                    "/api/books",
                    {
                        method: "POST",
                        headers: {
                            "Content-Type":
                                "application/x-www-form-urlencoded"
                        },
                        body: body
                    }
                );

            const data =
                await res.json();

            showMessage(
                data.message,
                data.success
            );

            if (data.success) {

                e.target.reset();

                closeModal();

                await loadBooks();
            }

        }
        catch (error) {

            console.error(error);

            showMessage(
                "Add book failed",
                false
            );
        }
    }
);

// ============================================================
// SEARCH BOOK
// GET /api/books/:id
// ============================================================

async function searchBook() {

    const id =
        $("search").value.trim();

    if (!id) {

        loadBooks();

        return;
    }

    try {

        const res =
            await fetch(
                `/api/books/${encodeURIComponent(id)}`
            );

        const data =
            await res.json();

        if (data.success === false) {

            renderBooks([]);

            showMessage(
                data.message,
                false
            );

            return;
        }

        renderBooks([data]);

        showMessage(
            "Book found using Hash Table."
        );

    }
    catch (error) {

        console.error(error);

        showMessage(
            "Search failed",
            false
        );
    }
}

// ============================================================
// DELETE BOOK
// DELETE /api/books/:id
// ============================================================

async function deleteBook(id) {

    if (
        !confirm(
            "Delete Book ID " +
            id +
            "?"
        )
    )
        return;

    try {

        const res =
            await fetch(
                `/api/books/${id}`,
                {
                    method: "DELETE"
                }
            );

        const data =
            await res.json();

        showMessage(
            data.message,
            data.success
        );

        if (data.success)
            await loadBooks();

    }
    catch (error) {

        console.error(error);

        showMessage(
            "Delete failed",
            false
        );
    }
}

// ============================================================
// ISSUE BOOK
// POST /api/issue/:id
// ============================================================

async function issueBook(id) {

    const memberId =
        prompt(
            "Enter Member ID:"
        );

    if (!memberId)
        return;

    try {

        const body =
            new URLSearchParams({
                memberId: memberId
            });

        const res =
            await fetch(
                `/api/issue/${id}`,
                {
                    method: "POST",
                    headers: {
                        "Content-Type":
                            "application/x-www-form-urlencoded"
                    },
                    body: body
                }
            );

        const data =
            await res.json();

        showMessage(
            data.message,
            data.success
        );

        if (data.success)
            await loadBooks();

    }
    catch (error) {

        console.error(error);

        showMessage(
            "Issue failed",
            false
        );
    }
}

// ============================================================
// RETURN BOOK
// POST /api/return/:id
// ============================================================

async function returnBook(id) {

    try {

        const res =
            await fetch(
                `/api/return/${id}`,
                {
                    method: "POST"
                }
            );

        const data =
            await res.json();

        showMessage(
            data.message,
            data.success
        );

        if (data.success)
            await loadBooks();

    }
    catch (error) {

        console.error(error);

        showMessage(
            "Return failed",
            false
        );
    }
}

// ============================================================
// INITIAL LOAD
// ============================================================

loadBooks();
