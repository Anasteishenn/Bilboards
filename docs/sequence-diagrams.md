# Диаграммы последовательности AdBoard

Раздел пояснительной записки Разработчика №3. Диаграммы построены по фактическому коду
(`src/ui`, `src/service`, `src/infrastructure`) и показывают, как запрос пользователя проходит
через три слоя: **CLI → сервисы ядра → репозитории SQLite**. Интерфейс не обращается к базе
напрямую — только через сервисы, а сервисы видят лишь интерфейсы репозиториев (`I…Repository`).

## 1. Вход в систему и выбор меню по роли

```mermaid
sequenceDiagram
    autonumber
    actor U as Пользователь
    participant App as ConsoleApp
    participant Auth as AuthService
    participant Repo as SqliteUserRepository
    participant DB as SQLite (users)

    U->>App: пункт 1, логин и пароль
    App->>App: readLine() — проверка, что поля не пустые
    App->>Auth: login(login, password)
    Auth->>Repo: getByLogin(login)
    Repo->>DB: SELECT id, username, password_hash, role FROM users WHERE username = ?
    DB-->>Repo: строка или пусто
    Repo-->>Auth: optional с User (или пусто)
    Auth->>Auth: hashPassword(login, password) и сравнение с password_hash
    alt пользователь не найден или хеш не совпал
        Auth-->>App: nullptr
        App-->>U: «Неверный логин или пароль»
    else данные верны
        Auth-->>App: указатель на User (сессия открыта)
        App->>App: routeUserSession() по getRole()
        App-->>U: ClientView / ManagerView / AdminView
    end
```

## 2. Оформление заявки на аренду (клиент)

Ключевой сценарий: расчёт цены через **Strategy** и проверка пересечения дат с одобренными заказами.

```mermaid
sequenceDiagram
    autonumber
    actor C as Клиент
    participant V as ClientView
    participant BS as BookingService
    participant BR as SqliteBillboardRepository
    participant OR as SqliteOrderRepository
    participant PS as IPricingStrategy
    participant DB as SQLite

    C->>V: пункт 3, ID щита, даты ДД.ММ.ГГГГ
    V->>V: readInt() / readDate() — повтор запроса при ошибке ввода
    V->>V: DateRange(start, end) — конец не раньше начала
    V->>BS: calculatePrice(clientId, billboardId, period)
    BS->>BR: getById(billboardId)
    BR->>DB: SELECT … FROM billboards WHERE id = ?
    BR-->>BS: Billboard (должен быть активен)
    BS->>OR: getByBillboardId(billboardId)
    OR->>DB: SELECT … FROM orders WHERE billboard_id = ?
    OR-->>BS: заказы щита
    BS->>BS: findConflict(): Approved и start1 ≤ end2 и start2 ≤ end1
    alt даты заняты
        BS-->>V: runtime_error «Щит №1 уже занят с … по …»
        V-->>C: [Отклонено]
    else даты свободны
        BS->>OR: getByClientId(clientId) — число одобренных заказов
        BS->>PS: calculate({dailyRate, period, approvedOrders})
        PS-->>BS: PriceQuote (дни, база, скидка, итог)
        BS-->>V: PriceQuote
        V-->>C: блок [Расчёт] и вопрос y/n
        C->>V: y
        V->>BS: createBooking(clientId, billboardId, period)
        BS->>BS: calculatePrice() ещё раз — даты могли занять
        BS->>OR: save(BookingOrder, статус Pending)
        OR->>DB: INSERT INTO orders (…) VALUES (…)
        BS->>OR: getByClientId(clientId) — id, назначенный базой
        BS-->>V: BookingOrder
        V-->>C: «Заявка №N создана и отправлена менеджеру»
    end
```

## 3. Одобрение заявки менеджером

```mermaid
sequenceDiagram
    autonumber
    actor M as Менеджер
    participant V as ManagerView
    participant BS as BookingService
    participant OR as SqliteOrderRepository
    participant O as BookingOrder
    participant DB as SQLite (orders)

    M->>V: пункт 2
    V->>BS: getPendingOrders()
    BS->>OR: getAll()
    OR->>DB: SELECT … FROM orders
    BS-->>V: заявки в статусе Pending
    V-->>M: таблица очереди
    M->>V: номер заявки, решение 1 (одобрить)
    V->>BS: approveOrder(orderId)
    BS->>OR: getById(orderId)
    OR-->>BS: BookingOrder
    BS->>OR: getByBillboardId(billboardId)
    BS->>BS: findConflict(…, ignoreOrderId = orderId)
    alt пересечение с другим одобренным заказом
        BS-->>V: runtime_error «Щит уже занят …»
        V-->>M: [Ошибка обработки заявки]
    else даты свободны
        BS->>O: approve()
        Note over O: только из Pending, иначе logic_error
        BS->>OR: updateStatus(orderId, Approved)
        OR->>DB: UPDATE orders SET status = 'Approved' WHERE id = ?
        BS-->>V: успех
        V-->>M: «Заявка одобрена. Щит на выбранные даты заблокирован»
    end
```

После одобрения заказ начинает блокировать даты (`BookingOrder::blocksDates()`), поэтому следующая
заявка другого клиента на пересекающиеся даты будет отклонена на шаге `findConflict` диаграммы 2.

## 4. Отчёт по занятости и выручке

```mermaid
sequenceDiagram
    autonumber
    actor M as Менеджер
    participant V as ManagerView
    participant BS as BookingService
    participant BR as SqliteBillboardRepository
    participant OR as SqliteOrderRepository

    M->>V: пункт 3, начало и конец периода
    V->>BS: getOccupancyReport(DateRange)
    BS->>BR: getAll()
    loop для каждого щита
        BS->>OR: getByBillboardId(id)
        BS->>BS: Approved ∩ период — DateRange::intersection().getDays()
        BS->>BS: выручка += totalPrice, загрузка = дни / дней в периоде
    end
    BS-->>V: строки отчёта
    V-->>M: таблица и итоги (средняя загрузка, выручка, заявки Pending)
```
