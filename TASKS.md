# cpp-commons — Backlog de Melhorias

> Tarefas ordenadas por prioridade dentro de cada categoria.  
> **P0** = bloqueante para uso em produção · **P1** = necessário para feature completeness · **P2** = qualidade e experiência

---

## CI/CD & Quality Gates

- [ ] **[P0] GitHub Actions — pipeline principal**  
  Criar `.github/workflows/ci.yml` com stages: configure → build → test → lint → format-check. Executar em push e pull_request para `main`.

- [ ] **[P0] Matrix de compiladores**  
  Testar com GCC 12, GCC 13, Clang 16 e Clang 17 na mesma pipeline. Detectar divergências de comportamento entre compiladores cedo.

- [ ] **[P0] Sanitizers no CI**  
  Job separado com `-DCPP_COMMONS_ENABLE_SANITIZERS=address` (ASan + LeakSanitizer) e outro com `=thread` (TSan). Falha em qualquer erro de sanitizer quebra o CI.

- [ ] **[P0] UBSan no preset `ci`**  
  Corrigir `CMakePresets.json`: preset `ci` declara `"CPP_COMMONS_ENABLE_SANITIZERS": "address"` mas a spec diz UBSan. Separar em dois jobs: ASan (dev) e UBSan (ci).

- [ ] **[P0] Cobertura de testes com gcov/lcov**  
  Adicionar preset `coverage` com `-DCMAKE_CXX_FLAGS=--coverage`. Gerar relatório HTML via `lcov` e publicar no GitHub Pages ou Codecov. Gate mínimo: 80% de cobertura de linhas.

- [ ] **[P0] Gate de clang-tidy no CI**  
  Executar `run-clang-tidy -p build/ci` nos arquivos de `include/` e `src/`. O CI deve falhar se houver findings. Excluir `_deps/` do scan.

- [ ] **[P0] Gate de clang-format no CI**  
  Executar `clang-format --dry-run --Werror` em todos os `.hpp` e `.cpp` do projeto. Nenhum diff aceito.

- [ ] **[P1] Release automation**  
  Workflow `.github/workflows/release.yml` acionado por tag `v*`. Gera changelog via `git-cliff` ou similar, cria GitHub Release com binários e tarball de headers.

- [ ] **[P1] Dependabot para FetchContent**  
  Criar `.github/dependabot.yml` para alertar sobre novas versões de tl-expected, nlohmann_json, spdlog, googletest e google-benchmark.

- [ ] **[P2] Build em macOS no CI**  
  Adicionar job `macos-latest` com Apple Clang 15. Garantir portabilidade POSIX além de Linux.

---

## Documentação

- [ ] **[P0] README.md completo**  
  O arquivo atual está vazio. Escrever: o que é a lib, como adicionar via FetchContent, tabela de módulos, exemplo de 15 linhas com `PlaceOrderUseCase`, badges de CI/cobertura.

- [ ] **[P0] Doxygen em todos os headers públicos**  
  Adicionar `/** @brief ... */` em cada classe, função e concept em `include/cpp_commons/**`. Criar `Doxyfile` e target CMake `doc` que gera HTML.

- [ ] **[P1] CONTRIBUTING.md**  
  Explicar: como montar o ambiente com o preset `dev`, convenções de código, processo de PR, como escrever testes, regras de commit.

- [ ] **[P1] Guia de migração de ts-commons / python-commons**  
  Tabela mapeando construtos equivalentes (`Result<T,E>` ≡ `Either<L,R>`, `UseCase` ≡ `UseCase`, `CorrelationContext` ≡ `AsyncLocalStorage`, etc.).

- [ ] **[P1] ADRs (Architecture Decision Records)**  
  Criar `docs/adr/` com registros das decisões chave: uso de FetchContent vs vcpkg, header-only kernel vs compilado, `tl::expected` vs `std::expected`, AES-GCM via OpenSSL guard.

- [ ] **[P2] CHANGELOG.md**  
  Criar e manter com entradas a partir de `v0.1.0`. Seguir formato Keep a Changelog.

