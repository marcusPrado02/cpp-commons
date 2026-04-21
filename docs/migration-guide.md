# Migration Guide: ts-commons / python-commons → cpp-commons

A side-by-side mapping of equivalent constructs across the three commons libraries.

---

## Error Handling

| TypeScript (`ts-commons`) | Python (`python-commons`) | C++ (`cpp-commons`) |
|--------------------------|--------------------------|---------------------|
| `Either<L, R>` | `Result[T, E]` | `Result<T, E>` |
| `right(value)` | `Ok(value)` | `Result<T,E>::ok(value)` |
| `left(error)` | `Err(error)` | `Result<T,E>::err(error)` |
| `.map(fn)` | `.map(fn)` | `.map(fn)` |
| `.flatMap(fn)` | `.and_then(fn)` | `.and_then(fn)` |
| `.mapLeft(fn)` | `.map_err(fn)` | `.map_err(fn)` |
| `isRight()` | `is_ok()` | `is_ok()` |
| `isLeft()` | `is_err()` | `is_err()` |

### Example

TypeScript:
```typescript
const result: Either<ValidationError, User> = validateUser(input);
result.map(user => user.email).getOrElse("unknown");
```

Python:
```python
result: Result[User, ValidationError] = validate_user(input)
result.map(lambda u: u.email).value_or("unknown")
```

C++:
```cpp
Result<User, ValidationError> result = validate_user(input);
result.map([](const User& u) { return u.email(); }).value_or("unknown");
```

---

## Optional Values

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `Option<T>` / `Maybe<T>` | `Option[T]` | `Option<T>` |
| `some(value)` | `Option.some(value)` | `Option<T>::some(value)` |
| `none()` | `Option.none()` | `Option<T>::none()` |
| `.map(fn)` | `.map(fn)` | `.map(fn)` |
| `.filter(pred)` | `.filter(pred)` | `.filter(pred)` |
| `.getOrElse(def)` | `.value_or(def)` | `.value_or(def)` |
| `.getOrElse(() => fn())` | `.value_or_else(fn)` | `.value_or_else(fn)` |

---

## Identity

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `UUID.generate()` | `UUID.generate()` | `UUID::generate()` |
| `UUID.fromString(s)` | `UUID.from_string(s)` | `UUID::from_string(s)` → `std::optional<UUID>` |
| `StrongId<UserTag>` | `StrongId[UserTag]` | `StrongId<UserTag>` |
| `StrongId.generate()` | `StrongId.generate()` | `StrongId<Tag>{}` (default constructor) |
| `id.toString()` | `str(id)` | `id.to_string()` |

### Key difference
In C++, `UUID::from_string()` returns `std::optional<UUID>` (not a `Result`). In TypeScript/Python it throws or returns a `Result`. Check for `nullopt` before use:

```cpp
auto id = UUID::from_string(raw_string);
if (!id) return Result<Order, ParseError>::err(ParseError{"invalid UUID"});
```

---

## Value Objects

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `class Email extends ValueObject` | `@value_object class Email` | `class Email : public ValueObject<Email>` |
| `Email.parse(s)` → `Either<ValidationError, Email>` | `Email.parse(s)` → `Result[Email, ValidationError]` | `Email::parse(s)` → `Result<Email, ValidationError>` |
| `email.value` | `email.value` | `email.value()` |
| `PhoneNumber.parse(s)` | `PhoneNumber.parse(s)` | `PhoneNumber::parse(s)` |
| `Money(cents, currency)` | `Money(cents, currency)` | `Money{cents, CurrencyCode{"USD"}}` |
| `money.add(other)` | `money + other` | `money + other` (throws `DomainError` on currency mismatch) |

### CRTP equality
C++ `ValueObject<Derived>` uses CRTP. Equality is value-based via `fields()`:
```cpp
auto e1 = Email::parse("a@b.com").value();
auto e2 = Email::parse("a@b.com").value();
assert(e1 == e2);  // true — structural equality
```

---

## Domain Events

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `class OrderPlaced extends DomainEvent` | `class OrderPlaced(DomainEvent)` | `class OrderPlaced : public DomainEvent` |
| `event.eventId` | `event.event_id` | `event.event_id()` |
| `event.occurredAt` | `event.occurred_at` | `event.occurred_at()` |
| `event.correlationId` | `event.correlation_id` | `event.correlation_id()` |

Constructor in C++ passes `event_type` to the base:
```cpp
class OrderPlaced : public kernel::DomainEvent {
public:
    explicit OrderPlaced(std::string order_id)
        : DomainEvent{"order.placed"}, order_id_{std::move(order_id)} {}
    const std::string& order_id() const noexcept { return order_id_; }
private:
    std::string order_id_;
};
```

---

## Aggregate Root

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `class Order extends AggregateRoot<OrderId>` | `class Order(AggregateRoot[OrderId])` | `class Order : public AggregateRoot<OrderId>` |
| `this.record(new OrderPlaced(...))` | `self.record(OrderPlaced(...))` | `record<OrderPlaced>(order_id_)` |
| `aggregate.pullEvents()` | `aggregate.pull_events()` | `aggregate.pull_events()` |

---

## Use Case

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `class PlaceOrder implements UseCase<Input, Output>` | `class PlaceOrder(UseCase[Input, Output])` | Template-based, no base class required |
| `execute(input): Either<E, Output>` | `execute(input) -> Result[Output, E]` | `execute(input) -> Result<Output, E>` |

