const $ = id =>
    document.getElementById(id);


// ============================================================
// MESSAGE
// ============================================================

function showMessage(text, ok = true) {

    const message = $("message");

    if (!message) {
        return;
    }

    message.innerHTML =
        `<div class="${ok ? "ok" : "err"}">
            ${escapeHtml(text)}
        </div>`;

    setTimeout(() => {
        message.innerHTML = "";
    }, 3000);
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
// LOAD BOOKS
// ============================================================

async function loadBooks() {

    try {

        const response =
            await fetch("/api/books");

        if (!response.ok) {
            throw new Error(
                "HTTP " + response.status
            );
        }

        const books =
            await response.json();

        renderBooks(books);

    }
    catch (error) {

        console.error(
            "LOAD BOOKS ERROR:",
            error
        );

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
            book => !book.issued
        ).length;

    $("issued").textContent =
        books.filter(
            book => book.issued
        ).length;


    $("bookRows").innerHTML =
        books.length

            ? books.map(book => `

                <tr>

                    <td>
                        <b>${book.id}</b>
                    </td>

                    <td>
                        ${escapeHtml(book.title)}
                    </td>

                    <td>
                        ${escapeHtml(book.author)}
                    </td>

                    <td>
                        ${escapeHtml(
                            book.category ||
                            "General"
                        )}
                    </td>

                    <td>

                        <span class="badge ${
                            book.issued
                                ? "issued"
                                : "available"
                        }">

                            ${
                                book.issued
                                    ? "Issued"
                                    : "Available"
                            }

                        </span>

                        ${
                            book.issued &&
                            book.memberId
                                ? `
                                    <br>
                                    <small>
                                        Member:
                                        ${escapeHtml(
                                            book.memberId
                                        )}
                                    </small>
                                  `
                                : ""
                        }

                    </td>

                    <td class="actions">

                        ${
                            book.issued

                                ? `
                                    <button
                                        class="return"
                                        onclick="
                                            returnBook(
                                                ${book.id}
                                            )
                                        "
                                    >
                                        Return
                                    </button>
                                  `

                                : `
                                    <button
                                        onclick="
                                            issueBook(
                                                ${book.id}
                                            )
                                        "
                                    >
                                        Issue
                                    </button>
                                  `
                        }


                        <button
                            class="danger"
                            onclick="
                                deleteBook(
                                    ${book.id}
                                )
                            "
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
                            color:#777;
                        "
                    >
                        No books found.
                        Add your first book.
                    </td>

                </tr>

            `;
}


// ============================================================
// MODAL
// ============================================================

function openAdd() {

    $("modal").style.display =
        "flex";

    setTimeout(() => {

        const idInput =
            $("bookId");

        if (idInput) {
            idInput.focus();
        }

    }, 100);
}


function closeModal() {

    $("modal").style.display =
        "none";
}


// ============================================================
// ADD BOOK
// POST /api/books
// ============================================================

const bookForm =
    $("bookForm");


if (bookForm) {

    bookForm.addEventListener(
        "submit",
        async event => {

            event.preventDefault();


            try {

                // Get values directly
                const id =
                    $("bookId").value.trim();

                const title =
                    $("bookTitle").value.trim();

                const author =
                    $("bookAuthor").value.trim();

                const category =
                    $("bookCategory").value.trim();


                // Basic validation
                if (!id) {

                    showMessage(
                        "Please enter Book ID.",
                        false
                    );

                    return;
                }


                if (!title) {

                    showMessage(
                        "Please enter Book Title.",
                        false
                    );

                    return;
                }


                if (!author) {

                    showMessage(
                        "Please enter Author.",
                        false
                    );

                    return;
                }


                // Create request body
                const body =
                    new URLSearchParams();

                body.append(
                    "id",
                    id
                );

                body.append(
                    "title",
                    title
                );

                body.append(
                    "author",
                    author
                );

                body.append(
                    "category",
                    category || "General"
                );


                console.log(
                    "ADD BOOK REQUEST:",
                    body.toString()
                );


                // Send to C++ server
                const response =
                    await fetch(
                        "/api/books",
                        {
                            method: "POST",

                            headers: {
                                "Content-Type":
                                    "application/x-www-form-urlencoded"
                            },

                            body:
                                body.toString()
                        }
                    );


                const data =
                    await response.json();


                console.log(
                    "ADD BOOK RESPONSE:",
                    data
                );


                showMessage(
                    data.message ||
                    "Operation completed",
                    data.success === true
                );


                if (
                    data.success === true
                ) {

                    bookForm.reset();

                    closeModal();

                    await loadBooks();
                }

            }
            catch (error) {

                console.error(
                    "ADD BOOK ERROR:",
                    error
                );

                showMessage(
                    "Add book failed. Check browser console.",
                    false
                );
            }

        }
    );

}


// ============================================================
// SEARCH BOOK
// ============================================================

async function searchBook() {

    const input =
        $("search");

    const id =
        input.value.trim();


    if (!id) {

        await loadBooks();

        return;
    }


    try {

        const response =
            await fetch(
                `/api/books/${encodeURIComponent(id)}`
            );


        const data =
            await response.json();


        if (!response.ok) {

            renderBooks([]);

            showMessage(
                data.message ||
                "Book not found",
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

        console.error(
            "SEARCH ERROR:",
            error
        );

        showMessage(
            "Search failed",
            false
        );
    }
}


// ============================================================
// DELETE BOOK
// ============================================================

async function deleteBook(id) {

    if (
        !confirm(
            "Delete Book ID " +
            id +
            "?"
        )
    ) {
        return;
    }


    try {

        const response =
            await fetch(
                `/api/books/${id}`,
                {
                    method: "DELETE"
                }
            );


        const data =
            await response.json();


        showMessage(
            data.message,
            data.success
        );


        if (data.success) {

            await loadBooks();
        }

    }
    catch (error) {

        console.error(
            "DELETE ERROR:",
            error
        );

        showMessage(
            "Delete failed",
            false
        );
    }
}


// ============================================================
// ISSUE BOOK
// ============================================================

async function issueBook(id) {

    const memberId =
        prompt(
            "Enter Member ID:"
        );


    if (!memberId) {
        return;
    }


    try {

        const body =
            new URLSearchParams();

        body.append(
            "memberId",
            memberId.trim()
        );


        const response =
            await fetch(
                `/api/issue/${id}`,
                {
                    method: "POST",

                    headers: {
                        "Content-Type":
                            "application/x-www-form-urlencoded"
                    },

                    body:
                        body.toString()
                }
            );


        const data =
            await response.json();


        showMessage(
            data.message,
            data.success
        );


        if (data.success) {

            await loadBooks();
        }

    }
    catch (error) {

        console.error(
            "ISSUE ERROR:",
            error
        );

        showMessage(
            "Issue failed",
            false
        );
    }
}


// ============================================================
// RETURN BOOK
// ============================================================

async function returnBook(id) {

    try {

        const response =
            await fetch(
                `/api/return/${id}`,
                {
                    method: "POST"
                }
            );


        const data =
            await response.json();


        showMessage(
            data.message,
            data.success
        );


        if (data.success) {

            await loadBooks();
        }

    }
    catch (error) {

        console.error(
            "RETURN ERROR:",
            error
        );

        showMessage(
            "Return failed",
            false
        );
    }
}


// ============================================================
// CLOSE MODAL BY OUTSIDE CLICK
// ============================================================

window.addEventListener(
    "click",
    event => {

        const modal =
            $("modal");

        if (
            modal &&
            event.target === modal
        ) {
            closeModal();
        }

    }
);


// ============================================================
// ESC KEY CLOSE MODAL
// ============================================================

window.addEventListener(
    "keydown",
    event => {

        if (
            event.key === "Escape"
        ) {
            closeModal();
        }

    }
);


// ============================================================
// INITIAL LOAD
// ============================================================

loadBooks();