- [ ] **[P2] Documentação de performance**  
  Publicar os números de benchmark (`bench_kernel`, `bench_resilience`, `bench_web`) como baseline na wiki ou no README, com metodologia (Release build, hardware, compilador).

---

## Kernel

- [ ] **[P0] `std::hash` para `UUID` e `StrongId<Tag>`**  
  `FakeRepository` usa `std::map` como workaround porque não há `std::hash`. Adicionar especialização em `identity.hpp` usando `hi_ ^ (lo_ * 0x9e3779b97f4a7c15ULL)` (hash combiner). Isso remove a limitação e permite `unordered_map` em qualquer adaptador de repositório.

- [ ] **[P0] `UUID::from_string(std::string_view)` — parser**  
  Necessário para desserializar IDs recebidos via HTTP, Kafka, banco de dados. Retornar `Result<UUID, ParseError>`. Validar formato `xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`.

- [ ] **[P1] `Result<T,E>` — métodos `map_err`, `flatten`, `transform_error`**  
  Completar a API railway-oriented: `map_err` transforma o tipo de erro; `flatten` achata `Result<Result<T,E>,E>`; `transform_error` converte entre hierarquias de erro.

- [ ] **[P1] `Option<T>` — métodos `map`, `filter`, `value_or_else`**  
  `map` transforma o valor interno sem desembrulhar; `filter` descarta se predicado falhar; `value_or_else` aceita callable em vez de valor estático.

- [ ] **[P1] `Money` value object**  
  `Money{int64_t cents, CurrencyCode currency}`. Operadores de soma, subtração, comparação. `to_string()` formatado. `operator+` retorna `Result<Money, DomainError>` em overflow. Invariante: moedas devem ser iguais para operar.

- [ ] **[P1] `Email` e `PhoneNumber` value objects**  
  `Email::parse(std::string_view)` retorna `Result<Email, ValidationError>` com regex RFC 5322 simplificada. `PhoneNumber` valida E.164. Ambos são `ValueObject<T>` com CRTP.

- [ ] **[P2] `Specification<T>` — testes de composição completos**  
  Os testes atuais são básicos. Adicionar: `AndSpec`, `OrSpec`, `NotSpec` com 3+ levels de aninhamento, assert que `to_string()` produz expressão legível para logging de rejeições.

---

## Config

- [ ] **[P0] Aggregação de erros em `ConfigError`**  
  O `ConfigLoader` lança na primeira variável inválida. Deve coletar todos os erros e lançar uma vez com lista completa. Impede múltiplas tentativas de inicialização para descobrir erros um a um.

- [ ] **[P0] `env_as<std::chrono::duration>()` com parser de sufixo**  
  Parsear `"5s"`, `"100ms"`, `"2m"`, `"1h"` para `std::chrono::duration`. Necessário para timeouts e TTLs configuráveis via env.

- [ ] **[P1] Prefixo de namespace para variáveis de ambiente**  
  `Settings::with_prefix("APP_")` faz com que `get_env("PORT")` busque `APP_PORT`. Padrão 12-factor para serviços multi-instância no mesmo host.

- [ ] **[P1] Suporte a arquivo YAML**  
  Adicionar `YamlSource` usando `yaml-cpp` via FetchContent. Mesmo contrato de `IConfigSource`. Útil para Kubernetes ConfigMaps.

- [ ] **[P1] Validação com predicados**  
  `require<int>("PORT", [](int v){ return v > 0 && v < 65536; })` — falha com mensagem descritiva se predicado retornar false. Compõe com `env_as<T>`.

- [ ] **[P2] Hot reload de configuração**  
  `ConfigWatcher` que detecta mudanças em `.env` ou arquivo e notifica callbacks registrados. Usar `inotify` no Linux, `kqueue` no macOS.

---

## Observability

- [ ] **[P0] `PrometheusMetrics` — adaptador de `MetricsPort`**  
  Implementar `class PrometheusMetrics` em `src/observability/` usando `prometheus-cpp` via FetchContent. Expõe `/metrics` endpoint no formato text exposition. Satisfaz `kernel::MetricsPort`.

