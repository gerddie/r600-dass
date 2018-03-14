#ifndef NODE_H
#define NODE_H

#include <cstdint>
#include <iosfwd>

class node
{
public:
        node();
        node(unsigned bytecode_size);
        int bytecode_size() const;

        friend std::ostream& operator << (std::ostream&os, const node& n);

        virtual uint64_t as_bytecode() const = 0;

protected:
        static const char *component_names;
private:
        int m_bytecode_size;

        virtual void print(std::ostream& os) const = 0;


};

inline std::ostream& operator << (std::ostream&os, const node& n)
{
        n.print(os);
        return os;
}

#endif // NODE_H