C++ does not enforce the `UseCase` base class — any class with an `execute()` method satisfies the pattern. Use the command bus for dispatch:

```cpp
struct PlaceOrderCommand { std::string product_id; int64_t amount_cents; };
struct OrderId { UUID id; };

class PlaceOrderUseCase {
public:
    Result<OrderId, DomainError> execute(const PlaceOrderCommand& cmd) { ... }
};
```

---

## Dependency Ports (Ports & Adapters)

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `interface LoggerPort` | `Protocol LoggerPort` | `concept LoggerPort` |
| Duck typing at runtime | Duck typing (Protocol) | **Compile-time structural check** |
| `implements LoggerPort` | Implicit | `static_assert(kernel::LoggerPort<MyLogger>)` |

The C++ concepts give the same "implements-by-shape" semantics as TypeScript interfaces and Python Protocols, but validated at compile time with better error messages than SFINAE.

---

## Observability Context

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `AsyncLocalStorage` / `CorrelationContext` | `contextvars.ContextVar` | `CorrelationContext` (explicit pass-through) |
| Automatic propagation via async context | Automatic via context vars | Must be passed explicitly to each layer |

**Important:** C++ has no thread-local magic equivalent to `AsyncLocalStorage`. `CorrelationContext` must be forwarded explicitly through function arguments or stored in a request-scoped object.

---

## Error Hierarchy

| Category | TypeScript | Python | C++ |
|----------|-----------|--------|-----|
| Business rule | `DomainError` | `DomainError` | `errors::DomainError` |
| Not found | `NotFoundError` | `NotFoundError` | `errors::NotFoundError` |
| Conflict | `ConflictError` | `ConflictError` | `errors::ConflictError` |
| Validation | `ValidationError` | `ValidationError` | `errors::ValidationError` |
| Unauthorized | `UnauthorizedError` | `UnauthorizedError` | `errors::UnauthorizedError` |
| Infrastructure | `InfrastructureError` | `InfrastructureError` | `errors::InfrastructureError` |

HTTP mapping via `ProblemDetails`:
```cpp
// C++ — explicit mapping
auto pd = errors::ProblemDetails::not_found("Order 123 not found");
auto response = HttpResponse::json(pd.to_json(), pd.status);

// TypeScript equivalent
throw new NotFoundError("Order 123 not found");
// middleware maps → 404 ProblemDetails automatically
```

---

## Testing Doubles

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `FakeLogger` | `FakeLogger` | `testing::FakeLogger` |
| `FakeMetrics` | `FakeMetrics` | `testing::NoopMetrics` / `testing::SpyMetrics` |
| `FakeTracer` | `FakeTracer` | `testing::FakeTracer` |
| `FakeClock` | `FakeClock` | `testing::FakeClock` |
| `FakeRepository<T>` | `FakeRepository[T]` | `testing::FakeRepository<T, TId>` |
| `InMemoryEventBus` | `InMemoryEventBus` | `testing::InMemoryEventBus` |

### GTest matchers
```cpp
#include <cpp_commons/testing/matchers.hpp>

EXPECT_THAT(result, IsOk());
EXPECT_THAT(result, IsOkWith(expectedValue));
EXPECT_THAT(result, IsErr());
EXPECT_THAT(option, IsSome());
EXPECT_THAT(option, IsNone());
EXPECT_THAT(option, IsSomeWith(42));
```

---

## Clock / Time

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `new Date()` / injectable clock | `datetime.now()` / injectable | `SystemClock` / `FrozenClock` |
| `clock.now()` | `clock.now()` | `clock.now()` → `TimePoint` |
| `frozenClock.advance(ms)` | `frozen_clock.advance(ms)` | `frozen_clock.advance(std::chrono::milliseconds{ms})` |

The `Clock` concept is satisfied by any type with `now() -> TimePoint`:
```cpp
template<kernel::Clock TClock>
class OrderService {
    TClock& clock_;
    // Uses clock_.now() internally — testable with FrozenClock
};
```

---

## Resilience

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `RetryPolicy` | `RetryPolicy` | `RetryPolicy` |
| `CircuitBreaker` | `CircuitBreaker` | `CircuitBreaker` |
| `Deadline` | `Deadline` | `kernel::Deadline` |
| `hedge(fn, delay)` | `hedge(fn, delay)` | `resilience::hedge(delay, fn)` |

The C++ `hedge()` is a free function template that takes the delay first:
```cpp
auto result = resilience::hedge(std::chrono::milliseconds{50}, [&](std::atomic<bool>& cancelled) {
    return call_external_service(cancelled);
});
```

---

## Security

| TypeScript | Python | C++ |
|-----------|--------|-----|
| `SecureRandom.generateToken(n)` | `SecureRandom.generate_token(n)` | `security::SecureRandom::generate_token(n)` |
| `JwtDecoder.decode(token, key)` | `JwtDecoder.decode(token, key)` | `security::JwtDecoder::decode(token, key)` |
| `PiiRedactor.redact(text)` | `PiiRedactor.redact(text)` | `security::PiiRedactor::redact(text)` |
| `PasswordHasher.hash(pw)` | `PasswordHasher.hash(pw)` | `security::Argon2Hasher{}.hash(pw)` |
| `PasswordHasher.verify(hash, pw)` | `PasswordHasher.verify(hash, pw)` | `security::Argon2Hasher{}.verify(hash, pw)` |