- [ ] **[P0] Campos estruturados no `JsonLogger`**  
  `logger.info("order placed", {{"order_id", "123"}, {"amount", 4200}})` — injetar campos extras no JSON além de `cid/tid/rid`. Necessário para queries eficientes em Loki/Elasticsearch.

- [ ] **[P0] Adaptador OpenTelemetry para `TracerPort`**  
  Implementar `OtelTracer` usando `opentelemetry-cpp` via FetchContent. Propaga `traceparent` / `tracestate` (W3C). Satisfaz `kernel::TracerPort`.

- [ ] **[P1] W3C Trace Context no `CorrelationContext`**  
  Adicionar campos `traceparent` e `tracestate` ao `CorrelationContext`. Middleware HTTP extrai e re-injeta. Necessário para interoperabilidade com Jaeger, Zipkin, AWS X-Ray.

- [ ] **[P1] Health check HTTP endpoint**  
  `HealthHttpHandler` que expõe `GET /health/live` e `GET /health/ready` usando `HealthRegistry`. Retorna JSON `{"status":"up","checks":{...}}` e status HTTP 200/503.

- [ ] **[P2] Nível de log por módulo**  
  `JsonLogger::set_level("cpp_commons.resilience", spdlog::level::warn)` para silenciar módulos verbose em produção sem afetar o restante.

- [ ] **[P2] Multi-sink logger**  
  Suporte a saída simultânea para stdout (JSON) e arquivo rotativo (`spdlog::rotating_logger_mt`). Configurável via `Settings`.

---

## Resilience

- [ ] **[P0] `RetryPolicy` com `co_await` (não-bloqueante)**  
  O backoff atual usa `std::this_thread::sleep_for` — bloqueia a thread. Implementar versão assíncrona com `co_await asio::steady_timer`. Retornar `asio::awaitable<Result<T,E>>`.

- [ ] **[P0] Estratégias de jitter no retry**  
  Adicionar `FullJitter` e `DecorrelatedJitter` (AWS well-architected). Previne thundering herd em falhas de serviço downstream. Selecionável em `RetryConfig::backoff_strategy`.

- [ ] **[P0] `retry_on<E>()` — filtro de exceção**  
  `retry_on<NetworkError>()` — só retenta para tipos específicos de erro. Erros de negócio (`ValidationError`, `NotFoundError`) não devem gerar retentativa.

- [ ] **[P1] `Deadline` e propagação**  
  `Deadline` captura `time_point` absoluto. Cada operação recebe o `Deadline` e pode verificar `deadline.remaining()`. `RetryPolicy` respeita deadline — não tenta se já expirou.

- [ ] **[P1] `RateLimiter` — token bucket**  
  `RateLimiter{max_tokens, refill_rate}`. `acquire()` retorna `Result<void, RateLimitError>`. Thread-safe com `std::atomic`. Usar em middlewares de entrada e chamadas a APIs externas.

- [ ] **[P1] `CircuitBreaker` — testes de concorrência**  
  Testes atuais são single-threaded. Adicionar teste com 8 threads chamando simultaneamente, verificar que transições de estado são thread-safe e sem data race (rodar com TSan).

- [ ] **[P2] `Hedge` — paralelismo otimista**  
  Dispara segunda requisição se a primeira demorar mais que `p95_latency`. Retorna a que chegar primeiro. Cancela a outra. Útil para reduzir tail latency.

---

## Application

- [ ] **[P0] `TracingMiddleware`**  
  Cria span OpenTelemetry por comando/query, propaga `traceparent`. Injetar em `CommandBus` e `QueryBus` como middleware. Necessário para observabilidade de ponta a ponta.

- [ ] **[P0] Testes para `QueryBus`**  
  Não existe `query_bus_test.cpp`. Adicionar: dispatch sem handler lança, handler retorna valor correto, múltiplos tipos de query coexistem, tipo errado de retorno não compila.

- [ ] **[P1] `IdempotencyMiddleware`**  
  Extrai chave de idempotência (`X-Idempotency-Key`) e consulta cache. Se a requisição já foi processada, retorna resultado cacheado sem executar o handler. Necessário para operações financeiras.

