#pragma once
#include <memory>

namespace cpp_commons::kernel {

template<typename T>
class Specification {
public:
    virtual ~Specification() = default;
    [[nodiscard]] virtual bool is_satisfied_by(const T& candidate) const = 0;
};

template<typename T>
class AndSpecification final : public Specification<T> {
public:
    AndSpecification(std::shared_ptr<Specification<T>> l, std::shared_ptr<Specification<T>> r)
        : left_{std::move(l)}, right_{std::move(r)} {}

    [[nodiscard]] bool is_satisfied_by(const T& c) const override {
        return left_->is_satisfied_by(c) && right_->is_satisfied_by(c);
    }
private:
    std::shared_ptr<Specification<T>> left_, right_;
};

template<typename T>
class OrSpecification final : public Specification<T> {
public:
    OrSpecification(std::shared_ptr<Specification<T>> l, std::shared_ptr<Specification<T>> r)
        : left_{std::move(l)}, right_{std::move(r)} {}

    [[nodiscard]] bool is_satisfied_by(const T& c) const override {
        return left_->is_satisfied_by(c) || right_->is_satisfied_by(c);
    }
private:
    std::shared_ptr<Specification<T>> left_, right_;
};

template<typename T>
class NotSpecification final : public Specification<T> {
public:
    explicit NotSpecification(std::shared_ptr<Specification<T>> s) : spec_{std::move(s)} {}

    [[nodiscard]] bool is_satisfied_by(const T& c) const override {
        return !spec_->is_satisfied_by(c);
    }
private:
    std::shared_ptr<Specification<T>> spec_;
};

// Composable wrapper — supports &&, ||, ! operators
template<typename T>
class Spec {
public:
    explicit Spec(std::shared_ptr<Specification<T>> impl) : impl_{std::move(impl)} {}

    [[nodiscard]] bool is_satisfied_by(const T& c) const { return impl_->is_satisfied_by(c); }

    [[nodiscard]] Spec operator&&(const Spec& o) const {
        return Spec{std::make_shared<AndSpecification<T>>(impl_, o.impl_)};
    }
    [[nodiscard]] Spec operator||(const Spec& o) const {
        return Spec{std::make_shared<OrSpecification<T>>(impl_, o.impl_)};
    }
    [[nodiscard]] Spec operator!() const {
        return Spec{std::make_shared<NotSpecification<T>>(impl_)};
    }

private:
    std::shared_ptr<Specification<T>> impl_;
};

} // namespace cpp_commons::kernel
