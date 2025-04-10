<?php
header("Content-Type: text/plain"); // Optional, für saubere Textausgabe

try {
    $pdo = new PDO(
        'mysql:host=db555990891.db.1and1.com;dbname=db555990891;charset=utf8mb4',
        'dbo555990891',
        '4irL6RMXGvRBfEul',
        [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]
    );

    $stmt = $pdo->query("SELECT * FROM mirwal_gbuch ORDER BY id DESC LIMIT 1");

    if ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        // Hier gibst du nur den Text des Eintrags aus (z. B. Nachricht + Name)
        $name = $row["title"] ?? "---";
        $nachricht = strip_tags($row["content"] ?? "---");
        echo "$name $nachricht";
    } else {
        echo "Noch kein Eintrag.";
    }
} catch (PDOException $e) {
    http_response_code(500);
    echo "Fehler: " . $e->getMessage();
}