- [ ] **[P1] `OutboxPublisher` — reliable event publishing**  
  Persiste `DomainEvent`s em uma outbox antes de publicar. Worker em background publica e remove após confirmação. Garante at-least-once sem acoplamento ao banco principal.

- [ ] **[P1] `UseCase` com decoradores de métricas**  
  Decorator que injeta contadores (`use_case.calls`, `use_case.errors`, `use_case.duration_ms`) automaticamente. Configurável com `MetricsPort`.

- [ ] **[P2] `SagaOrchestrator`**  
  Implementa padrão Saga: sequência de steps com compensações. Cada step produz `Result<NextStep, CompensationChain>`. Persiste estado para recovery após falha.

---

## Web

- [ ] **[P0] CORS middleware**  
  `CorsMiddleware{origins, methods, headers}` — injeta cabeçalhos `Access-Control-Allow-*`. Suporte a wildcard e lista explícita de origens. Necessário para qualquer API consumida por browser.

- [ ] **[P0] Testes de middleware chain completos**  
  Adicionar testes para: ordem de execução de múltiplos middlewares, middleware que faz short-circuit, propagação de erros entre camadas, middleware com estado por request.

- [ ] **[P1] Content negotiation**  
  `HttpRequest::accepts("application/json")` e `HttpResponse::with_content_type(...)`. Middleware que retorna 406 se `Accept` não for suportado.

- [ ] **[P1] Rate limiting middleware**  
  `RateLimitMiddleware{limiter}` usando o `RateLimiter` da camada de resilience. Retorna 429 com `Retry-After` header quando limite excedido.

- [ ] **[P1] `HttpResponse::from_error(DomainError)` — mapeamento completo**  
  Mapeamento automático: `NotFoundError` → 404 + ProblemDetails, `ValidationError` → 422, `UnauthorizedError` → 401, `ConflictError` → 409. Eliminar switch/if em cada handler.

- [ ] **[P2] Request body size limiting middleware**  
  Rejeita requests com `Content-Length` acima do limite configurado. Retorna 413. Proteção contra ataques de upload de payload grande.

- [ ] **[P2] Compression middleware**  
  Middleware que comprime response com gzip/deflate se cliente aceitar (`Accept-Encoding`). Usar zlib (disponível em qualquer Linux).

---

## Security

- [ ] **[P0] JWT signing — não só decoding**  
  `JwtEncoder::sign(payload, private_key)` produz token assinado. Completar o ciclo: biblioteca que só decodifica é incompleta para serviços que emitem tokens (auth server, API gateway).

- [ ] **[P0] Testes adversariais para `JwtDecoder`**  
  Adicionar: token expirado, assinatura inválida, algoritmo `none` rejeitado, claims obrigatórios faltando, tamanho máximo de token, token malformado (sem `.`).

- [ ] **[P0] Testes completos para `PiiRedactor`**  
  Adicionar: múltiplos emails na mesma string, cartão sem separadores, CPF, CNPJ, telefone, combinação de padrões no mesmo input. Testar que redação não produz output diferente ao re-aplicar.

- [ ] **[P1] `SecureRandom` — geração de tokens**  
  `SecureRandom::generate_token(32)` — 32 bytes de `/dev/urandom` encodados em base64url. Para reset de senha, CSRF tokens, API keys temporárias. Diferente de UUID que é previsível em sequência.

- [ ] **[P1] `KeyRotationService`**  
  Gerencia versões de chave AES-GCM. `encrypt(data)` usa chave atual e tag versão no output. `decrypt(ciphertext)` seleciona chave pela versão. Permite rotação sem downtime.

- [ ] **[P2] Password hashing com Argon2**  
  `PasswordHasher::hash(password)` e `verify(password, hash)` usando `libsodium` ou `argon2` via FetchContent. Distinção clara de `ApiKey` (para autenticação de serviço) vs passwords de usuário.

---

## Testing Infrastructure

