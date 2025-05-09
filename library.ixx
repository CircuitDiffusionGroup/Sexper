module;

#include <charconv>
#include <string>
#include <utility>
#include <vector>
#include <memory>

export module Sexper;

export namespace Sexper {
    template<typename T, typename S>
    concept derived_from = std::derived_from<T, S>;

    enum SexpElementType {
        FLOAT,
        INT,
        STRING,
        SYMBOL,
        LIST
    };

    class SexpElement {
    public:
        SexpElementType type;

        explicit SexpElement(const SexpElementType type) : type(type) {
        }

        virtual ~SexpElement() = default;
    };

    class SexpFloatElement final : public SexpElement {
    public:
        double value;

        explicit SexpFloatElement(const double value) : SexpElement(FLOAT), value(value) {
        }
    };

    class SexpIntElement final : public SexpElement {
    public:
        int value;

        explicit SexpIntElement(const int value) : SexpElement(INT), value(value) {
        }
    };

    class SexpStringElement final : public SexpElement {
    public:
        std::string value;

        explicit SexpStringElement(std::string value) : SexpElement(STRING), value(std::move(value)) {
        }
    };

    class SexpSymbolElement final : public SexpElement {
    public:
        std::string symbol;

        explicit SexpSymbolElement(std::string symbol) : SexpElement(SYMBOL), symbol(std::move(symbol)) {
        }
    };

    class SexpListElement final : public SexpElement {
    public:
        std::vector<std::unique_ptr<SexpElement> > elements;

        SexpListElement *parent;

        explicit SexpListElement(SexpListElement *parent) : SexpElement(LIST), parent(parent) {
        }

        template<derived_from<SexpElement> T>
        void operator+=(T &element) {
            elements.push_back(std::make_unique<T>(element));
        }

        template<derived_from<SexpElement> T>
        void operator+=(T &&element) {
            elements.push_back(std::make_unique<T>(element));
        }

        template<derived_from<SexpElement> T>
        void operator+=(std::unique_ptr<T> &element) {
            elements.push_back(std::move(element));
        }

        template<derived_from<SexpElement> T>
        void operator+=(std::unique_ptr<T> &&element) {
            elements.push_back(element);
        }
    };

    SexpListElement parse(const std::string &sexp) {
        SexpListElement root{nullptr};

        SexpListElement *current = &root;

        for (uint64_t cursor = 0; cursor < sexp.length(); ++cursor) {
            switch (sexp.at(cursor)) {
                case '(': {
                    auto element = std::make_unique<SexpListElement>(current);
                    SexpListElement *newCurrent = element.get();
                    *current += element;
                    current = newCurrent;
                }
                break;
                case ')':
                    if (current->parent == nullptr)
                        throw std::invalid_argument("Unexpected end-of-expression");
                    current = current->parent;
                    break;
                case '"': {
                    const uint64_t start = cursor++ + 1;
                    while (sexp.at(cursor) != '"') cursor++;
                    *current += SexpStringElement(sexp.substr(start, cursor - start));
                }
                break;
                default: {
                    const uint64_t start = cursor++;
                    while (!isblank(sexp.at(cursor))
                           && sexp.at(cursor) != '(' && sexp.at(cursor) != ')')
                        cursor++;
                    const std::string text = sexp.substr(start, cursor - start);

                    int32_t intValue;
                    if (
                        false /*auto [ptr, ec] =
                                std::from_chars(text.data(), text.data() + text.size(), intValue);
                        ec == std::errc() && ptr == text.data() + text.size()*/
                    ) {
                        *current += SexpIntElement(intValue);
                    } else {
                        double doubleValue;
                        if (
                            auto [ptr, ec] =
                                    std::from_chars(text.data(), text.data() + text.size(), doubleValue);
                            ec == std::errc() && ptr == text.data() + text.size()
                        ) {
                            *current += SexpFloatElement(doubleValue);
                        } else {
                            *current += SexpSymbolElement(text);
                        }
                    }
                    cursor--;
                }
                break;
                case ' ':
                case '\n':
                case '\r':
                case '\t':
                    break;
            }
        }

        return root;
    }
}
