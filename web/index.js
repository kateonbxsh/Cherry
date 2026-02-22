const typewriter = document.getElementById("typewriter");

const lines = [
  "Standard.println(\"good morning!\") unless night == true;",
  "let a = 1;\nfunction incrementA = () => {\n  a += 1;\n}",
  "class Array3D {\n  public get[int x, int y, int z] { ... }\n}",
  "type Username = string x when x.length() > 5 default \"sample\";",
  "type Optional<T> = T | null;"
];

let lineIndex = 0;
let charIndex = 0;
let deleting = false;

function escapeHtml(value) {
  return value
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}

function highlightSnippet(snippet) {
  const tokenRegex = /"([^"\\]|\\.)*"|\b(function|class|public|private|protected|static|type|when|default|unless|let|null|true|false|get|set)\b|\b(int|real|string|any|void)\b|\b([0-9]+(?:\.[0-9]+)?)\b/g;
  let result = "";
  let last = 0;
  let match = tokenRegex.exec(snippet);

  while (match) {
    if (match.index > last) {
      result += escapeHtml(snippet.slice(last, match.index));
    }

    const token = match[0];
    const escaped = escapeHtml(token);
    if (token.startsWith("\"")) {
      result += `<span class="text-emerald-300">${escaped}</span>`;
    } else if (match[2]) {
      result += `<span class="text-pink-300 font-semibold">${escaped}</span>`;
    } else if (match[3]) {
      result += `<span class="text-sky-300">${escaped}</span>`;
    } else if (match[4]) {
      result += `<span class="text-amber-300">${escaped}</span>`;
    } else {
      result += escaped;
    }

    last = tokenRegex.lastIndex;
    match = tokenRegex.exec(snippet);
  }

  if (last < snippet.length) {
    result += escapeHtml(snippet.slice(last));
  }
  return result;
}

function tick() {
  if (!typewriter) return;
  const current = lines[lineIndex];
  if (!deleting) {
    charIndex += 1;
    typewriter.innerHTML = highlightSnippet(current.slice(0, charIndex));
    if (charIndex >= current.length) {
      deleting = true;
      setTimeout(tick, 2500);
      return;
    }
    setTimeout(tick, 45 + Math.random() * 35);
  } else {
    charIndex -= 1;
    typewriter.innerHTML = highlightSnippet(current.slice(0, Math.max(0, charIndex)));
    if (charIndex <= 0) {
      deleting = false;
      lineIndex = (lineIndex + 1) % lines.length;
      setTimeout(tick, 250);
      return;
    }
    setTimeout(tick, 12);
  }
}

document.addEventListener("DOMContentLoaded", () => {
  if (typewriter) tick();
});
