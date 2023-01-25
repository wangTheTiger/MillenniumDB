#pragma once

#include <string>
#include <vector>

class Decimal {
public:
    static unsigned MAX_SIGNIFICANT_FIGURES;

    Decimal();
    Decimal(int64_t);
    Decimal(std::string_view);
    Decimal(std::vector<uint8_t>);
    void from_external(std::string_view);

    void trim_zeros();

    bool operator==(const Decimal&) const;
    bool operator<(const Decimal&) const;

    inline bool operator!=(const Decimal& rhs) const {
        return !(*this == rhs);
    }

    inline bool operator>(const Decimal& rhs) const {
        return !(*this < rhs) && *this != rhs;
    }

    inline bool operator>=(const Decimal& rhs) const {
        return *this > rhs || *this == rhs;
    }

    inline bool operator<=(const Decimal& rhs) const {
        return *this < rhs || *this == rhs;
    }

    inline Decimal operator+() const {
        return *this;
    };

    Decimal operator-() const;

    Decimal operator+(const Decimal&) const;
    Decimal operator-(const Decimal&) const;
    Decimal operator*(const Decimal&) const;
    Decimal operator/(const Decimal&) const;

    float                to_float() const;
    double               to_double() const;
    std::string          to_string() const;
    std::vector<uint8_t> to_bytes() const;
    std::string          to_external() const;

private:
    std::vector<uint8_t> digits;
    int8_t               exponent = 0;
    bool                 sign     = false;

    Decimal sub_digits(size_t start, size_t end) const;
    void insert_digit(uint8_t);

    static std::pair<std::string_view, std::string_view> get_parts(std::string_view);

public:
    static std::pair<uint8_t, Decimal> quotient_remainder(const Decimal &lhs, const Decimal &rhs);
};

std::ostream& operator<<(std::ostream& os, const Decimal& dec);
