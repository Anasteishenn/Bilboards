PRAGMA foreign_keys = ON;

-- Таблица пользователей
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL,
    contacts TEXT
);

-- Таблица рекламных щитов
CREATE TABLE IF NOT EXISTS billboards (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    address TEXT NOT NULL,
    district TEXT NOT NULL,
    surface_type TEXT NOT NULL,
    size TEXT NOT NULL,
    daily_rate REAL NOT NULL,
    is_active INTEGER NOT NULL DEFAULT 1
);

-- Таблица заявок на аренду
CREATE TABLE IF NOT EXISTS orders (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL,
    billboard_id INTEGER NOT NULL,
    start_date TEXT NOT NULL,
    end_date TEXT NOT NULL,
    total_price REAL NOT NULL,
    status TEXT NOT NULL DEFAULT 'Pending',
    FOREIGN KEY (user_id) REFERENCES users(id),
    FOREIGN KEY (billboard_id) REFERENCES billboards(id)
);

-- Стартовые данные для проверки
INSERT OR IGNORE INTO users (id, username, password_hash, role, contacts) VALUES
    (1, 'admin', 'admin_hash', 'Admin', 'admin@adboard.ru'),
    (2, 'manager', 'manager_hash', 'Manager', '+7 (900) 111-22-33'),
    (3, 'client', 'client_hash', 'Client', 'client@mail.ru');

INSERT OR IGNORE INTO billboards (id, address, district, surface_type, size, daily_rate, is_active) VALUES
    (1, 'пр. Независимости, 4', 'Московский', 'Digital', '6x3', 60.0, 1),
    (2, 'пр. Партизанский, 12', 'Ленинский', 'Static', '6x3', 40.0, 1),
    (3, 'ул. Притыцкого, 48', 'Фрунзенский', 'Prismatron', '6x3', 50.0, 1);