- [ ] **[P0] Fuzzing com libFuzzer**  
  Adicionar harnesses de fuzz em `tests/fuzz/`: `fuzz_uuid_parse.cpp`, `fuzz_jwt_decode.cpp`, `fuzz_pii_redactor.cpp`. Integrar com `oss-fuzz` ou rodar localmente com preset `fuzz`.

- [ ] **[P1] `FakeHttpServer` — test double para testes de integração web**  
  Server em memória que aceita `use(middleware)` e `route(method, path, handler)`. Permite testar middleware chain sem porta TCP. Retorna `HttpResponse` diretamente.

- [ ] **[P1] Custom GTest matchers**  
  `EXPECT_RESULT_OK(expr)`, `EXPECT_RESULT_ERR(expr, type)`, `EXPECT_LOG_CONTAINS(logger, "substring")`, `EXPECT_METRIC_EQ(spy, "name", count)`. Reduz boilerplate nos testes.

- [ ] **[P1] Teste de concorrência para `InMemoryEventBus`**  
  Publicação e subscrição simultâneas de 4 threads. Verificar ausência de data race com TSan. Documentar garantias de thread safety.

- [ ] **[P2] Benchmark baseline persistido**  
  Salvar output de `cpp_commons_benchmarks --benchmark_format=json` como artefato no CI. Comparar com run anterior e alertar se regressão > 10%.

---

## Developer Experience

- [ ] **[P0] `.devcontainer/devcontainer.json`**  
  Dev Container com Ubuntu 24.04, GCC 13, Clang 17, Ninja, cmake 3.28, clang-tidy, clang-format pré-instalados. Permite onboarding em 1 clique via VS Code Remote Containers ou GitHub Codespaces.

- [ ] **[P1] `.pre-commit-config.yaml`**  
  Hooks: `clang-format` (auto-fix em staged files), `cmake-format`, `trailing-whitespace`, `end-of-file-fixer`. Instalar com `pre-commit install` documentado no CONTRIBUTING.

- [ ] **[P1] `.vscode/settings.json` e `extensions.json`**  
  Configurar `clangd` com `compile_commands.json`, `cmake-tools` apontando para preset `dev`, `clang-tidy` integrado. Recomendar extensões: clangd, cmake-tools, test-explorer.

- [ ] **[P1] `Makefile` — targets de qualidade faltantes**  
  Adicionar: `make coverage` (gera relatório HTML), `make fuzz TARGET=fuzz_uuid_parse`, `make docs` (Doxygen), `make install PREFIX=/usr/local`.

- [ ] **[P2] `ccache` no preset `dev`**  
  Adicionar `CMAKE_CXX_COMPILER_LAUNCHER=ccache` no preset `dev`. Reduz rebuild incremental de ~30s para ~3s após mudança em header de teste.

- [ ] **[P2] pkg-config — `cpp_commons.pc.in`**  
  Gerar `cpp_commons.pc` no install com `Cflags` e `Libs` corretos. Permite usar a lib em projetos que não usam CMake.

---

## Módulos Futuros (out of v1 — registrados para planejamento)

- [ ] **[P2] `cpp_commons::persistence` — adaptador PostgreSQL**  
  `PgRepository<T, TId>` implementando `RepositoryPort`. Usa `libpqxx` via FetchContent. Migrations via `pqxx::transaction`. Connection pool com Asio executor.

- [ ] **[P2] `cpp_commons::cache_redis` — adaptador Redis**  
  `RedisCache<T>` com `get/set/del/ttl`. Serialização via nlohmann_json. Usa `hiredis` assíncrono. Satisfaz interface genérica de cache.

- [ ] **[P2] `cpp_commons::messaging_kafka` — Kafka producer/consumer**  
  `KafkaProducer::publish<E>(topic, event)` e `KafkaConsumer::subscribe<E>(topic, handler)`. Usa `librdkafka` via FetchContent. Integra `CorrelationContext` no header Kafka.

- [ ] **[P2] `cpp_commons::http_client` — cliente HTTP assíncrono**  
  `HttpClient::get/post/put/delete` retornando `asio::awaitable<Result<HttpResponse, InfrastructureError>>`. Retry e circuit breaker integrados. Baseado em Asio Beast ou libcurl.
